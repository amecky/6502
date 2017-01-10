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
