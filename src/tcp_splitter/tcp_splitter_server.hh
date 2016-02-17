/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_SERVER_HH
#define TCP_SPLITTER_SERVER_HH

#include <string>
#include <map>

#include "socket.hh"
#include "epoller.hh"
#include "autosocket.hh"
#include "split_tcp_connection.hh"

class Epoller;

class TCP_Splitter_Server
{
private:
    Epoller epoller_;
    TCPSocket client_listener_socket_;
    std::vector<TCPSocket> splitter_client_sockets_;
    std::map<uint64_t, SplitTCPConnection> connections_; // bool if eof

public:
    TCP_Splitter_Server( const Address & listen_address );

    void establish_new_tcp_connection( TCPSocket &splitter_client_socket, uint64_t connection_uid, Address &dest_addr );

    Epoller::Action::Result receive_packet_from_splitter_client( TCPSocket &splitter_client_socket );

    Address local_address( void ) { return client_listener_socket_.local_address( ); }

    int loop( void );
};

#endif /* TCP_SPLITTER_SERVER_HH */
