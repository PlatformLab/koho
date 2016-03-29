/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_SERVER_HH
#define TCP_SPLITTER_SERVER_HH

#include <string>
#include <map>
#include <mutex>

#include "socket.hh"
#include "epoller.hh"
#include "autosocket.hh"
#include "split_tcp_connection.hh"

class Epoller;

class TCP_Splitter_Server
{
private:
    Epoller epoller_;
    AutoSocket splitter_client_socket_;
    std::mutex connections_mutex_;
    std::map<uint64_t, SplitTCPConnection> connections_; // bool if eof

public:
    TCP_Splitter_Server( const Address & listen_address );

    bool establish_new_tcp_connection( uint64_t connection_uid, Address &dest_addr );

    Address local_address( void ) { return splitter_client_socket_.local_address( ); }

    int loop( void );
};

#endif /* TCP_SPLITTER_SERVER_HH */
