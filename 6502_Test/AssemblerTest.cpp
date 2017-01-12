#include "catch.hpp"
#include "..\6502.h"

TEST_CASE("Assemble1", "[ASM]") {
	vm_context* ctx = vm_create();
	int num = vm_assemble("LDA #$01\nSTA $0200\nLDA #$05\nSTA $0201\nLDA #$08\nSTA $0202\n");
	int bytes[] = { 0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02 };
	for (int i = 0; i < 15; ++i) {
		int v = ctx->read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
	vm_release();
}

TEST_CASE("Assemble2", "[ASM]") {
	vm_context* ctx = vm_create();
	int num = vm_assemble("LDX #$08\ndecrement:\nDEX\nSTX $0200\nCPX #$03\nBNE decrement\nSTX $0201\nBRK\n");
	int bytes[] = { 0xa2, 0x08, 0xca, 0x8e, 0x00, 0x02, 0xe0, 0x03, 0xd0, 0xf8, 0x8e, 0x01, 0x02, 0x00 };
	for (int i = 0; i < 14; ++i) {
		int v = ctx->read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
	vm_release();
}

TEST_CASE("AssembleJMP", "[ASM]") {
	vm_context* ctx = vm_create();
	int num = vm_assemble("LDA #$03\nJMP there\nBRK\nBRK\nBRK\nthere:\nSTA $0200\n");
	int bytes[] = { 0xa9, 0x03, 0x4c, 0x08, 0x06, 0x00, 0x00, 0x00, 0x8d, 0x00, 0x02 };
	for (int i = 0; i < 11; ++i) {
		int v = ctx->read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
	vm_release();
}