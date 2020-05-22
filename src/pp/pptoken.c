#include "pp/pptoken.h"
#include "pp/common.h"
#include "pp/pre.h"
#include "stream/stream.h"

#include <stdlib.h>

int preprocessor_init(struct Preprocessor *pp, const char *name) {
	int ret = -1;
	Stream stream = stream_init(name, C_STREAM_READ|C_STREAM_TEXT);
	if (!stream) goto early;
	long size = stream_size(stream);
	pp->buf = malloc(size+1);
	if (!pp->buf) goto io;
	if (stream_read(stream, pp->buf, size) != size) goto io;
	pp->buf[size] = '\0';
	pp->len = size;
	ret = (pp->buf = preprocessor_internalize(pp->buf, &pp->len)) ? 0: -1;
	pp->cur = pp->buf;
	// pp->pptoken.kind = PREPROCESSING_TOKEN_NONE;
io:
	stream_fini(stream);
	if (ret != 0) free(pp->buf);
early:
	return ret;
}

int preprocessor_fini(struct Preprocessor *pp, const char *name) {
	int ret = -1;
	if (!name) goto early;
	Stream stream = stream_init(name, C_STREAM_WRITE|C_STREAM_TEXT);
	if (!stream) goto early;
	if (stream_write(stream, pp->buf, pp->len) != pp->len) goto w;
	ret = 0;
w:
	stream_fini(stream);
early:
	free(pp->buf);
	return ret;
}

