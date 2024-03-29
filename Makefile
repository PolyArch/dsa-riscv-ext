RISCV_GNU_TOOLCHAIN ?= ../ss-riscv-tools/riscv-tools-feedstock/riscv-gnu-toolchain

all: patch-gnu RISCVInstrInfoSS.td install-header

.PHONY: patch-gnu
patch-gnu: riscv-dsa.h riscv-dsa.c auto-patch.py
	cd $(RISCV_GNU_TOOLCHAIN)/riscv-binutils && git stash && git stash clear
	./auto-patch.py riscv-dsa.h $(RISCV_GNU_TOOLCHAIN)/riscv-binutils/include/opcode/riscv-opc.h \
		riscv-dsa.c $(RISCV_GNU_TOOLCHAIN)/riscv-binutils/opcodes/riscv-opc.c

.PHONY: opcodes-dsa
opcodes-dsa riscv-dsa.c:%: isa.ext
	./pp.py $^ opcodes-dsa riscv-dsa.c 

.PHONY: riscv-dsa.h
riscv-dsa.h: opcodes-dsa
	cat opcodes-dsa | ./riscv-opcodes/parse_opcodes -c > $@

.PHONY: RISCVInstrInfoSS.td
RISCVInstrInfoSS.td: isa.ext
	./llvm.py $^ > ../dsa-llvm-project/llvm/lib/Target/RISCV/$@

.PHONY: install-header
install-header:
	mkdir -p $(SS_TOOLS)/include/dsa-ext/
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/ss_insts.h
	ln -sf `git rev-parse --show-toplevel`/dsaintrin.h $(SS_TOOLS)/include/dsaintrin.h
	ln -sf `git rev-parse --show-toplevel`/intrin_impl.h $(SS_TOOLS)/include/intrin_impl.h
	ln -sf `git rev-parse --show-toplevel`/spec.h $(SS_TOOLS)/include/dsa-ext/spec.h
	ln -sf `git rev-parse --show-toplevel`/spec.attr $(SS_TOOLS)/include/dsa-ext/spec.attr
	ln -sf `git rev-parse --show-toplevel`/rf.h $(SS_TOOLS)/include/dsa-ext/rf.h
	ln -sf `git rev-parse --show-toplevel`/rf.def $(SS_TOOLS)/include/dsa-ext/rf.def

clean:
	rm -f opcodes-dsa
	rm -f riscv-dsa.h riscv-dsa.c
	rm -f $(SS_TOOLS)/include/ss_insts.h
	rm -f $(SS_TOOLS)/include/dsaintrin.h
	rm -f $(SS_TOOLS)/include/intrin_impl.h
	rm -rf $(SS_TOOLS)/include/dsa-ext/
	rm -f ../dsa-llvm-project/llvm/lib/Target/RISCV/RISCVInstrInfoSS.td
	cd $(RISCV_GNU_TOOLCHAIN)/riscv-binutils && git stash && git stash clear

