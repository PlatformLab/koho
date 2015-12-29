/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_CLIENT_HH 
#define TCP_SPLITTER_CLIENT_HH 

#include <string>
#include <vector>
#include <map>

#include "socket.hh"
#include "poller.hh"

class EventLoop;
//class Poller;

class TCP_Splitter_Client
{
private:
    TCPSocket listener_socket_;
    UDPSocket splitter_server_socket_;
    Poller incoming_tcp_connections_;
    std::map<Address, std::unique_ptr<std::pair<TCPSocket, std::vector<std::string>>>> connections_;

public:
    TCP_Splitter_Client( const Address & listener_addr, const Address & splitter_server_addr );

    TCPSocket & tcp_listener( void ) { return listener_socket_; }

    void handle_new_tcp_connection( );

    int loop( void );
};

#endif /* TCP_SPLITTER_CLIENT_HH */
