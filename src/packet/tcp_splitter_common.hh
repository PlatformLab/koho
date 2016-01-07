/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_COMMON_HH
#define TCP_SPLITTER_COMMON_HH

#include <string>
#include <map>

#include "socket.hh"
#include "file_descriptor.hh"
#include "split_tcp_packet.pb.h"

void receive_bytes_from_split_tcp_connection( std::map<uint64_t, std::pair<TCPSocket, bool>> &connection_map, const uint64_t connection_uid, FileDescriptor &other_side_socket )
{
    auto connection = connection_map.find( connection_uid );
    if ( connection  == connection_map.end() ) {
        std::cerr << "connection uid " << connection_uid <<" does not exist, ignoring it." << std::endl;
        return;
    }
    TCPSocket & incoming_socket = connection->second.first;

    KohoProtobufs::SplitTCPPacket toSend;
    toSend.set_uid( connection_uid );
    toSend.set_body( incoming_socket.read() );

    if ( toSend.body().size() == 0 ) {
        toSend.set_eof( true );
        toSend.clear_body();
        connection->second.second = true; // eof from other side of tcp connection means we are done
    } else {
        toSend.set_eof( false );
    }

    std::string serialized_proto;
    if ( !toSend.SerializeToString( &serialized_proto ) ) {
        throw std::runtime_error( "TCP splitter failed to serialize protobuf." );
    }

    other_side_socket.write( serialized_proto );
}

bool if_done_plus_erase_on_completion( std::map<uint64_t, std::pair<TCPSocket, bool>> &connection_map, const uint64_t connection_uid )
{
    auto connection_map_iter = connection_map.find( connection_uid );
    assert( connection_map_iter != connection_map.end() );
    if ( connection_map_iter->second.second ) {
        int erased = connection_map.erase( connection_uid );
        std::cerr <<" ON recieved EOF with erased = " << ( erased == 1 ) << std::endl;
        return true;
    }
    return false;
}


#endif /* TCP_SPLITTER_COMMON_HH */
