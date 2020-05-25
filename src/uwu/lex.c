#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <inttypes.h>

#include "uwu/lex.h"
#include "stream/stream.h"
#include <intern/intern.h>
#include "common/data.h"
#include <stream/utf-8.h>

static enum TokenKind keyword_id(const uint8_t *str, ptrdiff_t len);

static inline bool is_token(struct Lexer *lexer, enum TokenKind kind) {
	return lexer->token.kind == kind;
}

static inline bool match_token(struct Lexer *lexer, enum TokenKind kind) {
	if (is_token(lexer, kind)) {
		lexer_next(lexer);
		return true;
	}
	return false;
}

static inline void expect_token(struct Lexer *lexer, enum TokenKind kind) {
	if (!match_token(lexer, kind)) {
		printf("expected token detail %d, got %d.\n", kind, lexer->token.kind);
		lexer->token.kind = TOKEN_NONE;
		lexer->cur = lexer->buf + lexer->len;
	}
}

int cp2hex(uint32_t c) {
	uint32_t l = tolower(c);
	if (l > '9') return l - 'a' + 0xa;
	return l - '0';
}

static bool is_valid_universal(uint32_t c);
static uint8_t xtoint(uint32_t x);
static uint32_t xstoint(const uint8_t *x, int n, uint8_t **endptr);
static uint32_t read_escape_sequence(const uint8_t *start, uint8_t **endptr);

static const uint8_t *lex_word(struct Lexer *lexer);
static const uint8_t *lex_number(struct Lexer *lexer);
static const uint8_t *lex_character(struct Lexer *lexer);
static const uint8_t *lex_string(struct Lexer *lexer);
static const uint8_t *lex_integer(struct Lexer *lexer, int base);
static const uint8_t *lex_floating(struct Lexer *lexer, int base);
static const uint8_t *lex_wide_character(struct Lexer *lexer);
static const uint8_t *lex_wide_string(struct Lexer *lexer);

uint32_t next_codepoint(const uint8_t *stream, uint8_t **endptr);

int lexer_init(struct Lexer *lexer, const char *name) {
	int ret = -1;
	if (!lexer) goto early;
	Stream stream = stream_init(name, C_STREAM_READ|C_STREAM_TEXT|C_STREAM_UTF_8);
	if (!stream) goto early;
	long size = stream_size(stream);
	lexer->buf = malloc(size+1);
	if (!lexer->buf) goto end;

	lexer->cur = lexer->buf;
	lexer->token.kind = TOKEN_NONE;
	if ((ret = intern_init(&lexer->identifiers))) goto end;
	lexer->len = size;
	ret = -1;

	if (stream_read(stream, lexer->buf, size) != size) goto end;
	lexer->buf[size] = '\0';
	ret = 0;
end:
	stream_fini(stream);
	if (ret != 0) free(lexer->buf);
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
	if (lexer->token.kind == TOKEN_STRING_LITERAL) {
		free(lexer->token.lit.sequence);
	}
	const uint8_t *end;
again:
#ifndef NDEBUG
	lexer->token.start = lexer->cur;
#endif
	;
	uint8_t *out;
	uint32_t cp = codepoint_utf_8(lexer->cur, &out);
	end = out;
	switch (cp) {
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
			if (*end == '.' && lexer->cur[1] == '.') {
				lexer->token.kind = TOKEN_ELLIPSIS;
				end += 2;
			} else {
				lexer->token.kind = TOKEN_DOT;
			}
		}
		break;

// for ~, !, =, ...
#define CASE1(k1, v1) \
	case k1: \
		end = lexer->cur + 1; \
		lexer->token.kind = TOKEN_ ## v1; \
		break

// for */*=, %,%=, ...
#define CASE2(k1, v1, k2, v2) \
	case k1: \
		end = lexer->cur + 1; \
		if (*end == k2) { \
			lexer->token.kind = TOKEN_ ## v2; \
			end++; \
		} else { \
			lexer->token.kind = TOKEN_ ## v1; \
		} \
		break

// for +/++/+=, ...
#define CASE3(k1, v1, k2, v2, k3, v3) \
	case k1: \
		end = lexer->cur + 1; \
		if (*end == k2) { \
			lexer->token.kind = TOKEN_ ## v2; \
			end++; \
		} else if (*end == k3) { \
			lexer->token.kind = TOKEN_ ## v3; \
			end++; \
		} else { \
			lexer->token.kind = TOKEN_ ## v1; \
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
		end = lexer->cur + 1;
		if (*end == '-') {
			lexer->token.kind = TOKEN_DECREMENT;
			end++;
		} else if (*end == '>') {
			lexer->token.kind = TOKEN_ARROW;
			end++;
		} else if (*end == '=') {
			lexer->token.kind = TOKEN_MINUS_ASSIGN;
			end++;
		} else {
			lexer->token.kind = TOKEN_MINUS;
		}
		break;
	case '<':
		end = lexer->cur + 1;
		if (*end == '<') {
			end++;
			if (*end == '=') {
				end++;
				lexer->token.kind = TOKEN_LSHIFT_ASSIGN;
			} else {
				lexer->token.kind = TOKEN_LSHIFT;
			}
		} else if (*end == '=') {
			end++;
			lexer->token.kind = TOKEN_LOWER_EQUAL;
		} else if (*end == ':') { // digraph
			end++;
			lexer->token.kind = TOKEN_LSQUARE_BRACKET;
		} else if (*end == '%') { // digraph
			end++;
			lexer->token.kind = TOKEN_LCURLY_BRACE;
		} else {
			lexer->token.kind = TOKEN_LOWER;
		}
		break;

#undef CASE3
#undef CASE2
#undef CASE1
	default:
		printf("unexpected character %lc (U+%04" PRIX32 ").\n", cp, cp);
		end = NULL;
		break;
	}
	if (!end) {
		lexer->token.kind = TOKEN_NONE;
		codepoint_utf_8(lexer->cur, &out);
		end = out;
	}
#ifndef NDEBUG
	lexer->token.len = end - lexer->token.start;
#endif
	lexer->cur = end;
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

enum TokenKind keyword_id(const uint8_t *str, ptrdiff_t len) {
	const struct Interns *kws = get_keyword_interns();
	const struct InternString *in = intern_find(kws, str, len);
	if (in) {
		return in - kws->interns + TOKEN_KEYWORD_START;
	}
	return TOKEN_NONE;
}

bool is_valid_universal(uint32_t c) {
	return (c >= 0x00A0 || c == 0x0024 || c == 0x0040 || c == 0x0060)
		&& !((c >= 0xD800 && c <= 0xDFFF) || c >= 0x10FFFF);
}

uint8_t xtoint(uint32_t x) {
	if (!isxdigit(x)) __builtin_unreachable();
	if (isdigit(x)) return x - '0';
	if (islower(x)) return x - 'a' + 0xa;
	return x - 'A' + 0xA;
}

uint32_t xstoint(const uint8_t *x, int n, uint8_t **endptr) {
	uint32_t value = 0;
	const uint8_t *cur = x;
	for (int i = 0; i < n; i++, cur++) {
		if (!isxdigit(*cur)) goto err;
		value = value * 16 | xtoint(*cur);
	}
	if (endptr) *endptr = (uint8_t *) cur;
	return value;
err:
	if (endptr) *endptr = (uint8_t *) x;
	return 0;
}

uint32_t read_escape_sequence(const uint8_t *start, uint8_t **endptr) {
	uint32_t value = 0;
	const uint8_t *end = start;
	uint8_t *out;
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
			value = value * 16 | cp2hex(*end);
		break;
	case 'u':
		// skip the u/U
		value = xstoint(start+1, 4, &out);
		if (out == start+1) {
			printf("universal character escape sequence needs 4 nibbles.\n");
		}
		if (!is_valid_universal(value)) {
			printf("code point %lc (U+%" PRIX32 ") is invalid.\n", value, value);
		}
		end = out;
		break;
	case 'U':
		value = xstoint(start+1, 8, &out);
		if (out == start+1) {
			printf("universal character escape sequence needs 8 nibbles.\n");
		}
		if (!is_valid_universal(value)) {
			printf("code point %lc (U+%" PRIX32 ") is invalid.\n", value, value);
		}
		end = out;
		break;
	default:
		printf("unknown escape sequence `\\%c`.\n", *end);
		break;
	}
	if (endptr) *endptr = (uint8_t *) end;
	return value;
}

const uint8_t *lex_word(struct Lexer *lexer) {
	const uint8_t *start = lexer->cur, *end = start;
	while (isalnum(*end) || *end == '_') end++;
	if (*end == '\0') {
		printf("end of file in identifier name.\n");
		return NULL;
	}
	long len = end - start;
	enum TokenKind id;
	if ((id = keyword_id(start, len)) != TOKEN_NONE) {
		lexer->token.kind = id;
	} else {
		lexer->token.kind = TOKEN_IDENTIFIER;
		lexer->token.ident.detail = intern_string(&lexer->identifiers, start, len);
		if (!lexer->token.ident.detail) {
			fprintf(stderr, "could not intern string `%.*s`.\n", (int) len, start);
			return start;
		}
	}
	return end;
}

const uint8_t *lex_number(struct Lexer *lexer) {
	int base = 10;
	const uint8_t *cur = lexer->cur;
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

uint32_t read_character(const uint8_t *ch, bool is_wide, uint8_t **endptr) {
	const uint8_t *start = ch, *end = start;
	assert(*end == '\'');
	end++;
	uint32_t value = 0;
	int cnt = 0;
	while (*end && *end != '\n' && *end != '\'') {
		value = value * 256 | *end;
		cnt++;
		if (*end++ != '\\') continue;
		uint8_t *out;
		uint32_t cp = read_escape_sequence(end, &out);
		if (out == end) goto err;
		end = out;
		if (!is_wide && cp >= 0x7F) {
			printf("universal character in character literal.\n");
		}
		value = cp;
	}
	if (cnt > 1) {
		printf("multiple characters inside of one character literal.\n");
	}
	if (*end == '\0') {
		printf("unexpected end of file in character literal.\n");
		goto err;
	} else if (*end == '\n') {
		printf("unexpected newline in character literal.\n");
		goto err;
	}
	assert(*end == '\'');
	end++;
	if (endptr) *endptr = (uint8_t *) end;
	return value;
err:
	if (endptr) *endptr = (uint8_t *) ch;
	return 0;
}

const uint8_t *lex_character(struct Lexer *lexer) {
	const uint8_t *start = lexer->cur;
	uint8_t *out;
	uint32_t value = read_character(start, false, &out);
	if (out == start) return NULL;
	lexer->token.kind = TOKEN_CHARACTER_CONSTANT;
	lexer->token.cst.kind = CONSTANT_CHARACTER;
	lexer->token.cst.character.prefix = CONSTANT_AFFIX_NONE;
	lexer->token.cst.character.value = value;
	return out;
}

ptrdiff_t string_lit_len_narrow(const uint8_t *str, uint32_t boundary, uint8_t **endptr) {
	assert(*str == boundary);
	const uint8_t *start = str + 1, *end = start;
	ptrdiff_t len = 0;
	while (*end && *end != '\n' && *end != boundary) {
		len++;
		if (*end++ != '\\') continue;
		uint8_t *out;
		uint32_t cp = read_escape_sequence(end, &out);
		if (out == end) goto err;
		end = out;
		len += len_character_utf_8(cp) - 1; // already incremented
	}
	if (*end == '\0' || *end == '\n') len = -1;
	assert(*end == boundary);
	end++;
	if (endptr) *endptr = (uint8_t *) end;
	return len;
err:
	if (endptr) *endptr = (uint8_t *) str;
	return -1;
}

void encode_narrow(uint8_t *out, uint32_t boundary, const uint8_t *in) {
	assert(*in == boundary);
	in++;
	for (uint8_t *c = out; *in != boundary;) {
		*c = *in;
		if (*in == '\0' || *in == '\n') __builtin_unreachable();
		if (*in++ != '\\') {
			c++;
			continue;
		}
		uint8_t *end;
		uint32_t cp = read_escape_sequence(in, &end);
		in = end;
		c += encode_utf_8(c, &cp, 1);
	}
}

const uint8_t *lex_string(struct Lexer *lexer) {
	const uint8_t *start = lexer->cur, *end;
	uint8_t *out;
	ptrdiff_t len = string_lit_len_narrow(start, '"', &out);
	if (len == -1) return NULL;
	end = out;
	lexer->token.kind = TOKEN_STRING_LITERAL;

	uint8_t *string = malloc(len + 1);
	if (!string) {
		fprintf(stderr, "could not interpret string of length %td.\n", len + 1);
		return NULL;
	}
	encode_narrow(string, '"', start);
	string[len] = '\0';
	lexer->token.lit.sequence = string;
	lexer->token.lit.len = len;
	lexer->token.lit.prefix = CONSTANT_AFFIX_NONE;
	return end;
}

uintmax_t read_integer(const uint8_t *i, int base, uint8_t **endptr) {
	uintmax_t val = 0;
	const uint8_t *c = i;
	for (; isxdigit(*c); c++) {
		int d = cp2hex(*c);
		if (d >= base) goto end;
		uintmax_t next = (val * base) + d;
		if (next < val) goto end;
		val = next;
	}
	if (endptr) *endptr = (uint8_t *) c;
	return val;
end:
	if (endptr) *endptr = (uint8_t *) i;
	return 0;
}

const uint8_t *lex_integer(struct Lexer *lexer, int base) {
	const uint8_t *start = lexer->cur, *end = start;
	if (base == 16) {
		end += 2;
	}
	uint8_t *out;
	uintmax_t val = read_integer(end, base, &out);
	if (out == end) return NULL;
	end = out;
	int usuffix = 0, lsuffix = 0, llsuffix = 0;
	for (uint32_t suff, it = 0; suff = tolower(*end), suff == 'u' || suff == 'l'; end++, it++) {
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

	lexer->token.kind = TOKEN_INTEGER_CONSTANT;
	lexer->token.cst.kind = CONSTANT_INTEGER;
	lexer->token.cst.integer.value = val;
	lexer->token.cst.integer.suffix = suffix;
	return end;
}

long double read_floating_exponent(const uint8_t *f, long double expbase, uint8_t **endptr) {
	const uint8_t *cur = f + 1; // skip e/E/p/P
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
		int i = cp2hex(*cur);
		if (i >= 10) goto err;
		uintmax_t next = e * 10 + i;
		if (next < e) {
			printf("exponent of floating constant overflows.\n");
		}
		e = next;
		cur++;
	}
	if (endptr) *endptr = (uint8_t *) cur;
	return powl(expbase, e * sign);
err:
	if (endptr) *endptr = (uint8_t *) f;
	return 0.0l;
}

long double read_floating_decimal(const uint8_t *f, uint8_t **endptr) {
	long double d = 0.0l, base = 10.0l;
	const uint8_t *cur = f;
	for (; isdigit(*cur); cur++)
		d = d * base + cp2hex(*cur); // no need to worry about overflow in floating point
	if (*cur == '.') {
		cur++;
		for (long double div = base; isdigit(*cur); div *= base, cur++)
			d += cp2hex(*cur) / div;
	}
	if (tolower(*cur) == 'e') {
		uint8_t *out;
		d *= read_floating_exponent(cur, 10.0l, &out);
		if (out == cur) goto err;
		cur = out;
	}
	if (endptr) *endptr = (uint8_t *) cur;
	return d;
err:
	if (endptr) *endptr = (uint8_t *) f;
	return 0.0l;
}

long double read_floating_hexadecimal(const uint8_t *f, uint8_t **endptr) {
	long double d = 0.0l, base = 16.0l;
	const uint8_t *cur = f;
	for (; isxdigit(*cur); cur++)
		d = d * base + cp2hex(*cur);
	if (*cur == '.') {
		cur++;
		for (long double div = base; isxdigit(*cur); div *= base, cur++)
			d += cp2hex(*cur) / div;
	}
	if (tolower(*cur) == 'p') {
		uint8_t *out;
		d *= read_floating_exponent(cur, 2.0l, &out);
		if (out == cur) goto err;
		cur = out;
	}
	if (endptr) *endptr = (uint8_t *) cur;
	return d;
err:
	if (endptr) *endptr = (uint8_t *) f;
	return 0.0l;
}

const uint8_t *lex_floating(struct Lexer *lexer, int base) {
	const uint8_t *start = lexer->cur, *end = start;
	uint8_t *out;
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
	for (uint32_t suff, it = 0; suff = tolower(*end), suff == 'l' || suff == 'f'; end++, it++) {
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

	lexer->token.kind = TOKEN_FLOATING_CONSTANT;
	lexer->token.cst.kind = CONSTANT_FLOATING;
	lexer->token.cst.floating.value = val;
	lexer->token.cst.floating.suffix = suffix;
	return end;
}

const uint8_t *lex_wide_character(struct Lexer *lexer) {
	const uint8_t *start = lexer->cur + 1;
	assert(start[-1] == 'L');
	uint8_t *out;
	uint32_t value = read_character(start, true, &out);
	if (out == start) return NULL;
	lexer->token.kind = TOKEN_CHARACTER_CONSTANT;
	lexer->token.cst.kind = CONSTANT_CHARACTER;
	lexer->token.cst.character.value = value;
	lexer->token.cst.character.prefix = CONSTANT_AFFIX_L;
	return out;
}

ptrdiff_t string_lit_len_wide(const uint8_t *str, uint32_t boundary, uint8_t **endptr) {
	assert(*str == boundary);
	const uint8_t *start = str + 1, *end = start;
	ptrdiff_t len = 0;
	while (*end && *end != '\n' && *end != boundary) {
		len++;
		uint8_t *out;
		if (*end > 0x7F) {
			codepoint_utf_8(end, &out);
			end = out;
			continue;
		}
		if (*end++ != '\\') continue;
		read_escape_sequence(end, &out);
		end = out;
	}
	if (*end == '\0' || *end == '\n') len = -1;
	assert(*end == boundary);
	end++;
	if (endptr) *endptr = (uint8_t *) end;
	return len;
}

void encode_wide(uint32_t *out, uint32_t boundary, const uint8_t *in) {
	assert(*in == boundary);
	in++;
	for (uint32_t *c = out, cp; *in != boundary; c++) {
		*c = *in;
		uint8_t *end;
		if (*in > 0x7F) {
			cp = codepoint_utf_8(in, &end);
			in = end;
			*c = cp;
			continue;
		}
		if (*in == '\0' || *in == '\n') __builtin_unreachable();
		if (*in++ != '\\') continue;
		cp = read_escape_sequence(in, &end);
		in = end;
		*c = cp;
	}
}

const uint8_t *lex_wide_string(struct Lexer *lexer) {
	const uint8_t *start = lexer->cur + 1, *end;
	uint8_t *out;
	assert(start[-1] == 'L');
	ptrdiff_t len = string_lit_len_wide(start, '"', &out);
	if (len == -1) return NULL;
	end = out;
	lexer->token.kind = TOKEN_STRING_LITERAL;

	uint32_t *string = malloc((len + 1) * sizeof (*string));
	if (!string) {
		fprintf(stderr, "could not interpret string of length %td.\n", (len + 1) * sizeof (*string));
		return NULL;
	}
	encode_wide(string, '"', start);
	string[len] = '\0';
	lexer->token.lit.sequence = string;
	lexer->token.lit.len = len;
	lexer->token.lit.prefix = CONSTANT_AFFIX_L;
	return end;
}

int print_token(const struct Token *token) {
	int prn = 0;
	if (token->kind < TOKEN_NONE || token->kind >= TOKEN_END) __builtin_unreachable();
	switch (token->kind) {
	case TOKEN_NONE:
		prn += printf("%s", token2str[TOKEN_NONE]);
		break;
	case TOKEN_KEYWORD_START ... TOKEN_KEYWORD_END - 1:
		prn += printf("%s", token2str[token->kind]);
		break;
	case TOKEN_IDENTIFIER:
		prn += printf("%s", token2str[token->kind]);
		prn += print_intern(token->ident.detail);
		break;
	case TOKEN_INTEGER_CONSTANT:
		prn += printf("%s%ju%s",
				token2str[token->kind],
				token->cst.integer.value,
				affix2str[token->cst.integer.suffix]
		);
		break;
	case TOKEN_FLOATING_CONSTANT:
		prn += printf("%s%.9Lg%s",
				token2str[token->kind],
				token->cst.floating.value,
				affix2str[token->cst.floating.suffix]
		);
		break;
	case TOKEN_ENUMERATION_CONSTANT:
		prn += printf("%s", token2str[token->kind]);
		prn += print_intern(token->ident.detail);
		break;
	case TOKEN_CHARACTER_CONSTANT:
		prn += printf(token->cst.character.prefix == CONSTANT_AFFIX_L ? "%sL'%lc'": "%s'%c'",
				token2str[token->kind],
				token->cst.character.value
		);
		break;
	case TOKEN_STRING_LITERAL:
		if (token->lit.prefix == CONSTANT_AFFIX_NONE) {
			prn += printf("%s\"%.*s\"",
					token2str[token->kind],
					(int) token->lit.len,
					(char *) token->lit.sequence
			);
		} else if (token->lit.prefix == CONSTANT_AFFIX_L) {
			prn += printf("%s\"%.*ls\"",
					token2str[token->kind],
					(int) encode_utf_8_len(token->lit.sequence, NULL),
					(uint32_t *) token->lit.sequence
			);
		} else __builtin_unreachable();
		break;
	case TOKEN_PUNCTUATOR_START ... TOKEN_PUNCTUATOR_END - 1:
		prn += printf("%s", token2str[token->kind]);
		break;
	case TOKEN_KEYWORD_END:
	case TOKEN_CONSTANT_END:
	case TOKEN_PUNCTUATOR_END:
	case TOKEN_END:
	default:
		__builtin_unreachable();
	}
	return prn;
}

