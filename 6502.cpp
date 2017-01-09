// 6502.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <vector>
#include "6502.h"

// https://skilldrick.github.io/easy6502/
// http://www.obelisk.me.uk/6502/registers.html
// http://www.6502.org/tutorials/6502opcodes.html
// http://www.obelisk.me.uk/6502/reference.html
// https://en.wikibooks.org/wiki/6502_Assembly
// http://www.dwheeler.com/6502/oneelkruns/asm1step.html

int _tmain(int argc, _TCHAR* argv[]) {
	vm::VirtualMachine machine;
	int nc = 0;
	machine.parseFile("prog\\branching.txt");
	machine.run(0x600);
	machine.dump(0x200, 8);
	//int size = machine.parse("LDA #$C0\nSTA $0200\nTAX\nSTX $0201\n",&nc);	
	//printf("size: %d commands: %d\n", size, nc);
	/*
	int pc = 0x600;
	for (int i = 0; i < nc; ++i) {
		pc = machine.step(pc);
		machine.dumpRegisters();
	}
	*/
	//machine.disassemble();
	/*
	if (machine.load("bin\\second.prg")) {
		machine.run();
		machine.dump(0x200, 8);
	}
	*/
	return 0;
}

