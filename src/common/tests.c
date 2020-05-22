#include <stream/stream.h>
#include <pp/pp.h>
#include <uwu/uwu.h>

#include <stdio.h>

void run_tests(int argc, char **argv) {
	(void) argc, (void) argv;
	int (*tests[]) (void) = {
		&stream_test,
		&pp_test,
		&lex_test,
	}, (**end) (void) = tests + sizeof (tests) / sizeof (*tests);
	for (int (**test) (void) = tests; test != end; test++) {
		int err = (*test)();
		printf("[exit status = %d]\n", err);
		printf("\n\n");
	}
}

