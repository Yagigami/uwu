#include <stdlib.h>
#include <stdio.h>

#include "pp/tests.h"
#include "pp/pp.h"

int pp_test(void) {
	printf("pp:\n");
	const char *f = "foo.c";
	const char *o = "foo.i";
	struct Preprocessor pp;
	int err;
	if ((err = preprocessor_init(&pp, f))) {
		fprintf(stderr, "could not initialize preprocessor for `%s`.\n", f);
		return -1;
	}
	err = preprocess(&pp);
	if (err) {
		fprintf(stderr, "error occured during preprocessing of `%s`.\n", f);
		preprocessor_fini(&pp, NULL);
		return -1;
	}
	printf("%s", pp.buf);
	if ((err = preprocessor_fini(&pp, o))) {
		fprintf(stderr, "could not write preprocessor output to `%s`.\n", o);
		return -1;
	}
	return 0;
}

