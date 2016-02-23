/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef DNS_PROXY_HH
#define DNS_PROXY_HH

#include <memory>

#include "socket.hh"

class EventLoop;

class DNSProxy_UDPtoTCP
{
private:
    UDPSocket udp_listener_;
    TCPSocket tcp_listener_;
    Address tcp_target_;

public:
    DNSProxy_UDPtoTCP( const Address & listen_address, const Address & s_tcp_target );

    /* accept already-bound TCP and UDP sockets (can be useful if these
       need to be bound to the same port number) */
    DNSProxy_UDPtoTCP( UDPSocket && udp_listener, TCPSocket && tcp_listener,
              const Address & s_tcp_target );

    UDPSocket & udp_listener( void ) { return udp_listener_; }
    TCPSocket & tcp_listener( void ) { return tcp_listener_; }

    void handle_udp( void );
    void handle_tcp( void );

    void register_handlers( EventLoop & event_loop );
};

#endif /* DNS_PROXY_HH */
