#ifndef C_AST_H
#define C_AST_H

#include "ast/expression.h"
#include "ast/declaration.h"
#include "ast/statement.h"
#include "ast/common.h"

#include <stddef.h>
#include <setjmp.h>

int ast_init(jmp_buf *env);
int ast_fini(void);

struct Expression *expr_identifier(struct Identifier ident);
struct Expression *expr_constant(struct Constant cst);
struct Expression *expr_string_literal(struct StringLiteral lit);
struct Expression *expr_index(struct Expression *arr, struct Expression *index);
struct Expression *expr_call(struct Expression *fun, struct Expression *const *args, ptrdiff_t n);
struct Expression *expr_field(struct Expression *agg, struct Identifier field);
struct Expression *expr_postfix(struct Expression *expr, enum Operator op);
struct Expression *expr_compound_literal(struct TypeName *type, struct InitializerListElem *inits, ptrdiff_t n);
struct Expression *expr_unary(struct Expression *expr, enum Operator op);
struct Expression *expr_unary_type(struct TypeName *type, enum Operator op);
struct Expression *expr_binary(struct Expression *lhs, struct Expression *rhs);
struct Expression *expr_ternary(struct Expression *cond, struct Expression *then, struct Expression *other);
struct Expression *expr_comma(struct Expression **exprs, ptrdiff_t n);

struct Declarator *declt_identifier(struct Identifier ident);
struct Declarator *declt_pointer(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n);
struct Declarator *declt_array(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n,
		struct Expression *cnt);
struct Declarator *declt_array_least(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n,
		struct Expression *cnt);
struct Declarator *declt_function(struct Declarator *base,
		struct ParameterDeclaration *params, ptrdiff_t n);
struct Declarator *declt_function_variadic(struct Declarator *base,
		struct ParameterDeclaration *params, ptrdiff_t n);
struct Declarator *declt_function_old_style(struct Declarator *base,
		struct Identifier *idents, ptrdiff_t n);

struct Declaration *declaration(struct DeclarationSpecifier *specs, ptrdiff_t spec_num,
		struct InitDeclarator *inits, ptrdiff_t init_num);

struct Statement *stmt_label(struct Statement *stmt, struct Identifier ident);
struct Statement *stmt_case(struct Statement *stmt, struct Expression *expr);
struct Statement *stmt_default(struct Statement *stmt);
struct Statement *stmt_compound(struct BlockItem *items, ptrdiff_t n);
struct Statement *stmt_expression(struct Expression *expr);
struct Statement *stmt_if(struct Expression *select, struct Statement *then_stmt, struct Statement *else_stmt);
struct Statement *stmt_switch(struct Expression *select, struct Statement *stmt);
struct Statement *stmt_while(struct Expression *cond, struct Statement *stmt);
struct Statement *stmt_do_while(struct Expression *cond, struct Statement *stmt);
struct Statement *stmt_for_expr(struct Expression *init, struct Expression *cond, struct Expression *iter,
				struct Statement *stmt);
struct Statement *stmt_for_decl(struct Declaration *decl, struct Expression *cond, struct Expression *iter,
				struct Statement *stmt);
struct Statement *stmt_goto(struct Identifier ident);
struct Statement *stmt_continue(void);
struct Statement *stmt_break(void);
struct Statement *stmt_return(struct Expression *expr);

#endif /* C_AST_H */

