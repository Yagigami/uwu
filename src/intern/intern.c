#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

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
	for (ptrdiff_t i = 0; i < interns->len; i++) {
		free(interns->interns[i].str);
	}
	free(interns->interns);
}

const struct InternString *intern_string(struct Interns *interns, const uint8_t *str, ptrdiff_t len) {
	const struct InternString *in = intern_find(interns, str, len);
	if (in) return in;
	if (interns->len+1 > interns->cap) {
		ptrdiff_t cap = interns->cap*2+1;
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

const struct InternString *intern_find(const struct Interns *interns, const uint8_t *str, ptrdiff_t len) {
	for (ptrdiff_t i=0; i<interns->len; i++) {
		const struct InternString *intern = interns->interns+i;
		if (len == intern->len && memcmp(str, intern->str, len) == 0) {
			return intern;
		}
	}
	return NULL;
}

bool intern_contains(const struct Interns *interns, const uint8_t *str, ptrdiff_t len) {
	return intern_find(interns, str, len);
}

int print_interns(const struct Interns *interns) {
	int total = 0, printed = 0, stop = 80;
	if (interns) for (ptrdiff_t i = 0; i < interns->len; i++) {
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
	static const struct InternString kws[TOKEN_KEYWORD_END - TOKEN_KEYWORD_START] = {
#define ELEM(a, b) \
	[a - TOKEN_KEYWORD_START] = { \
		.str = (uint8_t *) # b, \
		.len = sizeof (# b)-1, \
	}
// sizeof (# b)-1 is like strlen but we need a constant expression
		ELEM(TOKEN_AUTO     , auto      ),
		ELEM(TOKEN_BREAK    , break     ),
		ELEM(TOKEN_CASE     , case      ),
		ELEM(TOKEN_CHAR     , char      ),
		ELEM(TOKEN_CONST    , const     ),
		ELEM(TOKEN_CONTINUE , continue  ),
		ELEM(TOKEN_DEFAULT  , default   ),
		ELEM(TOKEN_DO       , do        ),
		ELEM(TOKEN_DOUBLE   , double    ),
		ELEM(TOKEN_ELSE     , else      ),
		ELEM(TOKEN_ENUM     , enum      ),
		ELEM(TOKEN_EXTERN   , extern    ),
		ELEM(TOKEN_FLOAT    , float     ),
		ELEM(TOKEN_FOR      , for       ),
		ELEM(TOKEN_GOTO     , goto      ),
		ELEM(TOKEN_IF       , if        ),
		ELEM(TOKEN_INLINE   , inline    ),
		ELEM(TOKEN_INT      , int       ),
		ELEM(TOKEN_LONG     , long      ),
		ELEM(TOKEN_REGISTER , register  ),
		ELEM(TOKEN_RESTRICT , restrict  ),
		ELEM(TOKEN_RETURN   , return    ),
		ELEM(TOKEN_SHORT    , short     ),
		ELEM(TOKEN_SIGNED   , signed    ),
		ELEM(TOKEN_SIZEOF   , sizeof    ),
		ELEM(TOKEN_STATIC   , static    ),
		ELEM(TOKEN_STRUCT   , struct    ),
		ELEM(TOKEN_SWITCH   , switch    ),
		ELEM(TOKEN_TYPEDEF  , typedef   ),
		ELEM(TOKEN_UNION    , union     ),
		ELEM(TOKEN_UNSIGNED , unsigned  ),
		ELEM(TOKEN_VOID     , void      ),
		ELEM(TOKEN_VOLATILE , volatile  ),
		ELEM(TOKEN_WHILE    , while     ),
		ELEM(TOKEN_BOOL     , _Bool     ),
		ELEM(TOKEN_COMPLEX  , _Complex  ),
		ELEM(TOKEN_IMAGINARY, _Imaginary),
#undef ELEM
	};
	static const struct Interns interns = {
		.interns = (struct InternString *) kws, // hope no one tries to modify it :)
		.len = sizeof (kws) / sizeof (*kws),
		.cap = 0,
	};
	return &interns;
}

