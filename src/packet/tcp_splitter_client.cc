/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <thread>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>

#include "address.hh"
#include "socket.hh"
#include "system_runner.hh"
#include "poller.hh"
#include "bytestream_queue.hh"
#include "file_descriptor.hh"
#include "event_loop.hh"
#include "exception.hh"
#include "tcp_splitter_client.hh"

using namespace std;
using namespace PollerShortNames;

TCP_Splitter_Client::TCP_Splitter_Client( const Address & listener_addr, const Address & dest_addr )
    : listener_socket_(), 
    destination_socket_()
{
    listener_socket_.bind( listener_addr );
    listener_socket_.listen();
    destination_socket_.connect( dest_addr );
}

template <class SocketType>
void TCP_Splitter_Client::loop( SocketType & server, SocketType & client )
{
    Poller poller;

    const Address server_addr = client.original_dest();
    cerr << " got original dest " << server_addr.str() << endl;

    /* poll on original connect socket and new connection socket to ferry packets */
    /* responses from server go to response parser */
    poller.add_action( Poller::Action( server, Direction::In,
                                       [&] () {
                                           string buffer = server.read();
                                           cerr << "OMG server GOT " << buffer << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return not client.eof(); } ) );

    /* requests from client go to request parser */
    poller.add_action( Poller::Action( client, Direction::In,
                                       [&] () {
                                           string buffer = client.read();
                                           cerr << "OMG clien GOT " << buffer << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return not server.eof(); } ) );

    /* completed requests from client are serialized and sent to server */
    poller.add_action( Poller::Action( server, Direction::Out,
                                       [&] () {
                                           cerr << " server somethign out" << endl;
                                           return ResultType::Continue;
                                       },
                                       [&] () { return true; } ) );

    /* completed responses from server are serialized and sent to client */
    poller.add_action( Poller::Action( client, Direction::Out,
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
    thread newthread( [&] ( TCPSocket client ) {
            try {
                /* get original destination for connection request */
                Address server_addr = client.original_dest();

                /* create socket and connect to original destination and send original request */
                TCPSocket server;
                server.connect( server_addr );

                return loop( server, client );
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
