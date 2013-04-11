#!/usr/bin/env python
import re
import struct
import json
import sys

# load the defines from the original source - the interpreter header

f = open('../bytecodes.h', 'r')

re_define = re.compile('^#define')
re_parse = re.compile('''#define[\s\t\n]*|\s+|\t+|\n+''')

line = f.readline()
bytecodes = [{}, {}]  # map the bytecodes in both direcitons
while line:
    if re.match(re_define, line):
        split = re.split(re_parse, line)
        if split[2] != '':
            bytecodes[0][split[1]] = int(split[2])
            bytecodes[1][int(split[2])] = split[1]
    line = f.readline()

# pprint(bytecodes)

# interpret the given program and build a tree of its execution

#f = open('../samples/vm1.ncm')
f = sys.stdin

instr_size = 20
instr_arr = []
instr = f.read(instr_size)
while instr != "":
    values = struct.unpack_from("cIIII", instr, 0)
    parsed = {'instr': bytecodes[1][ord(values[0])],
              'arg0': values[1],
              'arg1': values[2],
              'arg2': values[3],
              'arg3': values[4]}
    instr_arr.append(parsed)
    instr = f.read(instr_size)

lengths = {'FUTURE': 1,
           'HALT': 1,
           'IF': 1,
           'MODE': 1,
           'CREATE': 1,
           'DESTROY': 1,
           'SEND': 1,
           'RECEIVE': 1,
           'SYNC': 1,
           'HANDLE': 1,
           'NOP': 1,
           'SET_COUNTER': 1,
           'ADD_TO_COUNTER': 1,
           'SUB_FROM_COUNTER': 1}


class TreeBuilder:
    future_queue = []

    def __init__(self, instrs):
        self.instrs = instrs

    def build_tree(self):
        return self.step(0)

    def step(self, i):
        instr = self.instrs[i]
        next = i+1
        tree = {}
        tree['instr'] = instr
        tree['length'] = lengths[instr['instr']]
        tree['children'] = []

        if(instr['instr'] == 'CREATE'):
            pass
        if(instr['instr'] == 'SEND'):
            pass
        if(instr['instr'] == 'SYNC'):
            pass
        if(instr['instr'] == 'RECEIVE'):
            pass
        if(instr['instr'] == 'IF'):
            next = [next, instr['arg1']]
        if(instr['instr'] == 'FUTURE'):
            self.future_queue.append(instr)
        elif(instr['instr'] == 'HALT'):
            self.future_queue = sorted(self.future_queue,
                                       key=lambda k: k['arg0'])
            next_future = self.future_queue.pop()
            next = next_future['arg1']
        else:
            pass

        if isinstance(next, (list, tuple)):
            for n in next:
                if n <= i:
                    tree['children'].append('LOOP')
                elif n < len(self.instrs):
                    tree['children'].append(self.step(n))
        elif next <= i:
            tree['children'].append('LOOP')
        elif next < len(self.instrs):
            tree['children'].append(self.step(next))
        return tree


treeBuilder = TreeBuilder(instr_arr)

sys.stdout.write("receive_data(")
json.dump(treeBuilder.build_tree(), sys.stdout)
sys.stdout.write(")")
