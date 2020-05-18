#include "stream/stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STREAM_NAME (128)

struct Stream {
	char *name;
	char *ext;
	long len;
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
	stream->name = malloc(l+1);
	if (!stream->name) goto copy_fail;
	strcpy(stream->name, title);
	stream->len = l;

	stream->f = fopen(title, mode_str);
	if (!stream->f) goto release;

	char *dot = strrchr(stream->name, '.');
	stream->ext = dot && dot != stream->name ? dot: stream->name+l;

	return stream;
release:
	free(stream->name);
copy_fail:
	free(stream);
alloc_fail:
	return NULL;
}

void stream_fini(Stream stream) {
	if (stream) {
		fclose(stream->f);
		free(stream->name);
		free(stream);
	}
}

const char *stream_name(Stream stream, long *len) {
	if (len) *len = stream->len;
	return stream->name;
}

const char *stream_basename(Stream stream, long *len) {
	if (len) *len = stream->ext - stream->name;
	return stream->name;
}

const char *stream_extension(Stream stream, long *len) {
	if (len) *len = stream->len - (stream->ext - stream->name);
	return stream->ext;
}

long stream_read(Stream stream, char *buf, long size) {
	return fread(buf, 1, size, stream->f);
}

long stream_write(Stream stream, const char *buf, long size) {
	return fwrite(buf, 1, size, stream->f);
}

char *stream_getline(Stream stream, long *len) {
	char *try = NULL, *last, *nl;
	unsigned long cap, new_cap = 8;
	try = malloc(new_cap);
	if (!try) goto early;
	if (!fgets(try, new_cap, stream->f)) goto eof;
	while (!(nl = strchr(try, '\n'))) {
		last = try;
		cap = new_cap;
		try = malloc(new_cap = cap*2);
		if (!try) goto nomem;
		memcpy(try, last, cap-1);
		free(last);
		if (!fgets(try+cap-1, cap+1, stream->f)) goto eof;
	}
	if (len) *len = nl+1-try;
	return try;
nomem:
	free(last);
eof:
	free(try);
early:
	if (len) *len = 0;
	return NULL;
}

