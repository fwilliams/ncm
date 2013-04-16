#!/usr/bin/env python
import json
import sys
import struct
import binascii

output = None

if sys.version < '3':
    def b(x):
        return x
    output = sys.stdout
else:
    import codecs

    def b(x):
        return codecs.utf_8_encode(x)[0]
    output = sys.stdout.buffer


def format_mac(mac):
    return binascii.unhexlify(b(mac.replace(':', '')))

params = json.load(sys.stdin)

output.write(struct.pack('I', len(params['channels'])))  # + '\x00\x00\x00\x00')
for param in params['channels']:
    output.write(b(param['dev'] + '\x00'*(16-len(param['dev']))))
for param in params['channels']:
    output.write(struct.pack('6s', format_mac(param['mac'])))
