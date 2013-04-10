#!/usr/bin/env python
import json
import sys
import struct


def format_mac(mac):
    return mac.replace(':', '').decode('hex')

params = json.load(sys.stdin)

sys.stdout.write(struct.pack('I6s', len(params['channels']),
                 format_mac(params['mac']))+'\x00'*2)

for param in params['channels']:
    # wirte our own mac
    # the device name is padded to 16 characters
    sys.stdout.write(struct.pack('16s6s', str(param['dev']),
                     format_mac(param['mac'])))
    pass
