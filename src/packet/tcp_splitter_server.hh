/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_SERVER_HH
#define TCP_SPLITTER_SERVER_HH

#include <string>
#include <map>

#include "socket.hh"
#include "poller.hh"
#include "autosocket.hh"
#include "split_tcp_connection.hh"
#include "split_tcp_packet.pb.h"

class Poller;

class TCP_Splitter_Server
{
private:
    Poller poller;
    AutoSocket splitter_client_socket_;
    std::map<uint64_t, SplitTCPConnection> connections_; // bool if eof

public:
    TCP_Splitter_Server( );

    void establish_new_tcp_connection( uint64_t connection_uid, Address &dest_addr );

    Address local_address( void ) { return splitter_client_socket_.local_address( ); }

    int loop( void );
};

#endif /* TCP_SPLITTER_SERVER_HH */
