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
#include "epoller.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "exception.hh"

using namespace std;
using namespace EpollerShortNames;

TCP_Splitter_Client::TCP_Splitter_Client( const Address & listener_addr, const Address & splitter_server_addr )
    : listener_socket_(),
    splitter_server_socket_(),
    epoller_(),
    connections_mutex_(),
    connections_()
{
    listener_socket_.bind( listener_addr );
    listener_socket_.listen();
    cout << "connecting to splitter server addr " << splitter_server_addr.str() << endl;
    splitter_server_socket_.connect( splitter_server_addr );
}

int TCP_Splitter_Client::loop( void )
{
    epoller_.add_action( Epoller::Action( listener_socket_, Direction::In, 
            [&] () {
            handle_new_tcp_connection();
            return ResultType::Continue;
            } ) );

    epoller_.add_action( Epoller::Action( splitter_server_socket_, Direction::In,
            [&] () {
            return receive_packet_from_splitter_server();
            } ) );


    while ( true ) {
        if ( epoller_.poll( -1 ).result == Epoller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter client" << endl;
            return -1;
        }
    }
}

void TCP_Splitter_Client::handle_new_tcp_connection( void )
{
    connections_mutex_.lock();
    try {
        auto connection_iter = connections_.emplace( make_pair( get_connection_uid(), listener_socket_.accept() ) ).first;
        const uint64_t connection_uid = connection_iter->first;
        TCPSocket & incoming_socket = connection_iter->second.socket;

        /* send packet of metadata on this connectio to tcp splitter server so it can make its own connection to original client destination */
        SplitTCPPacket connection_metadata( true, connection_uid, incoming_socket.original_dest().str() );
        // TODO tighten critical section
        splitter_server_socket_.write( connection_metadata.toString() );
        /* add epoller routine so incoming datagrams on this socket go to splitter server */
        epoller_.add_action( Epoller::Action( incoming_socket, Direction::In,
                    [&, connection_uid ] () {
                    return receive_bytes_from_tcp_connection( connections_mutex_, connections_, connection_uid, splitter_server_socket_, epoller_ );
                    } ) );
    } catch ( const exception & e ) {
        connections_mutex_.unlock();
        print_exception( e );
    }
    connections_mutex_.unlock();
}

Result TCP_Splitter_Client::receive_packet_from_splitter_server( void )
{
    SplitTCPPacket received_packet( splitter_server_socket_.read() );
    assert( not received_packet.header.new_connection );

    //cerr << "DATA FROM SPLITTER SERVER for uid " << received_packet.header.uid << endl;
    connections_mutex_.lock();
    auto connection_iter = connections_.find( received_packet.header.uid );
    if ( connection_iter  == connections_.end() ) {
        cerr << "connection uid " << received_packet.header.uid <<" does not exist on client, ignoring it." << endl;
    } else {
        if ( received_packet.body.size() == 0 ) {
            // splitter server received eof so done with this connection
            cerr <<" got EOF from other side, erasing connection uid " << received_packet.header.uid << endl;
            close_connection( received_packet.header.uid, connections_, epoller_ );
        } else {
            assert( received_packet.body.size() > 0 );
            connection_iter->second.socket.write( received_packet.body );
        }
    }
    connections_mutex_.unlock();
    return ResultType::Continue;
}
