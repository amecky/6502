/*
* 6502 simulator
*
* MIT license
*/
#pragma once
#include <stdio.h>
#include <vector>
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
	INDIRECT_X,
	INDIRECT_Y,
	RELATIVE_ADR
} vm_addressing_mode;

// -----------------------------------------------------
// Flags
// -----------------------------------------------------
typedef enum vm_flags {
	C, // Carry Flag
	Z, // Zero Flag
	I,
	D,
	B,
	V,
	N
} vm_flags;

// -----------------------------------------------------
// The virtual machine vm_context
// -----------------------------------------------------
typedef struct vm_context {
	int registers[3];
	int pc;
	uint8_t mem[65536];
	bool flags[7];
	uint8_t sp;
	uint8_t cpuFlags;
	int numCommands;
	int numBytes;

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

	int readInt(int idx) const {
		uint8_t upper = read(idx + 1);
		int data = read(idx) + (upper << 8);
		return data;
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
} vm_context;


// ---------------------------------------------------------
//  API
// ---------------------------------------------------------
void vm_clear_context(vm_context* ctx);

bool vm_load(vm_context* ctx, const char* fileName);

void vm_save(vm_context* ctx, const char* fileName);

int vm_assemble_file(vm_context* ctx, const char* fileName);

void vm_disassemble(vm_context* ctx);

int vm_assemble(vm_context* ctx, const char* text, int* numCommands);

void vm_dump(vm_context* ctx, int pc, int num);

void vm_dump_registers(vm_context* ctx);

void vm_memory_dump(vm_context* ctx, int pc, int num);

int vm_step(vm_context* ctx, int pc);

void vm_run(vm_context* ctx, int pc,int num);


#if defined(VM_IMPLEMENTATION)

typedef int(*commandFunc)(vm_context* ctx, int data);

// -----------------------------------------------------
// Command
// -----------------------------------------------------
typedef struct vm_command {
	const char* name;
	commandFunc function;
	int supportedModes;

	bool isSupported(vm_addressing_mode mode) {
		return (supportedModes & mode) == mode;
	}
} vm_command;

// -----------------------------------------------------
// Command function definitions
// -----------------------------------------------------
PRIVATE int vm_op_nop(vm_context* ctx, int pc) {
	printf("calling NOP\n");
	return pc + 1;
}

// ------------------------------------------
// LDA
// ------------------------------------------
PRIVATE int vm_op_lda(vm_context* ctx, int data) {
	printf("=> LDA - data: %d\n", data);
	ctx->registers[0] = data;
	if (data == 0) {
		ctx->setFlag(vm_flags::Z);
	}
	if (data > 127) {
		ctx->setFlag(vm_flags::N);
	}
	return 0;
}

// ------------------------------------------
// LDX
// ------------------------------------------
PRIVATE int ldx(vm_context* ctx, int data) {
	printf("=> LDX - data: %d\n", data);
	ctx->registers[1] = data;
	return 0;
}

// ------------------------------------------
// STX
// ------------------------------------------
PRIVATE int stx(vm_context* ctx, int data) {
	printf("=> STX - data: %d\n", data);
	ctx->write(data, ctx->registers[1]);
	return 0;
}

// ------------------------------------------
// STA
// ------------------------------------------
PRIVATE int sta(vm_context* ctx, int data) {
	printf("=> STA data: %d\n", data);
	ctx->write(data, ctx->registers[0]);
	return 0;
}

// ------------------------------------------
// TAX
// ------------------------------------------
PRIVATE int tax(vm_context* ctx, int data) {
	printf("=> TAX data: %d\n", data);
	ctx->registers[1] = ctx->registers[0];
	return 0;
}

// ------------------------------------------
// INX
// ------------------------------------------
PRIVATE int inx(vm_context* ctx, int data) {
	printf("=> INX data: %d\n", data);
	ctx->registers[1] += 1;
	return 0;
}

// ------------------------------------------
// ADC
// ------------------------------------------
PRIVATE int adc(vm_context* ctx, int data) {
	printf("=> ADC data: %d\n", data);
	ctx->registers[0] += data;
	if (ctx->registers[0] == 0) {
		ctx->setFlag(vm_flags::Z);
	}
	else {
		ctx->clearFlag(vm_flags::Z);
	}
	return 0;
}

PRIVATE int decrement(vm_context* ctx, int pc, int idx) {
	--ctx->registers[idx];
	if (ctx->registers[idx] < 0) {
		ctx->registers[idx] = 255;
	}
	if (ctx->registers[idx] == 0) {
		ctx->setFlag(vm_flags::Z);
	}
	if ((ctx->registers[idx] & 128) == 128) {
		ctx->setFlag(vm_flags::N);
	}
	else {
		ctx->clearFlag(vm_flags::N);
	}
	return 0;
}

// ------------------------------------------
// DEX
// ------------------------------------------
PRIVATE int dex(vm_context* ctx, int data) {
	printf("=> DEX\n");
	return decrement(ctx, data, 1);
}

// ------------------------------------------
// CPX
// ------------------------------------------
PRIVATE int cpx(vm_context* ctx, int data) {
	printf("=> CPX - data: %d\n",data);
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
	return 0;
}

// ------------------------------------------
// DEY
// ------------------------------------------
PRIVATE int dey(vm_context* ctx, int pc) {
	printf("=> DEY\n");
	return decrement(ctx, pc, 2);
}

// ------------------------------------------
// DEC
// ------------------------------------------
PRIVATE int dec(vm_context* ctx, int pc) {
	printf("=> DEC\n");
	return decrement(ctx, pc, 0);
}

PRIVATE int brk(vm_context* ctx, int pc) {
	printf("calling brk\n");
	return -1;
}

// ------------------------------------------
// BNE
// ------------------------------------------
PRIVATE int vm_op_bne(vm_context* ctx, int data) {
	printf("=> BNE - data: %d\n", data);
	if (ctx->isSet(vm_flags::Z)) {
		return 0;
	}
	return data;
}

// ------------------------------------------
// BEQ
// ------------------------------------------
PRIVATE int vm_op_beq(vm_context* ctx, int data) {
	printf("=> BEQ - data: %d\n", data);
	if (ctx->isSet(vm_flags::Z)) {
		return data;
	}
	return 0;
}


// -----------------------------------------------------
// Array of all supported commands with function pointer
// and a bitset of supported addressing modes
// -----------------------------------------------------
const static vm_command VM_COMMANDS[] = {
	{ "ADC", &adc, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "AND", &vm_op_nop, 0 },
	{ "ASL", &vm_op_nop, 0 },
	{ "BCC", &vm_op_nop, 0 },
	{ "BCS", &vm_op_nop, 0 },
	{ "BEQ", &vm_op_beq, 1 << RELATIVE_ADR },
	{ "BIT", &vm_op_nop, 0 },
	{ "BMI", &vm_op_nop, 0 },
	{ "BNE", &vm_op_bne, 1 << RELATIVE_ADR },
	{ "BPL", &vm_op_nop, 0 },
	{ "BRK", &brk, 0 },
	{ "BVC", &vm_op_nop, 0 },
	{ "BVS", &vm_op_nop, 0 },
	{ "CLC", &vm_op_nop, 0 },
	{ "CLD", &vm_op_nop, 0 },
	{ "CLI", &vm_op_nop, 0 },
	{ "CLV", &vm_op_nop, 0 },
	{ "CMP", &vm_op_nop, 0 },
	{ "CPX", &cpx, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ABSOLUTE_ADR },
	{ "CPY", &vm_op_nop, 0 },
	{ "DEC", &dec, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "DEX", &dex, 0 },
	{ "DEY", &dey, 0 },
	{ "EOR", &vm_op_nop, 0 },
	{ "INC", &vm_op_nop, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "INX", &inx, 0 },
	{ "INY", &vm_op_nop, 0 },
	{ "JMP", &vm_op_nop, 0 },
	{ "JSR", &vm_op_nop, 0 },
	{ "LDA", &vm_op_lda, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "LDX", &ldx, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_Y },
	{ "LDY", &vm_op_nop, 1 << IMMEDIDATE | 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X },
	{ "LSR", &vm_op_nop, 0 },
	{ "NOP", &vm_op_nop, 0 },
	{ "ORA", &vm_op_nop, 0 },
	{ "PHA", &vm_op_nop, 0 },
	{ "PHP", &vm_op_nop, 0 },
	{ "PLA", &vm_op_nop, 0 },
	{ "PLP", &vm_op_nop, 0 },
	{ "ROL", &vm_op_nop, 0 },
	{ "ROR", &vm_op_nop, 0 },
	{ "RTI", &vm_op_nop, 0 },
	{ "RTS", &vm_op_nop, 0 },
	{ "SBC", &vm_op_nop, 0 },
	{ "SEC", &vm_op_nop, 0 },
	{ "SED", &vm_op_nop, 0 },
	{ "SEI", &vm_op_nop, 0 },
	{ "STA", &sta, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR | 1 << ABSOLUTE_X | 1 << ABSOLUTE_Y | 1 << INDIRECT_X | 1 << INDIRECT_Y },
	{ "STX", &stx, 1 << ZERO_PAGE | 1 << ZERO_PAGE_Y | 1 << ABSOLUTE_ADR },
	{ "STY", &vm_op_nop, 1 << ZERO_PAGE | 1 << ZERO_PAGE_X | 1 << ABSOLUTE_ADR },
	{ "TAX", &tax, 0 },
	{ "TAY", &vm_op_nop, 0 },
	{ "TSX", &vm_op_nop, 0 },
	{ "TXA", &vm_op_nop, 0 },
	{ "TXS", &vm_op_nop, 0 },
	{ "TYA", &vm_op_nop, 0 }
};

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
/*
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
*/
const static int DATA_SIZE[] = {
	0, 1, 2, 2, 2, 1, 1, 1, 2, 2, 1
};

	

// -----------------------------------------------------
// Command mapping
// -----------------------------------------------------
typedef struct vm_command_mapping {
	int cmd;
	vm_addressing_mode mode;
	uint8_t hex;
} vm_command_mapping;

const static vm_command_mapping NO_OP = vm_command_mapping{ 100, NONE, 0xEA };

// -----------------------------------------------------
// Array of all comand mappings
// -----------------------------------------------------
const static vm_command_mapping COMMAND_MAPPING[] = {
	// ADC
	{ 0, IMMEDIDATE,   0x69 },
	{ 0, ZERO_PAGE,    0x65 },
	{ 0, ZERO_PAGE_X,  0x75 },
	{ 0, ABSOLUTE_ADR, 0x6D },
	{ 0, ABSOLUTE_X,   0x7D },
	{ 0, ABSOLUTE_Y,   0x79 },
	{ 0, INDIRECT_X,   0x61 },
	{ 0, INDIRECT_Y,   0x71 },
	// BNE
	{ 8, RELATIVE_ADR, 0xD0 },
	// BRK
	{ 10, NONE,        0x00 },
	// CPX
	{ 18, IMMEDIDATE,  0xE0 },
	{ 18, ZERO_PAGE,   0xE4 },
	{ 18, ABSOLUTE_ADR,0xEC },
	// DEC
	{ 20, ZERO_PAGE,   0xC6 },
	{ 20, ZERO_PAGE_X, 0xD6 },
	{ 20, ABSOLUTE_ADR,0xCE },
	{ 20, ABSOLUTE_X,  0xDE },
	// DEX
	{ 21, NONE,        0xCA },
	// DEY
	{ 22, NONE,        0x88 },
	// INC
	{ 24, ZERO_PAGE,   0xE6 },
	{ 24, ZERO_PAGE_X, 0xF6 },
	{ 24, ABSOLUTE_ADR,0xEE },
	{ 24, ABSOLUTE_X,  0xFE },
	// INX
	{ 25, NONE,        0xE8 },
	// INY
	{ 26, NONE,        0xC8 },
	// LDA
	{ 29, IMMEDIDATE,  0xA9 },
	{ 29, ZERO_PAGE,   0xA5 },
	{ 29, ZERO_PAGE_X, 0xB5 },
	{ 29, ABSOLUTE_ADR,0xAD },
	{ 29, ABSOLUTE_X,  0xBD },
	{ 29, ABSOLUTE_Y,  0xB9 },
	{ 29, INDIRECT_X,  0xA1 },
	{ 29, INDIRECT_Y,  0xB1 },
	// LDX
	{ 30, IMMEDIDATE,  0xA2 },
	{ 30, ZERO_PAGE,   0xA6 },
	{ 30, ZERO_PAGE_Y, 0xB6 },
	{ 30, ABSOLUTE_ADR,0xAE },
	{ 30, ABSOLUTE_Y,  0xBE },
	// LDY
	{ 31, IMMEDIDATE,  0xA0 },
	{ 31, ZERO_PAGE,   0xA4 },
	{ 31, ZERO_PAGE_X, 0xB4 },
	{ 31, ABSOLUTE_ADR,0xAC },
	{ 31, ABSOLUTE_X,  0xBC },
	// PHA
	{ 35, NONE,        0x48 },
	// PLA
	{ 37, NONE,        0x68 },
	// STA
	{ 47, ZERO_PAGE,   0x85 },
	{ 47, ZERO_PAGE_X, 0x95 },
	{ 47, ABSOLUTE_ADR,0x8D },
	{ 47, ABSOLUTE_X,  0x9D },
	{ 47, ABSOLUTE_Y,  0x99 },
	{ 47, INDIRECT_X,  0x81 },
	{ 47, INDIRECT_Y,  0x91 },
	// STX
	{ 48, ZERO_PAGE,   0x86 },
	{ 48, ZERO_PAGE_Y, 0x96 },
	{ 48, ABSOLUTE_ADR,0x8E },
	// STY
	{ 49, ZERO_PAGE,   0x84 },
	{ 49, ZERO_PAGE_X, 0x94 },
	{ 49, ABSOLUTE_ADR,0x8C },
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
struct Token {

	enum TokenType { EMPTY, NUMBER, STRING, DOLLAR, HASHTAG, OPEN_BRACKET, CLOSE_BRACKET, COMMA, X, Y, SEPARATOR, COMMAND };

	Token(TokenType t) : type(t), value(0), hash(0) {}
	Token(TokenType t, int v) : type(t), value(v), hash(0) {}
	//Token(TokenType t, uint32_t st, int s) : type(t), value(0), hash(st), size(s) {}

	TokenType type;
	int value;
	uint32_t hash;
	int line;
};

	

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
			Token token(Token::EMPTY);
			if (isText(p)) {
				const char *identifier = p;
				while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
					p++;
				token = Token(Token::STRING);
				int cmdIdx = find_command(identifier);
				if (cmdIdx != -1) {
					token = Token(Token::COMMAND, cmdIdx);
				}
				else {
					token.hash = fnv1a(identifier, p - identifier);
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
				token = Token(Token::NUMBER, str2f(p, &out));
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

PRIVATE const char* translate_token_tpye(const Token& t) {
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
PRIVATE uint8_t get_hex_value(const Token& token, vm_addressing_mode mode) {
	int i = 0;
	vm_command_mapping m = COMMAND_MAPPING[i];
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
PRIVATE vm_command_mapping get_command_mapping(uint8_t hex) {
	int i = 0;
	vm_command_mapping m = COMMAND_MAPPING[i];
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
	default: return "UNKNOWN"; break;
	}
}

// -----------------------------------------------------------------
// get addressing mode 
// -----------------------------------------------------------------
PRIVATE vm_addressing_mode get_addressing_mode(const Tokenizer & tokenizer, int pos) {
	const Token& command = tokenizer.get(pos);
	if (command.type == Token::COMMAND) {
		const Token& next = tokenizer.get(pos + 1);
		if (next.type == Token::HASHTAG) {
			return vm_addressing_mode::IMMEDIDATE;
		}
		else if (next.type == Token::NUMBER) {
			int v = next.value;
			if (pos + 2 < tokenizer.num()) {
				const Token& nn = tokenizer.get(pos + 2);
				if (nn.type == Token::COMMA) {
					const Token& nn = tokenizer.get(pos + 3);
					if (nn.type == Token::X) {
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
		else if (next.type == Token::OPEN_BRACKET) {

		}
		else if (next.type == Token::STRING) {
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
		const vm_command& cmd = VM_COMMANDS[mapping.cmd];
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
		}
		printf("\n");
		pc += DATA_SIZE[mapping.mode] + 1;
	}
}

	

struct LabelDefinition {
	int pc;
	uint32_t hash;
};
// -----------------------------------------------------------------
// convert tokens 
// -----------------------------------------------------------------
PRIVATE int assemble(const Tokenizer & tokenizer, vm_context* ctx, int* numCommands) {
	printf("tokens: %d\n", tokenizer.num());
	int pc = 0x600;
	std::vector<LabelDefinition> definitions;
	std::vector<LabelDefinition> branches;
	for (size_t i = 0; i < tokenizer.num(); ++i) {
		const Token& t = tokenizer.get(i);
		printf("%d = %s (line: %d)\n", i, translate_token_tpye(t), t.line);
		if (t.type == Token::COMMAND) {
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
				const Token& next = tokenizer.get(i + 2);
				ctx->write(pc++, next.value);
			}
			else if (mode == vm_addressing_mode::ABSOLUTE_ADR || mode == vm_addressing_mode::ABSOLUTE_X || mode == vm_addressing_mode::ABSOLUTE_Y) {
				const Token& next = tokenizer.get(i + 1);
				ctx->write(pc++, low_value(next.value));
				ctx->write(pc++, high_value(next.value));
			}
			else if (mode == vm_addressing_mode::ZERO_PAGE || mode == vm_addressing_mode::ZERO_PAGE_X || mode == vm_addressing_mode::ZERO_PAGE_Y) {
				const Token& next = tokenizer.get(i + 1);
				ctx->write(pc++, low_value(next.value));
			}
			else if (mode == vm_addressing_mode::RELATIVE_ADR) {
				const Token& next = tokenizer.get(i + 1);
				LabelDefinition def;
				def.hash = next.hash;
				def.pc = pc;
				branches.push_back(def);
				ctx->write(pc++, 0);
			}
		}
		else if (t.type == Token::STRING) {
			const Token& next = tokenizer.get(i + 1);
			if (next.type == Token::SEPARATOR) {
				printf("=> label definition at %X\n", pc);
				LabelDefinition def;
				def.hash = t.hash;
				def.pc = pc;
				definitions.push_back(def);
			}
			//else {
				//printf("found string at %X\n", pc);
			//}
		}
	}
	printf("labels: %d\n", definitions.size());
	printf("branches: %d\n", branches.size());
	for (size_t i = 0; i < branches.size();++i) {
		const LabelDefinition& branch = branches[i];
		printf("branch %d\n", branch.hash);
		int idx = -1;
		for (size_t j = 0; j < definitions.size(); ++j) {
			if (definitions[j].hash == branch.hash) {
				int diff = definitions[j].pc - branch.pc;
				if (diff < 0) {
					diff = 255 - diff;
				}
				printf("found matching branch/label diff: %d\n",diff);					
				ctx->write(branch.pc, diff);
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
		//memoryDump(0x600, _num);
		vm_memory_dump(ctx, 0x600, ctx->numBytes);
		return ctx->numBytes;
	}
	else {
		printf("ERROR: cannot laod file: '%s'\n", fileName);
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
	printf("A: %X\n", ctx->registers[0]);
	printf("X: %X\n", ctx->registers[1]);
	printf("Y: %X\n", ctx->registers[2]);
	printf("CZIDBVN\n");
	for (int i = 0; i < 7; ++i) {
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

// ---------------------------------------------------------
//  run program
// ---------------------------------------------------------
void vm_run(vm_context* ctx, int pc, int num) {
	int current = pc;
	int end = pc + num;
	while (current < end) {
		uint8_t cmdIdx = ctx->read(current);
		const vm_command_mapping& mapping = get_command_mapping(cmdIdx);
		const vm_command& cmd = VM_COMMANDS[mapping.cmd];
		vm_addressing_mode mode = mapping.mode;
		int data = 0;
		if (mode == IMMEDIDATE) {
			data = ctx->read(current + 1);
		}
		else if (mode == ABSOLUTE_ADR) {
			uint8_t upper = ctx->read(current + 2);
			data = ctx->read(current + 1) + (upper << 8);
		}
		else if (mode == ABSOLUTE_X) {
			uint8_t upper = ctx->read(current + 2);
			data = ctx->read(current + 1) + (upper << 8) + ctx->registers[1];
		}
		else if (mode == ABSOLUTE_Y) {
			uint8_t upper = ctx->read(current + 2);
			data = ctx->read(current + 1) + (upper << 8) + ctx->registers[2];
		}
		else if (mode == ZERO_PAGE) {
			data = ctx->read(current + 1);
		}
		else if (mode == ZERO_PAGE_X) {
			data = ctx->read(current + 1) + ctx->registers[1];
		}
		else if (mode == ZERO_PAGE_Y) {
			data = ctx->read(current + 1) + ctx->registers[2];
		}
		else if (mode == RELATIVE_ADR) {
			data = ctx->read(current + 1);
		}
		int add = DATA_SIZE[mode] + 1;
		int ret = (*cmd.function)(ctx, data);
		printf("executing %s (%X) data: %d add: %d current: %d ret: %d\n", cmd.name, cmdIdx, data, add, current, ret);
		if (ret != 0) {
			current -= ret;
		}
		else {
			current += add;
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

// ---------------------------------------------------------
//  single step
// ---------------------------------------------------------
int vm_step(vm_context* ctx, int pc) {
	int current = pc;
	uint8_t cmdIdx = ctx->read(current);
	const vm_command_mapping& mapping = get_command_mapping(cmdIdx);
	const vm_command& cmd = VM_COMMANDS[mapping.cmd];
	vm_addressing_mode mode = mapping.mode;
	int data = 0;
	if (mode == IMMEDIDATE) {
		data = ctx->read(current + 1);
	}
	else if (mode == ABSOLUTE_ADR) {
		uint8_t upper = ctx->read(current + 2);
		data = ctx->read(current + 1) + (upper << 8);
	}
	else if (mode == ABSOLUTE_X) {
		uint8_t upper = ctx->read(current + 2);
		data = ctx->read(current + 1) + (upper << 8) + ctx->registers[1];
	}
	else if (mode == ABSOLUTE_Y) {
		uint8_t upper = ctx->read(current + 2);
		data = ctx->read(current + 1) + (upper << 8) + ctx->registers[2];
	}
	else if (mode == ZERO_PAGE) {
		data = ctx->read(current + 1);
	}
	else if (mode == ZERO_PAGE_X) {
		data = ctx->read(current + 1) + ctx->registers[1];
	}
	else if (mode == ZERO_PAGE_Y) {
		data = ctx->read(current + 1) + ctx->registers[2];
	}
	int add = DATA_SIZE[mode] + 1;
	printf("executing %s (%X) data: %d add: %d current: %d\n", cmd.name, cmdIdx, data, add, current);
	int ret = (*cmd.function)(ctx, data);
	current += add;
	return current;
}

#endif