#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int argc, char* const argv[]) {
	// global setup...	
	Catch::Session session;
	int result = session.run(argc, argv);
}