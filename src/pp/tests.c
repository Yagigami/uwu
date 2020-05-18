#include <stdlib.h>
#include <stdio.h>

#include "pp/tests.h"
#include "pp/translate.h"
#include "stream/stream.h"

void pp_test(void) {
	printf("pp:\n");
	Stream stream = stream_init("foo.c", C_STREAM_READ|C_STREAM_TEXT);
	if (!stream) {
		fprintf(stderr, "could not initialize stream from `%s`.\n", "foo.c");
	}
	Stream out = preprocess(stream);
	if (!out) {
		fprintf(stderr, "could not preprocess stream from `%s`.\n", "foo.c");
	}
	stream_fini(out);
}

