/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <thread>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>

#include "address.hh"
#include "socket.hh"
#include "system_runner.hh"
#include "tcp_splitter_common.hh"
#include "tcp_splitter_client.hh"
#include "poller.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "event_loop.hh"
#include "exception.hh"

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
    incoming_tcp_connections_.add_action( Poller::Action( listener_socket_, Direction::In, 
            [&] () {
            handle_new_tcp_connection();
            return ResultType::Continue;
            } ) );

    incoming_tcp_connections_.add_action( Poller::Action( splitter_server_socket_, Direction::In,
            [&] () {
            return receive_packet_from_splitter_server();
            },
            [&] () { return not splitter_server_socket_.eof(); } ) );


    while ( true ) {
        if ( incoming_tcp_connections_.poll( -1 ).result == Poller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter client" << endl;
            return -1;
        }
    }
}

void TCP_Splitter_Client::handle_new_tcp_connection( void )
{
    try {
        auto connection_iter = connections_.emplace( make_pair(get_connection_uid(), listener_socket_.accept() )).first;
        const uint64_t connection_uid = connection_iter->first;
        TCPSocket & incoming_socket = connection_iter->second.socket;

        /* send packet of metadata on this connectio to tcp splitter server so it can make its own connection to original client destination */
        SplitTCPPacket connection_metadata( true, connection_uid, incoming_socket.original_dest().str() );

        splitter_server_socket_.write( connection_metadata.toString() );

        /* add poller routine so incoming datagrams on this socket go to splitter server */
        incoming_tcp_connections_.add_action( Poller::Action( incoming_socket, Direction::In,
                    [&, connection_uid ] () {
                    return receive_bytes_from_tcp_connection( connections_, connection_uid, splitter_server_socket_ );
                    } ) );
    } catch ( const exception & e ) {
        print_exception( e );
    }
}

Result TCP_Splitter_Client::receive_packet_from_splitter_server( void )
{
    SplitTCPPacket received_packet( splitter_server_socket_.read() );
    assert( not received_packet.header.new_connection );

    cerr << "DATA FROM SPLITTER SERVER for uid " << received_packet.header.uid << endl;
    auto connection_iter = connections_.find( received_packet.header.uid );
    if ( connection_iter  == connections_.end() ) {
        cerr << "connection uid " << received_packet.header.uid <<" does not exist on client, ignoring it." << endl;
    } else {
        if ( received_packet.body.size() == 0 ) {
            cerr <<" got EOF from other side, erasing connection " << received_packet.header.uid << endl;
            // splitter server received eof so done with this connection
            int erased = connections_.erase( received_packet.header.uid );
            assert( erased == 1 );
        } else {
            assert( received_packet.body.size() > 0 );
            connection_iter->second.socket.write( received_packet.body );
        }
    }
    return ResultType::Continue;
}
