#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import click
import socket
import sys
import time

@click.command()
@click.argument('address', type=click.STRING, required=True)
@click.argument('port', type=click.INT, required=True)
@click.option('--message', type=click.STRING, default="")
@click.option('--udp', default=False)
@click.option('--timeout', type=click.FLOAT , default=0.0)
def client(address, port, udp, message, timeout):
  socket_type = socket.SOCK_DGRAM if udp else socket.SOCK_STREAM
  s = socket.socket(socket.AF_INET, socket_type)
  resp = None
  try:
    s.connect((address, port))
    s.settimeout(timeout)
    if len(message) > 0:
      s.sendall(message.encode('utf-8'))
      resp = s.recv(1024)
      print(resp.decode())
  except (BrokenPipeError, socket.error) as e:
    print(str(e), file=sys.stderr)
  finally:
    s.close()

if __name__ == '__main__':
  client()
  
