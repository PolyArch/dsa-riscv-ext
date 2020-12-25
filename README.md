# DSA-RISCV

This repo is to minimize the invasion to `riscv-gnu-toolchain`.
There is no point to commit our ISA extension to a forked
`riscv-gnu-toolchain` repo, which increases the efforts of rebasing.
The scripts here applies the extended patch to the
`riscv-gnu-toolchain` repo.

|imm     |rs1,imm        |rs1,rs2,imm    |rd,imm    |rd,rs1     |rd,rs1,rs2|opcode2 |opcode1 |opcode|
|--------|---------------|---------------|----------|-----------|----------|--------|--------|------|
| 14..12 | 14..12        | 14..12        | 14..12   | 14..12    |14..12    |6..5    |4..2    |6..2  |
| 0      | 2             |     3         |  4       |    6      |  7       |  -     |  -     |-     |
|        |ss_cfg_port (I)|ss_cfg_para (I)|          |ss_recv (S)|          |  0     |  2     |0x2   |
|        |ss_lin_strm (I)|               |          |ss_wait (S)|          |  1     |  2     |0xa   |
|        |ss_ind_strm (I)|               |          |           |          |  2     |  6     |0x16  |
|        |ss_rec_strm (I)|               |          |           |          |  3     |  6     |0x1e  |
