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
    outgoing_sockets_()
{
    splitter_client_socket_.bind( Address() );
}

int TCP_Splitter_Server::loop( void )
{
    FileDescriptor & splitter_client_socket = splitter_client_socket_;
    Poller poller;

    poller.add_action( Poller::Action( splitter_client_socket, Direction::In,
                                       [&] () {
                                           string buffer = splitter_client_socket.read();
                                           cerr << "DATA FROM SPLITTER CLIENT: " << buffer << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return not splitter_client_socket.eof(); } ) );

    while ( true ) {
        if ( poller.poll( -1 ).result == Poller::Result::Type::Exit ) {
            cerr << "exiting loop on splitter server" << endl;
            return -1;
        }
    }
}
