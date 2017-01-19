#include <iostream>
#include <map>
#define VM_IMPLEMENTATION
#include "..\6502.h"
#include "TextLine.h"

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
	TOK_SET_PC,
	TOK_HELP
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
	ShellCommand() {}
	virtual ~ShellCommand() {}
	virtual void execute(const TextLine& line) = 0;
	virtual void write_syntax() = 0;
	virtual int num_params() = 0;
	virtual CommandType get_token_type() const = 0;
	virtual const char* get_command() const = 0;
};

// ------------------------------------------------------
// Assemble
// ------------------------------------------------------
class ShellAssemble : public ShellCommand {

public:
	ShellAssemble() {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_assemble_file(buffer);
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
	ShellDisassemble() {}
	void execute(const TextLine& line) {
		std::string code;
		vm_disassemble(code);
		printf("%s\n", code.c_str());
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
	ShellSave() {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_save(buffer);
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
	ShellLoad() {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		vm_load(buffer);
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
	ShellDumpMemory() {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		int pc = hex2int(buffer);
		vm_memory_dump(pc, 128);
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
	ShellSetProgramCounter() {}
	void execute(const TextLine& line) {
		char buffer[128];
		line.get_string(1, buffer);
		int pc = hex2int(buffer);
		//_ctx->programCounter = pc;
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
	ShellRun() {}
	void execute(const TextLine& line) {
		vm_run();
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
	ShellStep() {}
	void execute(const TextLine& line) {
		vm_step();
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
	ShellQuit() {}
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

// ------------------------------------------------------
// Help
// ------------------------------------------------------
class ShellHelp : public ShellCommand {

public:
	ShellHelp() {}
	void execute(const TextLine& line) {
		// nothing to do here
	}
	void write_syntax() {
		printf("help - list all commands and their syntax\n");
	}
	CommandType get_token_type() const {
		return TOK_HELP;
	}
	const char* get_command() const {
		return "help";
	}
	int num_params() {
		return 0;
	}
};
/*
TOK_DUMP_REGISTERS,
TOK_DISASSEMBLE,
*/
class Shell {

	typedef std::map<CommandType, ShellCommand*> Commands;

public:
	Shell() {
		_ctx = vm_create();
		_commands[TOK_QUIT] = new ShellQuit();
		_commands[TOK_ASSEMBLE] = new ShellAssemble();
		_commands[TOK_SAVE] = new ShellSave();
		_commands[TOK_LOAD] = new ShellLoad();
		_commands[TOK_DUMP_MEMORY] = new ShellDumpMemory();
		_commands[TOK_DISASSEMBLE] = new ShellDisassemble();
		_commands[TOK_RUN] = new ShellRun();
		_commands[TOK_SET_PC] = new ShellSetProgramCounter();
		_commands[TOK_HELP] = new ShellHelp();
	}

	~Shell() {
		vm_release();
	}

	bool extract(const char* p, CommandLine * command_line) {
		command_line->line.set(p, ' ');
		command_line->type = TOK_UNKNOWN;
		if (command_line->line.num_tokens() > 0) {
			char buffer[256];
			int idx = -1;
			int len = command_line->line.get_string(0, buffer);
			Commands::iterator it = _commands.begin();
			int cnt = 0;
			while (it != _commands.end()) {
				if (strncmp(it->second->get_command(), buffer, len) == 0 && strlen(it->second->get_command()) == len) {
					command_line->type = it->second->get_token_type();
					return true;
				}
				++cnt;
				++it;
			}
		}
		return false;
	}

	bool execute(CommandType type, const TextLine& line) {
		ShellCommand* cmd = _commands[type];
		if (type == TOK_HELP) {
			Commands::iterator it = _commands.begin();
			while (it != _commands.end()) {
				it->second->write_syntax();
				++it;
			}
		}
		else {
			cmd->execute(line);
		}
		return true;
	}
private:
	Commands _commands;
	vm_context* _ctx;
};

int main(int argc, char* argv[]) {
	bool running = true;
	char buffer[256];
	CommandLine line;
	Shell shell;
	std::cout << "6502 - Shell\n";
	while (running) {
		std::cout << ":> ";
		std::cin.getline(buffer, 256);
		if (shell.extract(buffer, &line)) {
			if (line.type == TOK_QUIT) {
				running = false;
			}
			else {
				shell.execute(line.type, line.line);
			}
		}
		else {
			std::cout << "UNKNOWN COMMAND: '" << buffer << "'\n";
		}
	}
	return 0;
}