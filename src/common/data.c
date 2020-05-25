#include "common/data.h"
#include <uwu/enums.h>

const char *token_detail_to_str[] = {
	[TOKEN_DETAIL_NONE] = "x:<END>",
	[TOKEN_DETAIL_KEYWORD_AUTO] = "k:auto",
	[TOKEN_DETAIL_KEYWORD_BREAK] = "k:break",
	[TOKEN_DETAIL_KEYWORD_CASE] = "k:case",
	[TOKEN_DETAIL_KEYWORD_CHAR] = "k:char",
	[TOKEN_DETAIL_KEYWORD_CONST] = "k:const",
	[TOKEN_DETAIL_KEYWORD_CONTINUE] = "k:continue",
	[TOKEN_DETAIL_KEYWORD_DEFAULT] = "k:default",
	[TOKEN_DETAIL_KEYWORD_DO] = "k:do",
	[TOKEN_DETAIL_KEYWORD_DOUBLE] = "k:double",
	[TOKEN_DETAIL_KEYWORD_ELSE] = "k:else",
	[TOKEN_DETAIL_KEYWORD_ENUM] = "k:enum",
	[TOKEN_DETAIL_KEYWORD_EXTERN] = "k:extern",
	[TOKEN_DETAIL_KEYWORD_FLOAT] = "k:float",
	[TOKEN_DETAIL_KEYWORD_FOR] = "k:for",
	[TOKEN_DETAIL_KEYWORD_GOTO] = "k:goto",
	[TOKEN_DETAIL_KEYWORD_IF] = "k:if",
	[TOKEN_DETAIL_KEYWORD_INLINE] = "k:inline",
	[TOKEN_DETAIL_KEYWORD_INT] = "k:int",
	[TOKEN_DETAIL_KEYWORD_LONG] = "k:long",
	[TOKEN_DETAIL_KEYWORD_REGISTER] = "k:register",
	[TOKEN_DETAIL_KEYWORD_RESTRICT] = "k:restrict",
	[TOKEN_DETAIL_KEYWORD_RETURN] = "k:return",
	[TOKEN_DETAIL_KEYWORD_SHORT] = "k:short",
	[TOKEN_DETAIL_KEYWORD_SIGNED] = "k:signed",
	[TOKEN_DETAIL_KEYWORD_SIZEOF] = "k:sizeof",
	[TOKEN_DETAIL_KEYWORD_STATIC] = "k:static",
	[TOKEN_DETAIL_KEYWORD_STRUCT] = "k:struct",
	[TOKEN_DETAIL_KEYWORD_SWITCH] = "k:switch",
	[TOKEN_DETAIL_KEYWORD_TYPEDEF] = "k:typedef",
	[TOKEN_DETAIL_KEYWORD_UNION] = "k:union",
	[TOKEN_DETAIL_KEYWORD_UNSIGNED] = "k:union",
	[TOKEN_DETAIL_KEYWORD_VOID] = "k:void",
	[TOKEN_DETAIL_KEYWORD_VOLATILE] = "k:volatile",
	[TOKEN_DETAIL_KEYWORD_WHILE] = "k:while",
	[TOKEN_DETAIL_KEYWORD_BOOL] = "k:_Bool",
	[TOKEN_DETAIL_KEYWORD_COMPLEX] = "k:_Complex",
	[TOKEN_DETAIL_KEYWORD_IMAGINARY] = "k:_Imaginary",
	[TOKEN_DETAIL_IDENTIFIER] = "i:",
	[TOKEN_DETAIL_STRING_LITERAL] = "s:",
	[TOKEN_DETAIL_WIDE_STRING_LITERAL] = "s:L",
	[TOKEN_DETAIL_INTEGER_CONSTANT] = "ic:",
	[TOKEN_DETAIL_FLOATING_CONSTANT] = "fc:",
	[TOKEN_DETAIL_ENUMERATION_CONSTANT] = "ec:",
	[TOKEN_DETAIL_CHARACTER_CONSTANT] = "cc:",
	[TOKEN_DETAIL_WIDE_CHARACTER_CONSTANT] = "cc:L",
	[TOKEN_DETAIL_LSQUARE_BRACKET] = "p:[",
	[TOKEN_DETAIL_RSQUARE_BRACKET] = "p:]",
	[TOKEN_DETAIL_LBRACKET] = "p:(",
	[TOKEN_DETAIL_RBRACKET] = "p:)",
	[TOKEN_DETAIL_LCURLY_BRACE] = "p:{",
	[TOKEN_DETAIL_RCURLY_BRACE] = "p:}",
	[TOKEN_DETAIL_DOT] = "p:.",
	[TOKEN_DETAIL_ARROW] = "p:->",
	[TOKEN_DETAIL_INCREMENT] = "p:++",
	[TOKEN_DETAIL_DECREMENT] = "p:--",
	[TOKEN_DETAIL_AMPERSAND] = "p:&",
	[TOKEN_DETAIL_ASTERISK] = "p:*",
	[TOKEN_DETAIL_PLUS] = "p:+",
	[TOKEN_DETAIL_MINUS] = "p:-",
	[TOKEN_DETAIL_TILDE] = "p:~",
	[TOKEN_DETAIL_BANG] = "p:!",
	[TOKEN_DETAIL_FSLASH] = "p:/",
	[TOKEN_DETAIL_PERCENT] = "p:%",
	[TOKEN_DETAIL_LSHIFT] = "p:<<",
	[TOKEN_DETAIL_RSHIFT] = "p:>>",
	[TOKEN_DETAIL_LOWER] = "p:<",
	[TOKEN_DETAIL_GREATER] = "p:>",
	[TOKEN_DETAIL_LOWER_EQUAL] = "p:<=",
	[TOKEN_DETAIL_GREATER_EQUAL] = "p:>=",
	[TOKEN_DETAIL_EQUAL] = "p:==",
	[TOKEN_DETAIL_NOT_EQUAL] = "p:!=",
	[TOKEN_DETAIL_CIRCUMFLEX] = "p:^",
	[TOKEN_DETAIL_BAR] = "p:|",
	[TOKEN_DETAIL_AND] = "p:&&",
	[TOKEN_DETAIL_OR] = "p:||",
	[TOKEN_DETAIL_QUESTION] = "p:?",
	[TOKEN_DETAIL_COLON] = "p::",
	[TOKEN_DETAIL_SEMICOLON] = "p:;",
	[TOKEN_DETAIL_ELLIPSIS] = "p:...",
	[TOKEN_DETAIL_ASSIGN] = "p:=",
	[TOKEN_DETAIL_ASTERISK_ASSIGN] = "p:*=",
	[TOKEN_DETAIL_FSLASH_ASSIGN] = "p:/=",
	[TOKEN_DETAIL_PERCENT_ASSIGN] = "p:%=",
	[TOKEN_DETAIL_PLUS_ASSIGN] = "p:+=",
	[TOKEN_DETAIL_MINUS_ASSIGN] = "p:-=",
	[TOKEN_DETAIL_LSHIFT_ASSIGN] = "p:<<=",
	[TOKEN_DETAIL_RSHIFT_ASSIGN] = "p:>>=",
	[TOKEN_DETAIL_AMPERSAND_ASSIGN] = "p:&=",
	[TOKEN_DETAIL_CIRCUMFLEX_ASSIGN] = "p:^=",
	[TOKEN_DETAIL_BAR_ASSIGN] = "p:|=",
	[TOKEN_DETAIL_COMMA] = "p:,",
};

const char *constant_suffix_to_str[] = {
	[CONSTANT_AFFIX_NONE] = "-",
	[CONSTANT_AFFIX_U] = "U",
	[CONSTANT_AFFIX_L] = "L",
	[CONSTANT_AFFIX_LL] = "LL",
	[CONSTANT_AFFIX_UL] = "UL",
	[CONSTANT_AFFIX_ULL] = "ULL",
	[CONSTANT_AFFIX_F] = "F",
};
