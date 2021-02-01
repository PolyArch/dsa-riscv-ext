#!/usr/bin/env python3
import sys

opcodes = [(0, 2), (1, 2), (2, 6), (3, 6)]

with open(sys.argv[1]) as f:
    for raw in f.readlines():
        mn, ty, operands, op0, op1, func3 = raw.split()
        print('let hasSideEffects = 1, mayLoad = 1, mayStore = 1 in')
        opcode = opcodes.index((int(op0), int(op1)))
        assert opcode != -1, (op0, op1)
        print('def %s : RVInst%s<%s, OPC_CUSTOM_%d,' % (mn.upper(), ty, func3, opcode))
        outs = []
        ins = []
        if 'rd' in operands:
            outs.append('GPR:$rd')
        print('                    (outs %s),' % ', '.join(outs))
        if 'rs1' in operands:
            ins.append('GPR:$rs1')
        if 'rs2' in operands:
            ins.append('GPR:$rs2')
        if 'imm' in operands:
            assert ty in ['S', 'I']
            ins.append('simm12:$simm12')
        print('                    (ins %s),' % ', '.join(ins))
        print('                    "%s", "%s">,' % (mn, ', '.join(i[i.index(':')+1:] for i in (outs + ins))))
        print('                    Sched<[SSRead, SSWrite]>;\n')
