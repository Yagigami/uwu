#ifndef C_UWU_LEX_H
#define C_UWU_LEX_H

#include <stdbool.h>
#include <stdint.h>

#include "uwu/enums.h"
#include <common/enums.h>
#include <intern/intern.h>

struct Token {
	enum TokenKind kind;
	enum TokenDetail detail;
	const uint8_t *start;
	enum ConstantAffix affix;
	int len;
	__extension__ union {
		uint32_t character;
		struct {
			uint8_t *start;
			int len;
		} string;
		struct {
			uint32_t *start;
			int len;
		} wstring;
		uintmax_t integer;
		long double floating;
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

