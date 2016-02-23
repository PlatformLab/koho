/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdexcept>
#include <vector>
#include <string>

#include "util.hh"
#include "address.hh"
#include "interfaces.hh"
#include "ezio.hh"
#include "dns_proxy.hh"
#include "event_loop.hh"
#include "tcp_splitter_server.hh"


int main( int argc, char *argv[] )
{
    if ( argc != 1 ) {
        throw std::runtime_error( "Usage: " + std::string( argv[ 0 ] ) );
    }

    const Address nameserver = first_nameserver();
    const Address splitter_server_listen, dns_server;

    EventLoop main_event_loop;
    DNSProxy dns_outside( dns_server, nameserver );
    TCP_Splitter_Server tcp_splitter_server = TCP_Splitter_Server( splitter_server_listen );

    std::cerr << "koho-client 127.1 " << tcp_splitter_server.local_address().port() << " 100.64.1 " << dns_outside.tcp_listener().local_address().port() << std::endl; // XXX making assumption on address

    main_event_loop.add_child_process( "tcp-splitter-server", [&]() {
            return tcp_splitter_server.loop();
            } );

    dns_outside.register_handlers( main_event_loop );

    return main_event_loop.loop();
}
