#ifndef C_PP_PRE_H
#define C_PP_PRE_H

#include "stream/stream.h"

char *preprocessor_internalize(char *buf, long *len);

char *expand_trigraphs(char *buf, long *len);
char *discard_bsnl(char *buf, long *len);
char *discard_comments(char *buf, long *len);

#endif /* C_PP_PRE_H */

