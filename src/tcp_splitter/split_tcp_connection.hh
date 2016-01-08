/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_CONNECTION_HH
#define SPLIT_TCP_CONNECTION_HH

#include <string>
#include <map>

#include "socket.hh"
struct SplitTCPConnection
{
    TCPSocket socket;

    SplitTCPConnection( TCPSocket && socket_s ) : socket( std::forward<TCPSocket>(socket_s)) { };

    SplitTCPConnection() : socket( ) { };
};

#endif /* SPLIT_TCP_CONNECTION_HH */
