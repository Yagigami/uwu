#ifndef C_PP_PPTOKEN_H
#define C_PP_PPTOKEN_H

#include "pp/directives.h"

struct Preprocessor {
	char *buf;
	char *cur;
	long len;
	struct PreprocessingToken pptoken;
};

int preprocessor_init(struct Preprocessor *pp, const char *name);
int preprocessor_fini(struct Preprocessor *pp, const char *name);

#endif /* C_PP_PPTOKEN_H */

