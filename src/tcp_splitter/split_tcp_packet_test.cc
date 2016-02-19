/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "split_tcp_packet.hh"
#include <string.h>
#include <iostream>
#include <assert.h>

int main()
{
    const bool new_connection = true;
    const uint64_t uid = 1232323;
    const std::string body = "This is a test packet, we want to make sure we can serialize and deserialize a packet and recieve identical contents. Lets hope we do. Koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho koho.";

    SplitTCPPacket toSerialize( new_connection, uid, body );
    std::string serialized = toSerialize.toString();
    SplitTCPPacket deserialized(serialized);

    if ( deserialized.header.new_connection == toSerialize.header.new_connection &&
            deserialized.header.uid == toSerialize.header.uid &&
            deserialized.body == toSerialize.body )
    {
        return 0;
    } else {
        return 1;
    }
}
