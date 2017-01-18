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
	vm_context* ctx = vm_create();
	vm_op_lda(ctx, 100,IMMEDIDATE);
	REQUIRE(ctx->registers[0] == 100);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_op_lda(ctx, 200, IMMEDIDATE);
	REQUIRE(ctx->registers[0] == 200);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(ctx->isSet(vm_flags::N));
	vm_op_lda(ctx, 0, IMMEDIDATE);
	REQUIRE(ctx->registers[0] == 0);
	REQUIRE(ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_release();
}

TEST_CASE("LDX", "[CommandTest]") {
	vm_context* ctx = vm_create();
	vm_op_ldx(ctx, 100, IMMEDIDATE);
	REQUIRE(ctx->registers[1] == 100);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_op_ldx(ctx, 200, IMMEDIDATE);
	REQUIRE(ctx->registers[1] == 200);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(ctx->isSet(vm_flags::N));
	vm_op_ldx(ctx, 0, IMMEDIDATE);
	REQUIRE(ctx->registers[1] == 0);
	REQUIRE(ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_release();
}

TEST_CASE("LDY", "[CommandTest]") {
	vm_context* ctx = vm_create();
	vm_op_ldy(ctx, 100, IMMEDIDATE);
	REQUIRE(ctx->registers[2] == 100);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_op_ldy(ctx, 200, IMMEDIDATE);
	REQUIRE(ctx->registers[2] == 200);
	REQUIRE(!ctx->isSet(vm_flags::Z));
	REQUIRE(ctx->isSet(vm_flags::N));
	vm_op_ldy(ctx, 0, IMMEDIDATE);
	REQUIRE(ctx->registers[2] == 0);
	REQUIRE(ctx->isSet(vm_flags::Z));
	REQUIRE(!ctx->isSet(vm_flags::N));
	vm_release();
}

TEST_CASE("TAX", "[CommandTest]") {
	vm_context* ctx = vm_create();
	ctx->registers[0] = 100;
	vm_op_tax(ctx, 100, NONE);
	REQUIRE(ctx->registers[0] == 100);
	ctx->registers[0] = 200;
	vm_op_tax(ctx, 200, NONE);
	REQUIRE(ctx->registers[0] == 200);
	ctx->registers[0] = 0;
	vm_op_tax(ctx, 0, NONE);
	REQUIRE(ctx->registers[2] == 0);
	vm_release();
}

TEST_CASE("LSR", "[CommandTest]") {
	vm_context* ctx = vm_create();
	ctx->write(100, 4);
	vm_op_lsr(ctx, 100, NONE);
	uint8_t v = ctx->read(100);
	REQUIRE(v == 2);
	ctx->write(100, 1);
	vm_op_lsr(ctx, 100, NONE);
	v = ctx->read(100);
	REQUIRE(v == 0);
	REQUIRE(ctx->isSet(vm_flags::C) == true);
	vm_release();	
}

TEST_CASE("ROL", "[CommandTest]") {
	vm_context* ctx = vm_create();
	ctx->write(100, 4);
	vm_op_rol(ctx, 100, NONE);
	uint8_t v = ctx->read(100);
	REQUIRE(v == 2);
	REQUIRE(ctx->isSet(vm_flags::C) == false);
	ctx->write(100, 1);
	vm_op_rol(ctx, 100, NONE);
	v = ctx->read(100);
	REQUIRE(v == 0);
	REQUIRE(ctx->isSet(vm_flags::C) == false);
	ctx->write(100, 129);
	vm_op_rol(ctx, 100, NONE);
	v = ctx->read(100);
	REQUIRE(v == 64);
	REQUIRE(ctx->isSet(vm_flags::C) == true);
	ctx->write(100, 16);
	ctx->setFlag(vm_flags::C);
	vm_op_rol(ctx, 100, NONE);
	v = ctx->read(100);
	REQUIRE(v == 9);
	REQUIRE(ctx->isSet(vm_flags::C) == false);
	vm_release();
}

TEST_CASE("GetData", "[ADR_MODE]") {
	vm_context* ctx = vm_create();
	ctx->programCounter = 0;
	ctx->mem[1] = 6;
	ctx->registers[1] = 2;
	int tst = get_data(ctx, vm_addressing_mode::ZERO_PAGE_X);
	REQUIRE(tst == 8);
	ctx->mem[1] = 0xfe;
	ctx->registers[1] = 5;
	tst = get_data(ctx, vm_addressing_mode::ZERO_PAGE_X);
	REQUIRE(tst == 3);
	vm_release();
}