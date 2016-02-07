/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_PACKET_HH
#define SPLIT_TCP_PACKET_HH

#include <string>
#include <sstream>
#include <iostream>

struct SplitTCPPacket {
    bool new_connection;
    uint64_t uid;
    std::string body;

    SplitTCPPacket( const bool &new_connection, const uint64_t &uid, std::string body ) 
    : new_connection( new_connection ), uid( uid ), body( body )
    { };

    std::string toString() const {
        std::ostringstream ret;
        ret << new_connection;
        ret << uid;
        ret << body;

        std::cerr << "good one serialized is " << ret.str() << std::endl;
        SplitTCPPacket other( ret.str() );
        std::cerr << "made other " << std::endl;
        assert( new_connection == other.new_connection );
        std::cerr << "uid " << uid  << " while other " << other.uid << std::endl;
        assert( uid == other.uid );
        assert( body == other.body );
        std::cerr << "serialization good" << std::endl;
        return ret.str();
    };

    /* Parse from string */
    SplitTCPPacket(std::string serialized) 
    : new_connection(false), uid(-1), body((char *)&uid)
    { 
        std::istringstream str( serialized.data() );
        str >> new_connection;
        str >> uid;
        std::cerr << "got uid " << uid << std::endl;
        str >> body;
    };
};


#endif /* SPLIT_TCP_PACKET_HH */
