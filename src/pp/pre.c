#include "pp/pre.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define WHITESPACE ' '

char *preprocessor_internalize(char *buf, long *len) {
	buf = expand_trigraphs(buf, len);
	buf = discard_bsnl(buf, len);
	buf = discard_comments(buf, len);
	return buf;
}

char *expand_trigraphs(char *buf, long *len) {
	char *insert, *read;
	for (insert = read = buf; *read; insert++) {
		*insert = *read;
		if (*read++ != '?') continue;
		if (*read++ != '?') continue;
		switch (*read++) {
#define CASE(a, b) case a: *insert = b; break;
		CASE('=', '#')
		CASE('(', '[')
		CASE('/', '\\')
		CASE(')', ']')
		CASE('\'', '^')
		CASE('<', '{')
		CASE('!', '|')
		CASE('>', '}')
		CASE('-', '~')
#undef CASE
		default:
			memmove(insert, read-3, 3);
			insert += 2;
			break;
		}
	}
	long l=insert-buf;
	if (len) *len = l;
	buf[l] = '\0';
	return buf;
}

char *discard_bsnl(char *buf, long *len) {
	char *insert, *read;
	for (insert = read = buf; *read; insert++) {
		*insert = *read;
		if (*read++ != '\\') continue;
		if (*read   != '\n') continue;
		insert--;
		read++;
		if (*read == '\0') {
			printf("file ends in a backslash-newline.\n");
		}
	}
	long l=insert-buf;
	if (len) *len = l;
	buf[l] = '\0';
	return buf;
}

char *discard_comments(char *buf, long *len) {
	// return buf;
	char *insert, *read;
	for (insert = read = buf; *read;) {
		*insert++ = *read;
		switch (*read++) {
		case '"': // we need to find the end, a non-escaped double quote
			for (; *read && *read != '\n' && *read != '"';) {
				*insert++ = *read;
				if (*read++ != '\\') continue;
				*insert++ = *read;
				if (*read++ != '"' ) continue;
			}
			if (*read != '"') {
				printf("unterminated string literal.\n");
				goto error;
			}
			*insert++ = *read++;
			break;
		case '/': // could be a comment
			if (*read == '/') {
				insert[-1] = WHITESPACE;
				while (*read && *read != '\n') read++;
			} else if (*read == '*') {
				insert[-1] = WHITESPACE;
				read++;
			loop:
				while (*read && *read != '*') read++;
				if (*read == '\0') {
					printf("file ends mid-comment.\n");
					goto error;
				}
				read++;
				if (*read != '/') goto loop;
				read++;
			} else continue;
			break;
		default:
			continue;
		}
	}
	long l = insert-buf;
	if (len) *len = l;
	buf[l] = '\0';
	return buf;
error:
	if (len) *len = 0;
	free(buf);
	return NULL;
}

