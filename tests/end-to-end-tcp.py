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

server = subprocess.Popen(['nc', '-l', '12345']) 

splitter_server = subprocess.Popen('koho-server',
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

toRun = splitter_server.stderr.readline()
print(toRun)

sys.stderr.write("Running koho-client... ")
os.system(toRun)
os.system('echo greg is the best | nc 127.1 12345')
sys.stderr.write("done.\n")

#sys.stderr.write("Waiting for receiver to print out incoming UDP datagram... ")
#expected_payload = "Hello, world."
#incoming_payload = prog.stdout.read()
#
#if incoming_payload == expected_payload:
#    sys.stderr.write("success.\n")
#    sys.exit( 0 )
#else:
#    sys.stderr.write('\nError: expected "%s" and received "%s".\n' % (expected_payload, incoming_payload))
#    sys.exit( 1 )
