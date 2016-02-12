/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <vector>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <net/route.h>

#include "nat.hh"
#include "util.hh"
#include "interfaces.hh"
#include "address.hh"
#include "netdevice.hh"
#include "event_loop.hh"
#include "socketpair.hh"
#include "config.h"
#include "exception.hh"
#include "dns_server.hh"
#include "system_runner.hh"

#include "util.hh"
#include "ezio.hh"
#include "tcp_splitter_client.hh"

using namespace std;

int main( int argc, char *argv[] )
{
    try {
        /* clear environment while running as root */
        char ** const user_environment = environ;
        environ = nullptr;

        check_requirements( argc, argv );

        const size_t num_args = 4;
        if ( num_args < 4 ) {
            throw runtime_error( "Usage: " + string( argv[ 0 ] ) + " TCP-SPLITTER-SERVER-IP TCP-SPLITTER-SERVER-PORT DNS-SERVER-PORT [command...]" );
        }

        Address tcp_splitter_server_address = { argv[ 1 ], argv[ 2 ] };

        vector< string > command;

        /* what command will we run inside the container? */
        if ( argc == num_args ) {
            command.push_back( shell_path() );
        } else {
            for ( int i = num_args; i < argc; i++ ) {
                command.push_back( argv[ i ] );
            }
        }

        /* set egress and ingress ip addresses */
        Address egress_addr, ingress_addr;
        tie( egress_addr, ingress_addr ) = two_unassigned_addresses();

        Address caching_dns_server_address = { egress_addr.ip(), argv[ 3 ] }; // XXX HACK only works when server and client on same machine

        /* make pair of devices */
        string egress_name = "veth-" + to_string( getpid() ), ingress_name = "veth-i" + to_string( getpid() );
        VirtualEthernetPair veth_devices( egress_name, ingress_name );

        /* bring up egress */
        assign_address( egress_name, egress_addr, ingress_addr );

        /* set up NAT between egress and eth0 */
        NAT nat_rule( ingress_addr );

        /* set up http proxy for tcp */
        //HTTPProxy http_proxy( egress_addr );
        TCP_Splitter_Client tcp_splitter_client( egress_addr, tcp_splitter_server_address);

        /* set up dnat */
        DNAT dnat( tcp_splitter_client.tcp_listener().local_address(), egress_name );

        /* prepare event loop */
        EventLoop outer_event_loop;

        /* Fork */
        {
            /* Make pipe for start signal */
            auto pipe = UnixDomainSocket::make_pair();

            ChildProcess container_process( "kohoclient", [&]() {
                    /* wait for the go signal */
                    pipe.second.read();

                    /* bring up localhost */
                    interface_ioctl( SIOCSIFFLAGS, "lo",
                                     [] ( ifreq &ifr ) { ifr.ifr_flags = IFF_UP; } );

                    /* bring up veth device */
                    assign_address( ingress_name, ingress_addr, egress_addr );

                    /* create default route */
                    rtentry route;
                    zero( route );

                    route.rt_gateway = egress_addr.to_sockaddr();
                    route.rt_dst = route.rt_genmask = Address().to_sockaddr();
                    route.rt_flags = RTF_UP | RTF_GATEWAY;

                    SystemCall( "ioctl SIOCADDRT", ioctl( UDPSocket().fd_num(), SIOCADDRT, &route ) );

                    /* prepare child's event loop */
                    EventLoop shell_event_loop;

                    /* run dnsmasq as local caching nameserver */
                    std::cout << "trying to start dnsmasq with addr " << caching_dns_server_address.str( "#" ) << std::endl;
                    shell_event_loop.add_child_process( start_dnsmasq( { "-S", caching_dns_server_address.str( "#" ) } ) );

                    /* Fork again after dropping root privileges */
                    drop_privileges();

                    /* restore environment and tweak prompt */
                    environ = user_environment;

                    shell_event_loop.add_child_process( join( command ), [&]() {
                            prepend_shell_prefix( "[koho "+ tcp_splitter_server_address.str() + "] " );

                            
                    std::cout << "trying exec " << join( command ) << std::endl;
                            return ezexec( command, true );
                        } );

                    return shell_event_loop.loop();
                }, true ); /* new network namespace */

            /* give ingress to container */
            run( { IP, "link", "set", "dev", ingress_name, "netns", to_string( container_process.pid() ) } );
            veth_devices.set_kernel_will_destroy();

            /* tell ChildProcess it's ok to proceed */
            pipe.first.write( "x" );

            /* now that we have its pid, move container process to event loop */
            outer_event_loop.add_child_process( move( container_process ) );
        }

        /* run the tcp splitter client in a different unprivileged child */
        outer_event_loop.add_child_process( "tcp-splitter-client", [&]() {
                drop_privileges();

                return tcp_splitter_client.loop();
                } );

        return outer_event_loop.loop();
    } catch ( const exception & e ) {
        print_exception( e );
        return EXIT_FAILURE;
    }
}
