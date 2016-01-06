/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_COMMON_HH
#define TCP_SPLITTER_COMMON_HH

#include <string>
#include <map>

#include "socket.hh"
#include "file_descriptor.hh"
#include "split_tcp_packet.pb.h"

    void receive_bytes_from_split_tcp_connection( std::map<uint64_t, TCPSocket> &connection_map, const uint64_t connection_uid, FileDescriptor &other_side_socket )
    {
    auto connection = connection_map.find( connection_uid );
    if ( connection  == connection_map.end() ) {
        std::cerr << "connection uid " << connection_uid <<" does not exist, ignoring it." << std::endl;
        return;
    }
    TCPSocket & incoming_socket = connection->second;

    KohoProtobufs::SplitTCPPacket toSend;
    toSend.set_uid( connection_uid );

    if ( toSend.body().size() == 0 ) {
        toSend.set_eof( true );
        std::cerr << "GOT EOF" << std::endl;
    } else {
        toSend.set_eof( false );
        toSend.set_body( incoming_socket.read() );
    }

    //std::cerr << "TCP DATA FROM INSIDE CLIENT SHELL for connection uid " << connection_uid << std::endl;

    std::string serialized_proto;
    if ( !toSend.SerializeToString( &serialized_proto ) ) {
        throw std::runtime_error( "TCP splitter failed to serialize protobuf." );
    }

    other_side_socket.write( serialized_proto );
    }

#endif /* TCP_SPLITTER_COMMON_HH */
