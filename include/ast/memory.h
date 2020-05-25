#ifndef C_AST_MEMORY_H
#define C_AST_MEMORY_H

#include <stddef.h>
#include <setjmp.h>

int ast_init(jmp_buf *env);
int ast_fini(ptrdiff_t *amt);

__attribute__((malloc, returns_nonnull))
void *ast_alloc(ptrdiff_t size);

#endif /* C_AST_MEMORY_H */

