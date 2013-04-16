#!/usr/bin/env python
import json
import sys
import struct


def format_mac(mac):
    return mac.replace(':', '').decode('hex')

params = json.load(sys.stdin)

sys.stdout.write(struct.pack('I', len(params['channels'])) + '\x00\x00\x00\x00')
for param in params['channels']:
    sys.stdout.write(struct.pack('16s', str(param['dev'])))
for param in params['channels']:
    sys.stdout.write(struct.pack('6s', format_mac(param['mac'])))
