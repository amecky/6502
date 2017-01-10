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

TEST_CASE("LDA", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int ret = vm_op_lda(&ctx, 100);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[0] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	ret = vm_op_lda(&ctx, 200);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[0] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	ret = vm_op_lda(&ctx, 0);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[0] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}

TEST_CASE("LDX", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int ret = vm_op_ldx(&ctx, 100);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[1] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	ret = vm_op_ldx(&ctx, 200);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[1] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	ret = vm_op_ldx(&ctx, 0);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[1] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}

TEST_CASE("LDY", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	int ret = vm_op_ldy(&ctx, 100);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[2] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	ret = vm_op_ldy(&ctx, 200);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[2] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	ret = vm_op_ldy(&ctx, 0);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[2] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}

TEST_CASE("TAX", "[CommandTest]") {
	vm_context ctx;
	vm_clear_context(&ctx);
	ctx.registers[0] = 100;
	int ret = vm_op_tax(&ctx, 100);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[0] == 100);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
	ctx.registers[0] = 200;
	ret = vm_op_tax(&ctx, 200);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[0] == 200);
	REQUIRE(!ctx.isSet(vm_flags::Z));
	REQUIRE(ctx.isSet(vm_flags::N));
	ctx.registers[0] = 0;
	ret = vm_op_tax(&ctx, 0);
	REQUIRE(ret == 0);
	REQUIRE(ctx.registers[2] == 0);
	REQUIRE(ctx.isSet(vm_flags::Z));
	REQUIRE(!ctx.isSet(vm_flags::N));
}
