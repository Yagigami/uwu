#ifndef C_AST_EXTERNAL_H
#define C_AST_EXTERNAL_H

#include <stddef.h>

#include <ast/declaration.h>
#include <ast/statement.h>

struct DeclarationList {
	ptrdiff_t num;
	struct Declaration *list;
};

struct FunctionDefinition {
	struct DeclarationSpecifierList specs;
	struct Declarator *declt;
	struct DeclarationList decls_k_and_r;
	struct Statement *stmt;
};

enum ExternalDeclarationKind {
	EXTERNAL_NONE = 0,
	EXTERNAL_FUNCDEF,
	EXTERNAL_DECL,
	EXTERNAL_END,
};

struct ExternalDeclaration {
	enum ExternalDeclarationKind kind;
	__extension__ union {
		struct FunctionDefinition def;
		struct Declaration *decl;
	};
};

#endif /* C_AST_EXTERNAL_H */

