all: riscv-dsa.h riscv-dsa.c auto-patch.py install-header
	cd ../riscv-gnu-toolchain/riscv-binutils && git stash && git stash clear
	./auto-patch.py riscv-dsa.h ../riscv-gnu-toolchain/riscv-binutils/include/opcode/riscv-opc.h \
                  riscv-dsa.c ../riscv-gnu-toolchain/riscv-binutils/opcodes/riscv-opc.c

riscv-dsa.h: opcodes-dsa
	cat opcodes-dsa | ./riscv-opcodes/parse_opcodes -c > $@

.PHONY: install-header
install-header:
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/ss_insts.h
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/dsaintrin.h

clean:
	rm -f riscv-opc.h
	rm -f $(SS_TOOLS)/include/ss_insts.h
	rm -f $(SS_TOOLS)/include/dsaintrin.h

