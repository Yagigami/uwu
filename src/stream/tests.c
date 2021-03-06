#include "stream/stream.h"

#include <stdio.h>
#include <stdlib.h>

int stream_test(void) {
	printf("stream:\n");
	Stream stream = stream_init("foo.c", C_STREAM_READ | C_STREAM_TEXT);
	if (!stream) {
		fprintf(stderr, "couldn't initialize stream from `%s`.\n", "foo.c");
	}
	char *line;
	const char *name;
	long len;
	name = stream_name(stream, &len);
	printf("<%.*s>\n", (int)len, name);
	name = stream_basename(stream, &len);
	printf("<%.*s>\n", (int)len, name);
	name = stream_extension(stream, &len);
	printf("<%.*s>\n", (int)len, name);
	while ((line = stream_getline(stream, &len))) {
		printf("[%-4zd]\t%.*s\n", len, (int)(len-1), line);
		free(line);
	}
	stream_fini(stream);
	return 0;
}

