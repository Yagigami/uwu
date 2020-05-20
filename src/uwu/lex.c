#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "uwu/lex.h"
#include "stream/stream.h"

static const struct Interns *_get_keyword_interns(void);

int intern_init(struct Interns *interns) {
	if (!interns) return -1;
	interns->len = interns->cap = 0;
	interns->interns = NULL;
	return 0;
}

void intern_fini(struct Interns *interns) {
	if (!interns) return;
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
		long cap = interns->cap*2;
		void *tmp = realloc(interns->interns, cap);
		if (!tmp) return NULL;
		interns->interns = tmp;
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
	lexer->keywords = _get_keyword_interns();
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
	(void) lexer;
}

static const struct Interns *_get_keyword_interns(void) {
	return NULL;
}
