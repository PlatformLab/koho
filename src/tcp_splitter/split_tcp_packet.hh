/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_PACKET_HH
#define SPLIT_TCP_PACKET_HH

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

/* Adapted from https://github.com/keithw/sourdough/blob/master/datagrump/contest_message.hh */

struct SplitTCPPacket {
    struct Header {
        bool new_connection;
        uint64_t uid;

        /* Header for new message */
        Header( const bool &new_connection, const uint64_t &uid );

        /* Parse header from wire */
        Header( const std::string & str );

        /* Make wire representation of header */
        std::string toString( void ) const;
    } header;
    std::string body;

    SplitTCPPacket( const bool &new_connection, const uint64_t &uid, std::string body );

    std::string toString() const;

    SplitTCPPacket( const std::string &str );
};


#endif /* SPLIT_TCP_PACKET_HH */
