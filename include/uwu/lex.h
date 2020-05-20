#ifndef C_UWU_LEX_H
#define C_UWU_LEX_H

#include <stdbool.h>
#include <stdint.h>

#include "uwu/enums.h"
#include "ast/common.h"

struct Token {
	enum TokenKind kind;
	enum TokenDetail detail;
	const char *start;
	int len;
	__extension__ union {
		uintmax_t integer;
		long double floating;
	};
};

struct Interns {
	struct InternString *interns;
	long len, cap;
};

struct InternString {
	char *str;
	long len;
};

struct Lexer {
	char *buf, *cur;
	struct Token token;
	const struct Interns *keywords;
	struct Interns identifiers;
};

int intern_init(struct Interns *interns);
void intern_fini(struct Interns *interns);
const struct InternString *intern_string(struct Interns *interns, const char *str, long len);
bool intern_contains(const struct Interns *interns, const char *str, long len);

int lexer_init(struct Lexer *lexer, const char *name);
void lexer_fini(struct Lexer *lexer);
void lexer_next(struct Lexer *lexer);

#endif /* C_UWU_LEX_H */

