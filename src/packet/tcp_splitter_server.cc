/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <thread>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>

#include "address.hh"
#include "socket.hh"
#include "system_runner.hh"
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

void TCP_Splitter_Server::receive_bytes_from_split_tcp_connection( uint64_t connection_uid )
{
    assert( connections_.count( connection_uid ) == 1 );
    TCPSocket &incomingSocket = connections_[ connection_uid ];

    string recieved_bytes = incomingSocket.read();
    if ( recieved_bytes.size() == 0 ) {
        cerr << "ignoring empty payload tcp packet recieved at splitter server" << endl;
        return;
    }
    cerr << "TCP DATA recieved at splitter server: " << recieved_bytes << "for connection uid " << connection_uid << endl;

    KohoProtobufs::SplitTCPPacket toSend;
    toSend.set_uid( connection_uid );
    toSend.set_body( recieved_bytes );

    string serialized_proto;
    if ( !toSend.SerializeToString( &serialized_proto ) ) {
        throw runtime_error( "TCP splitter server failed to serialize protobuf to send to client." );
    }

    FileDescriptor &splitter_client_socket = splitter_client_socket_;
    splitter_client_socket.write( serialized_proto );
}

void TCP_Splitter_Server::establish_new_tcp_connection( uint64_t connection_uid, Address &dest_addr )
{
    assert( connections_.count( connection_uid ) == 0 );
    TCPSocket &newSocket = connections_[ connection_uid ];

    poller.add_action( Poller::Action( newSocket, Direction::In,
                [&, connection_uid] () {
                    receive_bytes_from_split_tcp_connection( connection_uid );
                    return ResultType::Continue;
                },
                [&] () { return not newSocket.eof(); } ) );

    newSocket.connect( dest_addr );
}

int TCP_Splitter_Server::loop( void )
{
    FileDescriptor & splitter_client_socket = splitter_client_socket_;
    
    poller.add_action( Poller::Action( splitter_client_socket, Direction::In,
                [&] () {
                const string buffer = splitter_client_socket.read();
                

                KohoProtobufs::SplitTCPPacket recieved_packet;
                if (!recieved_packet.ParseFromString( buffer ) ) {
                    cerr << "Failed to deserialize packet from splitter client, ignoring it." << endl;
                    return ResultType::Continue;
                }
                cerr << "DATA FROM SPLITTER CLIENT uid " << recieved_packet.uid() << " and has body " << recieved_packet.has_body() << endl;
                auto connection = connections_.find( recieved_packet.uid() );
                if ( connection  == connections_.end() ) {
                    assert( recieved_packet.has_address() );
                    assert( recieved_packet.has_port() );

                    Address dest_addr( recieved_packet.address(), recieved_packet.port() );
                    establish_new_tcp_connection( recieved_packet.uid(), dest_addr );
                } else {
                    assert( recieved_packet.has_body() );
                    assert( recieved_packet.body().size() > 0 );

                    cerr << "forwarding packet with body " << recieved_packet.body() << " to established connection" << endl;
                    connection->second.write( recieved_packet.body() );
                }

                return ResultType::Continue;
                }/*,
                [&] () { return not splitter_client_socket.eof(); } */) );

    while ( true ) {
        if ( poller.poll( -1 ).result == Poller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter server" << endl;
            return -1;
        }
    }
}
