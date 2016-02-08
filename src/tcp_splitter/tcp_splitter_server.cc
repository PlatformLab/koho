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
#include "tcp_splitter_server.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "event_loop.hh"
#include "exception.hh"

using namespace std;
using namespace PollerShortNames;

TCP_Splitter_Server::TCP_Splitter_Server( )
    : poller(),
    splitter_client_socket_(),
    connections_()
{
    splitter_client_socket_.bind( Address() );
}

void TCP_Splitter_Server::establish_new_tcp_connection( uint64_t connection_uid, Address &dest_addr )
{
    assert( connections_.count( connection_uid ) == 0 );
    TCPSocket &newSocket = connections_[ connection_uid ].socket;

    poller.add_action( Poller::Action( newSocket, Direction::In,
                [&, connection_uid] () {
                    return receive_bytes_from_tcp_connection( connections_, connection_uid, splitter_client_socket_ );
                } ) );

    newSocket.connect( dest_addr );
}

int TCP_Splitter_Server::loop( void )
{
    FileDescriptor & splitter_client_socket = splitter_client_socket_;
    
    poller.add_action( Poller::Action( splitter_client_socket, Direction::In,
                [&] () {
                SplitTCPPacket received_packet( splitter_client_socket.read() );

                cerr << "DATA FROM SPLITTER CLIENT uid " << received_packet.header.uid << endl;
                auto connection_iter = connections_.find( received_packet.header.uid );
                if ( connection_iter == connections_.end() ) {
                    assert( received_packet.header.new_connection );

                    size_t pos = received_packet.body.find(':');
                    assert( pos != std::string::npos );
                    Address dest_addr(received_packet.body.substr(0,pos), uint16_t(atoi(received_packet.body.substr(pos+1).c_str())) );
                    establish_new_tcp_connection( received_packet.header.uid, dest_addr );
                } else {
                    assert( not received_packet.header.new_connection );
                    if ( received_packet.body.size() == 0 ) {
                        cout<< "got eof, erasing connection " << received_packet.header.uid << endl;
                        int erased = connections_.erase( received_packet.header.uid );
                        assert( erased == 1 );
                    } else {
                        assert( received_packet.body.size() > 0 );

                        cerr << "forwarding packet with body " << received_packet.body << " to established connection" << endl;
                        connection_iter->second.socket.write( received_packet.body );
                    }
                }

                return ResultType::Continue;
                } ) );

    while ( true ) {
        if ( poller.poll( -1 ).result == Poller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter server" << endl;
            return -1;
        }
    }
}
