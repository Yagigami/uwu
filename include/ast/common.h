#ifndef C_AST_COMMON_H
#define C_AST_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <common/enums.h>
#include "ast/enums.h"

struct Identifier {
	const struct InternString *detail;
};

struct IdentifierList {
	ptrdiff_t num;
	struct Identifier *list;
};

struct IntegerConstant {
	enum ConstantAffix suffix;
	uintmax_t value;
};

struct FloatingConstant {
	enum ConstantAffix suffix;
	long double value;
};

struct EnumerationConstant {
	struct Identifier ident;
};

struct CharacterConstant {
	enum ConstantAffix prefix;
	uint32_t value;
};

struct Constant {
	enum ConstantKind kind;
	__extension__ union {
		struct IntegerConstant integer;
		struct FloatingConstant floating;
		struct EnumerationConstant enumeration;
		struct CharacterConstant character;
	};
};

struct StringLiteral {
	void *sequence;
	enum ConstantAffix prefix;
	ptrdiff_t len;
};

struct HeaderName {
	const uint8_t *sequence;
	ptrdiff_t len;
};

struct SpecifierQualifier {
	bool is_specifier;
	__extension__ union {
		struct TypeSpecifier *spec;
		enum TypeQualifier *qual;
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

struct TypeQualifierList {
	ptrdiff_t num;
	enum TypeQualifier *list;
};

struct ParameterTypeList {
	ptrdiff_t num;
	bool ellipsis;
	struct ParameterDeclaration *params;
};

struct DeclarationSpecifierList {
	ptrdiff_t num;
	struct DeclarationSpecifier *list;
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

#endif /* C_AST_COMMON_H */

