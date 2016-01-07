/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_CONNECTION_HH
#define SPLIT_TCP_CONNECTION_HH

#include <string>
#include <map>

#include "socket.hh"
struct split_tcp_connection
{
    TCPSocket socket;
    bool shutdown = false;

    split_tcp_connection( TCPSocket && socket_s ) : socket( std::forward<TCPSocket>(socket_s)) { };

    split_tcp_connection() : socket( ) { };
};

#endif /* SPLIT_TCP_CONNECTION_HH */
