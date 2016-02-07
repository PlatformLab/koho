/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef SPLIT_TCP_PACKET_HH
#define SPLIT_TCP_PACKET_HH

#include <string>
#include <sstream>

struct SplitTCPPacket {
    enum connection_type { NEW, EXISTING } conn_type;
    uint64_t uid = 1;
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
            case EXISTING:
                existing_connection.body.~basic_string<char>();
        }
    };


    std::string toString() const {
        std::stringstream ret;
        ret << uid << conn_type;
        switch (conn_type) {
            case NEW:
                ret << new_connection.address << new_connection.port;
            case EXISTING:
                ret << existing_connection.eof << existing_connection.body;
        }
        switch (conn_type) {
            case NEW:
                ret << new_connection.address << new_connection.port;
            case EXISTING:
                ret << existing_connection.eof << existing_connection.body;
        }
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
    : conn_type(*reinterpret_cast<const enum connection_type *>( serialized.data() ))
    { 
        std::stringstream str( serialized.data() + sizeof(enum connection_type) );
        str >> uid;
        switch (conn_type) {
            case NEW:
                str >> new_connection.address >> new_connection.port;
            case EXISTING:
                str >> existing_connection.eof >> existing_connection.body;
        }
    };
};


#endif /* SPLIT_TCP_PACKET_HH */
