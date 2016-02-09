/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdexcept>
#include <vector>
#include <string>

#include "trivial_queue.hh"
#include "util.hh"
#include "ezio.hh"
#include "tcp_splitter_server.hh"

using namespace std;

int main( int argc, char *argv[] )
{
    if ( argc != 1 ) {
        throw runtime_error( "Usage: " + string( argv[ 0 ] ) );
    }

    TCP_Splitter_Server splitter_server;
    return -1;
}
