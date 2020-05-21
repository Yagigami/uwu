#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#include "uwu/lex.h"
#include "stream/stream.h"
#include <intern/intern.h>
#include "data.h"

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
		lexer->token.kind = TOKEN_NONE;
		lexer->token.detail = TOKEN_DETAIL_NONE;
		lexer->cur = lexer->buf + lexer->len;
	}
}

static bool is_valid_universal(uint32_t c);
static uint8_t xtoint(char x);
static uint32_t xstoint(char *x, int n, char **endptr);
static uint32_t read_escape_sequence(char *start, char **endptr);

static void lex_word(struct Lexer *lexer);
static void lex_number(struct Lexer *lexer);
static void lex_character(struct Lexer *lexer);
static void lex_string(struct Lexer *lexer);
static void lex_integer(struct Lexer *lexer, int base);
static void lex_floating(struct Lexer *lexer, int base);
static void lex_wide_string(struct Lexer *lexer);

static int print_token(const struct Token *token);

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
	lexer->len = size;

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
	print_interns(get_keyword_interns());
	printf("identifiers:\n");
	print_interns(&lexer->identifiers);
}

static int keyword_id(const char *str, long len) {
	const struct Interns *kws = get_keyword_interns();
	const struct InternString *in = intern_find(kws, str, len);
	if (in) {
		return in - kws->interns + TOKEN_DETAIL_KEYWORDS_START;
	}
	return TOKEN_DETAIL_NONE;
}

__attribute__((unused))
bool is_valid_universal(uint32_t c) {
	return (c >= 0x00A0 || c == 0x0024 || c == 0x0040 || c == 0x0060) && !(c >= 0xD800 && c <= 0xDFFF);
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

uint32_t read_escape_sequence(char *start, char **endptr) {
	uint32_t value = 0;
	switch (*start) {
	case '\'':
	case '"':
	case '?':
	case '\\':
		value = *start++;
		break;
	case 'a':
		value = '\a';
		start++;
		break;
	case 'b':
		value = '\b';
		start++;
		break;
	case 'f':
		value = '\f';
		start++;
		break;
	case 'n':
		value = '\n';
		start++;
		break;
	case 'r':
		value = '\r';
		start++;
		break;
	case 't':
		value = '\t';
		start++;
		break;
	case 'v':
		value = '\v';
		start++;
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
	case 'U':
		printf("universal characters are unhandled.\n");
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
	if ((id = keyword_id(start, len)) != TOKEN_DETAIL_NONE) {
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
	int base = 10;
	char *cur = lexer->cur;
	if (*cur++ == '0') {
		if (tolower(*cur) == 'x') {
			base = 16;
			cur++;
		} else {
			base = 8;
		}
	}
	while (isxdigit(*cur)) cur++;
	if (*cur == '.' || tolower(*cur) == 'e' || tolower(*cur) == 'p') {
		lex_floating(lexer, base);
	} else {
		lex_integer(lexer, base);
	}
}

void lex_character(struct Lexer *lexer) {
	char *start = lexer->cur, *end = start + 1;
	uint32_t value = 0;
	if (*start != '\'') __builtin_unreachable();
	while (*end && *end != '\n' && *end != '\'') {
		value <<= 8;
		value |= *end;
		if (*end++ != '\\') continue;
		value = read_escape_sequence(end, &end);
	}
	if (*end == '\0') {
		printf("unexpected end of file in character literal.\n");
		goto err;
	} else if (*end == '\n') {
		printf("unexpected newline in character literal.\n");
		goto err;
	}
	long len = end - start;
	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_CHARACTER_CONSTANT;
	lexer->token.start = start;
	lexer->token.len = len;
	lexer->token.character = value;
	lexer->cur = end;
	return;
err:
	lexer->token.kind = TOKEN_NONE;
	lexer->token.detail = TOKEN_DETAIL_NONE;
	lexer->cur = lexer->buf + lexer->len;
}

ptrdiff_t string_lit_len_narrow(char *str, char **endptr) {
	assert(*str == '"');
	char *start = str + 1, *end = start;
	ptrdiff_t len = 0;
	while (*end && *end != '\n' && *end != '"') {
		len++;
		if (*end++ != '\\') continue;
		read_escape_sequence(end, &end);
	}
	if (*end == '\0' || *end == '\n') {
		len = -1;
	}
	if (endptr) *endptr = end;
	return len;
}

void encode_narrow(char *out, char *in) {
	for (char *c = out, *r = in + 1; *r != '"'; c++) {
		*c = *r;
		if (*r == '\0' || *r == '\n') __builtin_unreachable();
		if (*r++ != '\\') continue;
		uint32_t value = read_escape_sequence(r, &r);
		*c = value & 0xFF; // we do not handle universal characters correctly yet
	}
}

void lex_string(struct Lexer *lexer) {
	char *start = lexer->cur, *end;
	ptrdiff_t len = string_lit_len_narrow(start, &end);
	lexer->token.kind = TOKEN_STRING_LITERAL;
	lexer->token.detail = TOKEN_DETAIL_STRING_LITERAL;
	lexer->token.start = start;
	lexer->token.len = end - start + 1;
	lexer->cur = end;
	lexer->cur++;
	if (len == -1) goto err;

	char *string = malloc(len+1);
	if (!string) {
		fprintf(stderr, "could not interpret string of length %ld.\n", len+1);
		goto err;
	} else {
		encode_narrow(string, start);
		string[len] = '\0';
		lexer->token.string.start = string;
		lexer->token.string.len = len;
	}
	return;
err:
	lexer->token.kind = TOKEN_NONE;
	lexer->token.detail = TOKEN_DETAIL_NONE;
	lexer->cur = lexer->buf + lexer->len;
}

int char2hex(char c) {
	char l = tolower(c);
	if (l > '9') return l - 'a' + 0xa;
	return l - '0';
}

void lex_integer(struct Lexer *lexer, int base) {
	char *start = lexer->cur++, *end = start;
	uintmax_t val = 0;
	if (base == 16) {
		end += 2;
	} else if (base != 10 && base != 8) {
		printf("invalid base %d for an integer.\n", base);
		goto err;
	}
	while (isxdigit(*end)) {
		int i = char2hex(*end);
		if (i >= base) {
			printf("digit `%c` is out of range in base %d.\n", *end, base);
			goto err;
		}
		uintmax_t next = (val * base) + i;
		if (next < val) {
			printf("constant is too big.\n");
			goto err;
		}
		val = next;
		end++;
	}
	bool usuffix = false, lsuffix = false, llsuffix = false;
	char suff;
	while (suff = tolower(*end), suff == 'u' || suff == 'l') {
		if (suff == 'u') {
			if (usuffix) {
				printf("integer already has suffix `u`.\n");
				goto err;
			} else {
				usuffix = true;
			}
		} else if (suff == 'l') {
			if (tolower(end[1]) == 'l') {
				end++;
				if (llsuffix) {
					printf("integer already has suffix `ll`.\n");
					goto err;
				} else {
					llsuffix = true;
				}
			} else if (lsuffix) {
				printf("integer already has suffix `l`.\n");
				goto err;
			} else {
				lsuffix = true;
			}
		}
		end++;
	}
	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_INTEGER_CONSTANT;
	lexer->token.start = start;
	lexer->token.len = end - start;
	lexer->token.integer.value = val;
	lexer->cur = end;

	enum IntegerSuffix suffix;
	if (!usuffix && !lsuffix && !llsuffix) suffix = INTEGER_SUFFIX_NONE;
	else if (usuffix && !lsuffix && !llsuffix) suffix = INTEGER_SUFFIX_U;
	else if (!usuffix && lsuffix && !llsuffix) suffix = INTEGER_SUFFIX_L;
	else if (!usuffix && !lsuffix && llsuffix) suffix = INTEGER_SUFFIX_LL;
	else if (usuffix && lsuffix && !llsuffix)  suffix = INTEGER_SUFFIX_UL;
	else if (usuffix && !lsuffix && llsuffix)  suffix = INTEGER_SUFFIX_ULL;
	else if (lsuffix && llsuffix) {
		printf("integer constant contains both `l` and `ll` suffixes.\n");
		goto err;
	}
	lexer->token.integer.suffix = suffix;
	return;
err:
	lexer->token.kind = TOKEN_NONE;
	lexer->token.detail = TOKEN_DETAIL_NONE;
	lexer->cur = lexer->buf + lexer->len;
}

void lex_floating(struct Lexer *lexer, int base) {
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
	int prn = 0;
	if (token->detail < TOKEN_DETAIL_NONE || token->detail >= TOKEN_DETAIL_NUM) __builtin_unreachable();
	switch (token->kind) {
	case TOKEN_NONE:
		prn += printf("%s", token_detail_to_str[TOKEN_DETAIL_NONE]);
		break;
	case TOKEN_KEYWORD:
		prn += printf("%s", token_detail_to_str[token->detail]);
		break;
	case TOKEN_IDENTIFIER:
		prn += printf("%s%.*s", token_detail_to_str[token->detail], token->len, token->start);
		break;
	case TOKEN_CONSTANT:
		switch (token->detail) {
		case TOKEN_DETAIL_INTEGER_CONSTANT:
			prn += printf("%s%ju", token_detail_to_str[token->detail], token->integer.value);
			break;
		case TOKEN_DETAIL_FLOATING_CONSTANT:
			prn += printf("%s%Lg", token_detail_to_str[token->detail], token->floating);
			break;
		case TOKEN_DETAIL_ENUMERATION_CONSTANT:
			prn += printf("%s%.*s", token_detail_to_str[token->detail], token->len, token->start);
			break;
		case TOKEN_DETAIL_CHARACTER_CONSTANT:
			prn += printf("%s'%c'", token_detail_to_str[token->detail], token->character);
			break;
		default:
			__builtin_unreachable();
		}
		break;
	case TOKEN_STRING_LITERAL:
		prn += printf("%s\"%.*s\"", token_detail_to_str[token->detail], token->string.len, token->string.start);
		break;
	case TOKEN_PUNCTUATOR:
		prn += printf("%s", token_detail_to_str[token->detail]);
		break;
	default:
		__builtin_unreachable();
	}
	return prn;
}

