/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <vector>
#include <string>

#include "util.hh"
#include "ezio.hh"
#include "tcp_splitter_server.hh"

using namespace std;

int main( int argc, char *argv[] )
{
    if ( argc != 1 ) {
        throw runtime_error( "Usage: " + string( argv[ 0 ] ) );
    }

    TCP_Splitter_Server tcp_splitter_server();
    /* clreate splitter server process */
    //cerr << "koho-client " << tcp_splitter_server.local_address().str( " " );
    //return tcp_splitter_server.loop();
    return -1;
}
