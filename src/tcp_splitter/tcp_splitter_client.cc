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
        KohoProtobufs::SplitTCPPacket connection_metadata;
        connection_metadata.set_connection_id( connection_uid );
        connection_metadata.set_eof( false );
        connection_metadata.set_address( incoming_socket.original_dest().ip() );
        connection_metadata.set_port( incoming_socket.original_dest().port() );
        string serialized_metadata_proto;
        if ( !connection_metadata.SerializeToString( &serialized_metadata_proto ) ) {
            throw runtime_error( "TCP splitter client failed to serialize protobuf metadata." );
        }

        splitter_server_socket_.write( serialized_metadata_proto );

        /* add poller routine so incoming datagrams on this socket go to splitter server */
        incoming_tcp_connections_.add_action( Poller::Action( incoming_socket, Direction::In,
                    [&, connection_uid ] () {
                    return receive_bytes_from_split_tcp_connection( connections_, connection_uid, splitter_server_socket_ );
                    } ) );
    } catch ( const exception & e ) {
        print_exception( e );
    }
}

Result TCP_Splitter_Client::receive_packet_from_splitter_server( void )
{
    KohoProtobufs::SplitTCPPacket received_packet;
    if ( !received_packet.ParseFromString( splitter_server_socket_.read() ) ) {
        cerr << "Failed to deserialize packet from splitter server, ignoring it." << endl;
        return ResultType::Continue;
    }

    cerr << "DATA FROM SPLITTER SERVER for connection id " << received_packet.connection_id() << endl;
    auto connection_iter = connections_.find( received_packet.connection_id() );
    if ( connection_iter  == connections_.end() ) {
        cerr << "connection id " << received_packet.connection_id() <<" does not exist on client, ignoring it." << endl;
    } else {
        if ( received_packet.eof() ) {
            cerr <<" got EOF" << endl;
            // splitter server received eof so done with this connection
            int erased = connections_.erase( received_packet.connection_id() );
            assert( erased == 1 );
            return ResultType::Cancel;
        } else {
            assert( received_packet.has_body() );
            assert( received_packet.body().size() > 0 );
            connection_iter->second.socket.write( received_packet.body() );
        }
    }
    return ResultType::Continue;
}
