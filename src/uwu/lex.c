#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "uwu/lex.h"
#include "stream/stream.h"

__attribute__((unused))
static const struct Interns *_get_keyword_interns(void);

static bool is_keyword(const char *str, long len);

static void lex_word(struct Lexer *lexer);
static void lex_number(struct Lexer *lexer);
static void lex_character(struct Lexer *lexer);
static void lex_string(struct Lexer *lexer);
static void lex_floating(struct Lexer *lexer);
static void lex_wide_character(struct Lexer *lexer);
static void lex_wide_string(struct Lexer *lexer);

static int print_token(const struct Token *token);
static int print_interns(const struct Interns *interns);
static int print_intern(const struct InternString *intern);

int intern_init(struct Interns *interns) {
	if (!interns) return -1;
	interns->len = interns->cap = 0;
	interns->interns = NULL;
	return 0;
}

void intern_fini(struct Interns *interns) {
	if (!interns) return;
	for (long i = 0; i < interns->len; i++) {
		free(interns->interns[i].str);
	}
	free(interns->interns);
}

const struct InternString *intern_string(struct Interns *interns, const char *str, long len) {
	for (long i=0; i<interns->len; i++) {
		const struct InternString *intern = interns->interns+i;
		if (len == intern->len && strncmp(str, intern->str, len) == 0) {
			return intern;
		}
	}
	if (interns->len+1 > interns->cap) {
		long cap = interns->cap*2+1;
		struct InternString *tmp = realloc(interns->interns, cap * sizeof (*tmp));
		if (!tmp) return NULL;
		interns->interns = tmp;
		interns->cap = cap;
	}
	struct InternString *intern = interns->interns + interns->len;
	if (!(intern->str = malloc(len+1))) return NULL;
	memcpy(intern->str, str, len);
	intern->str[intern->len = len] = '\0';
	interns->len++;
	return intern;
}

bool intern_contains(const struct Interns *interns, const char *str, long len) {
	for (long i=0; i<interns->len; i++) {
		const struct InternString *intern = interns->interns+i;
		if (len == intern->len && strncmp(str, intern->str, len) == 0) {
			return true;
		}
	}
	return false;
}

int lexer_init(struct Lexer *lexer, const char *name) {
	int ret = -1;
	if (!lexer) goto early;
	Stream stream = stream_init(name, C_STREAM_READ|C_STREAM_TEXT);
	if (!stream) goto early;
	long size = stream_size(stream);
	lexer->buf = malloc(size+1);
	if (!lexer->buf) goto end;

	lexer->cur = lexer->buf;
	lexer->token.kind = TOKEN_NONE;
	if ((ret = intern_init(&lexer->identifiers))) goto end;

	if (stream_read(stream, lexer->buf, size) != size) goto end;
	lexer->buf[size] = '\0';
	ret = 0;
end:
	stream_fini(stream);
early:
	return ret;
}

void lexer_fini(struct Lexer *lexer) {
	if (!lexer) return;
	free(lexer->buf);
	intern_fini(&lexer->identifiers);
}

void lexer_next(struct Lexer *lexer) {
again:
	switch (*lexer->cur) {
	case ' ':
	case '\t':
	case '\v':
	case '\r':
	case '\f':
	case '\n':
		while (isspace(*lexer->cur)) {
			if (*lexer->cur++ == '\n') lexer->line++;
		}
		goto again;
	case '\0':
		lexer->token.kind = TOKEN_NONE;
		lexer->token.start = lexer->cur;
		lexer->token.len = 1;
		break;
	case 'L':
		if (lexer->cur[1] == '\'') lex_wide_character(lexer);
		else if (lexer->cur[1] == '"') lex_wide_string(lexer);
		else lex_word(lexer);
		break;
	case 'a' ... 'z':
	case 'A' ... 'L'-1: // L is special
	case 'L'+1 ... 'Z':
	case '_':
		lex_word(lexer);
		break;
	case '0' ... '9':
		lex_number(lexer);
		break;
	case '\'':
		lex_character(lexer);
		break;
	case '"':
		lex_string(lexer);
		break;
	case '.': // can also be the start of a floating constant
		if (isdigit(lexer->cur[1])) {
			lex_string(lexer);
		} else {
			lexer->token.start = lexer->cur++;
			lexer->token.kind = TOKEN_PUNCTUATOR;
			if (*lexer->cur == '.' && lexer->cur[1] == '.') {
				lexer->token.len = 3;
				lexer->token.detail = TOKEN_DETAIL_ELLIPSIS;
				lexer->cur += 2;
			} else {
				lexer->token.len = 1;
				lexer->token.detail = TOKEN_DETAIL_DOT;
			}
		}
		break;

// for ~, !, =, ...
#define CASE1(k1, v1) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		lexer->token.start = lexer->cur++; \
		lexer->token.len = 1; \
		lexer->token.detail = TOKEN_DETAIL_ ## v1; \
		break

// for */*=, %,%=, ...
#define CASE2(k1, v1, k2, v2) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		lexer->token.start = lexer->cur++; \
		if (*lexer->cur == k2) { \
			lexer->token.len = 2; \
			lexer->token.detail = TOKEN_DETAIL_ ## v2; \
			lexer->cur++; \
		} else { \
			lexer->token.len = 1; \
			lexer->token.detail = TOKEN_DETAIL_ ## v1; \
		} \
		break

// for +/++/+=, ...
#define CASE3(k1, v1, k2, v2, k3, v3) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		lexer->token.start = lexer->cur++; \
		if (*lexer->cur == k2) { \
			lexer->token.len = 2; \
			lexer->token.detail = TOKEN_DETAIL_ ## v2; \
			lexer->cur++; \
		} else if (*lexer->cur == k3) { \
			lexer->token.len = 2; \
			lexer->token.detail = TOKEN_DETAIL_ ## v3; \
			lexer->cur++; \
		} else { \
			lexer->token.len = 1; \
			lexer->token.detail = TOKEN_DETAIL_ ## v1; \
		} \
		break

	CASE1('[', LSQUARE_BRACKET);
	CASE1(']', RSQUARE_BRACKET);
	CASE1('(', LBRACKET);
	CASE1(')', RBRACKET);
	CASE1('{', LCURLY_BRACE);
	CASE1('}', RCURLY_BRACE);
	CASE3('+', PLUS, '+', INCREMENT, '=', PLUS_ASSIGN);
	CASE3('&', AMPERSAND, '&', AND, '=', AMPERSAND_ASSIGN);
	CASE2('*', ASTERISK, '=', ASTERISK_ASSIGN);
	CASE1('~', TILDE);
	CASE2('!', BANG, '=', NOT_EQUAL);
	CASE2('/', FSLASH, '=', FSLASH_ASSIGN);
	CASE3('%', PERCENT, '=', PERCENT_ASSIGN, '>', RCURLY_BRACE);
	CASE2('=', ASSIGN, '=', EQUAL);
	CASE2('^', CIRCUMFLEX, '=', CIRCUMFLEX_ASSIGN);
	CASE3('|', BAR, '|', OR, '=', BAR_ASSIGN);
	CASE1('?', QUESTION);
	CASE2(':', COLON, '>', RSQUARE_BRACKET);
	CASE1(';', SEMICOLON);
	CASE1(',', COMMA);

	case '-':
		lexer->token.kind = TOKEN_PUNCTUATOR;
		lexer->token.start = lexer->cur++;
		if (*lexer->cur == '-') {
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_DECREMENT;
			lexer->cur++;
		} else if (*lexer->cur == '>') {
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_ARROW;
			lexer->cur++;
		} else if (*lexer->cur == '=') {
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_MINUS_ASSIGN;
			lexer->cur++;
		} else {
			lexer->token.len = 1;
			lexer->token.detail = TOKEN_DETAIL_MINUS;
		}
		break;
	case '<':
		lexer->token.kind = TOKEN_PUNCTUATOR;
		lexer->token.start = lexer->cur++;
		if (*lexer->cur == '<') {
			lexer->cur++;
			if (*lexer->cur == '=') {
				lexer->cur++;
				lexer->token.len = 3;
				lexer->token.detail = TOKEN_DETAIL_LSHIFT_ASSIGN;
			} else {
				lexer->token.len = 2;
				lexer->token.detail = TOKEN_DETAIL_LSHIFT;
			}
		} else if (*lexer->cur == '=') {
			lexer->cur++;
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_LOWER_EQUAL;
		} else if (*lexer->cur == ':') { // digraph
			lexer->cur++;
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_LSQUARE_BRACKET;
		} else if (*lexer->cur == '%') { // digraph
			lexer->cur++;
			lexer->token.len = 2;
			lexer->token.detail = TOKEN_DETAIL_LCURLY_BRACE;
		} else {
			lexer->token.len = 1;
			lexer->token.detail = TOKEN_DETAIL_LOWER;
		}
		break;

#undef CASE3
#undef CASE2
#undef CASE1
	default:
		printf("unexpected character '%c' (ASCII %d) in program.\n", *lexer->cur, *lexer->cur);
		lexer->token.kind = TOKEN_NONE;
		lexer->token.start = lexer->cur++;
		lexer->token.len = 1;
		break;
	}
	printf("at token: ");
	print_token(&lexer->token);
}

void lexer_dump(const struct Lexer *lexer) {
	printf("in:\n");
	printf("%s", lexer->buf);
	printf("keywords:\n");
	print_interns(_get_keyword_interns());
	printf("identifiers:\n");
	print_interns(&lexer->identifiers);
}

const struct Interns *_get_keyword_interns(void) {
	static const struct InternString kws[TOKEN_DETAIL_KEYWORDS_END-TOKEN_DETAIL_KEYWORDS_START] = {
#define ELEM(a, b) \
	[TOKEN_DETAIL_KEYWORD_ ## a - TOKEN_DETAIL_KEYWORDS_START] = { \
		.str = # b, \
		.len = sizeof (# b)-1, \
	}
// sizeof (# b)-1 is like strlen but we need a constant expression
		ELEM(AUTO, auto),
		ELEM(BREAK, break),
		ELEM(CASE, case),
		ELEM(CHAR, char),
		ELEM(CONST, const),
		ELEM(CONTINUE, continue),
		ELEM(DEFAULT, default),
		ELEM(DO, do),
		ELEM(DOUBLE, double),
		ELEM(ELSE, else),
		ELEM(ENUM, enum),
		ELEM(EXTERN, extern),
		ELEM(FLOAT, float),
		ELEM(FOR, for),
		ELEM(GOTO, goto),
		ELEM(IF, if),
		ELEM(INLINE, inline),
		ELEM(INT, int),
		ELEM(LONG, long),
		ELEM(REGISTER, register),
		ELEM(RESTRICT, restrict),
		ELEM(RETURN, return),
		ELEM(SHORT, short),
		ELEM(SIGNED, signed),
		ELEM(SIZEOF, sizeof),
		ELEM(STATIC, static),
		ELEM(STRUCT, struct),
		ELEM(SWITCH, switch),
		ELEM(TYPEDEF, typedef),
		ELEM(UNION, union),
		ELEM(UNSIGNED, unsigned),
		ELEM(VOID, void),
		ELEM(VOLATILE, volatile),
		ELEM(WHILE, while),
		ELEM(BOOL, _Bool),
		ELEM(COMPLEX, _Complex),
		ELEM(IMAGINARY, _Imaginary),
#undef ELEM
	};
	static const struct Interns interns = {
		.interns = (struct InternString *) kws, // hope no one tries to modify it :)
		.len = sizeof (kws) / sizeof (*kws),
		.cap = 0,
	};
	return &interns;
}

static bool is_keyword(const char *str, long len) {
	const struct Interns *kws = _get_keyword_interns();
	return intern_contains(kws, str, len);
}

static void lex_word(struct Lexer *lexer) {
	char *start = lexer->cur, *end = start;
	while (isalnum(*end) || *end == '_') end++;
	if (*end == '\0') {
		printf("end of file in identifier name.\n");
	}
	long len = end - start;
	if (is_keyword(start, len)) {
		lexer->token.kind = TOKEN_KEYWORD;
	} else {
		lexer->token.kind = TOKEN_IDENTIFIER;
		if (!intern_string(&lexer->identifiers, start, len)) {
			fprintf(stderr, "could not intern string `%.*s`.\n", (int) len, start);
			lexer->token.kind = TOKEN_NONE;
		}
	}
	lexer->token.start = start;
	lexer->cur = end;
	lexer->token.len = len;
}

void lex_number(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

void lex_character(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

void lex_string(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

void lex_floating(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

void lex_wide_character(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

void lex_wide_string(struct Lexer *lexer) {
	lexer->token.kind = TOKEN_NONE;
	lexer->token.start = lexer->cur;
	lexer->token.len = 1;
	lexer->cur++;
}

static int print_token(const struct Token *token) {
	return printf("`%.*s`\n", token->len, token->start);
}

static int print_interns(const struct Interns *interns) {
	int total = 0, printed = 0, stop = 80;
	if (interns) for (long i = 0; i < interns->len; i++) {
		int ret = print_intern(interns->interns+i);
		total += ret;
		printed += ret;
		if (printed >= stop) {
			printed = 0;
			total += printf("\n");
		}
	}
	total += printf("\n");
	return total;
}

static int print_intern(const struct InternString *intern) {
	return printf("<%.*s> ", (int)intern->len, intern->str);
}

