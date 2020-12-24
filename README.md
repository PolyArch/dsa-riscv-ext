# DSA-RISCV

This repo is to minimize the invasion to `riscv-gnu-toolchain`.
There is no point to commit our ISA extension to a forked
`riscv-gnu-toolchain` repo, which increases the efforts of rebasing.
The scripts here will apply the just extended patch to the
`riscv-gnu-toolchain` repo.

|funct3 | 14..12 | .. | .. | .. |6..5|4..2|
| 0     | 2      | 3  | 4  | 6  |  0 | 2  |
