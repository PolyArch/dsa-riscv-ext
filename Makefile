all: riscv-dsa.h riscv-dsa.c auto-patch.py install-header RISCVInstrInfoSS.td
	cd ../riscv-gnu-toolchain/riscv-binutils && git stash && git stash clear
	./auto-patch.py riscv-dsa.h ../riscv-gnu-toolchain/riscv-binutils/include/opcode/riscv-opc.h \
                        riscv-dsa.c ../riscv-gnu-toolchain/riscv-binutils/opcodes/riscv-opc.c

.PHONY: opcodes-dsa
opcodes-dsa riscv-dsa.c:%: isa.ext
	./pp.py $^ opcodes-dsa riscv-dsa.c 

.PHONY: riscv-dsa.h
riscv-dsa.h: opcodes-dsa
	cat opcodes-dsa | ./riscv-opcodes/parse_opcodes -c > $@

.PHONY: RISCVInstrInfoSS.td
RISCVInstrInfoSS.td: isa.ext
	./llvm.py $^ > ../llvm-project/llvm/lib/Target/RISCV/$@

.PHONY: install-header
install-header:
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(RISCV)/include/ss_insts.h
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(RISCV)/include/dsaintrin.h
	ln -sf `git rev-parse --show-toplevel`/intrin_impl.h $(RISCV)/include/intrin_impl.h
	ln -sf `git rev-parse --show-toplevel`/spec.h $(RISCV)/include/dsa/spec.h
	ln -sf `git rev-parse --show-toplevel`/rf.h $(RISCV)/include/dsa/rf.h
	ln -sf `git rev-parse --show-toplevel`/rf.def $(RISCV)/include/dsa/rf.def

clean:
	rm -f opcodes-dsa
	rm -f riscv-dsa.h riscv-dsa.c
	rm -f $(SS_TOOLS)/include/ss_insts.h
	rm -f $(SS_TOOLS)/include/dsaintrin.h
	rm -f $(SS_TOOLS)/include/intrin_impl.h
	rm -f $(SS_TOOLS)/include/dsa/rf.h
	rm -f $(SS_TOOLS)/include/dsa/rf.def
	rm -f $(SS_TOOLS)/include/dsa/spec.h
	rm -f ../llvm-project/llvm/lib/Target/RISCV/RISCVInstrInfoSS.td
	cd ../riscv-gnu-toolchain/riscv-binutils && git stash && git stash clear

