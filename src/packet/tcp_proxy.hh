/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_PROXY_HH
#define TCP_PROXY_HH

#include <string>

#include "socket.hh"

class EventLoop;
class Poller;

class TCPProxy
{
private:
    TCPSocket listener_socket_;

    template <class SocketType>
    void loop( SocketType & server, SocketType & client );

public:
    TCPProxy( const Address & listener_addr );

    TCPSocket & tcp_listener( void ) { return listener_socket_; }

    void handle_tcp( );

    /* register this TCPProxy's TCP listener socket to handle events with
       the given event_loop, saving request-response pairs to the given
       backing_store (which is captured and must continue to persist) */
    void register_handlers( EventLoop & event_loop );
};

#endif /* TCP_PROXY_HH */
