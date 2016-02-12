/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdexcept>
#include <vector>
#include <string>

#include "util.hh"
#include "address.hh"
#include "interfaces.hh"
#include "ezio.hh"
#include "tcp_splitter_server.hh"


int main( int argc, char *argv[] )
{
    if ( argc != 1 ) {
        throw std::runtime_error( "Usage: " + std::string( argv[ 0 ] ) );
    }

    const Address splitter_server_listen;

    TCP_Splitter_Server tcp_splitter_server = TCP_Splitter_Server( splitter_server_listen );
    std::cerr << "koho-client 127.1 " << tcp_splitter_server.local_address().port() << std::endl;
    return tcp_splitter_server.loop();
}
