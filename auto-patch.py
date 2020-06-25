#!/usr/bin/env python3
import sys

assert len(sys.argv) == 5, 'Usage: ./auto-patch.py [patched .h] [dst .h] [patched .c] [dst .c]'

to_remove, to_add = [], []

def gather_lines(a, cond):
    res = []
    for i, j in enumerate(a):
        if cond(j):
            res.append(i)
    assert res[-1] - res[0] + 1 == len(res), 'Range not continuous'
    return res[0], res[-1]

with open(sys.argv[1]) as f:
    src = f.readlines()
with open(sys.argv[2]) as f:
    dst = f.readlines()

def replace(src, dst, to_remove, to_add):
    src[to_remove[0]]

injectl, injectr = gather_lines(src, lambda x: '_SS_' in x and '#define' in x)
removel, remover = gather_lines(dst, lambda x: '_CUSTOM' in x and '#define' in x)
dst = dst[:removel] + src[injectl:injectr+1] + dst[remover+1:]

removel, remover = gather_lines(dst, lambda x: '_CUSTOM' in x and 'DECLARE_INSN' in x)
injectl, injectr = gather_lines(src, lambda x: '_SS_' in x and 'DECLARE_INSN' in x)
dst = dst[:removel] + src[injectl:injectr+1] + dst[remover+1:]

with open(sys.argv[2], 'w') as f:
    f.writelines(dst)

with open(sys.argv[4]) as f:
    cfile = f.readlines()

for i, j in enumerate(cfile):
    if 'riscv_opcodes[]' in j:
        assert '{\n' in cfile[i + 1]
        with open(sys.argv[4], 'w') as f:
            f.writelines(cfile[:i+2])
            f.writelines(open(sys.argv[3]).readlines())
            f.writelines(cfile[i+2:])
        quit()

assert False, 'Opcode text format written'
