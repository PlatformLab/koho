#!/usr/bin/env python

import socket
import sys
import string
from time import sleep

client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

client_sock.connect((sys.argv[1], int(sys.argv[2])))

msg = sys.argv[3]

totalsent = 0
while totalsent < len(msg):
    sent = client_sock.send(msg[totalsent:])
    if sent == 0:
        raise RuntimeError("socket connection broken")
    totalsent += sent
# print("Client finished sending " + str(totalsent) + " bytes")

# make sure we don't clean up too early
sleep(5)
