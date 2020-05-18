#include "pp/pre.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *expand_trigraphs(char *line, long len, long *new_len) {
	char *new_line = malloc(len);
	long off;
	if (!new_line) {
		if (new_len) *new_len = 0;
		return NULL;
	}
	char *cur=line, *last_cur=cur, *new_cur=new_line;
	while ((cur = strstr(last_cur, "??")) && cur < line+len) {
		off = cur-last_cur;
		memcpy(new_cur, last_cur, off);
		new_cur += off;
		last_cur = cur+3;
		switch (cur[2]) {
		case '=':
			*new_cur = '#';
			break;
		case '(':
			*new_cur = '[';
			break;
		case '/':
			*new_cur = '\\';
			break;
		case ')':
			*new_cur = ']';
			break;
		case '\'':
			*new_cur = '^';
			break;
		case '<':
			*new_cur = '{';
			break;
		case '!':
			*new_cur = '|';
			break;
		case '>':
			*new_cur = '}';
			break;
		case '-':
			*new_cur = '~';
			break;
		default:
			memcpy(new_cur, cur, 3);
			new_cur += 2;
			break;
		}
		new_cur++;
	}
	off = line+len-last_cur;
	memcpy(new_cur, last_cur, off);
	new_cur += off;
	if (new_len) *new_len = new_cur-new_line;
	free(line);
	return new_line;
}

char *discard_bsnl(Stream stream, long *len, long *src_lines) {
	char *line, *old=NULL, *part;
	long ltotal, lpart, jmps=0;

	if (!(part = stream_getline(stream, &lpart))) goto err;
	if (!(part = expand_trigraphs(part, lpart, &lpart))) goto err;
	ltotal = lpart;

	while (
		jmps++,
		line = realloc(old, ltotal+1),
		lpart >= 2 && part[lpart-2] == '\\'
	) {
		if (!line) {
			free(old);
			return NULL;
		}
		old = line;
		memcpy(line+ltotal-lpart, part, lpart-2);
		free(part);
		ltotal -= 2;
		if (!(part = stream_getline(stream, &lpart))) {
			line[ltotal-1] = '\n';
			fprintf(stderr, "file ends in escaped baskslash-newline.\n");
			goto early;
		}
		if (!(part = expand_trigraphs(part, lpart, &lpart))) goto err;
		ltotal += lpart;
	}
	
	if (!line) {
		free(old);
		return NULL;
	}
	old = line;
	memcpy(line+ltotal-lpart, part, lpart);
	free(part);

early:
	if (len) *len = ltotal;
	if (src_lines) *src_lines = jmps;
	return line;
err:
	if (len) *len = 0;
	if (src_lines) *src_lines = 0;
	free(old);
	return NULL;
}

