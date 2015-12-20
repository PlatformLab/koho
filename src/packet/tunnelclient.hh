/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TUNNELCLIENT_HH
#define TUNNELCLIENT_HH

#include <string>

#include "netdevice.hh"
#include "util.hh"
#include "address.hh"
#include "dns_proxy.hh"
#include "event_loop.hh"
#include "socketpair.hh"
#include "socket.hh"
#include "tcp_splitter_client.hh"

template <class FerryQueueType>
class TunnelClient
{
private:
    char ** const user_environment_;
    std::pair<Address, Address> egress_ingress;
    Address nameserver_;
    Address dns_addr_;
    TCP_Splitter_Client tcp_splitter_;

    UDPSocket server_socket_;

    EventLoop event_loop_;

    const Address & egress_addr( void ) { return egress_ingress.first; }
    const Address & ingress_addr( void ) { return egress_ingress.second; }

    class Ferry : public EventLoop
    {
    public:
        int loop( FerryQueueType & ferry_queue, FileDescriptor & tun, FileDescriptor & sibling );
    };

    Address get_mahimahi_base( void ) const;

public:
    TunnelClient( char ** const user_environment, const Address & server_address,
                  const Address & private_address,
                  const Address & dns_address, 
                  const Address & server_tcp_splitter_address );

    template <typename... Targs>
    void start_uplink( const std::string & shell_prefix,
                       const std::vector< std::string > & command,
                       Targs&&... Fargs );

    int wait_for_exit( void );

    TunnelClient( const TunnelClient & other ) = delete;
    TunnelClient & operator=( const TunnelClient & other ) = delete;
};

#endif /* TUNNELCLIENT_HH */
