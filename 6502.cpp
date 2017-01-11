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

const char* JUMPING = "LDA #$03\nJMP there\nBRK\nBRK\nBRK\nthere:\nSTA $0200\n";
const char* BRANCHING = "LDX #$08\ndecrement:\nDEX\nSTX $0200\nCPX #$03\nBNE decrement\nSTX $0201\nBRK\n";

int _tmain(int argc, _TCHAR* argv[]) {
	vm_context ctx;
	vm_clear_context(&ctx);
	int nc = 0;
	vm_assemble(&ctx, BRANCHING);
	vm_run(&ctx);
	vm_dump_registers(&ctx);
	return 0;
}

