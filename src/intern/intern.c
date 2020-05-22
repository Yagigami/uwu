#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "intern/intern.h"
#include <uwu/uwu.h>

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

const struct InternString *intern_string(struct Interns *interns, const uint8_t *str, long len) {
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

const struct InternString *intern_find(const struct Interns *interns, const uint8_t *str, long len) {
	for (long i=0; i<interns->len; i++) {
		const struct InternString *intern = interns->interns+i;
		if (len == intern->len && memcmp(str, intern->str, len) == 0) {
			return intern;
		}
	}
	return NULL;
}

bool intern_contains(const struct Interns *interns, const uint8_t *str, long len) {
	return intern_find(interns, str, len);
}

int print_interns(const struct Interns *interns) {
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

int print_intern(const struct InternString *intern) {
	return printf("<%.*s> ", (int)intern->len, intern->str);
}

const struct Interns *get_keyword_interns(void) {
	static const struct InternString kws[TOKEN_DETAIL_KEYWORDS_END-TOKEN_DETAIL_KEYWORDS_START] = {
#define ELEM(a, b) \
	[TOKEN_DETAIL_KEYWORD_ ## a - TOKEN_DETAIL_KEYWORDS_START] = { \
		.str = (uint8_t *) # b, \
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

