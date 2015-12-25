/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_SERVER_HH
#define TCP_SPLITTER_SERVER_HH

#include <string>

#include "socket.hh"
#include "autosocket.hh"

class Poller;

class TCP_Splitter_Server
{
private:
    AutoSocket splitter_client_socket_;

public:
    TCP_Splitter_Server( );

    Address local_address( void ) { return splitter_client_socket_.local_address( ); }

    int loop( void );
};

#endif /* TCP_SPLITTER_SERVER_HH */
