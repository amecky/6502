#include <iostream>
#include <map>
#include "ShellCommand.h"
/*
TOK_DUMP_REGISTERS,
TOK_DISASSEMBLE,
TOK_RUN,
TOK_STEP,
*/
class Shell {

	typedef std::map<CommandType, ShellCommand*> Commands;

public:
	Shell() {
		_commands[TOK_QUIT] = new ShellQuit(&_machine);
		_commands[TOK_ASSEMBLE] = new ShellAssemble(&_machine);
		_commands[TOK_SAVE] = new ShellSave(&_machine);
		_commands[TOK_LOAD] = new ShellLoad(&_machine);
		_commands[TOK_DUMP_MEMORY] = new ShellDumpMemory(&_machine);
		_commands[TOK_DISASSEMBLE] = new ShellDisassemble(&_machine);
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
		cmd->execute(line);
		return true;
	}
private:
	Commands _commands;
	vm::VirtualMachine _machine;
};

int main(int argc, char* argv[]) {
	bool running = true;
	char buffer[256];
	CommandLine line;
	Shell shell;
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
			printf("UNKNOWN COMMAND: '%s'\n",buffer);
		}
	}
	return 0;
}