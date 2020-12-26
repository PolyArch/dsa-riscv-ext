# DSA-RISCV

This repo is to minimize the invasion to `riscv-gnu-toolchain`.
There is no point to commit our ISA extension to a forked
`riscv-gnu-toolchain` repo, which increases the efforts of rebasing.
The scripts here applies the extended patch to the
`riscv-gnu-toolchain` repo.

|imm   |rs1,imm    |rs1,rs2,imm|rd,imm|rd,rs1 |rd,rs1,rs2|       |        |      |
|------|-----------|-----------|------|-------|----------|-------|--------|------|
|14..12| 14..12    | 14..12    |14..12|14..12 |14..12    |opcode2|opcode1 |opcode|
|0     | 2         |     3     | 4    |    6  |  7       |6..5   |4..2    |6..2  |
|      |cfg_port(S)|cfg_para(S)|      |recv(I)|          |  0    |  2     |0x2   |
|      |lin_strm(S)|           |      |wait(I)|          |  1    |  2     |0xa   |
|      |ind_strm(S)|           |      |       |          |  2    |  6     |0x16  |
|      |rec_strm(S)|           |      |       |          |  3    |  6     |0x1e  |
