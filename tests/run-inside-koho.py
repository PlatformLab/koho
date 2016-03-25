#!/usr/bin/env python

import subprocess
import socket
import sys
import string
import random
import os
import signal

# Takes a client program to run inside a koho tunnel.

def timeout(sig,frm):
  sys.stderr.write( "End-to-end test timeout." )
  sys.exit(1)

# Timeout after 10 seconds
signal.signal(signal.SIGALRM, timeout)
signal.alarm(10)

if len( sys.argv ) is not 2:
    raise ValueError("Usage: python run-insinde-koho-tunnel.py command_to_run")

# Run a koho-server and read koho-client command arguments from stderr
splitter_server = subprocess.Popen('koho-server', stderr=subprocess.PIPE)
kohoClientCommand = splitter_server.stderr.readline()
splitter_server.stderr.close() # now close stderr so it can't fill up a buffer and block our process if we print too much

subprocess.check_call("echo " + sys.argv[1] + " | " + kohoClientCommand, shell=True)
