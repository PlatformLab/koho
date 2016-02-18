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

local_port = server_sock.getsockname()[1]

splitter_server = subprocess.Popen('koho-server',
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

kohoClientCommand = splitter_server.stderr.readline()

payload_size = random.randint(100, 10000)

outgoing_payload = ''.join(random.choice(string.ascii_uppercase) for _ in range(payload_size))

# run tcp sender inside koho-client shell
local_address = kohoClientCommand.split()[3]
os.system("echo python tcp_sender.py %s %d %s | " % (local_address, local_port, outgoing_payload) + kohoClientCommand)

(incoming_connection, _) = server_sock.accept()

incoming_payload = incoming_connection.recv(payload_size)

if incoming_payload == outgoing_payload:
    sys.stderr.write("success.\n")
    sys.exit( 0 )
else:
    sys.stderr.write('\nError: expected "%s" and received "%s".\n' % (outgoing_payload, incoming_payload))
    sys.exit( 1 )
