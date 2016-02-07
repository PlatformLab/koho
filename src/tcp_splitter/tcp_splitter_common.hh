/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_COMMON_HH
#define TCP_SPLITTER_COMMON_HH

#include <string>
#include <map>

#include "poller.hh"
#include "socket.hh"
#include "file_descriptor.hh"
#include "split_tcp_connection.hh"
#include "split_tcp_packet.hh"

using namespace PollerShortNames;

ResultType receive_bytes_from_tcp_connection( std::map<uint64_t, SplitTCPConnection> &connection_map, const uint64_t connection_uid, FileDescriptor &other_side_socket )
{
    std::string body;
    { // we might be deleting this socket later so don't use it outside here
        auto connection_iter = connection_map.find( connection_uid );
        if ( connection_iter  == connection_map.end() ) {
            std::cerr << "connection uid " << connection_uid <<" does not exist, ignoring it." << std::endl;
            return ResultType::Continue;
        }
        TCPSocket & incoming_socket = connection_iter->second.socket;

        body = incoming_socket.read( 1024 ); // to avoid oversize packets TODO this could probably be better
    }

    if ( body.size() == 0 ) {
        int erased = connection_map.erase( connection_uid ); // delete self
        assert( erased == 1 );
        std::cerr <<"Closing connection on EOF" << std::endl;
    }
    SplitTCPPacket toSend( false, connection_uid, body );
    other_side_socket.write( toSend.toString() );
    return body.size() == 0 ? ResultType::Cancel : ResultType::Continue;
}

#endif /* TCP_SPLITTER_COMMON_HH */
