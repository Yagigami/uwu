#ifndef C_UWU_LEX_H
#define C_UWU_LEX_H

#include <stdbool.h>
#include <stdint.h>

#include "uwu/enums.h"
#include "ast/common.h"
#include <intern/intern.h>

struct Token {
	enum TokenKind kind;
	enum TokenDetail detail;
	const char *start;
	int len;
	__extension__ union {
		uint32_t character;
		struct {
			char *start;
			int len;
		} string;
		struct {
			uintmax_t value;
			enum IntegerSuffix suffix;
		} integer;
		long double floating;
	};
};

struct Lexer {
	char *buf, *cur;
	long len, line;
	struct Token token;
	struct Interns identifiers;
};

int lexer_init(struct Lexer *lexer, const char *name);
void lexer_fini(struct Lexer *lexer);
void lexer_next(struct Lexer *lexer);
void lexer_dump(const struct Lexer *lexer);

#endif /* C_UWU_LEX_H */

