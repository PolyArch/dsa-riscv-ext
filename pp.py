#!/usr/bin/env python3

"""The pre-processor of the extended instruction."""

import sys

max_width = 16

def padspace(s, width):
    return s + ' ' * (width - len(s))

def binary(name, ty, args, op0, op1, funct3):
    def do_operand(res, arg, args, ty):
        if arg in args:
            if arg != 'imm':
                res.append(arg)
            else:
                imm_ty = {'U': 'imm20', 'I': 'imm12', 'S': 'imm12hi imm12lo'}
                res.append(imm_ty[ty])
        else:
            rule = {'I': {'rs1': '19..15=0', 'rs2': '', 'rd': '11..7=0', 'imm': '31..20=0'},
                    'S': {'rs1': '19..15=0', 'rs2': '24..20=0', 'rd': '', 'imm': '31..25=0 11..7=0'},
                    'U': {'rs1': '', 'rs2': '', 'rd': '11..7=0', 'imm': '31..12=0'}}
            res.append(rule[ty][arg])
    res = [name]
    args = args.split(',')
    do_operand(res, 'rd', args, ty)
    do_operand(res, 'rs1', args, ty)
    do_operand(res, 'rs2', args, ty)
    do_operand(res, 'imm', args, ty)
    res.append('14..12=%s' % funct3)
    res.append('6..5=%s' % op0)
    res.append('4..2=%s' % op1)
    res.append('1..0=3')
    return ' '.join(res)

def asmtext(name, ty, args, op0, op1, funct3):
    args = args.split(',')
    operands = []
    if 'rd' in args:
        operands.append('d')
    if 'rs1' in args:
        operands.append('s')
    if 'rs2' in args:
        operands.append('t')
    if 'imm' in args:
        imm_ty = {'U': 'u', 'I': 'j', 'S': 'q'}
        operands.append(imm_ty[ty])
    res = []
    res.append(padspace('"%s"' % name, 16))
    res.append(padspace('"%s"' % ','.join(operands), 8))
    res.append(padspace('MATCH_' + name.upper(), 24))
    res.append(padspace('MATCH_' + name.upper(), 24))
    return '{%s, INSN_CLASS_I, %s, %s, %s, match_opcode, 0},' % tuple(res)

with open(sys.argv[1], 'r') as ext,   \
     open(sys.argv[2], 'w') as afile, \
     open(sys.argv[3], 'w') as bfile:
    for raw in ext.readlines():
        raw = raw.strip()
        if '#' in raw:
            raw = raw[raw.index('#')+1:]
        if not raw:
            continue
        raw = [i for i in raw.split() if i]
        afile.write(binary(*raw) + '\n')
        bfile.write(asmtext(*raw) + '\n')
