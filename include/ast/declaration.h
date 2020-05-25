#ifndef C_AST_DECLARATION_H
#define C_AST_DECLARATION_H

#include <stddef.h>

#include "ast/common.h"
#include "ast/expression.h"

struct InitDeclaratorList {
	ptrdiff_t num;
	struct InitDeclarator *list;
};

struct Declaration {
	struct DeclarationSpecifiers specs;
	struct InitDeclaratorList declarators;
};

struct Declarator {
	enum DeclaratorKind kind;
	struct Declarator *base;
	__extension__ union {
		__extension__ struct {
			struct TypeQualifierList quals;
			struct Expression *expr;
		};
		struct ParameterTypeList params;
		struct IdentifierList idents;
	};
};

struct InitDeclarator {
	struct Declarator declarator;
	struct Initializer *init;
};

#endif /* C_AST_DECLARATION_H */

