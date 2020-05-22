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
	while (lexer_next(&lexer) != LEXER_END) {
		(void) 0;
	}
	lexer_dump(&lexer);
	lexer_fini(&lexer);
end:
	return err;
}

