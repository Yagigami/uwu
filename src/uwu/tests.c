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
	enum LexerStatus s;
	while (s = lexer_next(&lexer), s != LEXER_END && s != LEXER_DECODE_ERROR) {
		printf("at token: ");
		print_token(&lexer.token);
		printf("\n");
	}
	lexer_dump(&lexer);
	lexer_fini(&lexer);
	if (s == LEXER_DECODE_ERROR) return s;
end:
	return err;
}

