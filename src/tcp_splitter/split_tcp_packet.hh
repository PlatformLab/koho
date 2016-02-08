/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_PACKET_HH
#define SPLIT_TCP_PACKET_HH

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

/* Adapted from https://github.com/keithw/sourdough/blob/master/datagrump/contest_message.cc */

/* helper to get the nth uint64_t field (in network byte order) */
uint64_t get_field( const size_t n, const std::string & str )
{
    if ( str.size() < (n + 1) * sizeof( uint64_t ) ) {
        throw std::runtime_error( "message too small to parse field " + n );
    }

    const uint64_t * const data_ptr
        = reinterpret_cast<const uint64_t *>( str.data() ) + n;

    return be64toh( *data_ptr );
}

/* helper to put a uint64_t field (in network byte order) */
std::string put_field( const uint64_t n )
{
      const uint64_t network_order = htobe64( n );
        return std::string( reinterpret_cast<const char *>( &network_order ),
                 sizeof( network_order ) );
}

struct SplitTCPPacket {
    bool new_connection;
    uint64_t uid;
    std::string body;

    SplitTCPPacket( const bool &new_connection, const uint64_t &uid, std::string body ) 
    : new_connection( new_connection ), uid( uid ), body( body )
    { };

    std::string toString() const {
        return put_field(new_connection) + put_field(uid) + body;
    };

    /* Parse from string */
    SplitTCPPacket(std::string serialized) 
    : new_connection( get_field( 0, serialized ) ),
    uid( get_field( 1, serialized ) ),
    body( serialized.begin() + 2 * sizeof( uint64_t ), serialized.end() )
    { };
};


#endif /* SPLIT_TCP_PACKET_HH */
