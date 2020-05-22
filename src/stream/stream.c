#include "stream/stream.h"
#include "stream/utf-8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_STREAM_NAME (128)

enum StreamEncoding {
	ENC_UTF_8,
	ENC_DEFAULT = ENC_UTF_8,
};

struct Stream {
	char *name;
	char *ext;
	long len;
	long size;
	FILE *f;
	enum StreamEncoding enc;
};

Stream stream_init(const char *title, int mode) {
	char mode_str[3] = { [2] = '\0' };
	enum StreamEncoding enc = ENC_DEFAULT;

	if      (mode & C_STREAM_READ  ) mode_str[0] = 'r';
	else if (mode & C_STREAM_WRITE ) mode_str[0] = 'w';
	if      (mode & C_STREAM_TEXT  ) mode_str[1] = '\0';
	else if (mode & C_STREAM_BINARY) mode_str[1] = 'b';

	if (mode & C_STREAM_UTF_8) enc = ENC_UTF_8;

	struct Stream *stream = malloc(sizeof (*stream));
	if (!stream) goto alloc_fail;
	
	long l = strlen(title);
	stream->name = malloc(l+1);
	if (!stream->name) goto copy_fail;
	strcpy(stream->name, title);
	stream->len = l;

	stream->f = fopen(title, mode_str);
	if (!stream->f) goto release;

	stream->size = 0;
	stream->size = stream_size(stream);
	stream->enc = enc;

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

long stream_size(Stream stream) {
	if (stream->size) return stream->size;
	long pos = ftell(stream->f);
	fseek(stream->f, 0, SEEK_END);
	stream->size = ftell(stream->f);
	fseek(stream->f, 0, pos);
	return stream->size;
}

long stream_read(Stream stream, void *buf, long size) {
	long r = fread(buf, 1, size, stream->f);
	if (r != size && ferror(stream->f)) return r;
	if (is_valid_buffer_utf_8(buf, size)) return r;
	return -1;
}

long stream_write(Stream stream, const void *buf, long size) {
	return fwrite(buf, 1, size, stream->f);
}

char *stream_getline(Stream stream, long *len) {
	char *try, *last, *nl;
	unsigned long cap, new_cap=8;
	last = try = malloc(new_cap);
	if (!try) goto err;
	if (!fgets(try, new_cap, stream->f)) {
		if (feof(stream->f)) goto nomem;
		if (len) *len = strlen(try);
		return try;
	}
	while (!(nl = strchr(try, '\n'))) {
		cap = new_cap;
		try = realloc(last, new_cap=cap*2);
		if (!try) goto nomem;
		last = try;
		if (!fgets(try+cap-1, cap+1, stream->f)) {
			if (len) *len = strlen(try);
			return try;
		}
	}
	if (len) *len = nl+1-try;
	return try;
nomem:
	free(last);
err:
	if (len) *len = 0;
	return NULL;
}


