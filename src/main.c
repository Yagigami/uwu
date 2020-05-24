#include "tests.h"
#include <locale.h>

int main(int argc, char **argv) {
	setlocale(LC_ALL, "C.UTF-8");
	run_tests(argc, argv);
	return 0;
}

