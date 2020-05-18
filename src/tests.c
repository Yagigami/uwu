#include <stream/stream.h>
#include <pp/pp.h>
#include <ast/ast.h>

#include <stdio.h>

void run_tests(int argc, char **argv) {
	(void) argc, (void) argv;
	void (*tests[]) (void) = {
		&stream_test,
		&pp_test,
	}, (**end) (void) = tests + sizeof (tests) / sizeof (*tests);
	for (void (**test) (void) = tests; test != end; test++) {
		(*test)();
		printf("\n\n");
	}
}

