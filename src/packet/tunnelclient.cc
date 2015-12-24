/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <thread>
#include <chrono>

#include <sys/socket.h>
#include <net/route.h>

#include "tunnelclient.hh"
#include "netdevice.hh"
#include "system_runner.hh"
#include "util.hh"
#include "interfaces.hh"
#include "address.hh"
#include "dns_server.hh"
#include "timestamp.hh"
#include "exception.hh"
#include "bindworkaround.hh"
#include "config.h"
#include "nat.hh"
#include "tcp_splitter_client.hh"

using namespace std;
using namespace PollerShortNames;

template <class FerryQueueType>
TunnelClient<FerryQueueType>::TunnelClient( char ** const user_environment,
                                            const Address & server_address,
                                            const Address & private_address,
                                            const Address & dns_addr )
    : user_environment_( user_environment ),
      egress_ingress( Address( dns_addr.ip(), "0" ), private_address ),
      nameserver_( first_nameserver() ),
      dns_addr_( dns_addr ),
      server_socket_(),
      event_loop_()
{
    /* make sure environment has been cleared */
    if ( environ != nullptr ) {
        throw runtime_error( "TunnelClient: environment was not cleared" );
    }

    /* initialize base timestamp value before any forking */
    initial_timestamp();

    /* connect the server_socket to the server_address */
    server_socket_.connect( server_address );
}

template <class FerryQueueType>
template <typename... Targs>
void TunnelClient<FerryQueueType>::start_uplink( const string & shell_prefix,
                                                const vector< string > & command,
                                                Targs&&... Fargs )
{
    /* g++ bug 55914 makes this hard before version 4.9 */
    BindWorkAround::bind<FerryQueueType, Targs&&...> ferry_maker( forward<Targs>( Fargs )... );

    /*
      This is a replacement for expanding the parameter pack
      inside the lambda, e.g.:

    auto ferry_maker = [&]() {
        return FerryQueueType( forward<Targs>( Fargs )... );
    };
    */

    /* Fork */
    event_loop_.add_child_process( "packetshell", [&]() {
            TunDevice ingress_tun( "ingress", ingress_addr(), egress_addr() );

            /* bring up localhost */
            interface_ioctl( SIOCSIFFLAGS, "lo",
                             [] ( ifreq &ifr ) { ifr.ifr_flags = IFF_UP; } );

            /* create default route */
            rtentry route;
            zero( route );

            route.rt_gateway = egress_addr().to_sockaddr();
            route.rt_dst = route.rt_genmask = Address().to_sockaddr();
            route.rt_flags = RTF_UP | RTF_GATEWAY;

            SystemCall( "ioctl SIOCADDRT", ioctl( UDPSocket().fd_num(), SIOCADDRT, &route ) );

            Ferry inner_ferry;

            //NAT nat_rule( ingress_addr() );

            /* set up connection splitting for tcp */
            TCP_Splitter_Client tcp_splitter_client( ingress_addr() );

            /* set up dnat */
            DNAT dnat( tcp_splitter_client.tcp_listener().local_address(), "ingress" );
            cerr << "DNAT TOO " << tcp_splitter_client.tcp_listener().local_address().str() << endl;

            /* run dnsmasq as local caching nameserver */
            inner_ferry.add_child_process( start_dnsmasq( { "-S", dns_addr_.str( "#" ) } ) );

            /* Fork again after dropping root privileges */
            drop_privileges();

            /* restore environment */
            environ = user_environment_;

            /* set MAHIMAHI_BASE if not set already to indicate outermost container */
            SystemCall( "setenv", setenv( "MAHIMAHI_BASE",
                                          egress_addr().ip().c_str(),
                                          false /* don't override */ ) );

            inner_ferry.add_child_process( join( command ), [&]() {
                    /* tweak bash prompt */
                    prepend_shell_prefix( shell_prefix );

                    return ezexec( command, true );
                } );

            /* do the actual recording in a different unprivileged child */
            inner_ferry.add_child_process( "tcp_splitter", [&]() {
                    EventLoop proxy_event_loop;
                    //dns_outside.register_handlers( recordr_event_loop );
                    tcp_splitter_client.register_handlers( proxy_event_loop );
                    return proxy_event_loop.loop();
                    } );


            FerryQueueType uplink_queue { ferry_maker() };
            return inner_ferry.loop( uplink_queue, ingress_tun, server_socket_ );
        }, true );  /* new network namespace */
}

template <class FerryQueueType>
int TunnelClient<FerryQueueType>::wait_for_exit( void )
{
    return event_loop_.loop();
}

template <class FerryQueueType>
int TunnelClient<FerryQueueType>::Ferry::loop( FerryQueueType & ferry_queue,
                                              FileDescriptor & local_tun,
                                              FileDescriptor & server )
{
    /* local_tun device gets datagram -> read it -> give to ferry */
    add_simple_input_handler( local_tun,
                              [&] () {
                                  ferry_queue.read_packet( local_tun.read() );
                                  return ResultType::Continue;
                              } );

    /* we get datagram from server -> write it to local_tun device or give to http proxy */
    add_simple_input_handler( server,
                              [&] () {
                                  local_tun.write( server.read() );
                                  return ResultType::Continue;
                              } );

    /* ferry ready to write datagram -> send to server process */
    add_action( Poller::Action( server, Direction::Out,
                                [&] () {
                                    ferry_queue.write_packets( server );
                                    return ResultType::Continue;
                                },
                                [&] () { return ferry_queue.pending_output(); } ) );

    /* exit if finished */
    add_action( Poller::Action( server, Direction::Out,
                                [&] () {
                                    return ResultType::Exit;
                                },
                                [&] () { return ferry_queue.finished(); } ) );

    return internal_loop( [&] () { return ferry_queue.wait_time(); } );
}

struct TemporaryEnvironment
{
    TemporaryEnvironment( char ** const env )
    {
        if ( environ != nullptr ) {
            throw runtime_error( "TemporaryEnvironment: cannot be entered recursively" );
        }
        environ = env;
    }

    ~TemporaryEnvironment()
    {
        environ = nullptr;
    }
};

template <class FerryQueueType>
Address TunnelClient<FerryQueueType>::get_mahimahi_base( void ) const
{
    /* temporarily break our security rule of not looking
       at the user's environment before dropping privileges */
    TemporarilyUnprivileged tu;
    TemporaryEnvironment te { user_environment_ };

    const char * const mahimahi_base = getenv( "MAHIMAHI_BASE" );
    if ( not mahimahi_base ) {
        return Address();
    }

    return Address( mahimahi_base, 0 );
}
