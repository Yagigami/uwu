#ifndef C_AST_COMMON_H
#define C_AST_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <common/enums.h>
#include "ast/enums.h"

struct Identifier {
	const uint8_t *name;
	ptrdiff_t len;
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
	uintmax_t value;
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
	enum ConstantAffix prefix;
	const uint8_t *sequence;
	ptrdiff_t len;
};

struct HeaderName {
	const uint8_t *sequence;
	ptrdiff_t len;
};

struct SpecifierQualifier {
	bool is_specifier;
	__extension__ union {
		struct TypeSpecifier *specifier;
		struct TypeQualifier *qualifier;
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
	struct StructDeclaratorList declarators;
};

struct EnumeratorList {
	ptrdiff_t num;
	struct Enumerator *list;
};

struct Enumerator {
	struct EnumerationConstant cst;
	struct ConstantExpression *expr;
};

struct TypeSpecifier {
	enum TypeSpecifierKind kind;
	struct Identifier *ident;
	__extension__ union {
		struct StructDeclarationList decls;
		struct EnumeratorList enums;
	};
};

struct TypeQualifier {
	enum TypeQualifierKind kind;
};

struct TypeName {
	struct SpecifierQualifierList qualifiers;
	struct Declarator *declarator;
};

struct TypeQualifierList {
	ptrdiff_t num;
	struct TypeQualifier *list;
};

struct ParameterTypeList {
	ptrdiff_t num;
	bool ellipsis;
	struct ParameterDeclaration *params;
};

struct DeclarationSpecifiers {
	ptrdiff_t num;
	enum DeclarationSpecifierKind *kinds;
	int *list;
};

struct ParameterDeclaration {
	struct DeclarationSpecifiers specifiers;
	struct Declarator *declarator;
};

#endif /* C_AST_COMMON_H */

