#ifndef ROAMING_SOCKET_HH
#define ROAMING_SOCKET_HH

#include "socket.hh"
#include "roaming_packet.pb.h"

#include <iostream>
#include <memory>
#include <map>
#include <tuple>

class RoamingSocket : public UDPSocket
{
    std::map<uint64_t, Address> addrs_ {};

public:
  using UDPSocket::UDPSocket;

  std::unique_ptr<std::pair<uint64_t, std::string>> recvfrom( )
  {
      std::string buffer;
      Address received_address;
      std::tie( received_address, buffer ) = UDPSocket::recvfrom();

      KohoProtobufs::RoamingPacket received_packet;
      if (!received_packet.ParseFromString( buffer ) ) {
          std::cerr << "Failed to deserialize packet in roaming server socket, ignoring it." << std::endl;
          return NULL;
      }

      uint64_t received_from_id = received_packet.client_id();
      addrs_[ received_from_id  ] = received_address;
      return std::unique_ptr<std::pair<uint64_t, std::string>>(new std::pair<uint64_t, std::string>( received_from_id, received_packet.body() ) );
  }

  bool write( const uint64_t dest_id, const std::string & buffer )
  {
      auto addr_iter = addrs_.find( dest_id );
      if ( addr_iter != addrs_.end() ) {
          UDPSocket::sendto( addr_iter->second, buffer );
          register_write();
          return true;
      } else {
          std::cerr << "Destination id does not exist, not sending anything" << std::endl;
          return false;
      }
  }
};

#endif /* ROAMING_SOCKET_HH */
