#ifndef C_AST_STATEMENT_H
#define C_AST_STATEMENT_H

#include "ast/common.h"
#include "ast/expression.h"
#include "ast/declaration.h"

struct BlockItemList {
	int num;
	struct BlockItem *list;
};

struct Statement {
	enum StatementKind kind;
	__extension__ union {
		struct {
			struct Statement *stmt;
			__extension__ union {
				struct Identifier ident;
				struct Expression *expr;
			};
		} labeled;
		struct BlockItemList stmts;
		struct Expression *expr;
		struct {
			struct Expression *cond_expr;
			struct Statement  *then_stmt;
			struct Statement  *else_stmt;
		} selection;
		struct {
			union {
				struct Expression  *expr;
				struct Declaration *decl;
			} init;
			struct Expression *cond;
			struct Expression *next_expr;
			struct Statement  *body;
		} iter;
		struct Identifier ident;
	};
};

struct BlockItem {
	enum BlockItemKind kind;
	__extension__ union {
		struct Declaration *decl;
		struct Statement *stmt;
	};
};

#endif /* C_AST_STATEMENT_H */

