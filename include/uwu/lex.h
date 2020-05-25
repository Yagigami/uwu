#ifndef C_UWU_LEX_H
#define C_UWU_LEX_H

#include <stdbool.h>
#include <stdint.h>

#include "uwu/enums.h"
#include <common/enums.h>
#include <intern/intern.h>
#include <ast/common.h>

struct Token {
	enum TokenKind kind;
#ifndef NDEBUG
	const uint8_t *start;
	ptrdiff_t len;
#endif
	__extension__ union {
		struct IntegerConstant integer;
		struct FloatingConstant floating;
		// not sure this one should be here
		struct EnumerationConstant enumeration;
		struct CharacterConstant character;
		struct StringLiteral lit;
		struct Identifier ident;
	};
};

struct Lexer {
	uint8_t *buf;
	const uint8_t *cur;
	long len, line;
	struct Token token;
	struct Interns identifiers;
};

int lexer_init(struct Lexer *lexer, const char *name);
void lexer_fini(struct Lexer *lexer);
enum LexerStatus lexer_next(struct Lexer *lexer);
void lexer_dump(const struct Lexer *lexer);

int print_token(const struct Token *token);

#endif /* C_UWU_LEX_H */

