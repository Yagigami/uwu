#include <uwu/uwu.h>
#include <stdio.h>

int lex_test(void) {
	printf("lex:\n");
	struct Lexer lexer;
	int err = -1;
	const char *f = "foo.i";
	if ((err = lexer_init(&lexer, f))) {
		fprintf(stderr, "could not initialize lexer with file `%s`.\n", f);
		goto end;
	}
	do {
		lexer_next(&lexer);
	} while (*lexer.cur);
	lexer_dump(&lexer);
	lexer_fini(&lexer);
end:
	return err;
}

