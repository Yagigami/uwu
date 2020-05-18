#include "tests.h"

#include "alloc.h"

int main(int argc, char **argv) {
	mem_init();
	run_tests(argc, argv);
	mem_close();
	return 0;
}

