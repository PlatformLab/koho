#!/usr/bin/env python

import subprocess
import socket
import sys
import string
import random
import os
import signal

# Takes a client program to run inside a koho tunnel. exits

def timeout(sig,frm):
  sys.stderr.write( "End-to-end test timeout." )
  sys.exit(1)

# Timeout after 10 seconds
signal.signal(signal.SIGALRM, timeout)
signal.alarm(10)

if len( sys.argv ) is not 2:
    raise ValueError("Usage: python run-insinde-koho-tunnel.py command_to_run")

# Run a koho-server and read koho-client command arguments from stderr
kohoServer = subprocess.Popen('koho-server', stderr=subprocess.PIPE)
kohoClientCommand = kohoServer.stderr.readline()
kohoServer.stderr.close() # now close stderr so it can't fill up a buffer and block our process if we print too much

kohoClient = subprocess.Popen("echo " + sys.argv[1] + " | " + kohoClientCommand, shell=True)
while 1:
    assert(kohoServer.poll() is None) # assert koho server didnt crash
    clientReturnCode = kohoClient.poll()
    if clientReturnCode is not None:
        print "client got return %d" % clientReturnCode
        sys.exit(clientReturnCode)
