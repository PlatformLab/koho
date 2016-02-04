/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_SERVER_HH
#define TCP_SPLITTER_SERVER_HH

#include <string>
#include <map>

#include "socket.hh"
#include "poller.hh"
#include "roaming_socket.hh"
#include "split_tcp_connection.hh"
#include "split_tcp_packet.pb.h"

class Poller;

class TCP_Splitter_Server
{
private:
    Poller poller;
    RoamingSocket splitter_clients_socket_;
    std::map<std::pair<uint64_t, uint64_t>, SplitTCPConnection> connections_; // key is client_id, connection_id

public:
    TCP_Splitter_Server( );

    void establish_new_tcp_connection( uint64_t client_id, uint64_t connection_id, Address &dest_addr );

    Address local_address( void ) { return splitter_clients_socket_.local_address( ); }

    int loop( void );
};

#endif /* TCP_SPLITTER_SERVER_HH */
