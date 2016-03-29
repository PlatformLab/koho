/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_CLIENT_HH 
#define TCP_SPLITTER_CLIENT_HH 

#include <string>
#include <mutex>
#include <vector>
#include <map>

#include "split_tcp_connection.hh"
#include "socket.hh"
#include "epoller.hh"

class EventLoop;
using namespace EpollerShortNames;

class TCP_Splitter_Client
{
private:
    TCPSocket listener_socket_;
    UDPSocket splitter_server_socket_;
    Epoller epoller_;
    uint64_t next_connection_uid_ = 1;
    std::mutex connections_mutex_;
    std::map<uint64_t, SplitTCPConnection> connections_;

public:
    TCP_Splitter_Client( const Address & listener_addr, const Address & splitter_server_addr );

    TCPSocket & tcp_listener( void ) { return listener_socket_; }

    void handle_new_tcp_connection( void );
    Result receive_packet_from_splitter_server( void );

    int loop( void );

    uint64_t get_connection_uid( void ) { return next_connection_uid_++; }
};

#endif /* TCP_SPLITTER_CLIENT_HH */
