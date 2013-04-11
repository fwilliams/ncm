#!/usr/bin/env python
from pprint import pprint
import re
import struct

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

f = open('../samples/vm1.ncm')

instr_size = 20
instr_arr = []
instr = f.read(instr_size)
while instr != "":
    values = struct.unpack_from("cIIII", instr, 0)
    parsed = {'instr': ord(values[0]),
              'arg0': values[1],
              'arg1': values[2],
              'arg2': values[3],
              'arg3': values[4]}
    instr_arr.append(parsed)
    instr = f.read(instr_size)

lengths = {bytecodes[0]['FUTURE']: 1,
           bytecodes[0]['HALT']: 1,
           bytecodes[0]['IF']: 1,
           bytecodes[0]['MODE']: 1,
           bytecodes[0]['CREATE']: 1,
           bytecodes[0]['DESTROY']: 1,
           bytecodes[0]['SEND']: 1,
           bytecodes[0]['RECEIVE']: 1,
           bytecodes[0]['SYNC']: 1,
           bytecodes[0]['HANDLE']: 1,
           bytecodes[0]['NOP']: 1,
           bytecodes[0]['SET_COUNTER']: 1,
           bytecodes[0]['ADD_TO_COUNTER']: 1,
           bytecodes[0]['SUB_FROM_COUNTER']: 1}


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

        if(instr['instr'] == bytecodes[0]['FUTURE']):
            self.future_queue.append(instr)
        elif(instr['instr'] == bytecodes[0]['HALT']):
            self.future_queue = sorted(self.future_queue,
                                       key=lambda k: k['arg0'])
            next_future = self.future_queue.pop()
            next = next_future['arg1']
        else:
            pass

        if next < len(self.instrs):
            if isinstance(next, (list, tuple)):
                for n in next:
                    tree['children'].append(self.step(n))
            else:
                tree['children'] = [self.step(next)]
        return tree


treeBuilder = TreeBuilder(instr_arr)

pprint(treeBuilder.build_tree())
