#!/usr/bin/env python

import subprocess
import socket
import sys
import string
import random
import os
import signal

client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

client_sock.connect((sys.argv[1], int(sys.argv[2])))

msg = sys.argv[3]

totalsent = 0
while totalsent < len(msg):
    sent = client_sock.send(msg[totalsent:])
    if sent == 0:
        raise RuntimeError("socket connection broken")
    totalsent += sent
print("Client finished sending " + str(totalsent) + " bytes")
