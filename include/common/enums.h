#ifndef C_COMMON_ENUMS_H
#define C_COMMON_ENUMS_H

enum ConstantKind {
	CONSTANT_NONE = 0,
	CONSTANT_INTEGER,
	CONSTANT_FLOATING,
	CONSTANT_ENUMERATION,
	CONSTANT_CHARACTER,
	CONSTANT_END,
};

enum ConstantAffix {
	CONSTANT_AFFIX_NONE = 0,
	CONSTANT_AFFIX_U,
	CONSTANT_AFFIX_L,
	CONSTANT_AFFIX_LL,
	CONSTANT_AFFIX_UL,
	CONSTANT_AFFIX_ULL,
	CONSTANT_AFFIX_F,
	CONSTANT_AFFIX_END,
};

enum Punctuator {
	PUNCTUATOR_NONE,
	PUNCTUATOR_LSQUARE_BRACKET,
	PUNCTUATOR_RSQUARE_BRACKET,
	PUNCTUATOR_LBRACKET,
	PUNCTUATOR_RBRACKET,
	PUNCTUATOR_LCURLY_BRACE,
	PUNCTUATOR_RCURLY_BRACE,
	PUNCTUATOR_DOT,
	PUNCTUATOR_ARROW,
	PUNCTUATOR_INCREMENT,
	PUNCTUATOR_DECREMENT,
	PUNCTUATOR_AMPERSAND,
	PUNCTUATOR_ASTERISK,
	PUNCTUATOR_PLUS,
	PUNCTUATOR_MINUS,
	PUNCTUATOR_TILDE,
	PUNCTUATOR_BANG,
	PUNCTUATOR_FSLASH,
	PUNCTUATOR_PERCENT,
	PUNCTUATOR_LSHIFT,
	PUNCTUATOR_RSHIFT,
	PUNCTUATOR_LOWER,
	PUNCTUATOR_GREATER,
	PUNCTUATOR_LOWER_EQUAL,
	PUNCTUATOR_GREATER_EQUAL,
	PUNCTUATOR_EQUAL,
	PUNCTUATOR_NOT_EQUAL,
	PUNCTUATOR_CIRCUMFLEX,
	PUNCTUATOR_BAR,
	PUNCTUATOR_AND,
	PUNCTUATOR_OR,
	PUNCTUATOR_QUESTION,
	PUNCTUATOR_COLON,
	PUNCTUATOR_SEMICOLON,
	PUNCTUATOR_ELLIPSIS,
	PUNCTUATOR_ASSIGN,
	PUNCTUATOR_ASTERISK_ASSIGN,
	PUNCTUATOR_FSLASH_ASSIGN,
	PUNCTUATOR_PERCENT_ASSIGN,
	PUNCTUATOR_PLUS_ASSIGN,
	PUNCTUATOR_MINUS_ASSIGN,
	PUNCTUATOR_LSHIFT_ASSIGN,
	PUNCTUATOR_RSHIFT_ASSIGN,
	PUNCTUATOR_AMPERSAND_ASSIGN,
	PUNCTUATOR_CIRCUMFLEX_ASSIGN,
	PUNCTUATOR_BAR_ASSIGN,
	PUNCTUATOR_COMMA,
	PUNCTUATOR_HASH,
	PUNCTUATOR_CAT,
	PUNCTUATOR_DIGRAPH_LESS_COLON,
	PUNCTUATOR_DIGRAPH_COLON_GREATER,
	PUNCTUATOR_DIGRAPH_LESS_PERCENT,
	PUNCTUATOR_DIGRAPH_PERCENT_GREATER,
	PUNCTUATOR_DIGRAPH_PERCENT_COLON,
	PUNCTUATOR_DIGRAPH_DOUBLE_PERCENT_COLON,
	PUNCTUATOR_END,
};

enum TokenKind {
	TOKEN_NONE = 0,
	TOKEN_AUTO,
	TOKEN_KEYWORD_START = TOKEN_AUTO,
	TOKEN_BREAK,
	TOKEN_CASE,
	TOKEN_CHAR,
	TOKEN_CONST,
	TOKEN_CONTINUE,
	TOKEN_DEFAULT,
	TOKEN_DO,
	TOKEN_DOUBLE,
	TOKEN_ELSE,
	TOKEN_ENUM,
	TOKEN_EXTERN,
	TOKEN_FLOAT,
	TOKEN_FOR,
	TOKEN_GOTO,
	TOKEN_IF,
	TOKEN_INLINE,
	TOKEN_INT,
	TOKEN_LONG,
	TOKEN_REGISTER,
	TOKEN_RESTRICT,
	TOKEN_RETURN,
	TOKEN_SHORT,
	TOKEN_SIGNED,
	TOKEN_SIZEOF,
	TOKEN_STATIC,
	TOKEN_STRUCT,
	TOKEN_SWITCH,
	TOKEN_TYPEDEF,
	TOKEN_UNION,
	TOKEN_UNSIGNED,
	TOKEN_VOID,
	TOKEN_VOLATILE,
	TOKEN_WHILE,
	TOKEN_BOOL,
	TOKEN_COMPLEX,
	TOKEN_IMAGINARY,
	TOKEN_KEYWORD_END,
	TOKEN_IDENTIFIER,
	TOKEN_STRING_LITERAL,
	TOKEN_INTEGER_CONSTANT,
	TOKEN_CONSTANT_START = TOKEN_INTEGER_CONSTANT,
	TOKEN_FLOATING_CONSTANT,
	TOKEN_ENUMERATION_CONSTANT,
	TOKEN_CHARACTER_CONSTANT,
	TOKEN_CONSTANT_END,
	TOKEN_LSQUARE_BRACKET,
	TOKEN_PUNCTUATOR_START = TOKEN_LSQUARE_BRACKET,
	TOKEN_RSQUARE_BRACKET,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_LCURLY_BRACE,
	TOKEN_RCURLY_BRACE,
	TOKEN_DOT,
	TOKEN_ARROW,
	TOKEN_INCREMENT,
	TOKEN_DECREMENT,
	TOKEN_AMPERSAND,
	TOKEN_ASTERISK,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_TILDE,
	TOKEN_BANG,
	TOKEN_FSLASH,
	TOKEN_PERCENT,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,
	TOKEN_LOWER,
	TOKEN_GREATER,
	TOKEN_LOWER_EQUAL,
	TOKEN_GREATER_EQUAL,
	TOKEN_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_CIRCUMFLEX,
	TOKEN_BAR,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_QUESTION,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_ELLIPSIS,
	TOKEN_ASSIGN,
	TOKEN_ASTERISK_ASSIGN,
	TOKEN_FSLASH_ASSIGN,
	TOKEN_PERCENT_ASSIGN,
	TOKEN_PLUS_ASSIGN,
	TOKEN_MINUS_ASSIGN,
	TOKEN_LSHIFT_ASSIGN,
	TOKEN_RSHIFT_ASSIGN,
	TOKEN_AMPERSAND_ASSIGN,
	TOKEN_CIRCUMFLEX_ASSIGN,
	TOKEN_BAR_ASSIGN,
	TOKEN_COMMA,
	TOKEN_PUNCTUATOR_END,
	TOKEN_END,
};

#endif /* C_COMMON_ENUMS_H */
