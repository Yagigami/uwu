#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

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

int char2hex(char c) {
	char l = tolower(c);
	if (l > '9') return l - 'a' + 0xa;
	return l - '0';
}

static bool is_valid_universal(uint32_t c);
static uint8_t xtoint(char x);
static uint32_t xstoint(char *x, int n, char **endptr);
static uint32_t read_escape_sequence(char *start, char **endptr);

static char *lex_word(struct Lexer *lexer);
static char *lex_number(struct Lexer *lexer);
static char *lex_character(struct Lexer *lexer);
static char *lex_string(struct Lexer *lexer);
static char *lex_integer(struct Lexer *lexer, int base);
static char *lex_floating(struct Lexer *lexer, int base);
static char *lex_wide_character(struct Lexer *lexer);
static char *lex_wide_string(struct Lexer *lexer);

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

enum LexerStatus lexer_next(struct Lexer *lexer) {
	if (!lexer->cur) goto empty;
	if (lexer->token.detail == TOKEN_DETAIL_STRING_LITERAL) {
		free(lexer->token.string.start);
	}
	char *end;
again:
	if ((uint8_t) *lexer->cur >= 0x80) {
		char *s = lexer->cur;
		while ((uint8_t) *lexer->cur >= 0x80) lexer->cur++;
		printf("non ASCII character in input: '%.*s', skipping.\n", (int)(lexer->cur - s), s);
	}
	lexer->token.start = lexer->cur;
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
		return LEXER_END;
	case 'L':
		if (lexer->cur[1] == '\'') end = lex_wide_character(lexer);
		else if (lexer->cur[1] == '"') end = lex_wide_string(lexer);
		else end = lex_word(lexer);
		break;
	case 'a' ... 'z':
	case 'A' ... 'L'-1: // L is special
	case 'L'+1 ... 'Z':
	case '_':
		end = lex_word(lexer);
		break;
	case '0' ... '9':
		end = lex_number(lexer);
		break;
	case '\'':
		end = lex_character(lexer);
		break;
	case '"':
		end = lex_string(lexer);
		break;
	case '.': // can also be the start of a decimal floating constant
		if (isdigit(lexer->cur[1])) {
			end = lex_floating(lexer, 10);
		} else {
			end = lexer->cur;
			lexer->token.kind = TOKEN_PUNCTUATOR;
			if (*end == '.' && lexer->cur[1] == '.') {
				lexer->token.detail = TOKEN_DETAIL_ELLIPSIS;
				end += 2;
			} else {
				lexer->token.detail = TOKEN_DETAIL_DOT;
			}
		}
		break;

// for ~, !, =, ...
#define CASE1(k1, v1) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		end = lexer->cur + 1; \
		lexer->token.detail = TOKEN_DETAIL_ ## v1; \
		break

// for */*=, %,%=, ...
#define CASE2(k1, v1, k2, v2) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		end = lexer->cur + 1; \
		if (*end == k2) { \
			lexer->token.detail = TOKEN_DETAIL_ ## v2; \
			end++; \
		} else { \
			lexer->token.detail = TOKEN_DETAIL_ ## v1; \
		} \
		break

// for +/++/+=, ...
#define CASE3(k1, v1, k2, v2, k3, v3) \
	case k1: \
		lexer->token.kind = TOKEN_PUNCTUATOR; \
		end = lexer->cur + 1; \
		if (*end == k2) { \
			lexer->token.detail = TOKEN_DETAIL_ ## v2; \
			end++; \
		} else if (*end == k3) { \
			lexer->token.detail = TOKEN_DETAIL_ ## v3; \
			end++; \
		} else { \
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
		end = lexer->cur + 1;
		if (*end == '-') {
			lexer->token.detail = TOKEN_DETAIL_DECREMENT;
			end++;
		} else if (*end == '>') {
			lexer->token.detail = TOKEN_DETAIL_ARROW;
			end++;
		} else if (*end == '=') {
			lexer->token.detail = TOKEN_DETAIL_MINUS_ASSIGN;
			end++;
		} else {
			lexer->token.detail = TOKEN_DETAIL_MINUS;
		}
		break;
	case '<':
		lexer->token.kind = TOKEN_PUNCTUATOR;
		end = lexer->cur + 1;
		if (*end == '<') {
			end++;
			if (*end == '=') {
				end++;
				lexer->token.detail = TOKEN_DETAIL_LSHIFT_ASSIGN;
			} else {
				lexer->token.detail = TOKEN_DETAIL_LSHIFT;
			}
		} else if (*end == '=') {
			end++;
			lexer->token.detail = TOKEN_DETAIL_LOWER_EQUAL;
		} else if (*end == ':') { // digraph
			end++;
			lexer->token.detail = TOKEN_DETAIL_LSQUARE_BRACKET;
		} else if (*end == '%') { // digraph
			end++;
			lexer->token.detail = TOKEN_DETAIL_LCURLY_BRACE;
		} else {
			lexer->token.detail = TOKEN_DETAIL_LOWER;
		}
		break;

#undef CASE3
#undef CASE2
#undef CASE1
	default:
		printf("unexpected character '%c' (ASCII %d) in program.\n", *lexer->cur, *lexer->cur);
		end = NULL;
		break;
	}
	if (end) {
		lexer->token.len = end - lexer->token.start;
		lexer->cur = end;
	} else {
		lexer->token.kind = TOKEN_NONE;
		lexer->token.detail = TOKEN_DETAIL_NONE;
		lexer->token.len = 1;
		lexer->cur++;
	}
	printf("at token: ");
	print_token(&lexer->token);
	printf("\n");
	return LEXER_VALID;
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
	char *cur = x;
	for (int i = 0; i < n; i++, cur++) {
		if (!isxdigit(*cur)) goto err;
		value = value * 16 | xtoint(*cur);
	}
	if (endptr) *endptr = cur;
	return value;
err:
	if (endptr) *endptr = x;
	return 0;
}

uint32_t read_escape_sequence(char *start, char **endptr) {
	uint32_t value = 0;
	char *end = start;
	switch (*end) {
	case '\'':
	case '"':
	case '?':
	case '\\':
		value = *end++;
		break;
	case 'a':
		value = '\a';
		end++;
		break;
	case 'b':
		value = '\b';
		end++;
		break;
	case 'f':
		value = '\f';
		end++;
		break;
	case 'n':
		value = '\n';
		end++;
		break;
	case 'r':
		value = '\r';
		end++;
		break;
	case 't':
		value = '\t';
		end++;
		break;
	case 'v':
		value = '\v';
		end++;
		break;
	case '0' ... '7':
		for (int i = 0; i < 3 && *end >= '0' && *end <= '7'; i++, end++)
			value = value * 8 | (*end - '0');
		break;
	case 'x':
		// having 1 character is okay here
		for (int i = 0; i < 2 && isxdigit(*end); i++, end++)
			value = value * 16 | char2hex(*end);
		break;
	case 'u':
		// skip the u/U
		value = xstoint(start+1, 4, &end);
		if (end == start+1) {
			printf("universal character escape sequence needs 4 nibbles.\n");
		}
		break;
	case 'U':
		value = xstoint(start+1, 8, &end);
		if (end == start+1) {
			printf("universal character escape sequence needs 8 nibbles.\n");
		}
		break;
	default:
		printf("unknown escape sequence `\\%c`.\n", *end);
		break;
	}
	if (endptr) *endptr = end;
	return value;
}

static char *lex_word(struct Lexer *lexer) {
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
			return start;
		}
	}
	lexer->token.start = start;
	lexer->token.len = len;
	return end;
}

char *lex_number(struct Lexer *lexer) {
	int base = 10;
	char *cur = lexer->cur;
	if (*cur++ == '0') {
		if (tolower(*cur) == 'x') {
			base = 16;
			cur++;
			while (isxdigit(*cur)) cur++;
		} else {
			base = 8;
			while (*cur >= '0' && *cur <= '7') cur++;
		}
	} else {
		while (isdigit(*cur)) cur++;
	}
	if (*cur == '.' || tolower(*cur) == 'e' || tolower(*cur) == 'p') {
		return lex_floating(lexer, base);
	}
	return lex_integer(lexer, base);
}

char *lex_character(struct Lexer *lexer) {
	char *start = lexer->cur, *end = start + 1;
	uint32_t value = 0;
	if (*start != '\'') __builtin_unreachable();
	while (*end && *end != '\n' && *end != '\'') {
		value <<= 8;
		value |= *end;
		if (*end++ != '\\') continue;
		char *out = end;
		value = read_escape_sequence(end, &out);
		if (out == end) return NULL;
	}
	if (*end == '\0') {
		printf("unexpected end of file in character literal.\n");
		return NULL;
	} else if (*end == '\n') {
		printf("unexpected newline in character literal.\n");
		return NULL;
	}
	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_CHARACTER_CONSTANT;
	lexer->token.character = value;
	return end;
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
	assert(*end == '"');
	if (endptr) *endptr = end + 1;
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

char *lex_string(struct Lexer *lexer) {
	char *start = lexer->cur, *end;
	ptrdiff_t len = string_lit_len_narrow(start, &end);
	lexer->token.kind = TOKEN_STRING_LITERAL;
	lexer->token.detail = TOKEN_DETAIL_STRING_LITERAL;
	if (len == -1) return NULL;

	char *string = malloc(len+1);
	if (!string) {
		fprintf(stderr, "could not interpret string of length %ld.\n", len+1);
		return NULL;
	}
	encode_narrow(string, start);
	string[len] = '\0';
	lexer->token.string.start = string;
	lexer->token.string.len = len;
	return end;
}

uintmax_t read_integer(char *i, int base, char **endptr) {
	uintmax_t val = 0;
	char *c = i;
	for (; isxdigit(*c); c++) {
		int d = char2hex(*c);
		if (d >= base) goto end;
		uintmax_t next = (val * base) + d;
		if (next < val) goto end;
		val = next;
	}
	if (endptr) *endptr = c;
	return val;
end:
	if (endptr) *endptr = i;
	return 0;
}

char *lex_integer(struct Lexer *lexer, int base) {
	char *start = lexer->cur, *end = start;
	if (base == 16) {
		end += 2;
	}
	char *out;
	uintmax_t val = read_integer(end, base, &out);
	if (out == end) return NULL;
	end = out;
	int usuffix = 0, lsuffix = 0, llsuffix = 0;
	for (char suff, it = 0; suff = tolower(*end), suff == 'u' || suff == 'l'; end++, it++) {
		if (it > 4) return NULL; // no suffix is that long, but just incase of a degenerate case
		if (suff == 'u') {
			usuffix++;
		} else if (suff == 'l') {
			if (tolower(end[1]) == 'l') {
				end++;
				llsuffix++;
			} else {
				lsuffix++;
			}
		}
	}
	enum ConstantAffix suffix;
	if      (usuffix == 0 && lsuffix == 0 && llsuffix == 0) suffix = CONSTANT_AFFIX_NONE;
	else if (usuffix == 1 && lsuffix == 0 && llsuffix == 0) suffix = CONSTANT_AFFIX_U;
	else if (usuffix == 0 && lsuffix == 1 && llsuffix == 0) suffix = CONSTANT_AFFIX_L;
	else if (usuffix == 0 && lsuffix == 0 && llsuffix == 1) suffix = CONSTANT_AFFIX_LL;
	else if (usuffix == 1 && lsuffix == 1 && llsuffix == 0) suffix = CONSTANT_AFFIX_UL;
	else if (usuffix == 1 && lsuffix == 0 && llsuffix == 1) suffix = CONSTANT_AFFIX_ULL;
	else {
		printf("invalid integer constant suffix : '%.*s'.\n", (int)(end - out), out);
		return NULL;
	}

	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_INTEGER_CONSTANT;
	lexer->token.integer = val;
	lexer->token.affix = suffix;
	return end;
}

long double read_floating_exponent(char *f, long double expbase, char **endptr) {
	char *cur = f + 1; // skip e/E/p/P
	long double sign = +1.0l;
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		cur++;
		sign = -1.0l;
	}

	// cannot use read_integer because 10f is a valid decimal exponent but not integer
	uintmax_t e = 0;
	while (isdigit(*cur)) {
		int i = char2hex(*cur);
		if (i >= 10) goto err;
		uintmax_t next = e * 10 + i;
		if (next < e) {
			printf("exponent of floating constant overflows.\n");
		}
		e = next;
		cur++;
	}
	if (endptr) *endptr = cur;
	return powl(expbase, e * sign);
err:
	if (endptr) *endptr = f;
	return 0.0l;
}

long double read_floating_decimal(char *f, char **endptr) {
	long double d = 0.0l, base = 10.0l;
	char *cur = f;
	for (; isdigit(*cur); cur++)
		d = d * base + char2hex(*cur); // no need to worry about overflow in floating point
	if (*cur == '.') {
		cur++;
		for (long double div = base; isdigit(*cur); div *= base, cur++)
			d += char2hex(*cur) / div;
	}
	if (tolower(*cur) == 'e') {
		char *out = cur;
		d *= read_floating_exponent(cur, 10.0l, &out);
		if (out == cur) goto err;
		cur = out;
	}
	if (endptr) *endptr = cur;
	return d;
err:
	if (endptr) *endptr = f;
	return 0.0l;
}

long double read_floating_hexadecimal(char *f, char **endptr) {
	long double d = 0.0l, base = 16.0l;
	char *cur = f;
	for (; isxdigit(*cur); cur++)
		d = d * base + char2hex(*cur);
	if (*cur == '.') {
		cur++;
		for (long double div = base; isxdigit(*cur); div *= base, cur++)
			d += char2hex(*cur) / div;
	}
	if (tolower(*cur) == 'p') {
		char *out = cur;
		d *= read_floating_exponent(cur, 2.0l, &out);
		if (out == cur) goto err;
		cur = out;
	}
	if (endptr) *endptr = cur;
	return d;
err:
	if (endptr) *endptr = f;
	return 0.0l;
}

char *lex_floating(struct Lexer *lexer, int base) {
	char *start = lexer->cur, *end = start;
	char *out = end;
	long double val;
	if (base == 10) {
		val = read_floating_decimal(end, &out);
	} else if (base == 16) {
		end += 2; // skip 0x
		val = read_floating_hexadecimal(end, &out);
	}
	if (out == end) return NULL;
	end = out;

	int lsuffix = 0, fsuffix = 0;
	for (char suff, it = 0; suff = tolower(*end), suff == 'l' || suff == 'f'; end++, it++) {
		if (it > 4) return NULL;
		if (suff == 'l') lsuffix++;
		else             fsuffix++;
	}
	enum ConstantAffix suffix;
	if      (lsuffix == 0 && fsuffix == 0) suffix = CONSTANT_AFFIX_NONE;
	else if (lsuffix == 1 && fsuffix == 0) suffix = CONSTANT_AFFIX_L;
	else if (lsuffix == 0 && fsuffix == 1) suffix = CONSTANT_AFFIX_F;
	else {
		printf("invalid floating constant suffix : '%.*s'.\n", (int)(end - out), out);
		return NULL;
	}

	lexer->token.kind = TOKEN_CONSTANT;
	lexer->token.detail = TOKEN_DETAIL_FLOATING_CONSTANT;
	lexer->token.affix = suffix;
	lexer->token.floating = val;
	return end;
}

char *lex_wide_character(struct Lexer *lexer) {
	(void) lexer;
	return NULL;
}

char *lex_wide_string(struct Lexer *lexer) {
	(void) lexer;
	return NULL;
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
			prn += printf("%s%ju%s",
					token_detail_to_str[token->detail],
					token->integer,
					constant_suffix_to_str[token->affix]
			);
			break;
		case TOKEN_DETAIL_FLOATING_CONSTANT:
			prn += printf("%s%.9Lg%s", token_detail_to_str[token->detail], token->floating, constant_suffix_to_str[token->affix]);
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

