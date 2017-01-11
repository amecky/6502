#define VM_TEST_SUPPORT
#define VM_IMPLEMENTATION
#include "catch.hpp"
#include "..\6502.h"

TEST_CASE("HexTest", "[StringTests]") {
	REQUIRE(isHex('0') == true);
	REQUIRE(isHex('A') == true);
	REQUIRE(isHex('H') == false);
}

TEST_CASE("Hex2Int", "[StringTests]") {
	char* out = nullptr;
	int ret = hex2int("0200", &out);
	REQUIRE(ret == 512);
	ret = hex2int("C0", &out);
	REQUIRE(ret == 192);
}
/*
TEST_CASE("LDA", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	vm_op_lda(&ctx, 100);
	REQUIRE(ctx.registers[0] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	vm_op_lda(&ctx, 200);
	REQUIRE(ctx.registers[0] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	vm_op_lda(&ctx, 0);
	REQUIRE(ctx.registers[0] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}

TEST_CASE("LDX", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	vm_op_ldx(&ctx, 100);
	REQUIRE(ctx.registers[1] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	vm_op_ldx(&ctx, 200);
	REQUIRE(ctx.registers[1] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	vm_op_ldx(&ctx, 0);
	REQUIRE(ctx.registers[1] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}

TEST_CASE("LDY", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	vm_op_ldy(&ctx, 100);
	REQUIRE(ctx.registers[2] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	vm_op_ldy(&ctx, 200);
	REQUIRE(ctx.registers[2] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	vm_op_ldy(&ctx, 0);
	REQUIRE(ctx.registers[2] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}
*/
TEST_CASE("TAX", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	ctx.registers[0] = 100;
	vm_op_tax(&ctx, 100);
	REQUIRE(ctx.registers[0] == 100);
	ctx.registers[0] = 200;
	vm_op_tax(&ctx, 200);
	REQUIRE(ctx.registers[0] == 200);
	ctx.registers[0] = 0;
	vm_op_tax(&ctx, 0);
	REQUIRE(ctx.registers[2] == 0);
}

TEST_CASE("LSR", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	ctx.write(100, 4);
	vm_op_lsr(&ctx, 100);
	uint8_t v = ctx.read(100);
	REQUIRE(v == 2);
	ctx.write(100, 1);
	vm_op_lsr(&ctx, 100);
	v = ctx.read(100);
	REQUIRE(v == 0);
	REQUIRE(ctx.isSet(vm_flags::C) == true);
	
	
}

TEST_CASE("ROL", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	ctx.write(100, 4);
	vm_op_rol(&ctx, 100);
	uint8_t v = ctx.read(100);
	REQUIRE(v == 2);
	REQUIRE(ctx.isSet(vm_flags::C) == false);
	ctx.write(100, 1);
	vm_op_rol(&ctx, 100);
	v = ctx.read(100);
	REQUIRE(v == 0);
	REQUIRE(ctx.isSet(vm_flags::C) == false);
	ctx.write(100, 129);
	vm_op_rol(&ctx, 100);
	v = ctx.read(100);
	REQUIRE(v == 64);
	REQUIRE(ctx.isSet(vm_flags::C) == true);
	ctx.write(100, 16);
	ctx.setFlag(vm_flags::C);
	vm_op_rol(&ctx, 100);
	v = ctx.read(100);
	REQUIRE(v == 9);
	REQUIRE(ctx.isSet(vm_flags::C) == false);
}

TEST_CASE("Assemble1", "[ASM]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int num = vm_assemble(&ctx, "LDA #$01\nSTA $0200\nLDA #$05\nSTA $0201\nLDA #$08\nSTA $0202\n");
	int bytes[] = { 0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d, 0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02 };
	for (int i = 0; i < 15; ++i) {		
		int v = ctx.read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
}

TEST_CASE("Assemble2", "[ASM]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int num = vm_assemble(&ctx, "LDX #$08\ndecrement:\nDEX\nSTX $0200\nCPX #$03\nBNE decrement\nSTX $0201\nBRK\n");
	int bytes[] = { 0xa2, 0x08, 0xca, 0x8e, 0x00, 0x02, 0xe0, 0x03, 0xd0, 0xf8, 0x8e, 0x01, 0x02, 0x00 };
	for (int i = 0; i < 14; ++i) {
		int v = ctx.read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
}

TEST_CASE("AssembleJMP", "[ASM]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int num = vm_assemble(&ctx, "LDA #$03\nJMP there\nBRK\nBRK\nBRK\nthere:\nSTA $0200\n");
	int bytes[] = { 0xa9, 0x03, 0x4c, 0x08, 0x06, 0x00, 0x00, 0x00, 0x8d, 0x00, 0x02 };
	for (int i = 0; i < 11; ++i) {
		int v = ctx.read(0x600 + i);
		REQUIRE(v == bytes[i]);
	}
}