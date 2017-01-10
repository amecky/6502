// 6502.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <vector>
#define VM_IMPLEMENTATION
#include "6502.h"

// https://skilldrick.github.io/easy6502/
// http://www.obelisk.me.uk/6502/registers.html
// http://www.6502.org/tutorials/6502opcodes.html
// http://www.obelisk.me.uk/6502/reference.html
// https://en.wikibooks.org/wiki/6502_Assembly
// http://www.dwheeler.com/6502/oneelkruns/asm1step.html

int _tmain(int argc, _TCHAR* argv[]) {
	vm_context ctx;
	vm_clear_context(&ctx);
	int nc = 0;
	vm_assemble(&ctx, "BVS #$20");
	//int num = vm_assemble_file(&ctx, "prog\\branching.txt");
	//vm_save(&ctx, "bin\\branching.prg");
	//vm_run(&ctx,0x600, num);
	//vm_dump(&ctx,0x200, 8);
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

