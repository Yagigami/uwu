#include <common/tests.h>
#include <common/log.h>

#include <stdio.h>
#include <assert.h>

int common_test(void) {
	printf("common:\n");
	long r = uwuprintf("%s%c, world!\n", "HELL", 'o');
	assert(r == 14);
	r = uwuprintf("He%-20.40ls!\n", L"llo, \u1234world");
	assert(r == 24);
	r = uwuprintf("%+4zd\t>%06hhi\n<% 16.8td>\n", (((char *) 42) - ((char *)  0)), -163,  42012345678l);
	assert(r == 32);
	r = uwuprintf("%dd\n", 0);
	assert(r == 3);
	r = uwuprintf("|%+#-*.5hu| |%8x-%-16.*llo|\n", 16l, 1234u, 0, 5, 0xDEADBEEFll);
	assert(r == 47);
	r = uwuprintf("(%p) \"%.*s\"\n", (void *) &common_test, 8l, "common_tests");
	assert(r == 22);
	r = uwuprintf("%#+030.15f\n", 15.3712354545);
	assert(r == 31);
	return 0;
}

