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
	vm_context* ctx = vm_create();
	vm_assemble("LDA #$20\nSTA $0200\n");
	vm_run();
	printf("A: %02X\n", ctx->registers[vm_registers::A]);
	printf("Memory: %02X\n", ctx->read(0x200));
	vm_dump_registers();
	vm_dump_memory(0x200, 16);
	//vm_assemble_file("test.txt");
	//vm_run();
	//vm_dump_registers();
	//vm_dump(0x00, 16);
	//vm_dump(0x100, 256);
	//vm_dump(0x200, 16);
	vm_release();
	return 0;
}

