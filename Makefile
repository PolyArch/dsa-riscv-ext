riscv-opc.h: opcodes-dsa
	cat `ls riscv-opcodes/opcodes-* | grep -v "^opcodes-custom$$"` $^ | \
	./riscv-opcodes/parse_opcodes -c > $@

clean:
	rm -f riscv-opc.h
