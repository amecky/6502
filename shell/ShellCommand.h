#pragma once
#include "TextLine.h"
#define VM_IMPLEMENTATION
#include "..\6502.h"

int hex2int(const char *hex) {
	int val = 0;
	while ((*hex >= '0' && *hex <= '9') || (*hex >= 'a' && *hex <= 'f') || (*hex >= 'A' && *hex <= 'F')) {
		// get current character then increment
		char byte = *hex++;
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
		// shift 4 to make space for new digit, and add the 4 bits of the new digit 
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

enum CommandType {
	TOK_UNKNOWN,
	TOK_QUIT,
	TOK_DUMP_REGISTERS,
	TOK_DUMP_MEMORY,
	TOK_ASSEMBLE,
	TOK_DISASSEMBLE,
	TOK_RUN,
	TOK_STEP,
	TOK_LOAD,
	TOK_SAVE,
	TOK_SET_PC
};

struct CommandLine {

	CommandType type;
	TextLine line;

};
// ------------------------------------------------------
// virtual base command
// ------------------------------------------------------
class ShellCommand {

public:
	ShellCommand(vm_context* ctx) : _ctx(ctx) {}
	virtual ~ShellCommand() {}
	virtual void execute(const TextLine& line) = 0;
	virtual void write_syntax() = 0;
	virtual int num_params() = 0;
	virtual CommandType get_token_type() const = 0;
	virtual const char* get_command() const = 0;
protected:
	vm_context* _ctx;
};

// ------------------------------------------------------
// Assemble
// ------------------------------------------------------
class ShellAssemble : public ShellCommand {

public:
	ShellAssemble(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_assemble_file(_ctx, buffer);
	}
	void write_syntax() {
		printf("asm - assemble file\n");
	}
	CommandType get_token_type() const {
		return TOK_ASSEMBLE;
	}
	const char* get_command() const {
		return "asm";
	}
	int num_params() {
		return 1;
	}
};

// ------------------------------------------------------
// Disassemble
// ------------------------------------------------------
class ShellDisassemble : public ShellCommand {

public:
	ShellDisassemble(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		vm_disassemble(_ctx);
	}
	void write_syntax() {
		printf("dsm - disassemble memory\n");
	}
	CommandType get_token_type() const {
		return TOK_DISASSEMBLE;
	}
	const char* get_command() const {
		return "dsm";
	}
	int num_params() {
		return 0;
	}
};

// ------------------------------------------------------
// Save
// ------------------------------------------------------
class ShellSave : public ShellCommand {

public:
	ShellSave(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_save(_ctx,buffer);
	}
	void write_syntax() {
		printf("save - saves memory\n");
	}
	CommandType get_token_type() const {
		return TOK_SAVE;
	}
	const char* get_command() const {
		return "save";
	}
	int num_params() {
		return 1;
	}
};

// ------------------------------------------------------
// Load
// ------------------------------------------------------
class ShellLoad : public ShellCommand {

public:
	ShellLoad(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_load(_ctx, buffer);
	}
	void write_syntax() {
		printf("load - load prg file\n");
	}
	CommandType get_token_type() const {
		return TOK_LOAD;
	}
	const char* get_command() const {
		return "load";
	}
	int num_params() {
		return 1;
	}
};

// ------------------------------------------------------
// Dump memory
// ------------------------------------------------------
class ShellDumpMemory : public ShellCommand {

public:
	ShellDumpMemory(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		int pc = hex2int(buffer);
		vm_memory_dump(_ctx, pc, 128);
	}
	void write_syntax() {
		printf("dump - dump memory\n");
	}
	CommandType get_token_type() const {
		return TOK_DUMP_MEMORY;
	}
	const char* get_command() const {
		return "dump";
	}
	int num_params() {
		return 1;
	}
};

// ------------------------------------------------------
// Set PC
// ------------------------------------------------------
class ShellSetProgramCounter : public ShellCommand {

public:
	ShellSetProgramCounter(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		int pc = hex2int(buffer);
		_ctx->programCounter = pc;
	}
	void write_syntax() {
		printf("pc - set program counter {adr}\n");
	}
	CommandType get_token_type() const {
		return TOK_SET_PC;
	}
	const char* get_command() const {
		return "set";
	}
	int num_params() {
		return 1;
	}
};

// ------------------------------------------------------
// Run
// ------------------------------------------------------
class ShellRun : public ShellCommand {

public:
	ShellRun(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		vm_run(_ctx);
	}
	void write_syntax() {
		printf("run\n");
	}
	CommandType get_token_type() const {
		return TOK_RUN;
	}
	const char* get_command() const {
		return "run";
	}
	int num_params() {
		return 0;
	}
};

// ------------------------------------------------------
// Run
// ------------------------------------------------------
class ShellStep : public ShellCommand {

public:
	ShellStep(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		vm_step(_ctx);
	}
	void write_syntax() {
		printf("step\n");
	}
	CommandType get_token_type() const {
		return TOK_STEP;
	}
	const char* get_command() const {
		return "step";
	}
	int num_params() {
		return 0;
	}
};

// ------------------------------------------------------
// Quit
// ------------------------------------------------------
class ShellQuit : public ShellCommand {

public:
	ShellQuit(vm_context* ctx) : ShellCommand(ctx) {}
	void execute(const TextLine& line) {
		// nothing to do here
	}
	void write_syntax() {
		printf("quit - quits the game\n");
	}
	CommandType get_token_type() const {
		return TOK_QUIT;
	}
	const char* get_command() const {
		return "quit";
	}
	int num_params() {
		return 0;
	}
};