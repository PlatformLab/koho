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
#include "poller.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "event_loop.hh"
#include "exception.hh"

using namespace std;
using namespace PollerShortNames;

TCP_Splitter_Server::TCP_Splitter_Server( )
    : splitter_client_socket_(),
    connections_()
{
    splitter_client_socket_.bind( Address() );
}

inline void handle_new_tcp_connection( const uint64_t connection_uid, Poller &poller, std::pair<TCPSocket, std::vector<std::string>> &socket_buffer_pair, const Address & dest_addr, FileDescriptor & splitter_client_socket )
{
    TCPSocket & toConnect = socket_buffer_pair.first;
    vector<string> & datagrams = socket_buffer_pair.second;

    poller.add_action( Poller::Action( toConnect, Direction::In,
                [&, connection_uid] () {
                    datagrams.emplace_back( toConnect.read() );
                    cerr << "TCP DATA recieved at splitter server: " << datagrams.back() << "for connection uid " << connection_uid << endl;

                    KohoProtobufs::SplitTCPPacket toSend;
                    toSend.set_uid( connection_uid );
                    toSend.set_body( datagrams.back() );

                    string serialized_proto;
                    if ( !toSend.SerializeToString( &serialized_proto ) ) {
                        throw runtime_error( "TCP splitter server failed to serialize protobuf to send to client." );
                    }

                    splitter_client_socket.write( serialized_proto );
                    return ResultType::Continue;
                },
                [&] () { return not toConnect.eof(); } ) );

    toConnect.connect( dest_addr );
}

int TCP_Splitter_Server::loop( void )
{
    FileDescriptor & splitter_client_socket = splitter_client_socket_;
    Poller poller;
    
    poller.add_action( Poller::Action( splitter_client_socket, Direction::In,
                [&] () {
                const string buffer = splitter_client_socket.read();
                

                KohoProtobufs::SplitTCPPacket recieved_packet;
                if (!recieved_packet.ParseFromString( buffer ) ) {
                    cerr << "Failed to deserialize packet from splitter client, ignoring it." << endl;
                    return ResultType::Continue;
                }
                cerr << "DATA FROM SPLITTER CLIENT uid " << recieved_packet.uid() << " and body " << recieved_packet.body() << endl;
                auto connection = connections_.find( recieved_packet.uid() );
                if ( connection  == connections_.end() ) {
                    assert( recieved_packet.has_address() );
                    assert( recieved_packet.has_port() );

                    auto &toConnect = connections_[ recieved_packet.uid() ];
                    Address dest_addr( recieved_packet.address(), recieved_packet.port() );
                    handle_new_tcp_connection( recieved_packet.uid(), poller, toConnect, dest_addr, splitter_client_socket_ );
                } else {
                    assert( recieved_packet.has_body() );
                    cerr << "forwarding packet to established connection" << endl;
                    connection->second.first.write( recieved_packet.body() );
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
