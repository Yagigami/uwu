#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stream/stream.h"
#include "pp/translate.h"
#include "pp/pre.h"

Stream preprocess(Stream stream) {
	Stream output=NULL;
	long len;
	const char *base = stream_basename(stream, &len);
	char *name = malloc(len+sizeof(".i"));
	if (!name) goto end;
	memcpy(name, base, len);
	strcpy(name+len, ".i");
	output = stream_init(name, C_STREAM_WRITE|C_STREAM_TEXT);
	free(name);
	if (!output) goto end;

	char *ppline;
	long pplen, linecnt=0, jmps;
	while ((ppline = discard_bsnl(stream, &pplen, &jmps))) {
		printf("%ld:\t%.*s", linecnt, (int)pplen, ppline);
		stream_write(output, ppline, pplen);
		linecnt += jmps;
		free(ppline);
	}

end:
	stream_fini(stream);
	return output;
}

