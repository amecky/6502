/*
 6502 simulator

This is based on the excellent reference documentation at http://www.obelisk.me.uk/6502/reference.htm

The 6502 uese little endian which means it starts with the least significant bit

MIT license
Copyright (c) 2017 Andreas Mecky (meckya@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once
#include <stdint.h>

// for unit testing we define all methods to be public
#if defined(VM_TEST_SUPPORT)
#define PRIVATE 
#else
#define PRIVATE static
#endif

// -----------------------------------------------------
// AddressingMode
//
// Enum of all supported addressing modes
// -----------------------------------------------------
typedef enum vm_addressing_mode {
	NONE,
	IMMEDIDATE,
	ABSOLUTE_ADR,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	ZERO_PAGE,
	ZERO_PAGE_X,
	ZERO_PAGE_Y,
	INDIRECT_ADR,
	INDIRECT_X,
	INDIRECT_Y,
	RELATIVE_ADR,
	JMP_ABSOLUTE,
	JMP_INDIRECT,
	ACCUMULATOR
} vm_addressing_mode;

// -----------------------------------------------------
// Flags
// -----------------------------------------------------
typedef enum vm_flags {
	UNUSED,
	C, // Carry Flag
	Z, // Zero Flag
	I,
	D,
	B,
	V,
	N
} vm_flags;

typedef void(*vm_LogFunc)(const char* message);

// -----------------------------------------------------
// The virtual machine vm_context
// -----------------------------------------------------
typedef struct vm_context {
	int registers[3];
	int programCounter;
	uint8_t mem[65536];
	uint8_t sp;
	uint8_t flags;
	int numCommands;
	int numBytes;
	vm_LogFunc logFunction;

	void clearFlags() {
		flags = 0;
	}

	void setFlag(int idx) {
		flags |= 1 << idx;
	}

	void clearFlag(int idx) {
		flags &= ~(1 << idx);
	}

	bool isSet(int idx) const {
		return ((flags >> idx) & 1 ) == idx;
	}
	void write(int idx, uint8_t v) {
		mem[idx] = v;
	}

	uint8_t read(int idx) const {
		return mem[idx];
	}

	int readInt(int idx) const {
		uint8_t upper = read(idx + 1);
		int data = read(idx) + (upper << 8);
		return data;
	}

	void push(uint8_t v) {
		mem[0x100 + sp] = v;
		--sp;
	}

	uint8_t pop() {
		uint8_t v = mem[0x100 + sp];
		++sp;
		return v;
	}
} vm_context;


// ---------------------------------------------------------
//  API
// ---------------------------------------------------------
void vm_clear_context(vm_context* ctx);

bool vm_load(vm_context* ctx, const char* fileName);

void vm_save(vm_context* ctx, const char* fileName);

int vm_assemble_file(vm_context* ctx, const char* fileName);

void vm_disassemble(vm_context* ctx);

int vm_assemble(vm_context* ctx, const char* text);

void vm_dump(vm_context* ctx, int pc, int num);

void vm_dump_registers(vm_context* ctx);

void vm_memory_dump(vm_context* ctx, int pc, int num);

bool vm_step(vm_context* ctx);

void vm_run(vm_context* ctx);


#if defined(VM_IMPLEMENTATION)

#include <stdio.h>
#include <vector>

typedef void(*commandFunc)(vm_context* ctx, int data);

PRIVATE uint8_t low_value(int value) {
	return value & 255;
}

PRIVATE uint8_t high_value(int value) {
	return (value >> 8) & 255;
}

// -----------------------------------------------------
// set program counter
// -----------------------------------------------------
PRIVATE void vm_set_program_counter(vm_context* ctx, uint8_t relativeAddress) {
	uint8_t adr = relativeAddress;
	++adr;
	if (adr > 127) {
		adr = 255 - adr;
	}
	ctx->programCounter -= adr;
}

// -----------------------------------------------------
// set zero flag
// -----------------------------------------------------
PRIVATE void vm_set_zero_flag(vm_context* ctx, int data) {
	if (data == 0) {
		ctx->setFlag(vm_flags::Z);
	}
	else {
		ctx->clearFlag(vm_flags::Z);
	}
}

// -----------------------------------------------------
// set negative flag
// -----------------------------------------------------
PRIVATE void vm_set_negative_flag(vm_context* ctx, int data) {
	if (data > 127) {
		ctx->setFlag(vm_flags::N);
	}
	else {
		ctx->clearFlag(vm_flags::N);
	}
}
// -----------------------------------------------------
// Command function definitions
// -----------------------------------------------------
PRIVATE void vm_op_nop(vm_context* ctx, int pc) {
}

// ------------------------------------------
// LDA
// ------------------------------------------
PRIVATE void vm_op_lda(vm_context* ctx, int data) {
	ctx->registers[0] = data;
	vm_set_zero_flag(ctx, data);
	vm_set_negative_flag(ctx, data);
}

// ------------------------------------------
// LDX
// ------------------------------------------
PRIVATE void vm_op_ldx(vm_context* ctx, int data) {
	ctx->registers[1] = data;
	vm_set_zero_flag(ctx, data);
	vm_set_negative_flag(ctx, data);
}

// ------------------------------------------
// LDY
// ------------------------------------------
PRIVATE void vm_op_ldy(vm_context* ctx, int data) {
	ctx->registers[2] = data;
	vm_set_zero_flag(ctx, data);
	vm_set_negative_flag(ctx, data);
}

// ------------------------------------------
// STX
// ------------------------------------------
PRIVATE void vm_op_stx(vm_context* ctx, int data) {
	ctx->write(data, ctx->registers[1]);
}

// ------------------------------------------
// STY
// ------------------------------------------
PRIVATE void vm_op_sty(vm_context* ctx, int data) {
	ctx->write(data, ctx->registers[1]);
}


// ------------------------------------------
// STA
// ------------------------------------------
PRIVATE void vm_op_sta(vm_context* ctx, int data) {
	ctx->write(data, ctx->registers[0]);
}

// ------------------------------------------
// TAX
// ------------------------------------------
PRIVATE void vm_op_tax(vm_context* ctx, int data) {
	ctx->registers[1] = ctx->registers[0];
	vm_set_zero_flag(ctx, ctx->registers[1]);
	vm_set_negative_flag(ctx, ctx->registers[1]);
}

// ------------------------------------------
// TAY
// ------------------------------------------
PRIVATE void vm_op_tay(vm_context* ctx, int data) {
	ctx->registers[2] = ctx->registers[0];
	vm_set_zero_flag(ctx, ctx->registers[2]);
	vm_set_negative_flag(ctx, ctx->registers[2]);
}

// ------------------------------------------
// TYA
// ------------------------------------------
PRIVATE void vm_op_tya(vm_context* ctx, int data) {
	ctx->registers[0] = ctx->registers[2];
	vm_set_zero_flag(ctx, ctx->registers[0]);
	vm_set_negative_flag(ctx, ctx->registers[0]);
}

// ------------------------------------------
// TXA
// ------------------------------------------
PRIVATE void vm_op_txa(vm_context* ctx, int data) {
	ctx->registers[0] = ctx->registers[1];
	vm_set_zero_flag(ctx, ctx->registers[0]);
	vm_set_negative_flag(ctx, ctx->registers[0]);
}

// ------------------------------------------
// INX
// ------------------------------------------
PRIVATE void vm_op_inx(vm_context* ctx, int data) {
	ctx->registers[1] += 1;
	vm_set_zero_flag(ctx, ctx->registers[1]);
	vm_set_negative_flag(ctx, ctx->registers[1]);
}

// ------------------------------------------
// INY
// ------------------------------------------
PRIVATE void vm_op_iny(vm_context* ctx, int data) {
	ctx->registers[2] += 1;
	vm_set_zero_flag(ctx, ctx->registers[2]);
	vm_set_negative_flag(ctx, ctx->registers[2]);
}

// ------------------------------------------
// INC
// ------------------------------------------
PRIVATE void vm_op_inc(vm_context* ctx, int data) {
	int v = ctx->read(data) + 1;
	ctx->write(data, v);
	vm_set_zero_flag(ctx, v);
	vm_set_negative_flag(ctx, v);
}

// ------------------------------------------
// ADC
// ------------------------------------------
PRIVATE void adc(vm_context* ctx, int data) {
	ctx->registers[0] += data;
	if (ctx->registers[0] == 0) {
		ctx->setFlag(vm_flags::Z);
	}
	else {
		ctx->clearFlag(vm_flags::Z);
	}
}



// ------------------------------------------
// CPX
// ------------------------------------------
PRIVATE void vm_op_cpx(vm_context* ctx, int data) {
	if (ctx->registers[1] == data) {
		ctx->setFlag(vm_flags::Z);
	}
	else {
		ctx->clearFlag(vm_flags::Z);
	}
	if (ctx->registers[1] >= data) {
		ctx->setFlag(vm_flags::C);
	}
	else {
		ctx->clearFlag(vm_flags::C);
	}
	// FIXME: negative flag handling
}

// ------------------------------------------
// CPY
// ------------------------------------------
PRIVATE void vm_op_cpy(vm_context* ctx, int data) {
	if (ctx->registers[2] == data) {
		ctx->setFlag(vm_flags::Z);
	}
	else {
		ctx->clearFlag(vm_flags::Z);
	}
	if (ctx->registers[2] >= data) {
		ctx->setFlag(vm_flags::C);
	}
	else {
		ctx->clearFlag(vm_flags::C);
	}
	// FIXME: negative flag handling
}

// ------------------------------------------
// decrement
// ------------------------------------------
PRIVATE void vm_decrement(vm_context* ctx, int pc, int idx) {
	--ctx->registers[idx];
	if (ctx->registers[idx] < 0) {
		ctx->registers[idx] = 255;
	}
	vm_set_zero_flag(ctx, ctx->registers[idx]);
	vm_set_negative_flag(ctx, ctx->registers[idx]);
}

// ------------------------------------------
// DEX
// ------------------------------------------
PRIVATE void vm_op_dex(vm_context* ctx, int data) {
	vm_decrement(ctx, data, 1);
}

// ------------------------------------------
// DEY
// ------------------------------------------
PRIVATE void vm_op_dey(vm_context* ctx, int pc) {
	vm_decrement(ctx, pc, 2);
}

// ------------------------------------------
// DEC
// ------------------------------------------
PRIVATE void vm_op_dec(vm_context* ctx, int pc) {
	// FIXME: this is wrong
	vm_decrement(ctx, pc, 0);
}

PRIVATE void brk(vm_context* ctx, int pc) {
}

// ------------------------------------------
// BNE
// ------------------------------------------
PRIVATE void vm_op_bne(vm_context* ctx, int data) {
	if (!ctx->isSet(vm_flags::Z)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BEQ
// ------------------------------------------
PRIVATE void vm_op_beq(vm_context* ctx, int data) {
	if (ctx->isSet(vm_flags::Z)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BPL
// ------------------------------------------
PRIVATE void vm_op_bpl(vm_context* ctx, int data) {
	if (!ctx->isSet(vm_flags::N)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BVC
// ------------------------------------------
PRIVATE void vm_op_bvc(vm_context* ctx, int data) {
	if (!ctx->isSet(vm_flags::V)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BVS
// ------------------------------------------
PRIVATE void vm_op_bvs(vm_context* ctx, int data) {
	if (ctx->isSet(vm_flags::V)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BCC
// ------------------------------------------
PRIVATE void vm_op_bcc(vm_context* ctx, int data) {
	if (!ctx->isSet(vm_flags::C)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BCS
// ------------------------------------------
PRIVATE void vm_op_bcs(vm_context* ctx, int data) {
	if (ctx->isSet(vm_flags::C)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// BMI
// ------------------------------------------
PRIVATE void vm_op_bmi(vm_context* ctx, int data) {
	if (ctx->isSet(vm_flags::N)) {
		vm_set_program_counter(ctx, data);
	}
	else {
		ctx->programCounter += 2;
	}
}

// ------------------------------------------
// CLC
// ------------------------------------------
PRIVATE void vm_op_clc(vm_context* ctx, int data) {
	ctx->clearFlag(vm_flags::C);
}

// ------------------------------------------
// CLD
// ------------------------------------------
PRIVATE void vm_op_cld(vm_context* ctx, int data) {
	ctx->clearFlag(vm_flags::D);
}

// ------------------------------------------
// CLI
// ------------------------------------------
PRIVATE void vm_op_cli(vm_context* ctx, int data) {
	ctx->clearFlag(vm_flags::I);
}

// ------------------------------------------
// CLV
// ------------------------------------------
PRIVATE void vm_op_clv(vm_context* ctx, int data) {
	ctx->clearFlag(vm_flags::V);
}

// ------------------------------------------
// PHA
// ------------------------------------------
PRIVATE void vm_op_pha(vm_context* ctx, int data) {
	ctx->push(ctx->registers[0]);
}

// ------------------------------------------
// PLA
// ------------------------------------------
PRIVATE void vm_op_pla(vm_context* ctx, int data) {
	ctx->registers[0] = ctx->pop();
}

// ------------------------------------------
// SEC
// ------------------------------------------
PRIVATE void vm_op_sec(vm_context* ctx, int data) {
	ctx->setFlag(vm_flags::C);
}

// ------------------------------------------
// SED
// ------------------------------------------
PRIVATE void vm_op_sed(vm_context* ctx, int data) {
	ctx->setFlag(vm_flags::D);
}

// ------------------------------------------
// BIT
// ------------------------------------------
PRIVATE void vm_op_bit(vm_context* ctx, int data) {
	uint8_t v = ctx->read(data);
	uint8_t a = ctx->registers[0];
	uint8_t r = v & a;
	vm_set_zero_flag(ctx, r);
	// V 	Overflow Flag 	Set to bit 6 of the memory value
	// N 	Negative Flag 	Set to bit 7 of the memory value
}

// ------------------------------------------
// ORA
// ------------------------------------------
PRIVATE void vm_op_ora(vm_context* ctx, int data) {
	uint8_t v = ctx->read(data);
	uint8_t a = ctx->registers[0];
	uint8_t r = v | a;
	vm_set_zero_flag(ctx, r);
	vm_set_negative_flag(ctx, r);
}

// ------------------------------------------
// EOR
// ------------------------------------------
PRIVATE void vm_op_eor(vm_context* ctx, int data) {
	uint8_t v = ctx->read(data);
	uint8_t a = ctx->registers[0];
	uint8_t r = 0;
	for (int i = 0; i < 8; ++i) {
		uint8_t x = 1 << i;
		if (((v & x) == x && (a & x) != x) || ((v & x) != x && (a & x) == x)) {
			r |= x;
		}
	}
	vm_set_zero_flag(ctx, r);
	vm_set_negative_flag(ctx, r);
}

// ------------------------------------------
// JMP
// ------------------------------------------
PRIVATE void vm_op_jmp(vm_context* ctx, int data) {
	ctx->programCounter = data;
}

// ------------------------------------------
// JSR
// ------------------------------------------
PRIVATE void vm_op_jsr(vm_context* ctx, int data) {
	ctx->push(high_value(ctx->programCounter));
	ctx->push(low_value(ctx->programCounter));
	ctx->programCounter = data;
}

// ------------------------------------------
// RTS
// ------------------------------------------
PRIVATE void vm_op_rts(vm_context* ctx, int data) {
	uint8_t low = ctx->pop();
	uint8_t high = ctx->pop();
	ctx->programCounter = low + (high << 8);
}

// ------------------------------------------------------------------------------------------------------------------------------
// AND - A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_and(vm_context* ctx, int data) {
	int a = ctx->registers[0];
	int cmp = a & data;
	vm_set_zero_flag(ctx, cmp);
	vm_set_negative_flag(ctx, cmp);
}

// ------------------------------------------------------------------------------------------------------------------------------
// PHP - Pushes a copy of the status flags on to the stack.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_php(vm_context* ctx, int data) {
	ctx->push(ctx->flags);
}

// ------------------------------------------------------------------------------------------------------------------------------
// ASL - This operation shifts all the bits of the accumulator or memory contents one bit left. Bit 0 is set to 0 and bit 7 is 
// placed in the carry flag. The effect of this operation is to multiply the memory contents by 2 
// (ignoring 2's complement considerations), setting the carry if the result will not fit in 8 bits.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_asl(vm_context* ctx, int data) {
	ctx->push(ctx->flags);
}

// ------------------------------------------------------------------------------------------------------------------------------
// PLP - Pulls an 8 bit value from the stack and into the processor flags. The flags will take on new states as determined by the value pulled.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_plp(vm_context* ctx, int data) {
	ctx->flags = ctx->pop();
}

// ------------------------------------------------------------------------------------------------------------------------------
// LSR - Each of the bits in A or M is shift one place to the right. The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_lsr(vm_context* ctx, int data) {
	int v = 0;
	if ( data == -1) {
		v = ctx->registers[0];
	}
	else {
		v = ctx->read(data);
	}
	bool bit = false;
	if ( (v & 1) == 1 ) {
		bit = true;
	}
	if ( bit) {
		ctx->setFlag(vm_flags::C);
	}
	else {
		ctx->clearFlag(vm_flags::C);
	}
	int n = v >> 1;
	if ( data == -1 ) {
		ctx->registers[0] = n;
	}
	else {
		ctx->write(data,n);
	}
}

// ------------------------------------------------------------------------------------------------------------------------------
// ROL - Move each of the bits in either A or M one place to the left. Bit 0 is filled with the current 
//       value of the carry flag whilst the old bit 7 becomes the new carry flag value.
// ------------------------------------------------------------------------------------------------------------------------------
PRIVATE void vm_op_rol(vm_context* ctx, int data) {
	int v = 0;
	if (data == -1) {
		v = ctx->registers[0];
	}
	else {
		v = ctx->read(data);
	}
	int n = v >> 1;
	if (ctx->isSet(vm_flags::C)) {
		n |= 1;
	}
	else {
		n &= ~1;
	}
	if ((v & 128) == 128) {
		ctx->setFlag(vm_flags::C);
	}
	else {
		ctx->clearFlag(vm_flags::C);
	}
	if (data == -1) {
		ctx->registers[0] = n;
	}
	else {
		ctx->write(data, n);
	}
}
// -----------------------------------------------------
// Command
// -----------------------------------------------------
typedef struct vm_command {
	const char* name;
	bool modifyPC;
	commandFunc function;
	int supportedModes;

	bool isSupported(vm_addressing_mode mode) {
		return (supportedModes & mode) == mode;
	}
} vm_command;

// ADC ASL CMP ROR RTI SBC SEI TSX TXS
// -----------------------------------------------------
// Array of all supported commands with function pointer
// and a bitset of supported addressing modes
// -----------------------------------------------------
const static vm_command VM_COMMANDS[] = {
	{ "ADC", false, &adc, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "AND", false, &vm_op_and, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "ASL", false, &vm_op_nop, 0 },
	{ "BCC", true , &vm_op_bcc, 1 << RELATIVE_ADR },
	{ "BCS", true , &vm_op_bcs, 1 << RELATIVE_ADR },
	{ "BEQ", true , &vm_op_beq, 1 << RELATIVE_ADR },
	{ "BIT", false, &vm_op_bit, 1 << ABSOLUTE_ADR | 1 << ZERO_PAGE },
	{ "BMI", true , &vm_op_bmi, 1 << RELATIVE_ADR },
	{ "BNE", true , &vm_op_bne, 1 << RELATIVE_ADR },
	{ "BPL", true , &vm_op_bpl, 1 << RELATIVE_ADR },
	{ "BRK", false, &brk, 0 },
	{ "BVC", true , &vm_op_bvc, 1 << RELATIVE_ADR },
	{ "BVS", true , &vm_op_bvs, 1 << RELATIVE_ADR },
	{ "CLC", false, &vm_op_clc, 0 },
	{ "CLD", false, &vm_op_cld, 0 },
	{ "CLI", false, &vm_op_cli, 0 },
	{ "CLV", false, &vm_op_clv, 0 },
	{ "CMP", false, &vm_op_nop, 0 },
	{ "CPX", false, &vm_op_cpx, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ABSOLUTE_ADR },
	{ "CPY", false, &vm_op_cpy, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ABSOLUTE_ADR },
	{ "DEC", false, &vm_op_dec, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "DEX", false, &vm_op_dex, 0 },
	{ "DEY", false, &vm_op_dey, 0 },
	{ "EOR", false, &vm_op_eor, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "INC", false, &vm_op_inc, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "INX", false, &vm_op_inx, 0 },
	{ "INY", false, &vm_op_iny, 0 },
	{ "JMP", true , &vm_op_jmp, 1 << JMP_ABSOLUTE | 1 << JMP_INDIRECT },
	{ "JSR", true , &vm_op_jsr, 1 << JMP_ABSOLUTE },
	{ "LDA", false, &vm_op_lda, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "LDX", false, &vm_op_ldx, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_Y },
	{ "LDY", false, &vm_op_ldy, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "LSR", false, &vm_op_lsr, 1 << ACCUMULATOR | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "NOP", false, &vm_op_nop, 0 },
	{ "ORA", false, &vm_op_ora, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "PHA", false, &vm_op_pha, 0 },
	{ "PHP", false, &vm_op_php, 0 },
	{ "PLA", false, &vm_op_pla, 0 },
	{ "PLP", false, &vm_op_plp, 0 },
	{ "ROL", false, &vm_op_rol, 1 << ACCUMULATOR | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "ROR", false, &vm_op_nop, 0 },
	{ "RTI", false, &vm_op_nop, 0 },
	{ "RTS", true , &vm_op_rts, 0 },
	{ "SBC", false, &vm_op_nop, 0 },
	{ "SEC", false, &vm_op_sec, 0 },
	{ "SED", false, &vm_op_sed, 0 },
	{ "SEI", false, &vm_op_nop, 0 },
	{ "STA", false, &vm_op_sta, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "STX", false, &vm_op_stx, 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR },
	{ "STY", false, &vm_op_sty, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR },
	{ "TAX", false, &vm_op_tax, 0 },
	{ "TAY", false, &vm_op_tay, 0 },
	{ "TSX", false, &vm_op_nop, 0 },
	{ "TXA", false, &vm_op_txa, 0 },
	{ "TXS", false, &vm_op_nop, 0 },
	{ "TYA", false, &vm_op_tya, 0 }
};

typedef enum vm_opcode {
	ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI, CLV,
	CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP, JSR, LDA, LDX, LDY, LSR, NOP,
	ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX,
	TAY, TSX, TXA, TXS, TYA, EOL
} vm_opcode;

const static int NUM_COMMANDS = 56;

PRIVATE int find_command(const char* text) {
	for (int i = 0; i < NUM_COMMANDS; ++i) {
		if (text[0] == VM_COMMANDS[i].name[0]) {
			if (text[1] == VM_COMMANDS[i].name[1] && text[2] == VM_COMMANDS[i].name[2]) {
				return i;
			}
		}
	}
	return -1;
}

// -----------------------------------------------------
// The number of bytes of data for every addressing
// mode
// -----------------------------------------------------
// NONE,IMMEDIDATE,ABSOLUTE_ADR,ABSOLUTE_X,ABSOLUTE_Y,ZERO_PAGE,ZERO_PAGE_X,ZERO_PAGE_Y,INDIRECT_X,INDIRECT_Y,RELATIVE_ADR
const static int VM_DATA_SIZE[] = {
	0, 1, 2, 2, 2, 1, 1, 1, 2, 2, 1, 2, 2, 0
};

	

// -----------------------------------------------------
// Command mapping
// -----------------------------------------------------
typedef struct vm_command_mapping {
	vm_opcode op_code;
	vm_addressing_mode mode;
	uint8_t hex;
} vm_command_mapping;

const static vm_command_mapping NO_OP = vm_command_mapping{ EOL, NONE, 0xFF };

// -----------------------------------------------------
// Array of all comand mappings
// -----------------------------------------------------
const static vm_command_mapping VM_COMMAND_MAPPING[] = {
	// ADC
	{ ADC, IMMEDIDATE,   0x69 },
	{ ADC, ZERO_PAGE,    0x65 },
	{ ADC, ZERO_PAGE_X,  0x75 },
	{ ADC, ABSOLUTE_ADR, 0x6D },
	{ ADC, ABSOLUTE_X,   0x7D },
	{ ADC, ABSOLUTE_Y,   0x79 },
	{ ADC, INDIRECT_X,   0x61 },
	{ ADC, INDIRECT_Y,   0x71 },
	{ AND, IMMEDIDATE,   0x29 },
	{ AND, ZERO_PAGE,    0x25 },
	{ AND, ZERO_PAGE_X,  0x35 },
	{ AND, ABSOLUTE_ADR, 0x2D },
	{ AND, ABSOLUTE_X,   0x3D },
	{ AND, ABSOLUTE_Y,   0x39 },
	{ AND, INDIRECT_X,   0x21 },
	{ ADC, INDIRECT_Y,   0x31 },
	{ BIT, ABSOLUTE_ADR, 0x2C },
	{ BIT, ZERO_PAGE,    0x24 },
	{ BMI, RELATIVE_ADR, 0x30 },
	{ BNE, RELATIVE_ADR, 0xD0 },
	{ BNE, RELATIVE_ADR, 0xD0 },
	{ BPL, RELATIVE_ADR, 0x10 },
	{ BRK, NONE,         0x00 },
	{ BVC, RELATIVE_ADR, 0x50 },
	{ BVS, RELATIVE_ADR, 0x70 },
	{ CLC, NONE,         0x18 },
	{ CLD, NONE,         0xD8 },
	{ CLI, NONE,         0x58 },
	{ CLV, NONE,         0xB8 },
	{ CPX, IMMEDIDATE,   0xE0 },
	{ CPX, ZERO_PAGE,    0xE4 },
	{ CPX, ABSOLUTE_ADR, 0xEC },
	{ CPY, IMMEDIDATE,   0xC0 },
	{ CPY, ZERO_PAGE,    0xC4 },
	{ CPY, ABSOLUTE_ADR, 0xCC },
	{ DEC, ZERO_PAGE,    0xC6 },
	{ DEC, ZERO_PAGE_X,  0xD6 },
	{ DEC, ABSOLUTE_ADR, 0xCE },
	{ DEC, ABSOLUTE_X,   0xDE },
	{ DEX, NONE,         0xCA },
	{ DEY, NONE,         0x88 },
	{ EOR, IMMEDIDATE,   0x49 },
	{ EOR, ZERO_PAGE,    0x45 },
	{ EOR, ZERO_PAGE_X,  0x55 },
	{ EOR, ABSOLUTE_ADR, 0x4D },
	{ EOR, ABSOLUTE_X,   0x5D },
	{ EOR, ABSOLUTE_Y,   0x59 },
	{ EOR, INDIRECT_X,   0x41 },
	{ EOR, INDIRECT_Y,   0x51 },
	{ INC, ZERO_PAGE,    0xE6 },
	{ INC, ZERO_PAGE_X,  0xF6 },
	{ INC, ABSOLUTE_ADR, 0xEE },
	{ INC, ABSOLUTE_X,   0xFE },
	{ INX, NONE,         0xE8 },
	{ INY, NONE,         0xC8 },
	{ JMP, JMP_ABSOLUTE, 0x4C },
	{ JMP, JMP_INDIRECT, 0x6C },
	{ JSR, JMP_ABSOLUTE, 0x20 },
	{ LDA, IMMEDIDATE,   0xA9 },
	{ LDA, ZERO_PAGE,    0xA5 },
	{ LDA, ZERO_PAGE_X,  0xB5 },
	{ LDA, ABSOLUTE_ADR, 0xAD },
	{ LDA, ABSOLUTE_X,   0xBD },
	{ LDA, ABSOLUTE_Y,   0xB9 },
	{ LDA, INDIRECT_X,   0xA1 },
	{ LDA, INDIRECT_Y,   0xB1 },
	{ LDX, IMMEDIDATE,   0xA2 },
	{ LDX, ZERO_PAGE,    0xA6 },
	{ LDX, ZERO_PAGE_Y,  0xB6 },
	{ LDX, ABSOLUTE_ADR, 0xAE },
	{ LDX, ABSOLUTE_Y,   0xBE },
	{ LDY, IMMEDIDATE,   0xA0 },
	{ LDY, ZERO_PAGE,    0xA4 },
	{ LDY, ZERO_PAGE_X,  0xB4 },
	{ LDY, ABSOLUTE_ADR, 0xAC },
	{ LDY, ABSOLUTE_X,   0xBC },
	{ LSR, ACCUMULATOR,  0x4A },
	{ LSR, ZERO_PAGE,    0x46 },
	{ LSR, ZERO_PAGE_X,  0x56 },
	{ LSR, ABSOLUTE_ADR, 0x4E },
	{ LSR, ABSOLUTE_X,   0x5E },
	{ ORA, IMMEDIDATE,   0x09 },
	{ ORA, ZERO_PAGE,    0x05 },
	{ ORA, ZERO_PAGE_X,  0x15 },
	{ ORA, ABSOLUTE_ADR, 0x0D },
	{ ORA, ABSOLUTE_X,   0x1D },
	{ ORA, ABSOLUTE_Y,   0x19 },
	{ ORA, INDIRECT_X,   0x01 },
	{ ORA, INDIRECT_Y,   0x11 },
	{ PHA, NONE,         0x48 },
	{ PHP, NONE,         0x08 },
	{ PLA, NONE,         0x68 },
	{ PLP, NONE,         0x28 },
	{ ROL, ACCUMULATOR,  0x2A },
	{ ROL, ZERO_PAGE,    0x26 },
	{ ROL, ZERO_PAGE_X,  0x36 },
	{ ROL, ABSOLUTE_ADR, 0x2E },
	{ ROL, ABSOLUTE_X,   0x3E },
	{ RTS, NONE,         0x60 },
	{ SEC, NONE,         0x38 },
	{ SED, NONE,         0xF8 },
	{ STA, ZERO_PAGE,    0x85 },
	{ STA, ZERO_PAGE_X,  0x95 },
	{ STA, ABSOLUTE_ADR, 0x8D },
	{ STA, ABSOLUTE_X,   0x9D },
	{ STA, ABSOLUTE_Y,   0x99 },
	{ STA, INDIRECT_X,   0x81 },
	{ STA, INDIRECT_Y,   0x91 },
	{ STX, ZERO_PAGE,    0x86 },
	{ STX, ZERO_PAGE_Y,  0x96 },
	{ STX, ABSOLUTE_ADR, 0x8E },
	{ STY, ZERO_PAGE,    0x84 },
	{ STY, ZERO_PAGE_X,  0x94 },
	{ STY, ABSOLUTE_ADR, 0x8C },
	{ TAX, NONE,         0xAA },
	{ TAY, NONE,         0xA8 },
	{ TSX, NONE,         0xBA },
	{ TXA, NONE,         0x8A },
	{ TXS, NONE,         0x9A },
	{ TYA, NONE,         0x98 },
	{ EOL, NONE,         0xFF },
};

const uint32_t FNV_Prime = 0x01000193; //   16777619
const uint32_t FNV_Seed = 0x811C9DC5; // 2166136261

PRIVATE uint32_t fnv1a(const char* text, int len, uint32_t hash = FNV_Seed) {
	const unsigned char* ptr = (const unsigned char*)text;
	for (int i = 0; i < len; ++i ) {
		hash = (ptr[i] ^ hash) * FNV_Prime;
	}
	return hash;
}
// -----------------------------------------------------
// Token
// -----------------------------------------------------
typedef struct vm_token {

	enum TokenType { EMPTY, NUMBER, STRING, DOLLAR, HASHTAG, OPEN_BRACKET, CLOSE_BRACKET, COMMA, X, Y, SEPARATOR, COMMAND,ACCUMULATOR };

	vm_token(TokenType t) : type(t), value(0), hash(0) {}
	vm_token(TokenType t, int v) : type(t), value(v), hash(0) {}
	//Token(TokenType t, uint32_t st, int s) : type(t), value(0), hash(st), size(s) {}

	TokenType type;
	int value;
	uint32_t hash;
	int line;
} vm_token;

	

PRIVATE char* read_file(const char* fileName) {
	FILE *fp = fopen(fileName, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		int sz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* data = new char[sz + 1];
		fread(data, 1, sz, fp);
		data[sz] = '\0';
		fclose(fp);
		return data;
	}
	return 0;
}

PRIVATE bool isDigit(const char* c) {
	if ((*c >= '0' && *c <= '9')) {
		return true;
	}
	return false;
}

PRIVATE bool isNumeric(const char c) {
	return ((c >= '0' && c <= '9'));
}

PRIVATE bool isHex(const char c) {
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

PRIVATE bool isWhitespace(const char c) {
	if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
		return true;
	}
	return false;
}

PRIVATE int hex2int(const char *hex, char** endPtr) {
	int val = 0;
	while (isHex(*hex)) {
		char byte = *hex++;
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
		val = (val << 4) | (byte & 0xF);
	}
	if (endPtr) {
		*endPtr = (char *)(hex);
	}
	return val;
}

PRIVATE float str2f(const char* p, char** endPtr) {
	while (isWhitespace(*p)) {
		++p;
	}
	float sign = 1.0f;
	if (*p == '-') {
		sign = -1.0f;
		++p;
	}
	else if (*p == '+') {
		++p;
	}
	float value = 0.0f;
	while (isNumeric(*p)) {
		value *= 10.0f;
		value = value + (*p - '0');
		++p;
	}
	if (*p == '.') {
		++p;
		float dec = 1.0f;
		float frac = 0.0f;
		while (isNumeric(*p)) {
			frac *= 10.0f;
			frac = frac + (*p - '0');
			dec *= 10.0f;
			++p;
		}
		value = value + (frac / dec);
	}
	if (endPtr) {
		*endPtr = (char *)(p);
	}
	return value * sign;
}

// -----------------------------------------------------------------
// Tokenizer
// -----------------------------------------------------------------
class Tokenizer {

public:
	Tokenizer::Tokenizer() : _text(nullptr), _created(false) {
	}

	Tokenizer::~Tokenizer() {
		if (_text != nullptr && _created) {
			delete[] _text;
		}
	}

	bool Tokenizer::parseFile(const char* fileName) {
		_text = read_file(fileName);
		_created = true;
		if (_text == 0) {
			return false;
		}
		return parse(_text);
	}

	bool Tokenizer::parse(const char* text) {
		_text = text;
		int cnt = 0;
		const char* p = _text;
		int line = 1;
		while (*p != 0) {
			vm_token token(vm_token::EMPTY);
			if (isText(p)) {
				const char *identifier = p;
				while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
					p++;
				token = vm_token(vm_token::STRING);
				int cmdIdx = find_command(identifier);
				if (cmdIdx != -1) {
					token = vm_token(vm_token::COMMAND, cmdIdx);
				}
				else {
					token.hash = fnv1a(identifier, p - identifier);
				}
			}
			else if (isHex(*p)) {
				const char* before = p - 1;
				if (*before == '$') {
					char *out;
					token = vm_token(vm_token::NUMBER, hex2int(p, &out));
					p = out;
				}
			}
			else if (isDigit(p)) {
				char *out;
				token = vm_token(vm_token::NUMBER, str2f(p, &out));
				p = out;
			}
			else {
				switch (*p) {
				case '(': token = vm_token(vm_token::OPEN_BRACKET); break;
				case ')': token = vm_token(vm_token::CLOSE_BRACKET); break;
				case ' ': case '\t': case '\r': break;
				case '\n': ++line; break;
				case ':': token = vm_token(vm_token::SEPARATOR); break;
				case 'X': token = vm_token(vm_token::X); break;
				case 'Y': token = vm_token(vm_token::Y); break;
				case 'A': token = vm_token(vm_token::ACCUMULATOR); break;
				case '#': token = vm_token(vm_token::HASHTAG); break;
				case ',': token = vm_token(vm_token::COMMA); break;
				}
				++p;
			}
			if (token.type != vm_token::EMPTY) {
				token.line = line;
				_tokens.push_back(token);
			}
		}
		return true;
	}

	size_t num() const {
		return _tokens.size();
	}

	const vm_token& get(size_t index) const {
		return _tokens[index];
	}

private:
	bool Tokenizer::isText(const char* p) {
		const char* prev = p - 1;
		if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
			if (prev >= _text && *prev == '$') {
				return false;
			}
			if (prev >= _text && *prev == ',') {
				return false;
			}
			return true;
		}
		return false;
	}

	std::vector<vm_token> _tokens;
	const char* _text;
	bool _created;
};

PRIVATE const char* translate_token_tpye(const vm_token& t) {
	switch (t.type) {
		case vm_token::EMPTY: return "EMPTY"; break;
		case vm_token::NUMBER: return "NUMBER"; break;
		case vm_token::STRING: return "STRING"; break;
		case vm_token::DOLLAR: return "DOLLAR"; break;
		case vm_token::HASHTAG: return "HASHTAG"; break;
		case vm_token::OPEN_BRACKET: return "OPEN_BRACKET"; break;
		case vm_token::CLOSE_BRACKET: return "CLOSE_BRACKET"; break;
		case vm_token::COMMA: return "COMMA"; break;
		case vm_token::SEPARATOR: return "SEPARATOR"; break;
		case vm_token::X: return "X"; break;
		case vm_token::Y: return "Y"; break;
		case vm_token::COMMAND: return "COMMAND"; break;
		case vm_token::ACCUMULATOR: return "ACCUMULATOR"; break;
		default: return "UNKNOWN";
	}
	return nullptr;
}

// -----------------------------------------------------------------
// get hex value from command token
// -----------------------------------------------------------------
PRIVATE uint8_t get_hex_value(const vm_token& token, vm_addressing_mode mode) {
	int i = 0;
	vm_command_mapping m = VM_COMMAND_MAPPING[i];
	while (m.op_code != EOL) {
		if (m.op_code == token.value && m.mode == mode) {
			return m.hex;
		}
		++i;
		m = VM_COMMAND_MAPPING[i];
	}
	printf("Error: Cannot find HEX value for %d at line %d\n", token.value, token.line);
	return 0xEA;
}

// -----------------------------------------------------------------
// get command mapping
// -----------------------------------------------------------------
PRIVATE vm_command_mapping get_command_mapping(uint8_t hex) {
	int i = 0;
	vm_command_mapping m = VM_COMMAND_MAPPING[i];
	while (m.op_code != EOL) {
		if (m.hex == hex) {
			return m;
		}
		++i;
		m = VM_COMMAND_MAPPING[i];
	}
	return NO_OP;
}

// ------------------------------------------
// translate addressing mode
// ------------------------------------------
PRIVATE const char* translate_addressing_mode(vm_addressing_mode mode) {
	switch (mode) {
		case NONE: return "NONE"; break;
		case IMMEDIDATE: return "IMMEDIDATE"; break;
		case ABSOLUTE_ADR: return "ABSOLUTE"; break;
		case ABSOLUTE_X: return "ABSOLUTE_X"; break;
		case ABSOLUTE_Y: return "ABSOLUTE_Y"; break;
		case ZERO_PAGE: return "ZERO_PAGE"; break;
		case ZERO_PAGE_X: return "ZERO_PAGE_X"; break;
		case ZERO_PAGE_Y: return "ZERO_PAGE_Y"; break;
		case INDIRECT_X: return "INDIRECT_X"; break;
		case INDIRECT_Y: return "INDIRECT_Y"; break;
		case RELATIVE_ADR: return "RELATIVE"; break;
		case JMP_ABSOLUTE: return "JMP_ABSOLUTE"; break;
		case JMP_INDIRECT: return "JMP_INDIRECT"; break;
		case ACCUMULATOR: return "ACCUMULATOR"; break;
		default: return "UNKNOWN"; break;
	}
}

// -----------------------------------------------------------------
// get addressing mode 
// -----------------------------------------------------------------
PRIVATE vm_addressing_mode get_addressing_mode(const Tokenizer & tokenizer, int pos) {
	const vm_token& command = tokenizer.get(pos);
	if (command.type == vm_token::COMMAND) {
		const vm_token& next = tokenizer.get(pos + 1);
		if (next.type == vm_token::HASHTAG) {
			return vm_addressing_mode::IMMEDIDATE;
		}
		else if (next.type == vm_token::NUMBER) {
			int v = next.value;
			if (pos + 2 < tokenizer.num()) {
				const vm_token& nn = tokenizer.get(pos + 2);
				if (nn.type == vm_token::COMMA) {
					const vm_token& nn = tokenizer.get(pos + 3);
					if (nn.type == vm_token::X) {
						if (v <= 255) {
							return vm_addressing_mode::ZERO_PAGE_X;
						}
						return vm_addressing_mode::ABSOLUTE_X;
					}
					else {
						if (v <= 255) {
							return vm_addressing_mode::ZERO_PAGE_Y;
						}
						return vm_addressing_mode::ABSOLUTE_Y;
					}
				}
			}
			if (v <= 255) {
				return vm_addressing_mode::ZERO_PAGE;
			}
			return vm_addressing_mode::ABSOLUTE_ADR;
		}
		else if (next.type == vm_token::OPEN_BRACKET) {

		}
		else if (next.type == vm_token::ACCUMULATOR) {
			return vm_addressing_mode::ACCUMULATOR;
		}
		else if (next.type == vm_token::STRING) {
			if (command.value == vm_opcode::JMP || command.value == vm_opcode::JSR) {
				return vm_addressing_mode::JMP_ABSOLUTE;
			}
			return vm_addressing_mode::RELATIVE_ADR;
		}
	}
	return vm_addressing_mode::NONE;
}

// -----------------------------------------------------------------
// disassemble memory
// -----------------------------------------------------------------
void vm_disassemble(vm_context* ctx) {
	int pc = 0x600;
	int end = pc + ctx->numBytes;
	while (pc < end) {
		uint8_t hex = ctx->read(pc);
		const vm_command_mapping& mapping = get_command_mapping(hex);
		const vm_command& cmd = VM_COMMANDS[mapping.op_code];
		printf("%04X ",pc);
		printf("%s ", cmd.name);
		switch (mapping.mode) {
			case IMMEDIDATE: printf("#$%02X",ctx->read(pc+1)); break;
			case ABSOLUTE_ADR : printf("$%04X",ctx->readInt(pc+1)); break;
			case ABSOLUTE_X: printf("$%04X,X", ctx->readInt(pc + 1)); break;
			case ABSOLUTE_Y: printf("$%04X,Y", ctx->readInt(pc + 1)); break;
			case ZERO_PAGE: printf("$%02X", ctx->read(pc + 1)); break;
			case ZERO_PAGE_X: printf("$%02X,X", ctx->read(pc + 1)); break;
			case ZERO_PAGE_Y: printf("$%02X,Y", ctx->read(pc + 1)); break;
			case INDIRECT_X: printf("$(%04X),X", ctx->readInt(pc + 1)); break;
			case INDIRECT_Y: printf("$(%04X),Y", ctx->readInt(pc + 1)); break;
			case RELATIVE_ADR: printf("$%02X", ctx->read(pc + 1)); break;
			case ACCUMULATOR: printf("A"); break;
		}
		printf("\n");
		pc += VM_DATA_SIZE[mapping.mode] + 1;
	}
}
	
// -----------------------------------------------------------------
// label definition
// -----------------------------------------------------------------
typedef struct vm_label_definition {
	int pc;
	uint32_t hash;
	int op_code;
} vm_label_definition;

// -----------------------------------------------------------------
// convert tokens 
// -----------------------------------------------------------------
PRIVATE int assemble(const Tokenizer & tokenizer, vm_context* ctx, int* numCommands) {
	printf("tokens: %d\n", tokenizer.num());
	int pc = 0x600;
	std::vector<vm_label_definition> definitions;
	std::vector<vm_label_definition> branches;
	for (size_t i = 0; i < tokenizer.num(); ++i) {
		const vm_token& t = tokenizer.get(i);
		printf("%d = %s (line: %d)\n", i, translate_token_tpye(t), t.line);
		if (t.type == vm_token::COMMAND) {
			if (numCommands != nullptr) {
				++*numCommands;
			}
			const vm_command& cmd = VM_COMMANDS[t.value];
			vm_addressing_mode mode = vm_addressing_mode::NONE;
			if (cmd.supportedModes != 0) {
				mode = get_addressing_mode(tokenizer, i);
			}
			uint8_t hex = get_hex_value(t, mode);
			printf("=> index: %d  mode: %s cmd: %s (%X)\n", t.value, translate_addressing_mode(mode), cmd.name, hex);
			ctx->write(pc++, hex);
			if (mode == vm_addressing_mode::IMMEDIDATE) {
				const vm_token& next = tokenizer.get(i + 2);
				ctx->write(pc++, next.value);
			}
			else if (mode == vm_addressing_mode::ABSOLUTE_ADR || mode == vm_addressing_mode::ABSOLUTE_X || mode == vm_addressing_mode::ABSOLUTE_Y) {
				const vm_token& next = tokenizer.get(i + 1);
				ctx->write(pc++, low_value(next.value));
				ctx->write(pc++, high_value(next.value));
			}
			else if (mode == vm_addressing_mode::ZERO_PAGE || mode == vm_addressing_mode::ZERO_PAGE_X || mode == vm_addressing_mode::ZERO_PAGE_Y) {
				const vm_token& next = tokenizer.get(i + 1);
				ctx->write(pc++, low_value(next.value));
			}
			else if (mode == vm_addressing_mode::RELATIVE_ADR) {
				const vm_token& next = tokenizer.get(i + 1);
				vm_label_definition def;
				def.hash = next.hash;
				def.pc = pc;
				def.op_code = t.value;
				branches.push_back(def);
				ctx->write(pc++, 0);
			}
			else if (mode == vm_addressing_mode::JMP_ABSOLUTE || mode == vm_addressing_mode::JMP_INDIRECT) {
				const vm_token& next = tokenizer.get(i + 1);
				vm_label_definition def;
				def.hash = next.hash;
				def.pc = pc;
				def.op_code = t.value;
				branches.push_back(def);
				ctx->write(pc++, 0);
				ctx->write(pc++, 0);
			}
		}
		else if (t.type == vm_token::STRING) {
			const vm_token& next = tokenizer.get(i + 1);
			if (next.type == vm_token::SEPARATOR) {
				vm_label_definition def;
				def.hash = t.hash;
				def.pc = pc;
				def.op_code = t.value;
				definitions.push_back(def);
			}
		}
	}
	for (size_t i = 0; i < branches.size();++i) {
		const vm_label_definition& branch = branches[i];
		for (size_t j = 0; j < definitions.size(); ++j) {
			if (definitions[j].hash == branch.hash) {
				int diff = definitions[j].pc - branch.pc;
				if (branch.op_code == vm_opcode::JMP || branch.op_code == vm_opcode::JSR) {
					int v = definitions[j].pc;
					ctx->write(branch.pc,low_value(v));
					ctx->write(branch.pc + 1, high_value(v));
				}
				else {
					if (diff < 0) {
						diff = 255 + diff;
					}
					
					ctx->write(branch.pc, diff);
				}
			}
		}
	}
	return pc - 0x600;
}

// ---------------------------------------------------------
//  assemble file
// ---------------------------------------------------------
int vm_assemble_file(vm_context* ctx, const char* fileName) {
	Tokenizer tokenizer;
	if (tokenizer.parseFile(fileName)) {
		ctx->numBytes = assemble(tokenizer, ctx, &ctx->numCommands);
		//save(buffer);
		vm_memory_dump(ctx, 0x600, ctx->numBytes);
		return ctx->numBytes;
	}
	else {
		printf("ERROR: cannot laod file: '%s'\n", fileName);
	}
	return 0;
}

// ---------------------------------------------------------
//  assemble 
// ---------------------------------------------------------
int vm_assemble(vm_context* ctx, const char* text) {
	Tokenizer tokenizer;
	if (tokenizer.parse(text)) {
		ctx->numBytes = assemble(tokenizer, ctx, &ctx->numCommands);
		//save(buffer);
		vm_memory_dump(ctx, 0x600, ctx->numBytes);
		return ctx->numBytes;
	}
	return 0;
}

// -------------------------------------------------------- -
//  dump registers and memory
// ---------------------------------------------------------
void vm_dump(vm_context* ctx, int pc, int num) {
	vm_dump_registers(ctx);
	vm_memory_dump(ctx, pc, num);
}

// ---------------------------------------------------------
//  dump registers 
// ---------------------------------------------------------
void vm_dump_registers(vm_context* ctx) {
	printf("------------- Dump -------------\n");
	printf("A=$%02X ", ctx->registers[0]);
	printf("X=$%02X ", ctx->registers[1]);
	printf("Y=$%02X\n", ctx->registers[2]);
	printf("PC=$%04X ", ctx->programCounter);
	printf("SP=$%02X\n", ctx->sp);
	printf("CZIDBVN\n");
	for (int i = 1; i < 8; ++i) {
		if (ctx->isSet(i)) {
			printf("1");
		}
		else {
			printf("0");
		}
	}
	printf("\n");
}

// ---------------------------------------------------------
//  memory dump
// ---------------------------------------------------------
void vm_memory_dump(vm_context* ctx, int pc, int num) {
	printf("---------- Memory dump -----------");
	for (size_t i = 0; i < num; ++i) {
		if (i % 8 == 0) {
			printf("\n%04X : ", (pc + i));
		}
		printf("%02X ", ctx->read(pc + i));
	}
	printf("\n");
}

PRIVATE int get_data(vm_context* ctx, const vm_addressing_mode& mode) {
	int data = 0;
	if (mode == IMMEDIDATE) {
		data = ctx->read(ctx->programCounter + 1);
	}
	else if (mode == ABSOLUTE_ADR) {
		uint8_t upper = ctx->read(ctx->programCounter + 2);
		data = ctx->read(ctx->programCounter + 1) + (upper << 8);
	}
	else if (mode == ABSOLUTE_X) {
		uint8_t upper = ctx->read(ctx->programCounter + 2);
		data = ctx->read(ctx->programCounter + 1) + (upper << 8) + ctx->registers[1];
	}
	else if (mode == ABSOLUTE_Y) {
		uint8_t upper = ctx->read(ctx->programCounter + 2);
		data = ctx->read(ctx->programCounter + 1) + (upper << 8) + ctx->registers[2];
	}
	else if (mode == ZERO_PAGE) {
		data = ctx->read(ctx->programCounter + 1);
	}
	else if (mode == ZERO_PAGE_X) {
		data = ctx->read(ctx->programCounter + 1) + ctx->registers[1];
	}
	else if (mode == ZERO_PAGE_Y) {
		data = ctx->read(ctx->programCounter + 1) + ctx->registers[2];
	}
	else if (mode == RELATIVE_ADR) {
		data = ctx->read(ctx->programCounter + 1);
	}
	else if (mode == JMP_ABSOLUTE) {
		data = ctx->readInt(ctx->programCounter + 1);
	}
	else if (mode == JMP_INDIRECT) {
		data = ctx->readInt(ctx->readInt(ctx->programCounter + 1));
	}
	else if (mode == ACCUMULATOR) {
		data = -1;
	}
	return data;
}

// ---------------------------------------------------------
//  internal execute single step
// ---------------------------------------------------------
PRIVATE bool vm_step(vm_context* ctx) {
	uint8_t cmdIdx = ctx->read(ctx->programCounter);
	const vm_command_mapping& mapping = get_command_mapping(cmdIdx);
	const vm_command& cmd = VM_COMMANDS[mapping.op_code];
	vm_addressing_mode mode = mapping.mode;
	int data = get_data(ctx, mode);
	int add = VM_DATA_SIZE[mode] + 1;
	(*cmd.function)(ctx, data);
	printf("%04X %s (%02X) data: %04X add: %d\n", ctx->programCounter, cmd.name, cmdIdx, data, add);
	if (!cmd.modifyPC) {
		ctx->programCounter += add;
	}
	if (mapping.op_code == BRK) {
		return false;
	}
	return true;
}

// ---------------------------------------------------------
//  run program
// ---------------------------------------------------------
void vm_run(vm_context* ctx) {
	ctx->programCounter = 0x600;
	int end = ctx->programCounter + ctx->numBytes;
	bool running = true;
	while (running) {
		running = vm_step(ctx);		
		if (ctx->programCounter >= end) {
			running = false;
		}
	}
}

// ---------------------------------------------------------
//  clear context
// ---------------------------------------------------------
void vm_clear_context(vm_context* ctx) {
	for (int i = 0; i < 65536; ++i) {
		ctx->mem[i] = 0;
	}
	ctx->registers[0] = 0;
	ctx->registers[1] = 0;
	ctx->registers[2] = 0;
	for (int i = 0; i < 7; ++i) {
		ctx->clearFlag(i);
	}
	ctx->numCommands = 0;
	ctx->numBytes = 0;
	ctx->programCounter = 0x600;
	ctx->sp = 255;
}

// ---------------------------------------------------------
//  load binary file
// ---------------------------------------------------------
bool vm_load(vm_context* ctx, const char* fileName) {
	int pc = 0x600;
	FILE* fp = fopen(fileName, "rb");
	if (fp) {
		fread(&ctx->numBytes, sizeof(int), 1, fp);
		fread(&ctx->numCommands, sizeof(int), 1, fp);
		for (int i = 0; i < ctx->numBytes; ++i) {
			uint8_t v;
			fread(&v, sizeof(uint8_t), 1, fp);
			ctx->write(pc + i, v);
		}
		fclose(fp);
		printf("Loaded bytes: %d commands: %d\n", ctx->numBytes,ctx->numCommands);
		return true;
	}
	printf("file '%s' not found", fileName);
	return false;
}

// ---------------------------------------------------------
//  save binary file
// ---------------------------------------------------------
void vm_save(vm_context* ctx, const char* fileName) {
	FILE* fp = fopen(fileName, "wb");
	if (fp) {
		int pc = 0x600;
		fwrite(&ctx->numBytes, sizeof(int), 1, fp);
		fwrite(&ctx->numCommands, sizeof(int), 1, fp);
		for (int i = 0; i < ctx->numBytes; ++i) {
			uint8_t v = ctx->read(pc + i);
			fwrite(&v, sizeof(uint8_t), 1, fp);
		}
		fclose(fp);
	}
}

#endif