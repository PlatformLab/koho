#!/usr/bin/env python

import subprocess
import socket
import sys
import string
import random
import os
import signal

def timeout(sig,frm):
  sys.stderr.write( "Timeout." )
  sys.exit(1)

signal.signal(signal.SIGALRM, timeout)
signal.alarm(2)

server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_sock.bind(("0", 0))
server_sock.listen(1)

splitter_server = subprocess.Popen('koho-server', stderr=subprocess.PIPE)

kohoClientCommand = splitter_server.stderr.readline()

payload_size = 3000
outgoing_payload = ''.join(random.choice(string.ascii_uppercase) for _ in range(payload_size)) # send payload_size random chars

# run tcp_sender inside koho-client shell, send to port/address of our server
local_port = server_sock.getsockname()[1]
local_address = kohoClientCommand.split()[3]
tcp_sender_command = "python tcp_sender.py %s %d %s " % (local_address, local_port, outgoing_payload)
os.system("echo " + tcp_sender_command + " | " + kohoClientCommand)

# have server accept first incoming connection, check payload is the same sent by tcp_sender
(incoming_connection, _) = server_sock.accept()

# adapted from https://docs.python.org/2/howto/sockets.html
chunks = []
bytes_recvd = 0
while bytes_recvd < payload_size:
    chunk = incoming_connection.recv(min(payload_size - bytes_recvd, 1400))
    if chunk == '':
        raise RuntimeError("socket connection broken")
    chunks.append(chunk)
    bytes_recvd += len(chunk)
    print("read " +str(len(chunk)) + "bytes, " + str(bytes_recvd) + " total" )

print("server read " + str(bytes_recvd) + " total bytes")

incoming_payload = ''.join(chunks)

if incoming_payload == outgoing_payload:
    sys.stderr.write("success.\n")
    sys.exit( 0 )
else:
    sys.stderr.write('\nError: expected "%s" and received "%s".\n' % (outgoing_payload, incoming_payload))
    sys.exit( 1 )
