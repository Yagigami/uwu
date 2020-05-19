#include <stdlib.h>

#include "pp/translate.h"
#include "pp/pre.h"
#include "pp/pptoken.h"

int preprocess(struct Preprocessor *pp) {
	return pp->buf ? 0: -1;
}

