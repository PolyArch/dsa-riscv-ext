TODO: Put this in GitWiki.

The philosophy of this repo is to minimize the invasion to riscv-gnu-toolchain.
Everytime we modify the ISA, a new commit is added to our customized riscv-gnu.
This increases the efforts of rebasing the upstream.
With the help of this repo, we only need to store the "incremental-delta".
