/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_PACKET_HH
#define SPLIT_TCP_PACKET_HH

#include <string>
#include <sstream>
#include <iostream>

struct SplitTCPPacket {
    enum connection_type : uint8_t { NEW, EXISTING } conn_type;
    uint64_t uid;
    union {
        struct {
            std::string address;
            uint16_t port;
        } new_connection;
        struct {
            bool eof;
            std::string body;
        } existing_connection;
    };

    ~SplitTCPPacket() {
        switch (conn_type) {
            case NEW:
                new_connection.address.~basic_string<char>();
                break;
            case EXISTING:
                existing_connection.body.~basic_string<char>();
                break;
        }
    };


    std::string toString() const {
        std::cerr << "serialization attempt" << std::endl;
        std::ostringstream ret;
        ret << (uint8_t) conn_type;
        ret << uid ;
        std::cerr << "serialization going" << std::endl;
        switch (conn_type) {
            case NEW:
                std::cerr << "serialization new" << std::endl;
                ret << new_connection.address;
                std::cerr << "serialization new address done" << std::endl;
                ret << new_connection.port;
                break;
            case EXISTING:
                std::cerr << "serialization existing" << std::endl;
                ret << existing_connection.eof;
                std::cerr << "serialization existing eof done" << std::endl;
                ret << existing_connection.body;
                break;
        }

        std::cerr << "good one serialized is " << ret.str() << std::endl;
        SplitTCPPacket other( ret.str() );
        std::cerr << "made other " << std::endl;
        assert( conn_type == other.conn_type );
        std::cerr << "uid " << uid  << " while other " << other.uid << std::endl;
        assert( uid == other.uid );
        switch (conn_type) {
            case NEW:
                assert( new_connection.address == other.new_connection.address );
                assert( new_connection.port == other.new_connection.port );
                break;
            case EXISTING:
                assert( existing_connection.eof == other.existing_connection.eof );
                assert( existing_connection.body == other.existing_connection.body );
                break;
        }
        std::cerr << "serialization good" << std::endl;
        return ret.str();
    };

    /* new connection packet */
    SplitTCPPacket( const uint64_t uid, std::string address, uint16_t port )
    : conn_type( connection_type::NEW ),
    uid( uid ),
    new_connection( { address, port } )
    { };

    /* exsting connection packet */
    SplitTCPPacket( const uint64_t uid, bool eof, std::string &body )
    : conn_type( connection_type::EXISTING ),
    uid( uid ),
    existing_connection( { eof, body } )
    { };

    /* Parse from string */
    SplitTCPPacket(std::string serialized) 
    : conn_type( NEW ), uid(-1)
    { 
        std::istringstream str( serialized.data() );
        uint8_t temp_type;
        str >> temp_type;
        conn_type = (connection_type) temp_type;
        uid = 0;
        str >> uid;
        std::cerr << "got uid " << uid << std::endl;
        switch (conn_type) {
            case NEW:
                str >> new_connection.address >> new_connection.port;
                break;
            case EXISTING:
                str >> existing_connection.eof >> existing_connection.body;
                break;
        }
    };
};


#endif /* SPLIT_TCP_PACKET_HH */
