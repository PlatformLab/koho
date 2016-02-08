/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "split_tcp_packet.hh"

/* Adapted from https://github.com/keithw/sourdough/blob/master/datagrump/contest_message.cc */

/* helper to get the nth uint64_t field (in network byte order) */
uint64_t get_header_field( const size_t n, const std::string & str )
{
    if ( str.size() < (n + 1) * sizeof( uint64_t ) ) {
        throw std::runtime_error( "message too small to parse field " + n );
    }

    const uint64_t * const data_ptr
        = reinterpret_cast<const uint64_t *>( str.data() ) + n;

    return be64toh( *data_ptr );
}

/* helper to put a uint64_t field (in network byte order) */
std::string put_header_field( const uint64_t n )
{
    const uint64_t network_order = htobe64( n );
    return std::string( reinterpret_cast<const char *>( &network_order ),
            sizeof( network_order ) );
}

SplitTCPPacket::Header::Header( const bool &new_connection, const uint64_t &uid )
    : new_connection( new_connection ),
    uid( uid )
{ }

SplitTCPPacket::Header::Header( const std::string & str )
    : new_connection( get_header_field( 0, str ) ),
    uid( get_header_field( 1, str ) )
{ }

std::string SplitTCPPacket::Header::toString() const
{
    return put_header_field(new_connection) + put_header_field(uid);
}


SplitTCPPacket::SplitTCPPacket( const bool &new_connection, const uint64_t &uid, std::string body ) 
    : header( new_connection, uid ),
    body( body )
{ }

std::string SplitTCPPacket::toString() const
{
    return header.toString() + body;
}

/* Parse from string */
SplitTCPPacket::SplitTCPPacket( std::string str )  // TODO change to const &
    : header( str ),
    body( str.begin() + sizeof( Header ), str.end() )
{ }
