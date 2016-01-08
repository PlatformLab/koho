/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef TCP_SPLITTER_COMMON_HH
#define TCP_SPLITTER_COMMON_HH

#include <string>
#include <map>

#include "poller.hh"
#include "socket.hh"
#include "file_descriptor.hh"
#include "split_tcp_connection.hh"
#include "split_tcp_packet.pb.h"

using namespace PollerShortNames;

ResultType receive_bytes_from_split_tcp_connection( std::map<uint64_t, SplitTCPConnection> &connection_map, const uint64_t connection_uid, FileDescriptor &other_side_socket )
{
    auto connection_iter = connection_map.find( connection_uid );
    if ( connection_iter  == connection_map.end() ) {
        std::cerr << "connection uid " << connection_uid <<" does not exist, ignoring it." << std::endl;
        return ResultType::Continue;
    }
    TCPSocket & incoming_socket = connection_iter->second.socket;

    KohoProtobufs::SplitTCPPacket toSend;
    toSend.set_uid( connection_uid );
    toSend.set_body( incoming_socket.read() );

    bool finished = false;
    if ( toSend.body().size() == 0 ) {
        toSend.set_eof( true );
        toSend.clear_body();

        finished = true;
        int erased = connection_map.erase( connection_uid );
        assert( erased == 1 );
        std::cerr <<"Closing connection on EOF" << std::endl;
    } else {
        toSend.set_eof( false );
    }

    std::string serialized_proto;
    if ( !toSend.SerializeToString( &serialized_proto ) ) {
        throw std::runtime_error( "TCP splitter failed to serialize protobuf." );
    }

    other_side_socket.write( serialized_proto ); // TODO rename other side?
    return finished ? ResultType::Cancel : ResultType::Continue;
}

#endif /* TCP_SPLITTER_COMMON_HH */
