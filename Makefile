all: riscv-dsa.h riscv-dsa.c auto-patch.py install-header
	cd ../riscv-gnu-toolchain/riscv-binutils && git stash && git stash clear
	./auto-patch.py riscv-dsa.h ../riscv-gnu-toolchain/riscv-binutils/include/opcode/riscv-opc.h \
                    riscv-dsa.c ../riscv-gnu-toolchain/riscv-binutils/opcodes/riscv-opc.c

.PHONY: opcodes-dsa
opcodes-dsa riscv-dsa.c:%: isa.ext
	./pp.py $^ opcodes-dsa riscv-dsa.c 

.PHONY: riscv-dsa.h
riscv-dsa.h: opcodes-dsa
	cat opcodes-dsa | ./riscv-opcodes/parse_opcodes -c > $@

.PHONY: install-header
install-header:
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/ss_insts.h
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/dsaintrin.h

clean:
	rm -f opcodes-dsa
	rm -f riscv-dsa.h riscv-dsa.c
	rm -f $(SS_TOOLS)/include/ss_insts.h
	rm -f $(SS_TOOLS)/include/dsaintrin.h
	cd ../riscv-gnu-toolchain/riscv-binutils && git stash && git stash clear

