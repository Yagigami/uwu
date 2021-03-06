#ifndef C_AST_DECLARATION_ENUMS_H
#define C_AST_DECLARATION_ENUMS_H

enum DeclarationSpecifierKind {
	DECLSPEC_NONE = 0,
	DECLSPEC_TYPEDEF,
	DECLSPEC_STORAGE_CLASS_START = DECLSPEC_TYPEDEF,
	DECLSPEC_EXTERN,
	DECLSPEC_STATIC,
	DECLSPEC_AUTO,
	DECLSPEC_REGISTER,
	DECLSPEC_STORAGE_CLASS_END,
	DECLSPEC_VOID,
	DECLSPEC_TYPESPEC_START = DECLSPEC_VOID,
	DECLSPEC_CHAR,
	DECLSPEC_SHORT,
	DECLSPEC_INT,
	DECLSPEC_LONG,
	DECLSPEC_FLOAT,
	DECLSPEC_DOUBLE,
	DECLSPEC_SIGNED,
	DECLSPEC_UNSIGNED,
	DECLSPEC_BOOL,
	DECLSPEC_COMPLEX,
	DECLSPEC_IMAGINARY,
	DECLSPEC_STRUCT,
	DECLSPEC_STRUCT_DEFINITION,
	DECLSPEC_UNION,
	DECLSPEC_UNION_DEFINITION,
	DECLSPEC_ENUM,
	DECLSPEC_ENUM_DEFINITION,
	DECLSPEC_TYPEDEF_NAME,
	DECLSPEC_TYPESPEC_END,
	DECLSPEC_CONST,
	DECLSPEC_TYPEQUAL_START = DECLSPEC_CONST,
	DECLSPEC_RESTRICT,
	DECLSPEC_VOLATILE,
	DECLSPEC_TYPEQUAL_END,
	DECLSPEC_INLINE,
	DECLSPEC_FUNCSPEC_START = DECLSPEC_INLINE,
	DECLSPEC_FUNCSPEC_END,
	DECLSPEC_END,
};

enum DeclaratorKind {
	DECLARATOR_NONE = 0,
	DECLARATOR_IDENTIFIER,
	DECLARATOR_POINTER,
	DECLARATOR_ARRAY,
	DECLARATOR_FUNCTION,
	DECLARATOR_FUNCTION_K_AND_R,
	DECLARATOR_END,
};

enum DesignatorKind {
	DESIGNATOR_NONE = 0,
	DESIGNATOR_INDEX,
	DESIGNATOR_FIELD,
	DESIGNATOR_END,
};

#endif /* C_AST_DECLARATION_ENUMS_H */
