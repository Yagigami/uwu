#include <ast/expression.h>
#include <ast/declaration.h>
#include <ast/statement.h>
#include <ast/memory.h>

#include <assert.h>
#include <stdbool.h>

static struct Expression *new_expression(enum ExpressionKind kind) {
	struct Expression *expr = ast_alloc(sizeof *expr);
	expr->kind = kind;
	return expr;
}

static struct Declarator *new_declarator(enum DeclaratorKind kind) {
	struct Declarator *declt = ast_alloc(sizeof  *declt);
	declt->kind = kind;
	return declt;
}

static struct Statement *new_statement(enum StatementKind kind) {
	struct Statement *stmt = ast_alloc(sizeof *stmt);
	stmt->kind = kind;
	return stmt;
}

struct Expression *expr_identifier(struct Identifier ident) {
	struct Expression *expr = new_expression(EXPRESSION_IDENTIFIER);
	expr->ident = ident;
	return expr;
}

struct Expression *expr_integer(struct IntegerConstant i) {
	struct Expression *expr = new_expression(EXPRESSION_INTEGER);
	expr->integer = i;
	return expr;
}

struct Expression *expr_floating(struct FloatingConstant f) {
	struct Expression *expr = new_expression(EXPRESSION_FLOATING);
	expr->floating = f;
	return expr;
}

struct Expression *expr_enumeration(struct EnumerationConstant e) {
	struct Expression *expr = new_expression(EXPRESSION_ENUMERATION);
	expr->enumeration = e;
	return expr;
}

struct Expression *expr_character(struct CharacterConstant c) {
	struct Expression *expr = new_expression(EXPRESSION_CHARACTER);
	expr->character = c;
	return expr;
}

struct Expression *expr_string_literal(struct StringLiteral lit) {
	struct Expression *expr = new_expression(EXPRESSION_STRING_LITERAL);
	expr->string_lit = lit;
	return expr;
}

struct Expression *expr_index(struct Expression *arr, struct Expression *index) {
	struct Expression *expr = new_expression(EXPRESSION_POSTFIX);
	expr->op = OPERATOR_INDEX;
	expr->postfix.expr = arr;
	expr->postfix.index = index;
	return expr;
}

struct Expression *expr_call(struct Expression *fun, struct Expression **args, ptrdiff_t n) {
	struct Expression *expr = new_expression(EXPRESSION_POSTFIX);
	expr->op = OPERATOR_CALL;
	expr->postfix.expr = fun;
	expr->postfix.args.list = args;
	expr->postfix.args.num = n;
	return expr;
}

struct Expression *expr_field(struct Expression *agg, struct Identifier field) {
	struct Expression *expr = new_expression(EXPRESSION_POSTFIX);
	expr->op = OPERATOR_FIELD;
	expr->postfix.expr = agg;
	expr->postfix.ident = field;
	return expr;
}

struct Expression *expr_arrow(struct Expression *agg, struct Identifier field) {
	struct Expression *expr = new_expression(EXPRESSION_POSTFIX);
	expr->op = OPERATOR_ARROW;
	expr->postfix.expr = agg;
	expr->postfix.ident = field;
	return expr;
}

struct Expression *expr_compound_literal(struct TypeName *type, struct InitializerListElem *inits, ptrdiff_t n) {
	struct Expression *expr = new_expression(EXPRESSION_COMPOUND_LITERAL);
	expr->compound_literal.type_name = type;
	expr->compound_literal.inits.list = inits;
	expr->compound_literal.inits.num = n;
	return expr;
}

struct Expression *expr_unary(struct Expression *operand, enum Operator op) {
	struct Expression *expr = new_expression(EXPRESSION_UNARY);
	assert(op >= OPERATOR_UNARY_START &&
			op < OPERATOR_UNARY_END &&
			op != OPERATOR_SIZEOF_TYPE);
	expr->op = op;
	expr->unary.expr = operand;
	return expr;
}

struct Expression *expr_unary_type(struct TypeName *type, enum Operator op) {
	struct Expression *expr = new_expression(EXPRESSION_UNARY);
	assert(op == OPERATOR_SIZEOF_TYPE);
	expr->op = op;
	expr->unary.type_name = type;
	return expr;
}

struct Expression *expr_binary(struct Expression *lhs, struct Expression *rhs, enum Operator op) {
	struct Expression *expr = new_expression(EXPRESSION_BINARY);
	assert((op >= OPERATOR_BINARY_START && op < OPERATOR_BINARY_END) ||
			(op >= OPERATOR_ASSIGN_START && op < OPERATOR_ASSIGN_END));
	expr->op = op;
	expr->binary.lhs = lhs;
	expr->binary.rhs = rhs;
	return expr;
}

struct Expression *expr_ternary(struct Expression *cond, struct Expression *then, struct Expression *other) {
	struct Expression *expr = new_expression(EXPRESSION_TERNARY);
	expr->ternary.cond_expr = cond;
	expr->ternary.then_expr = then;
	expr->ternary.else_expr = other;
	return expr;
}

struct Expression *expr_comma(struct Expression **exprs, ptrdiff_t n) {
	struct Expression *expr = new_expression(EXPRESSION_COMMA);
	expr->comma.exprs = exprs;
	expr->comma.num = n;
	return expr;
}

struct Expression *expr_defined(struct Identifier ident) {
	struct Expression *expr = new_expression(EXPRESSION_DEFINED);
	expr->ident = ident;
	return expr;
}

struct Declarator *declt_identifier(struct Identifier ident) {
	struct Declarator *declt = new_declarator(DECLARATOR_IDENTIFIER);
	declt->ident = ident;
	return declt;
}

struct Declarator *declt_pointer(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n) {
	struct Declarator *declt = new_declarator(DECLARATOR_POINTER);
	declt->base = base;
	declt->quals.list = quals;
	declt->quals.num = n;
	return declt;
}

struct Declarator *declt_array(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n,
		struct Expression *cnt) {
	struct Declarator *declt = new_declarator(DECLARATOR_ARRAY);
	declt->base = base;
	declt->quals.list = quals;
	declt->quals.num = n;
	declt->expr = cnt;
	return declt;
}

struct Declarator *declt_array_least(struct Declarator *base, enum TypeQualifier *quals, ptrdiff_t n,
		struct Expression *cnt) {
	struct Declarator *declt = new_declarator(DECLARATOR_VLA_STATIC);
	declt->base = base;
	declt->quals.list = quals;
	declt->quals.num = n;
	declt->expr = cnt;
	return declt;
}

struct Declarator *declt_function(struct Declarator *base,
		struct ParameterDeclaration *params, ptrdiff_t n) {
	struct Declarator *declt = new_declarator(DECLARATOR_FUNCTION);
	declt->base = base;
	declt->params.list = params;
	declt->params.num = n;
	declt->params.ellipsis = false;
	return declt;
}

struct Declarator *declt_function_variadic(struct Declarator *base,
		struct ParameterDeclaration *params, ptrdiff_t n) {
	struct Declarator *declt = new_declarator(DECLARATOR_FUNCTION);
	declt->base = base;
	declt->params.list = params;
	declt->params.num = n;
	declt->params.ellipsis = true;
	return declt;
}

struct Declarator *declt_function_old_style(struct Declarator *base,
		struct Identifier *idents, ptrdiff_t n) {
	struct Declarator *declt = new_declarator(DECLARATOR_FUNCTION_K_AND_R);
	declt->base = base;
	declt->idents.list = idents;
	declt->idents.num = n;
	return declt;
}

struct Declaration *declaration(struct DeclarationSpecifier *specs, ptrdiff_t spec_num,
		struct InitDeclarator *inits, ptrdiff_t init_num) {
	struct Declaration *decl = ast_alloc(sizeof *decl);
	decl->specs.list = specs;
	decl->specs.num = spec_num;
	decl->inits.list = inits;
	decl->inits.num = init_num;
	return decl;
}

struct Statement *stmt_label(struct Statement *base, struct Identifier ident) {
	struct Statement *stmt = new_statement(STATEMENT_LABEL);
	stmt->labeled.stmt = base;
	stmt->labeled.ident = ident;
	return stmt;
}

struct Statement *stmt_case(struct Statement *base, struct Expression *expr) {
	struct Statement *stmt = new_statement(STATEMENT_CASE);
	stmt->labeled.stmt = base;
	stmt->labeled.expr = expr;
	return stmt;
}

struct Statement *stmt_default(struct Statement *base) {
	struct Statement *stmt = new_statement(STATEMENT_DEFAULT);
	stmt->labeled.stmt = base;
	return stmt;
}

struct Statement *stmt_compound(struct BlockItem *items, ptrdiff_t n) {
	struct Statement *stmt = new_statement(STATEMENT_COMPOUND);
	stmt->stmts.list = items;
	stmt->stmts.num = n;
	return stmt;
}

struct Statement *stmt_expression(struct Expression *expr) {
	struct Statement *stmt = new_statement(STATEMENT_EXPRESSION);
	stmt->expr = expr;
	return stmt;
}

struct Statement *stmt_if(struct Expression *select, struct Statement *then_stmt, struct Statement *else_stmt) {
	struct Statement *stmt = new_statement(STATEMENT_IF);
	stmt->selection.cond_expr = select;
	stmt->selection.then_stmt = then_stmt;
	stmt->selection.else_stmt = else_stmt;
	return stmt;
}

struct Statement *stmt_switch(struct Expression *select, struct Statement *body) {
	struct Statement *stmt = new_statement(STATEMENT_SWITCH);
	stmt->selection.cond_expr = select;
	stmt->selection.then_stmt = body;
	return stmt;
}

struct Statement *stmt_while(struct Expression *cond, struct Statement *body) {
	struct Statement *stmt = new_statement(STAMENENT_WHILE);
	stmt->iter.cond = cond;
	stmt->iter.body = body;
	return stmt;
}

struct Statement *stmt_do_while(struct Expression *cond, struct Statement *body) {
	struct Statement *stmt = new_statement(STATEMENT_DO_WHILE);
	stmt->iter.cond = cond;
	stmt->iter.body = body;
	return stmt;
}

struct Statement *stmt_for_expr(struct Expression *init, struct Expression *cond, struct Expression *iter,
				struct Statement *body) {
	struct Statement *stmt = new_statement(STATEMENT_FOR_EXPR);
	stmt->iter.init.expr = init;
	stmt->iter.cond = cond;
	stmt->iter.next_expr = iter;
	stmt->iter.body = body;
	return stmt;
}

struct Statement *stmt_for_decl(struct Declaration *decl, struct Expression *cond, struct Expression *iter,
				struct Statement *body) {
	struct Statement *stmt = new_statement(STATEMENT_FOR_DECL);
	stmt->iter.init.decl = decl;
	stmt->iter.cond = cond;
	stmt->iter.next_expr = iter;
	stmt->iter.body = body;
	return stmt;
}

struct Statement *stmt_goto(struct Identifier ident) {
	struct Statement *stmt = new_statement(STATEMENT_GOTO);
	stmt->ident = ident;
	return stmt;
}

struct Statement *stmt_continue(void) {
	struct Statement *stmt = new_statement(STATEMENT_CONTINUE);
	return stmt;
}

struct Statement *stmt_break(void) {
	struct Statement *stmt = new_statement(STATEMENT_BREAK);
	return stmt;
}

struct Statement *stmt_return(struct Expression *expr) {
	struct Statement *stmt = new_statement(STATEMENT_RETURN);
	stmt->expr = expr;
	return stmt;
}

