#pragma once
#include <stdio.h>
#include <vector>

namespace vm {

	// -----------------------------------------------------
	// AddressingMode
	//
	// Enum of all supported addressing modes
	// -----------------------------------------------------
	enum AddressingMode {
		NONE,
		IMMEDIDATE,
		ABSOLUTE_ADR,
		ABSOLUTE_X,
		ABSOLUTE_Y,
		ZERO_PAGE,
		ZERO_PAGE_X,
		ZERO_PAGE_Y,
		INDIRECT_X,
		INDIRECT_Y,
		RELATIVE_ADR
	};

	// -----------------------------------------------------
	// Flags
	// -----------------------------------------------------
	enum Flags {
		C, // Carry Flag
		Z, // Zero Flag
		I,
		D,
		B,
		V,
		N
	};

	// -----------------------------------------------------
	// The virtual machine context
	// -----------------------------------------------------
	struct Context {
		int registers[3];
		int pc;
		uint8_t mem[65536];
		bool flags[7];
		uint8_t sp;
		uint8_t cpuFlags;

		void clearFlags() {
			for (int i = 0; i < 7; ++i) {
				flags[i] = false;
			}
		}

		void setFlag(int idx) {
			flags[idx] = true;
		}

		void clearFlag(int idx) {
			flags[idx] = false;
		}

		bool isSet(int idx) const {
			return flags[idx];
		}
		void write(int idx, uint8_t v) {
			mem[idx] = v;
		}
		uint8_t read(int idx) const {
			return mem[idx];
		}

		void push(uint8_t v) {
			mem[0x100 + sp] = v;
			++sp;
		}

		uint8_t pop() {
			uint8_t v = mem[0x100 + sp];
			--sp;
			return v;
		}
	};
	// -----------------------------------------------------
	// type definition of command function pointer
	// -----------------------------------------------------
	typedef int(*commandFunc)(Context* ctx, int data);

	// -----------------------------------------------------
	// Command
	// -----------------------------------------------------
	struct Command {
		const char* name;
		commandFunc function;
		int supportedModes;

		bool isSupported(AddressingMode mode) {
			return (supportedModes & mode) == mode;
		}
	};

	// -----------------------------------------------------
	// Command function definitions
	// -----------------------------------------------------
	static int nop(Context* ctx, int pc) {
		printf("calling NOP\n");
		return pc + 1;
	}

	// ------------------------------------------
	// LDA
	// ------------------------------------------
	static int lda(Context* ctx, int data) {
		printf("=> LDA - data: %d\n", data);
		ctx->registers[0] = data;
		if (data == 0) {
			ctx->setFlag(Flags::Z);
		}
		if (data > 127) {
			ctx->setFlag(Flags::N);
		}
		return 0;
	}

	// ------------------------------------------
	// LDX
	// ------------------------------------------
	static int ldx(Context* ctx, int data) {
		printf("=> LDX - data: %d\n", data);
		ctx->registers[1] = data;
		return 0;
	}

	// ------------------------------------------
	// STX
	// ------------------------------------------
	static int stx(Context* ctx, int data) {
		printf("=> STX - data: %d\n", data);
		ctx->write(data, ctx->registers[1]);
		return 0;
	}

	// ------------------------------------------
	// STA
	// ------------------------------------------
	static int sta(Context* ctx, int data) {
		printf("=> STA data: %d\n", data);
		ctx->write(data, ctx->registers[0]);
		return 0;
	}

	// ------------------------------------------
	// TAX
	// ------------------------------------------
	static int tax(Context* ctx, int data) {
		printf("=> TAX data: %d\n", data);
		ctx->registers[1] = ctx->registers[0];
		return 0;
	}

	// ------------------------------------------
	// INX
	// ------------------------------------------
	static int inx(Context* ctx, int data) {
		printf("=> INX data: %d\n", data);
		ctx->registers[1] += 1;
		return 0;
	}

	// ------------------------------------------
	// ADC
	// ------------------------------------------
	static int adc(Context* ctx, int data) {
		printf("=> ADC data: %d\n", data);
		ctx->registers[0] += data;
		if (ctx->registers[0] == 0) {
			ctx->setFlag(Flags::Z);
		}
		else {
			ctx->clearFlag(Flags::Z);
		}
		return 0;
	}

	static int decrement(Context* ctx, int pc, int idx) {
		--ctx->registers[idx];
		if (ctx->registers[idx] < 0) {
			ctx->registers[idx] = 255;
		}
		if (ctx->registers[idx] == 0) {
			ctx->setFlag(Flags::Z);
		}
		if ((ctx->registers[idx] & 128) == 128) {
			ctx->setFlag(Flags::N);
		}
		else {
			ctx->clearFlag(Flags::N);
		}
		return 0;
	}

	// ------------------------------------------
	// DEX
	// ------------------------------------------
	static int dex(Context* ctx, int data) {
		printf("=> DEX\n");
		return decrement(ctx, data, 1);
	}

	// ------------------------------------------
	// DEY
	// ------------------------------------------
	static int dey(Context* ctx, int pc) {
		printf("=> DEY\n");
		return decrement(ctx, pc, 2);
	}

	// ------------------------------------------
	// DEC
	// ------------------------------------------
	static int dec(Context* ctx, int pc) {
		printf("=> DEC\n");
		return decrement(ctx, pc, 0);
	}

	static int brk(Context* ctx, int pc) {
		printf("calling brk\n");
		return -1;
	}


	// -----------------------------------------------------
	// Array of all supported commands with function pointer
	// and a bitset of supported addressing modes
	// -----------------------------------------------------
	const static Command COMMANDS[] = {
		{ "ADC", &adc, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
		{ "AND", &nop, 0 },
		{ "ASL", &nop, 0 },
		{ "BCC", &nop, 0 },
		{ "BCS", &nop, 0 },
		{ "BEQ", &nop, 0 },
		{ "BIT", &nop, 0 },
		{ "BMI", &nop, 0 },
		{ "BNE", &nop, 0 },
		{ "BPL", &nop, 0 },
		{ "BRK", &brk, 0 },
		{ "BVC", &nop, 0 },
		{ "BVS", &nop, 0 },
		{ "CLC", &nop, 0 },
		{ "CLD", &nop, 0 },
		{ "CLI", &nop, 0 },
		{ "CLV", &nop, 0 },
		{ "CMP", &nop, 0 },
		{ "CPX", &nop, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ABSOLUTE_ADR },
		{ "CPY", &nop, 0 },
		{ "DEC", &dec, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
		{ "DEX", &dex, 0 },
		{ "DEY", &dey, 0 },
		{ "EOR", &nop, 0 },
		{ "INC", &nop, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
		{ "INX", &inx, 0 },
		{ "INY", &nop, 0 },
		{ "JMP", &nop, 0 },
		{ "JSR", &nop, 0 },
		{ "LDA", &lda, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
		{ "LDX", &ldx, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_Y },
		{ "LDY", &nop, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
		{ "LSR", &nop, 0 },
		{ "NOP", &nop, 0 },
		{ "ORA", &nop, 0 },
		{ "PHA", &nop, 0 },
		{ "PHP", &nop, 0 },
		{ "PLA", &nop, 0 },
		{ "PLP", &nop, 0 },
		{ "ROL", &nop, 0 },
		{ "ROR", &nop, 0 },
		{ "RTI", &nop, 0 },
		{ "RTS", &nop, 0 },
		{ "SBC", &nop, 0 },
		{ "SEC", &nop, 0 },
		{ "SED", &nop, 0 },
		{ "SEI", &nop, 0 },
		{ "STA", &sta, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
		{ "STX", &stx, 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR },
		{ "STY", &nop, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR },
		{ "TAX", &tax, 0 },
		{ "TAY", &nop, 0 },
		{ "TSX", &nop, 0 },
		{ "TXA", &nop, 0 },
		{ "TXS", &nop, 0 },
		{ "TYA", &nop, 0 }
	};

	const static int NUM_COMMANDS = 56;

	static int find_command(const char* text) {
		for (int i = 0; i < NUM_COMMANDS; ++i) {
			if (text[0] == COMMANDS[i].name[0]) {
				if (text[1] == COMMANDS[i].name[1] && text[2] == COMMANDS[i].name[2]) {
					return i;
				}
			}
		}
		return -1;
	}

	static Command get_command(int idx) {
		return COMMANDS[idx];
	}

	// -----------------------------------------------------
	// The number of bytes of data for every addressing
	// mode
	// -----------------------------------------------------
	const static int DATA_SIZE[] = {
		0, 1, 2, 3, 3, 1, 2, 2, 4, 4, 1
	};

	

	// -----------------------------------------------------
	// Command mapping
	// -----------------------------------------------------
	struct CommandMapping {
		int cmd;
		AddressingMode mode;
		uint8_t hex;
	};

	const static CommandMapping NO_OP = CommandMapping{ 100,NONE,0xEA };

	// -----------------------------------------------------
	// Array of all comand mappings
	// -----------------------------------------------------
	const static CommandMapping COMMAND_MAPPING[] = {
		// ADC
		{ 0, IMMEDIDATE,   0x69 },
		{ 0, ZERO_PAGE,    0x65 },
		{ 0, ZERO_PAGE_X,  0x75 },
		{ 0, ABSOLUTE_ADR,     0x6D },
		{ 0, ABSOLUTE_X,   0x7D },
		{ 0, ABSOLUTE_Y,   0x79 },
		{ 0, INDIRECT_X,   0x61 },
		{ 0, INDIRECT_Y,   0x71 },
		// BRK
		{ 10, NONE,        0x00 },
		// CPX
		{ 18, IMMEDIDATE,  0xE0 },
		{ 18, ZERO_PAGE,   0xE4 },
		{ 18, ABSOLUTE_ADR,    0xEC },
		// DEC
		{ 20, ZERO_PAGE,   0xC6 },
		{ 20, ZERO_PAGE_X, 0xD6 },
		{ 20, ABSOLUTE_ADR,    0xCE },
		{ 20, ABSOLUTE_X,  0xDE },
		// DEX
		{ 21, NONE,        0xCA },
		// DEY
		{ 22, NONE,        0x88 },
		// INC
		{ 24, ZERO_PAGE,   0xE6 },
		{ 24, ZERO_PAGE_X, 0xF6 },
		{ 24, ABSOLUTE_ADR,    0xEE },
		{ 24, ABSOLUTE_X,  0xFE },
		// INX
		{ 25, NONE,        0xE8 },
		// INY
		{ 26, NONE,        0xC8 },
		// LDA
		{ 29, IMMEDIDATE,  0xA9 },
		{ 29, ZERO_PAGE,   0xA5 },
		{ 29, ZERO_PAGE_X, 0xB5 },
		{ 29, ABSOLUTE_ADR,    0xAD },
		{ 29, ABSOLUTE_X,  0xBD },
		{ 29, ABSOLUTE_Y,  0xB9 },
		{ 29, INDIRECT_X,  0xA1 },
		{ 29, INDIRECT_Y,  0xB1 },
		// LDX
		{ 30, IMMEDIDATE,  0xA2 },
		{ 30, ZERO_PAGE,   0xA6 },
		{ 30, ZERO_PAGE_Y, 0xB6 },
		{ 30, ABSOLUTE_ADR,    0xAE },
		{ 30, ABSOLUTE_Y,  0xBE },
		// LDY
		{ 31, IMMEDIDATE,  0xA0 },
		{ 31, ZERO_PAGE,   0xA4 },
		{ 31, ZERO_PAGE_X, 0xB4 },
		{ 31, ABSOLUTE_ADR,    0xAC },
		{ 31, ABSOLUTE_X,  0xBC },
		// PHA
		{ 35, NONE,        0x48 },
		// PLA
		{ 37, NONE,        0x68 },
		// STA
		{ 47, ZERO_PAGE,   0x85 },
		{ 47, ZERO_PAGE_X, 0x95 },
		{ 47, ABSOLUTE_ADR,    0x8D },
		{ 47, ABSOLUTE_X,  0x9D },
		{ 47, ABSOLUTE_Y,  0x99 },
		{ 47, INDIRECT_X,  0x81 },
		{ 47, INDIRECT_Y,  0x91 },
		// STX
		{ 48, ZERO_PAGE,   0x86 },
		{ 48, ZERO_PAGE_Y, 0x96 },
		{ 48, ABSOLUTE_ADR,    0x8E },
		// STY
		{ 49, ZERO_PAGE,   0x84 },
		{ 49, ZERO_PAGE_X, 0x94 },
		{ 49, ABSOLUTE_ADR,    0x8C },
		// TAX
		{ 50, NONE,        0xAA },
		// TAY
		{ 51, NONE,        0xA8 },
		// TSX
		{ 52, NONE,        0xBA },
		// TXA
		{ 53, NONE,        0x8A },
		// TXS
		{ 54, NONE,        0x9A },
		// TYA
		{ 55, NONE,        0x98 },
		// END
		{ 100, NONE,        0xFF },
	};

	// -----------------------------------------------------
	// Token
	// -----------------------------------------------------
	struct Token {

		enum TokenType { EMPTY, NUMBER, STRING, DOLLAR, HASHTAG, OPEN_BRACKET, CLOSE_BRACKET, COMMA, X, Y, SEPARATOR, COMMAND };

		Token(TokenType t) : type(t), value(0), start(0), size(0) {}
		Token(TokenType t, int v) : type(t), value(v), start(0), size(0) {}
		Token(TokenType t, int st, int s) : type(t), value(0), start(st), size(s) {}

		TokenType type;
		int value;
		int start;
		int size;
		int line;
	};

	

	static char* read_file(const char* fileName) {
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

	static bool isDigit(const char* c) {
		if ((*c >= '0' && *c <= '9')) {
			return true;
		}
		return false;
	}

	static bool isNumeric(const char c) {
		return ((c >= '0' && c <= '9'));
	}

	static bool isHex(const char c) {
		return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
	}

	static bool isWhitespace(const char c) {
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
			return true;
		}
		return false;
	}

	static int hex2int(const char *hex, char** endPtr) {
		int val = 0;
		while (isHex(*hex)) {
			// get current character then increment
			char byte = *hex++;
			// transform hex character to the 4bit equivalent number, using the ascii table indexes
			if (byte >= '0' && byte <= '9') byte = byte - '0';
			else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
			else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
			// shift 4 to make space for new digit, and add the 4 bits of the new digit 
			val = (val << 4) | (byte & 0xF);
		}
		if (endPtr) {
			*endPtr = (char *)(hex);
		}
		return val;
	}

	static float strtof(const char* p, char** endPtr) {
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
				Token token(Token::EMPTY);
				if (isText(p)) {
					const char *identifier = p;
					while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
						p++;
					token = Token(Token::STRING, identifier - _text, p - identifier);
					int cmdIdx = find_command(identifier);
					if (cmdIdx != -1) {
						token = Token(Token::COMMAND, cmdIdx);
					}
				}
				else if (isHex(*p)) {
					const char* before = p - 1;
					if (*before == '$') {
						char *out;
						token = Token(Token::NUMBER, hex2int(p, &out));
						p = out;
					}
				}
				else if (isDigit(p)) {
					char *out;
					token = Token(Token::NUMBER, strtof(p, &out));
					p = out;
				}
				else {
					switch (*p) {
					case '(': token = Token(Token::OPEN_BRACKET); break;
					case ')': token = Token(Token::CLOSE_BRACKET); break;
					case ' ': case '\t': case '\r': break;
					case '\n': ++line; break;
					case ':': token = Token(Token::SEPARATOR); break;
					case 'X': token = Token(Token::X); break;
					case 'Y': token = Token(Token::Y); break;
					case '#': token = Token(Token::HASHTAG); break;
						//case '$': token = Token(Token::DOLLAR); break;
					case ',': token = Token(Token::COMMA); break;
					}
					++p;
				}
				if (token.type != Token::EMPTY) {
					token.line = line;
					_tokens.push_back(token);
				}
			}
			return true;
		}

		size_t num() const {
			return _tokens.size();
		}

		const Token& get(size_t index) const {
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

		std::vector<Token> _tokens;
		const char* _text;
		bool _created;
	};

	static const char* translate_token_tpye(const Token& t) {
		switch (t.type) {
			case Token::EMPTY: return "EMPTY"; break;
			case Token::NUMBER: return "NUMBER"; break;
			case Token::STRING: return "STRING"; break;
			case Token::DOLLAR: return "DOLLAR"; break;
			case Token::HASHTAG: return "HASHTAG"; break;
			case Token::OPEN_BRACKET: return "OPEN_BRACKET"; break;
			case Token::CLOSE_BRACKET: return "CLOSE_BRACKET"; break;
			case Token::COMMA: return "COMMA"; break;
			case Token::SEPARATOR: return "SEPARATOR"; break;
			case Token::X: return "X"; break;
			case Token::Y: return "Y"; break;
			case Token::COMMAND: return "COMMAND"; break;
			default: return "UNKNOWN";
		}
		return nullptr;
	}

	// -----------------------------------------------------------------
	// get hex value from command token
	// -----------------------------------------------------------------
	static uint8_t get_hex_value(const Token& token, AddressingMode mode) {
		int i = 0;
		CommandMapping m = COMMAND_MAPPING[i];
		while (m.cmd != 100) {
			if (m.cmd == token.value && m.mode == mode) {
				return m.hex;
			}
			++i;
			m = COMMAND_MAPPING[i];
		}
		printf("Error: Cannot find HEX value for %d at line %d\n", token.value, token.line);
		return 0xEA;
	}

	// -----------------------------------------------------------------
	// get command mapping
	// -----------------------------------------------------------------
	static CommandMapping get_command_mapping(uint8_t hex) {
		int i = 0;
		CommandMapping m = COMMAND_MAPPING[i];
		while (m.cmd != 100) {
			if (m.hex == hex) {
				return m;
			}
			++i;
			m = COMMAND_MAPPING[i];
		}
		return NO_OP;
	}

	static uint8_t low_value(int value) {
		return value & 255;
	}

	static uint8_t high_value(int value) {
		return (value >> 8) & 255;
	}

	// ------------------------------------------
	// translate addressing mode
	// ------------------------------------------
	static const char* translate_addressing_mode(AddressingMode mode) {
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
		default: return "UNKNOWN"; break;
		}
	}

	// -----------------------------------------------------------------
	// get addressing mode 
	// -----------------------------------------------------------------
	static AddressingMode get_addressing_mode(const Tokenizer & tokenizer, int pos) {
		const Token& command = tokenizer.get(pos);
		if (command.type == Token::COMMAND) {
			const Token& next = tokenizer.get(pos + 1);
			if (next.type == Token::HASHTAG) {
				return AddressingMode::IMMEDIDATE;
			}
			else if (next.type == Token::NUMBER) {
				int v = next.value;
				if (pos + 2 < tokenizer.num()) {
					const Token& nn = tokenizer.get(pos + 2);
					if (nn.type == Token::COMMA) {
						const Token& nn = tokenizer.get(pos + 3);
						if (nn.type == Token::X) {
							if (v <= 255) {
								return AddressingMode::ZERO_PAGE_X;
							}
							return AddressingMode::ABSOLUTE_X;
						}
						else {
							if (v <= 255) {
								return AddressingMode::ZERO_PAGE_Y;
							}
							return AddressingMode::ABSOLUTE_Y;
						}
					}
				}
				if (v <= 255) {
					return AddressingMode::ZERO_PAGE;
				}
				return AddressingMode::ABSOLUTE_ADR;
			}
			else if (next.type == Token::OPEN_BRACKET) {

			}
		}
		return AddressingMode::NONE;
	}

	// -----------------------------------------------------------------
	// convert tokens 
	// -----------------------------------------------------------------
	static int assemble(const Tokenizer & tokenizer, Context* ctx, int* numCommands) {
		printf("tokens: %d\n", tokenizer.num());
		int pc = 0x600;
		for (size_t i = 0; i < tokenizer.num(); ++i) {
			const Token& t = tokenizer.get(i);
			printf("%d = %s (line: %d)\n", i, translate_token_tpye(t), t.line);
			if (t.type == Token::COMMAND) {
				if (numCommands != nullptr) {
					++*numCommands;
				}
				const Command& cmd = get_command(t.value);
				AddressingMode mode = AddressingMode::NONE;
				if (cmd.supportedModes != 0) {
					mode = get_addressing_mode(tokenizer, i);
				}
				uint8_t hex = get_hex_value(t, mode);
				printf("=> index: %d  mode: %s cmd: %s (%X)\n", t.value, translate_addressing_mode(mode), cmd.name, hex);
				ctx->write(pc++, hex);
				if (mode == AddressingMode::IMMEDIDATE) {
					const Token& next = tokenizer.get(i + 2);
					ctx->write(pc++, next.value);
				}
				else if (mode == AddressingMode::ABSOLUTE_ADR || mode == AddressingMode::ABSOLUTE_X || mode == AddressingMode::ABSOLUTE_Y) {
					const Token& next = tokenizer.get(i + 1);
					ctx->write(pc++, low_value(next.value));
					ctx->write(pc++, high_value(next.value));
				}
				else if (mode == AddressingMode::ZERO_PAGE || mode == AddressingMode::ZERO_PAGE_X || mode == AddressingMode::ZERO_PAGE_Y) {
					const Token& next = tokenizer.get(i + 1);
					ctx->write(pc++, low_value(next.value));
				}
			}
		}
		return pc - 0x600;
	}


	// ------------------------------------------------------
	// VirtualMachine
	// ------------------------------------------------------
	class VirtualMachine {

	public:
		VirtualMachine::VirtualMachine() : _num(0) {
			for (int i = 0; i < 65536; ++i) {
				_ctx.mem[i] = 0;
			}
			_ctx.registers[0] = 0;
			_ctx.registers[1] = 0;
			_ctx.registers[2] = 0;
			for (int i = 0; i < 7; ++i) {
				_ctx.clearFlag(i);
			}
		}


		VirtualMachine::~VirtualMachine() {
		}

		bool VirtualMachine::load(const char* fileName, int pc) {
			FILE* fp = fopen(fileName, "rb");
			if (fp) {
				fread(&_num, sizeof(int), 1, fp);
				for (int i = 0; i < _num; ++i) {
					uint8_t v;
					fread(&v, sizeof(uint8_t), 1, fp);
					_ctx.write(pc + i, v);
				}
				fclose(fp);
				printf("Loaded bytes: %d\n", _num);
				return true;
			}
			printf("file '%s' not found", fileName);
			return false;
		}

		void VirtualMachine::save(const char* fileName) {
			FILE* fp = fopen(fileName, "wb");
			if (fp) {
				int pc = 0x600;
				fwrite(&_num, sizeof(int), 1, fp);
				for (int i = 0; i < _num; ++i) {
					uint8_t v = _ctx.read(pc + i);
					fwrite(&v, sizeof(uint8_t), 1, fp);
				}
				fclose(fp);
			}
		}

		// ---------------------------------------------------------
		//  parse file
		// ---------------------------------------------------------
		void VirtualMachine::parseFile(const char* fileName) {
			char buffer[256];
			sprintf(buffer, "prog\\%s.txt", fileName);
			Tokenizer tokenizer;
			if (tokenizer.parseFile(buffer)) {
				int num = assemble(tokenizer, &_ctx, nullptr);
				sprintf(buffer, "bin\\%s.prg", fileName);
				save(buffer);
				memoryDump(0x600, num);
			}
		}

		// ---------------------------------------------------------
		//  parse text
		// ---------------------------------------------------------
		int VirtualMachine::parse(const char* text, int* numCommands) {
			Tokenizer tokenizer;
			int num = -1;
			if (tokenizer.parse(text)) {
				num = assemble(tokenizer, &_ctx, numCommands);
				memoryDump(0x600, num);
			}
			return num;
		}

		void VirtualMachine::dump(int pc, int num) {
			printf("------------- Dump -------------\n");
			printf("A: %X\n", _ctx.registers[0]);
			printf("X: %X\n", _ctx.registers[1]);
			printf("Y: %X\n", _ctx.registers[2]);
			memoryDump(pc, num);
		}

		void VirtualMachine::dumpRegisters() {
			printf("------------- Dump -------------\n");
			printf("A: %X\n", _ctx.registers[0]);
			printf("X: %X\n", _ctx.registers[1]);
			printf("Y: %X\n", _ctx.registers[2]);
			printf("CZIDBVN\n");
			for (int i = 0; i < 7; ++i) {
				if (_ctx.isSet(i)) {
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
		void VirtualMachine::memoryDump(int pc, int num) {
			printf("---------- Memory dump -----------");
			for (size_t i = 0; i < num; ++i) {
				if (i % 8 == 0) {
					printf("\n%04X : ", (pc + i));
				}
				printf("%02X ", _ctx.read(pc + i));
			}
			printf("\n");
		}

		// ---------------------------------------------------------
		//  single step
		// ---------------------------------------------------------
		int VirtualMachine::step(int pc) {
			int current = pc;
			uint8_t cmdIdx = _ctx.read(current);
			CommandMapping mapping = get_command_mapping(cmdIdx);
			Command cmd = get_command(mapping.cmd);
			AddressingMode mode = mapping.mode;
			int data = 0;
			if (mode == IMMEDIDATE) {
				data = _ctx.read(current + 1);
			}
			else if (mode == ABSOLUTE_ADR) {
				uint8_t upper = _ctx.read(current + 2);
				data = _ctx.read(current + 1) + (upper << 8);
			}
			else if (mode == ABSOLUTE_X) {
				uint8_t upper = _ctx.read(current + 2);
				data = _ctx.read(current + 1) + (upper << 8) + _ctx.registers[1];
			}
			else if (mode == ABSOLUTE_Y) {
				uint8_t upper = _ctx.read(current + 2);
				data = _ctx.read(current + 1) + (upper << 8) + _ctx.registers[2];
			}
			else if (mode == ZERO_PAGE) {
				data = _ctx.read(current + 1);
			}
			else if (mode == ZERO_PAGE_X) {
				data = _ctx.read(current + 1) + _ctx.registers[1];
			}
			else if (mode == ZERO_PAGE_Y) {
				data = _ctx.read(current + 1) + _ctx.registers[2];
			}
			int add = DATA_SIZE[mode] + 1;
			printf("executing %s (%X) data: %d add: %d current: %d\n", cmd.name, cmdIdx, data, add, current);
			int ret = (*cmd.function)(&_ctx, data);
			current += add;
			return current;
		}
		// ---------------------------------------------------------
		//  run program
		// ---------------------------------------------------------
		void VirtualMachine::run(int pc) {
			int current = pc;
			int end = pc + _num;
			while (current < end) {
				uint8_t cmdIdx = _ctx.read(current);
				CommandMapping mapping = get_command_mapping(cmdIdx);
				Command cmd = get_command(mapping.cmd);
				AddressingMode mode = mapping.mode;
				int data = 0;
				if (mode == IMMEDIDATE) {
					data = _ctx.read(current + 1);
				}
				else if (mode == ABSOLUTE_ADR) {
					uint8_t upper = _ctx.read(current + 2);
					data = _ctx.read(current + 1) + (upper << 8);
				}
				else if (mode == ABSOLUTE_X) {
					uint8_t upper = _ctx.read(current + 2);
					data = _ctx.read(current + 1) + (upper << 8) + _ctx.registers[1];
				}
				else if (mode == ABSOLUTE_Y) {
					uint8_t upper = _ctx.read(current + 2);
					data = _ctx.read(current + 1) + (upper << 8) + _ctx.registers[2];
				}
				else if (mode == ZERO_PAGE) {
					data = _ctx.read(current + 1);
				}
				else if (mode == ZERO_PAGE_X) {
					data = _ctx.read(current + 1) + _ctx.registers[1];
				}
				else if (mode == ZERO_PAGE_Y) {
					data = _ctx.read(current + 1) + _ctx.registers[2];
				}
				int add = DATA_SIZE[mode] + 1;
				printf("executing %s (%X) data: %d add: %d current: %d\n", cmd.name, cmdIdx, data, add, current);
				int ret = (*cmd.function)(&_ctx, data);
				current += add;
			}
		}

		const Context& getContext() const {
			return _ctx;
		}
	private:
		Context _ctx;
		int _num;
	};
}