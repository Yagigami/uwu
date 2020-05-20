#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

#include "uwu/lex.h"
#include "stream/stream.h"

__attribute__((unused))
static const struct Interns *_get_keyword_interns(void);

static int keyword_id(const char *str, long len);

static inline bool is_token(struct Lexer *lexer, enum TokenDetail detail) {
	return lexer->token.detail == detail;
}

static inline bool match_token(struct Lexer *lexer, enum TokenDetail detail) {
	if (is_token(lexer, detail)) {
		lexer_next(lexer);
		return true;
	}
	return false;
}

static inline void expect_token(struct Lexer *lexer, enum TokenDetail detail) {
	if (!match_token(lexer, detail)) {
		printf("expected token detail %d, got %d.\n", detail, lexer->token.detail);
		lexer_fini(lexer);
	}
}

static uint8_t xtoint(char x);
static uint32_t xstoint(char *x, int n, char **endptr);
static uint32_t read_escape_sequence(char *start, char **endptr, int *width);

static void lex_word(struct Lexer *lexer);
static void lex_number(struct Lexer *lexer);
static void lex_character(struct Lexer *lexer);
static void lex_string(struct Lexer *lexer);
static void lex_floating(struct Lexer *lexer);
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
	const struct InternString *in = intern_find(interns, str, len);
	if (in) return in;
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

const struct InternString *intern_find(const struct Interns *interns, const char *str, long len) {
	for (long i=0; i<interns->len; i++) {
		const struct InternString *intern = interns->interns+i;
		if (len == intern->len && strncmp(str, intern->str, len) == 0) {
			return intern;
		}
	}
	return NULL;
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
	memset(lexer, 0, sizeof *lexer);
}

void lexer_next(struct Lexer *lexer) {
	if (!lexer->cur) goto empty;
	if (lexer->token.detail == TOKEN_DETAIL_STRING_LITERAL) {
		free(lexer->token.string.start);
	}
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
	empty:
	case '\0':
		lexer->token.kind = TOKEN_NONE;
		lexer->token.start = lexer->cur;
		lexer->token.len = 1;
		break;
	case 'L':
		if (lexer->cur[1] == '\'') lex_character(lexer);
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
		if (lexer->token.character.width > 1) {
			printf("wide character.\n");
		}
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
	printf("\n");
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

static int keyword_id(const char *str, long len) {
	const struct Interns *kws = _get_keyword_interns();
	const struct InternString *in = intern_find(kws, str, len);
	if (in) {
		return in - kws->interns + TOKEN_DETAIL_KEYWORDS_START;
	}
	return -1;
}

uint8_t xtoint(char x) {
	if (!isxdigit(x)) __builtin_unreachable();
	if (isdigit(x)) return x - '0';
	if (islower(x)) return x - 'a' + 0xa;
	return x - 'A' + 0xA;
}

uint32_t xstoint(char *x, int n, char **endptr) {
	uint32_t value = 0;
	for (int i = 0; i < n; i++) {
		if (!isxdigit(*x)) {
			printf("unexpected end of hex-sequence (%d characters instead of expected %d).\n", i, n);
			break;
		}
		value <<= 4;
		value |= xtoint(*x);
	}
	if (endptr) *endptr = x;
	return value;
}

uint32_t read_escape_sequence(char *start, char **endptr, int *width) {
	uint32_t value = 0;
	if (width) *width = 1;
	switch (*start) {
	case '\'':
	case '"':
	case '?':
	case '\\':
		value = *start++;
		break;
	case 'a':
		value = '\a';
		break;
	case 'b':
		value = '\b';
		break;
	case 'f':
		value = '\f';
		break;
	case 'n':
		value = '\n';
		break;
	case 'r':
		value = '\r';
		break;
	case 't':
		value = '\t';
		break;
	case 'v':
		value = '\v';
		break;
	case '0' ... '9':
		for (int i = 0; i < 3 && *start >= '0' && *start <= '7'; i++, start++) {
			value <<= 3;
			value |= *start - '0';
		}
		break;
	case 'x':
		value = xstoint(start, 2, &start);
		break;
	case 'u':
		value = xstoint(start, 4, &start);
		if (width) *width = 2;
		if ((value & 0xFF00) == 0) {
			printf("invalid universal character \\u%04x.\n", value);
		}
		break;
	case 'U':
		value = xstoint(start, 8, &start);
		if (width) *width = 4;
		if ((value & 0x0000FF00) == 0 || (value & 0x00FF0000) == 0 || (value & 0xFF000000) == 0) {
			printf("invalid universal character \\u%08x.\n", value);
		}
		break;
	default:
		printf("unknown escape sequence `\\%c`.\n", *start);
		break;
	}
	if (endptr) *endptr = start;
	return value;
}

static void lex_word(struct Lexer *lexer) {
	char *start = lexer->cur, *end = start;
	while (isalnum(*end) || *end == '_') end++;
	if (*end == '\0') {
		printf("end of file in identifier name.\n");
	}
	long len = end - start;
	int id;
	if ((id = keyword_id(start, len)) != -1) {
		lexer->token.kind = TOKEN_KEYWORD;
		lexer->token.detail = id;
	} else {
		lexer->token.kind = TOKEN_IDENTIFIER;
		lexer->token.detail = TOKEN_DETAIL_IDENTIFIER;
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
	char *start = lexer->cur, *end = start + 1;
	uint32_t value = 0;
	if (*start != '\'') __builtin_unreachable();
	while (*end && *end != '\n' && *end != '\'') {
		value <<= 8;
		value |= *end;
		if (*end++ != '\\') continue;
		value = read_escape_sequence(end, &end, &lexer->token.character.width);
	}
	if (*end == '\0') {
		printf("unexpected end of file in character literal.\n");
		lexer_fini(lexer);
		return;
	} else if (*end == '\n') {
		printf("unexpected newline in character literal.\n");
		lexer_fini(lexer);
		return;
	}
	long len = end - start;
	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_CHARACTER_CONSTANT;
	lexer->token.start = start;
	lexer->token.len = len;
	lexer->token.character.value = value;
	lexer->cur = end;
}

void lex_string(struct Lexer *lexer) {
	char *start = lexer->cur, *end = start + 1;
	long len = 0;
	bool too_long = false;
	if (*start != '"') __builtin_unreachable();
	while (*end && *end != '\n' && *end != '"') {
		if (*end++ != '\\') {
			if (len == LONG_MAX-1) {
				printf("string is too long :%ld characters.\n", len);
				too_long = true;
			}
			if (!too_long) len++;
		} else {
			int w;
			read_escape_sequence(end, &end, &w);
			if (len >= LONG_MAX-w) {
				printf("string is too long :%ld characters.\n", len);
				too_long = true;
			}
			if (!too_long) len += w;
		}
	}
	lexer->token.kind = TOKEN_STRING_LITERAL;
	lexer->token.detail = TOKEN_DETAIL_STRING_LITERAL;
	lexer->token.start = start;
	lexer->token.len = end - start + 1;
	lexer->cur = end;
	lexer->cur++;
	if (too_long || *end == '\0' || *end == '\n') {
		lexer->token.string.start = NULL;
		lexer->token.string.len = 0;
		lexer->token.string.width = 0;
		// TODO: those checks can probably be done when preprocessing
		if (*end == '\0') printf("unexpected end of file in string literal.\n");
		else if (*end == '\n') printf("unexpected newline in string literal.\n");
		lexer_fini(lexer);
		return;
	}
	char *string = malloc(len+1);
	if (!string) {
		fprintf(stderr, "could not interpret string of length %ld.\n", len+1);
		lexer->token.kind = TOKEN_NONE;
	} else {
		for (char *c = string, *r = start+1; *r != '"';) {
			*c = *r;
			if (*r == '\0' || *r == '\n') __builtin_unreachable();
			if (*r++ != '\\') {
				c++;
				continue;
			}
			int w;
			uint32_t value = read_escape_sequence(r, &r, &w);
			memcpy(c, &value, w);
			c += w;
		}
		string[len] = '\0';
		lexer->token.string.start = string;
		lexer->token.string.len = len;
		lexer->token.string.width = 1;
	}
}

void lex_floating(struct Lexer *lexer) {
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

static const char *token_detail_to_str[] = {
	[TOKEN_DETAIL_NONE] = "<unknown>",
	[TOKEN_DETAIL_KEYWORD_AUTO] = "k:auto",
	[TOKEN_DETAIL_KEYWORD_BREAK] = "k:break",
	[TOKEN_DETAIL_KEYWORD_CASE] = "k:case",
	[TOKEN_DETAIL_KEYWORD_CHAR] = "k:char",
	[TOKEN_DETAIL_KEYWORD_CONST] = "k:const",
	[TOKEN_DETAIL_KEYWORD_CONTINUE] = "k:continue",
	[TOKEN_DETAIL_KEYWORD_DEFAULT] = "k:default",
	[TOKEN_DETAIL_KEYWORD_DO] = "k:do",
	[TOKEN_DETAIL_KEYWORD_DOUBLE] = "k:double",
	[TOKEN_DETAIL_KEYWORD_ELSE] = "k:else",
	[TOKEN_DETAIL_KEYWORD_ENUM] = "k:enum",
	[TOKEN_DETAIL_KEYWORD_EXTERN] = "k:extern",
	[TOKEN_DETAIL_KEYWORD_FLOAT] = "k:float",
	[TOKEN_DETAIL_KEYWORD_FOR] = "k:for",
	[TOKEN_DETAIL_KEYWORD_GOTO] = "k:goto",
	[TOKEN_DETAIL_KEYWORD_IF] = "k:if",
	[TOKEN_DETAIL_KEYWORD_INLINE] = "k:inline",
	[TOKEN_DETAIL_KEYWORD_INT] = "k:int",
	[TOKEN_DETAIL_KEYWORD_LONG] = "k:long",
	[TOKEN_DETAIL_KEYWORD_REGISTER] = "k:register",
	[TOKEN_DETAIL_KEYWORD_RESTRICT] = "k:restrict",
	[TOKEN_DETAIL_KEYWORD_RETURN] = "k:return",
	[TOKEN_DETAIL_KEYWORD_SHORT] = "k:short",
	[TOKEN_DETAIL_KEYWORD_SIGNED] = "k:signed",
	[TOKEN_DETAIL_KEYWORD_SIZEOF] = "k:sizeof",
	[TOKEN_DETAIL_KEYWORD_STATIC] = "k:static",
	[TOKEN_DETAIL_KEYWORD_STRUCT] = "k:struct",
	[TOKEN_DETAIL_KEYWORD_SWITCH] = "k:switch",
	[TOKEN_DETAIL_KEYWORD_TYPEDEF] = "k:typedef",
	[TOKEN_DETAIL_KEYWORD_UNION] = "k:union",
	[TOKEN_DETAIL_KEYWORD_UNSIGNED] = "k:union",
	[TOKEN_DETAIL_KEYWORD_VOID] = "k:void",
	[TOKEN_DETAIL_KEYWORD_VOLATILE] = "k:volatile",
	[TOKEN_DETAIL_KEYWORD_WHILE] = "k:while",
	[TOKEN_DETAIL_KEYWORD_BOOL] = "k:_Bool",
	[TOKEN_DETAIL_KEYWORD_COMPLEX] = "k:_Complex",
	[TOKEN_DETAIL_KEYWORD_IMAGINARY] = "k:_Imaginary",
	[TOKEN_DETAIL_IDENTIFIER] = "i:%.*s",
	[TOKEN_DETAIL_STRING_LITERAL] = "s:\"%.*s\"",
	[TOKEN_DETAIL_INTEGER_CONSTANT] = "c:%ju",
	[TOKEN_DETAIL_FLOATING_CONSTANT] = "c:%Lg",
	[TOKEN_DETAIL_ENUMERATION_CONSTANT] = "c:%.*s",
	[TOKEN_DETAIL_CHARACTER_CONSTANT] = "c:'%lc'",
	[TOKEN_DETAIL_LSQUARE_BRACKET] = "p:[",
	[TOKEN_DETAIL_RSQUARE_BRACKET] = "p:]",
	[TOKEN_DETAIL_LBRACKET] = "p:(",
	[TOKEN_DETAIL_RBRACKET] = "p:)",
	[TOKEN_DETAIL_LCURLY_BRACE] = "p:{",
	[TOKEN_DETAIL_RCURLY_BRACE] = "p:}",
	[TOKEN_DETAIL_DOT] = "p:.",
	[TOKEN_DETAIL_ARROW] = "p:->",
	[TOKEN_DETAIL_INCREMENT] = "p:++",
	[TOKEN_DETAIL_DECREMENT] = "p:--",
	[TOKEN_DETAIL_AMPERSAND] = "p:&",
	[TOKEN_DETAIL_ASTERISK] = "p:*",
	[TOKEN_DETAIL_PLUS] = "p:+",
	[TOKEN_DETAIL_MINUS] = "p:-",
	[TOKEN_DETAIL_TILDE] = "p:~",
	[TOKEN_DETAIL_BANG] = "p:!",
	[TOKEN_DETAIL_FSLASH] = "p:/",
	[TOKEN_DETAIL_PERCENT] = "p:%",
	[TOKEN_DETAIL_LSHIFT] = "p:<<",
	[TOKEN_DETAIL_RSHIFT] = "p:>>",
	[TOKEN_DETAIL_LOWER] = "p:<",
	[TOKEN_DETAIL_GREATER] = "p:>",
	[TOKEN_DETAIL_LOWER_EQUAL] = "p:<=",
	[TOKEN_DETAIL_GREATER_EQUAL] = "p:>=",
	[TOKEN_DETAIL_EQUAL] = "p:==",
	[TOKEN_DETAIL_NOT_EQUAL] = "p:!=",
	[TOKEN_DETAIL_CIRCUMFLEX] = "p:^",
	[TOKEN_DETAIL_BAR] = "p:|",
	[TOKEN_DETAIL_AND] = "p:&&",
	[TOKEN_DETAIL_OR] = "p:||",
	[TOKEN_DETAIL_QUESTION] = "p:?",
	[TOKEN_DETAIL_COLON] = "p::",
	[TOKEN_DETAIL_SEMICOLON] = "p:;",
	[TOKEN_DETAIL_ELLIPSIS] = "p:...",
	[TOKEN_DETAIL_ASSIGN] = "p:=",
	[TOKEN_DETAIL_ASTERISK_ASSIGN] = "p:*=",
	[TOKEN_DETAIL_FSLASH_ASSIGN] = "p:/=",
	[TOKEN_DETAIL_PERCENT_ASSIGN] = "p:%=",
	[TOKEN_DETAIL_PLUS_ASSIGN] = "p:+=",
	[TOKEN_DETAIL_MINUS_ASSIGN] = "p:-=",
	[TOKEN_DETAIL_LSHIFT_ASSIGN] = "p:<<=",
	[TOKEN_DETAIL_RSHIFT_ASSIGN] = "p:>>=",
	[TOKEN_DETAIL_AMPERSAND_ASSIGN] = "p:&=",
	[TOKEN_DETAIL_CIRCUMFLEX_ASSIGN] = "p:^=",
	[TOKEN_DETAIL_BAR_ASSIGN] = "p:|=",
	[TOKEN_DETAIL_COMMA] = "p:,",
};

static int print_token(const struct Token *token) {
	int prn = 0;
	if (token->detail < TOKEN_DETAIL_NONE || token->detail >= TOKEN_DETAIL_NUM) __builtin_unreachable();
	switch (token->kind) {
	case TOKEN_NONE:
		prn += printf(token_detail_to_str[TOKEN_DETAIL_NONE]);
		break;
	case TOKEN_KEYWORD:
		prn += printf(token_detail_to_str[token->detail]);
		break;
	case TOKEN_IDENTIFIER:
		prn += printf(token_detail_to_str[token->detail], token->len, token->start);
		break;
	case TOKEN_CONSTANT:
		switch (token->detail) {
		case TOKEN_DETAIL_INTEGER_CONSTANT:
			prn += printf(token_detail_to_str[token->detail], token->integer);
			break;
		case TOKEN_DETAIL_FLOATING_CONSTANT:
			prn += printf(token_detail_to_str[token->detail], token->floating);
			break;
		case TOKEN_DETAIL_ENUMERATION_CONSTANT:
			prn += printf(token_detail_to_str[token->detail], token->len, token->start);
			break;
		case TOKEN_DETAIL_CHARACTER_CONSTANT:
			prn += printf(token_detail_to_str[token->detail], token->character.width);
			break;
		default:
			__builtin_unreachable();
		}
		break;
	case TOKEN_STRING_LITERAL:
		prn += printf(token_detail_to_str[token->detail], token->string.len, token->string.start);
		break;
	case TOKEN_PUNCTUATOR:
		prn += printf(token_detail_to_str[token->detail]);
		break;
	default:
		__builtin_unreachable();
	}
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

