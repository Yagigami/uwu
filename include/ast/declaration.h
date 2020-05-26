#ifndef C_AST_DECLARATION_H
#define C_AST_DECLARATION_H

#include <stddef.h>
#include <limits.h>

#include "ast/common.h"
#include "ast/expression.h"

struct InitDeclaratorList {
	ptrdiff_t num;
	struct InitDeclarator *list;
};

struct DeclarationSpecifierList {
	ptrdiff_t num;
	struct DeclarationSpecifier *list;
};

struct TypeQualifierList {
	ptrdiff_t num;
	enum DeclarationSpecifierKind *list;
};

struct Declaration {
	struct DeclarationSpecifierList specs;
	struct InitDeclaratorList inits;
};

struct ParameterTypeList {
	ptrdiff_t num: CHAR_BIT * sizeof (ptrdiff_t) - 1;
	bool ellipsis: 1;
	struct ParameterDeclaration *list;
};

struct Declarator {
	enum DeclaratorKind kind;
	struct Declarator *base;
	__extension__ union {
		struct Identifier ident;
		__extension__ struct {
			struct TypeQualifierList quals;
			struct Expression *expr;
		};
		struct ParameterTypeList params;
		struct IdentifierList idents;
	};
};

struct InitDeclarator {
	struct Declarator *declt;
	struct Initializer *init;
};

struct SpecifierQualifier {
	bool is_specifier;
	__extension__ union {
		struct TypeSpecifier *spec;
		enum DeclarationSpecifierKind *qual;
	};
};

struct StructDeclarationList {
	ptrdiff_t num;
	struct StructDeclaration *list;
};

struct SpecifierQualifierList {
	ptrdiff_t num;
	struct SpecifierQualifier *list;
};

struct StructDeclaratorList {
	ptrdiff_t num;
	struct StructDeclarator *list;
};

struct StructDeclarator {
	struct Declarator *declt;
	struct Expression *width;
};

struct StructDeclaration {
	struct SpecifierQualifierList specquals;
	struct StructDeclaratorList declts;
};

struct EnumeratorList {
	ptrdiff_t num;
	struct Enumerator *list;
};

struct Enumerator {
	struct EnumerationConstant cst;
	struct Expression *expr;
};

struct TypeSpecifier {
	enum DeclarationSpecifierKind kind;
	struct Identifier *ident;
	__extension__ union {
		struct StructDeclarationList decls;
		struct EnumeratorList enums;
	};
};

struct TypeName {
	struct SpecifierQualifierList quals;
	struct Declarator *declt;
};

struct DeclarationSpecifier {
	enum DeclarationSpecifierKind kind;
	struct Identifier *ident; // only for structs/unions/enums/typedef-names
	__extension__ union {
		struct StructDeclarationList decls;
		struct EnumeratorList enums;
	};
};

struct ParameterDeclaration {
	struct DeclarationSpecifierList specs;
	struct Declarator *declt;
};

#endif /* C_AST_DECLARATION_H */

