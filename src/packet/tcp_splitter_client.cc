/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <thread>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>

#include "address.hh"
#include "socket.hh"
#include "system_runner.hh"
#include "tcp_splitter_client.hh"
#include "poller.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "event_loop.hh"
#include "exception.hh"

#include "split_tcp_packet.pb.h"

using namespace std;
using namespace PollerShortNames;

TCP_Splitter_Client::TCP_Splitter_Client( const Address & listener_addr, const Address & splitter_server_addr )
    : listener_socket_(),
    splitter_server_socket_(),
    incoming_tcp_connections_(),
    connections_()
{
    listener_socket_.bind( listener_addr );
    listener_socket_.listen();
    cout << "connecting to splitter server addr " << splitter_server_addr.str() << endl;
    splitter_server_socket_.connect( splitter_server_addr );
}

int TCP_Splitter_Client::loop( void )
{
    splitter_server_socket_.write("initial packet from splitter client!");

    incoming_tcp_connections_.add_action( Poller::Action( listener_socket_, Direction::In, 
            [&] () {
            handle_new_tcp_connection( );
            return ResultType::Continue;
            } ) );

    incoming_tcp_connections_.add_action( Poller::Action( splitter_server_socket_, Direction::In,
                [&] () {
                string buffer = splitter_server_socket_.read();
                cerr << "DATA FROM SPLITTER SERVER: " << buffer << endl;
                return ResultType::Continue;
                },
                [&] () { return not splitter_server_socket_.eof(); } ) );


    while ( true ) {
        if ( incoming_tcp_connections_.poll( -1 ).result == Poller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter client" << endl;
            return -1;
        }
    }
}

void TCP_Splitter_Client::handle_new_tcp_connection( )
{
    try {
        vector<string> empty;
        unique_ptr<pair<TCPSocket, vector<string>>> to_ins( new pair<TCPSocket, vector<string>>(listener_socket_.accept(), empty));
        TCPSocket & incoming_socket = to_ins->first;
        vector<string> & data_buffer = to_ins->second;
        const uint64_t connection_uid = connections_.insert( make_pair(get_connection_uid(), move(to_ins)) ).first->first;
        const Address dest_addr = incoming_socket.original_dest();
        cerr << " got original dest " << dest_addr.str() << endl;

        KohoProtobufs::SplitTCPPacket connection_metadata;
        connection_metadata.set_uid( connection_uid );
        connection_metadata.set_body( incoming_socket.original_dest().str() );
        string serialized_metadata_proto;
        if ( !connection_metadata.SerializeToString( &serialized_metadata_proto ) ) {
            throw runtime_error( "TCP splitter client failed to serialize protobuf metadata." );
        }

        splitter_server_socket_.write( serialized_metadata_proto );

        /* incoming datagrams go to splitter server */
        incoming_tcp_connections_.add_action( Poller::Action( incoming_socket, Direction::In,
                    [&, connection_uid ] () {
                    data_buffer.emplace_back(incoming_socket.read());
                    cerr << "TCP DATA FROM INSIDE CLIENT SHELL: " << data_buffer.back() << "For connection uid " << connection_uid << endl;

                    KohoProtobufs::SplitTCPPacket toSend;
                    toSend.set_uid( connection_uid );
                    toSend.set_body( data_buffer.back() );

                    string serialized_proto;
                    if ( !toSend.SerializeToString( &serialized_proto ) ) {
                        throw runtime_error( "TCP splitter client failed to serialize protobuf." );
                    }

                    splitter_server_socket_.write( serialized_proto );
                    return ResultType::Continue;
                    },
                    [&] () { return not incoming_socket.eof(); } ) );
    } catch ( const exception & e ) {
        print_exception( e );
    }
}
