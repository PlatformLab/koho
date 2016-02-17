/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_COMMON_HH
#define TCP_SPLITTER_COMMON_HH

#include <string>
#include <map>

#include "epoller.hh"
#include "socket.hh"
#include "file_descriptor.hh"
#include "split_tcp_connection.hh"
#include "split_tcp_packet.hh"

using namespace EpollerShortNames;

inline void close_connection( uint64_t connection_uid, std::map<uint64_t, SplitTCPConnection> &connection_map, Epoller &epoller )
{
    auto connection_it = connection_map.find( connection_uid );
    assert( connection_it != connection_map.end() );

    /* remove action from epoller */
    epoller.remove_action( connection_it->second.socket.fd_num() );

    /* then erase connection, calling destructor for associated filedescriptor */
    connection_map.erase( connection_it );
}

ResultType receive_bytes_from_tcp_connection( std::map<uint64_t, SplitTCPConnection> &connection_map, const uint64_t connection_uid, FileDescriptor &other_side_socket, Epoller &epoller )
{
    std::string body;
    { // we might be deleting this socket later so don't use it outside here
        auto connection_iter = connection_map.find( connection_uid );
        if ( connection_iter  == connection_map.end() ) {
            std::cerr << "connection uid " << connection_uid <<" does not exist, ignoring it." << std::endl;
            return ResultType::Continue;
        }
        TCPSocket & incoming_socket = connection_iter->second.socket;
        /* to avoid oversize packets, given potential for double wrapped UDP and our own split packet header */
        body = incoming_socket.read( 1500 - 56 - 56 - sizeof( SplitTCPPacket::Header ) );
    }

    if ( body.size() == 0 ) {
        std::cerr <<"Closing connection on EOF" << std::endl;
        close_connection( connection_uid, connection_map, epoller );
    }
    SplitTCPPacket toSend( false, connection_uid, body );
    other_side_socket.write( toSend.toString() );
    return body.size() == 0 ? ResultType::Cancel : ResultType::Continue;
}

#endif /* TCP_SPLITTER_COMMON_HH */
