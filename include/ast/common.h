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

struct StringLiteral {
	void *sequence;
	enum ConstantAffix prefix;
	ptrdiff_t len;
};

struct HeaderName {
	const uint8_t *sequence;
	ptrdiff_t len;
};

#endif /* C_AST_COMMON_H */

