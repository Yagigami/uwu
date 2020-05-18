#ifndef C_PP_PRE_H
#define C_PP_PRE_H

#include "stream/stream.h"

char *expand_trigraphs(char *line, long len, long *new_len);
char *discard_bsnl(Stream stream, long *len, long *src_lines);

#endif /* C_PP_PRE_H */

