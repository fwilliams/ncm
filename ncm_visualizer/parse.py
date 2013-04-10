import re

# load the defines from the original source - the interpreter header

f = open('../interpreter.h', 'r')

re_define = re.compile('^#define')
re_parse = re.compile('''#define[\s\t\n]*|\s+|\t+|\n+''')

line = f.readline()
defines = {}
while line:
    if re.match(re_define, line):
        split = re.split(re_parse, line)
        if split[2] != '':
            defines[split[1]] = chr(int(split[2]))
    line = f.readline()

print defines
# interpret the given program and build a tree of its execution

f = open('../tx1.ncm')

future_queue = []

instr = f.read(20)
while instr != "":
    print str(instr[0]), '|', defines['FUTURE']
    if(instr[0] == defines['FUTURE']):
        future_queue.append(instr[4])
    instr = f.read(20)

print future_queue
