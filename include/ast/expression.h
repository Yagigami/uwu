#ifndef C_AST_EXPRESSION_H
#define C_AST_EXPRESSION_H

#include <stddef.h>

#include "ast/common.h"

struct ArgumentExpressionList {
	ptrdiff_t num;
	struct Expression **list;
};

struct InitializerList {
	ptrdiff_t num;
	struct InitializerListElem *list;
};

struct Expression {
	enum ExpressionKind kind;
	enum Operator op;
	__extension__ union {
		struct Identifier ident;
		struct IntegerConstant integer;
		struct FloatingConstant floating;
		// ...
		struct EnumerationConstant enumeration;
		struct CharacterConstant character;
		struct StringLiteral string_lit;
		struct {
			struct Expression *expr;
			__extension__ union {
				struct Expression *index;
				struct ArgumentExpressionList args;
				struct Identifier ident;
			};
		} postfix;
		struct {
			struct TypeName *type_name;
			struct InitializerList inits;
		} compound_literal;
		struct {
			__extension__ union {
				struct Expression *expr;
				struct TypeName *type_name;
			};
		} unary;
		struct {
			struct Expression *lhs;
			struct Expression *rhs;
		} binary;
		struct {
			struct Expression *cond_expr;
			struct Expression *then_expr;
			struct Expression *else_expr;
		} ternary;
		struct {
			ptrdiff_t num;
			struct Expression **exprs;
		} comma;
	};
};

struct DesignatorList {
	ptrdiff_t num;
	struct Designator *list;
};

struct Designator {
	enum DesignatorKind kind;
	__extension__ union {
		struct Expression *expr;
		struct Identifier ident;
	};
};

struct Initializer {
	enum InitializerKind kind;
	__extension__ union {
		struct Expression *expr;
		struct InitializerList inits;
	};
};

struct InitializerListElem {
	struct DesignatorList *desigs;
	struct Initializer init;
};

#endif /* C_AST_EXPRESSION_H */

