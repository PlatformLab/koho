#!/usr/bin/env python

import subprocess
import socket
import sys
import string
import random
import os
import signal

def timeout(sig,frm):
  sys.stderr.write( "End-to-end test timeout." )
  sys.exit(1)

# Timeout after 10 seconds
signal.signal(signal.SIGALRM, timeout)
signal.alarm(10)

server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_sock.bind(("0", 0))
server_sock.listen(1)

# Run a koho-server and read koho-client command arguments from stderr
splitter_server = subprocess.Popen('koho-server', stderr=subprocess.PIPE)
kohoClientCommand = splitter_server.stderr.readline()
splitter_server.stderr.close() # now close stderr so it can't fill up a buffer and block our process if we print too much

# make random payload
payload_size = 130000
outgoing_payload = ''.join(random.choice(string.ascii_uppercase) for _ in range(payload_size)) # send payload_size random chars

# run tcp_sender inside a koho-client shell, send to port/address of our server
local_port = server_sock.getsockname()[1]
local_address = kohoClientCommand.split()[3]

tcp_sender_command = "python tcp_sender.py %s %d %s " % (local_address, local_port, outgoing_payload)
tunnel_with_tcp_sender_command = "echo " + tcp_sender_command + " | " + kohoClientCommand

subprocess.Popen(tunnel_with_tcp_sender_command, shell=True)

# have server accept first incoming connection, check payload is the same sent by tcp_sender
(incoming_connection, _) = server_sock.accept()

bytes_recvd = 0
while bytes_recvd < payload_size:
    chunk = incoming_connection.recv(min(payload_size - bytes_recvd, 1400))
    if chunk == '':
        raise RuntimeError("socket connection broken")

    expected_chunk = outgoing_payload[bytes_recvd:bytes_recvd+len(chunk)]
    if expected_chunk != chunk:
        sys.stderr.write('\nError: expected chunk "%s" and received "%s".\n' % (expected_chunk, chunk))
        sys.exit( 1 )

    bytes_recvd += len(chunk)
    #print("recieved " + str(bytes_recvd) + " of " + str(payload_size))

assert(bytes_recvd == payload_size)
sys.exit( 0 )
