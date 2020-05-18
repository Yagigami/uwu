#include "stream/stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STREAM_NAME (128)
#define MAX_STREAM_BUFFER (1024)

struct Stream {
	char name[MAX_STREAM_NAME];
	FILE *f;
};

Stream stream_init(const char *title, int mode) {
	char mode_str[3] = { [2] = '\0' };

	if      (mode & C_STREAM_READ  ) mode_str[0] = 'r';
	else if (mode & C_STREAM_WRITE ) mode_str[0] = 'w';
	if      (mode & C_STREAM_TEXT  ) mode_str[1] = '\0';
	else if (mode & C_STREAM_BINARY) mode_str[1] = 'b';

	struct Stream *stream = malloc(sizeof (*stream));
	if (!stream) goto alloc_fail;
	
	long l = strlen(title);
	if (l+1 > MAX_STREAM_BUFFER) goto release;
	strcpy(stream->name, title);
	stream->f = fopen(title, mode_str);
	if (!stream->f) goto release;

	return stream;
release:
	free(stream);
alloc_fail:
	return NULL;
}

void stream_fini(Stream stream) {
	if (stream) {
		fclose(stream->f);
		free(stream);
	}
}

const char *stream_name(Stream stream) {
	return stream->name;
}

long stream_read(Stream stream, char *buf, long size) {
	return fread(buf, 1, size, stream->f);
}

long stream_write(Stream stream, const char *buf, long size) {
	return fwrite(buf, 1, size, stream->f);
}

char *stream_getline(Stream stream, long *len) {
	char *last, *try, *nl;
	unsigned long cap = 8, new_cap = cap;
	try = calloc(1, new_cap);
	if (!try) goto error;
	fgets(try, cap, stream->f);
	while (!(nl = strchr(try, '\n'))) {
		new_cap = cap*2;
		last = try;
		try = calloc(1, new_cap);
		if (!try) {
			free(last);
			goto error;
		}
		memcpy(try, last, cap-1);
		free(last);
		if (!fgets(try+cap-1, cap+1, stream->f)) goto error;
		cap = new_cap;
	}
	if (len) *len = nl+1 - try;
	return try;
error:
	free(try);
	if (len) *len = 0;
	return NULL;
}

