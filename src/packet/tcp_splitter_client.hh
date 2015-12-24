/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_CLIENT_HH 
#define TCP_SPLITTER_CLIENT_HH 

#include <string>

#include "socket.hh"

class EventLoop;
class Poller;

class TCP_Splitter_Client
{
private:
    TCPSocket listener_socket_;
    UDPSocket server_socket_;

    void loop( UDPSocket & server, TCPSocket & client );

public:
    TCP_Splitter_Client( const Address & listener_addr, const Address & server_addr );

    TCPSocket & tcp_listener( void ) { return listener_socket_; }

    void handle_tcp( );

    /* register this TCP_Splitter_Client's TCP listener socket to handle events with
       the given event_loop, saving request-response pairs to the given
       backing_store (which is captured and must continue to persist) */
    void register_handlers( EventLoop & event_loop );
};

#endif /* TCP_SPLITTER_CLIENT_HH */
