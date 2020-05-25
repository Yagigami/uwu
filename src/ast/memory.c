#include <ast/memory.h>

#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

static jmp_buf *_env = NULL;
static ptrdiff_t amount = 0;

int ast_init(jmp_buf *env) {
	if (_env) return -1;
	_env = env;
	return 0;
}

int ast_fini(ptrdiff_t *amt) {
	if (!_env) return -1;
	_env = NULL;
	if (amt) *amt = amount;
	amount = 0;
	return 0;
}

void *ast_alloc(ptrdiff_t size) {
	if (size < 1) longjmp(*_env, -1);
	void *alloc = malloc(size);
	if (!alloc) longjmp(*_env, -1);
	amount += size;
	return alloc;
}

