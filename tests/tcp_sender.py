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

print("sending " + sys.argv[3]) 
client_sock.send(sys.argv[3])
