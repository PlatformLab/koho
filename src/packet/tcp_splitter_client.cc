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

using namespace std;
using namespace PollerShortNames;

TCP_Splitter_Client::TCP_Splitter_Client( const Address & listener_addr, const Address & server_addr )
    : listener_socket_(),
    server_socket_()
{
    listener_socket_.bind( listener_addr );
    listener_socket_.listen();
    server_socket_.connect( server_addr );
}

void TCP_Splitter_Client::loop( UDPSocket & server_socket, TCPSocket & incoming_socket )
{
    Poller poller;

    const Address dest_addr = incoming_socket.original_dest();
    cerr << " got original dest " << dest_addr.str() << endl;

    /* poll on original connect socket and new connection socket to ferry packets */
    /* responses from server go to response parser */
    poller.add_action( Poller::Action( server_socket, Direction::In,
                                       [&] () {
                                           string buffer = server_socket.read();
                                           cerr << "OMG server GOT " << buffer << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return not incoming_socket.eof(); } ) );

    /* incoming requests go to request parser */
    poller.add_action( Poller::Action( incoming_socket, Direction::In,
                                       [&] () {
                                           string buffer = incoming_socket.read();
                                           cerr << "OMG clien GOT " << buffer << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return not server_socket.eof(); } ) );

    /* completed requests from client are serialized and sent to server */
    poller.add_action( Poller::Action( server_socket, Direction::Out,
                                       [&] () {
                                           cerr << " server somethign out" << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return true; } ) );

    /* completed responses from server are serialized and sent to client */
    poller.add_action( Poller::Action( incoming_socket, Direction::Out,
                                       [&] () {
                                           cerr << " client somethign out" << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return true; } ) );

    while ( true ) {
        if ( poller.poll( -1 ).result == Poller::Result::Type::Exit ) {
            return;
        }
    }
}

void TCP_Splitter_Client::handle_tcp( )
{
    thread newthread( [&] ( TCPSocket incoming_socket ) {
            try {
                /* get original destination for connection request
                Address dest_addr = client.original_dest(); */

                /* create socket and connect to original destination and send original request 
                TCPSocket server;
                server.connect( server_addr );*/

                return loop( server_socket_, incoming_socket );
            } catch ( const exception & e ) {
                print_exception( e );
            }
        }, listener_socket_.accept() );

    /* don't wait around for the reply */
    newthread.detach();
}

/* register this TCP_Splitter_Client's TCP listener socket to handle events with
   the given event_loop, saving request-response pairs to the given
   backing_store (which is captured and must continue to persist) */
void TCP_Splitter_Client::register_handlers( EventLoop & event_loop )
{
    event_loop.add_simple_input_handler( tcp_listener(),
                                         [&] () {
                                             handle_tcp( );
                                             return ResultType::Continue;
                                         } );
}
