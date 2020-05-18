#include <stdlib.h>
#include <stdio.h>

#include "pp/tests.h"
#include "pp/pre.h"
#include "stream/stream.h"

void pp_test(void) {
	printf("pp:\n");
	Stream stream = stream_init("foo.c", C_STREAM_READ|C_STREAM_TEXT);
	if (!stream) {
		fprintf(stderr, "could not initialize stream from `%s`.\n", "foo.c");
	}
	char *ppline;
	long pplen, linecnt=0, jmps;

	while ((ppline = discard_bsnl(stream, &pplen, &jmps))) {
		printf("%ld:\t%.*s", linecnt, (int)pplen, ppline);
		linecnt += jmps;
		free(ppline);
	}
	stream_fini(stream);
}

