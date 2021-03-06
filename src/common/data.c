#include <common/enums.h>
#include <ast/enums.h>

const char *token2str[TOKEN_END] = {
	[TOKEN_NONE                   ] = "x:<END>",
	[TOKEN_AUTO                   ] = "k:auto",
	[TOKEN_BREAK                  ] = "k:break",
	[TOKEN_CASE                   ] = "k:case",
	[TOKEN_CHAR                   ] = "k:char",
	[TOKEN_CONST                  ] = "k:const",
	[TOKEN_CONTINUE               ] = "k:continue",
	[TOKEN_DEFAULT                ] = "k:default",
	[TOKEN_DO                     ] = "k:do",
	[TOKEN_DOUBLE                 ] = "k:double",
	[TOKEN_ELSE                   ] = "k:else",
	[TOKEN_ENUM                   ] = "k:enum",
	[TOKEN_EXTERN                 ] = "k:extern",
	[TOKEN_FLOAT                  ] = "k:float",
	[TOKEN_FOR                    ] = "k:for",
	[TOKEN_GOTO                   ] = "k:goto",
	[TOKEN_IF                     ] = "k:if",
	[TOKEN_INLINE                 ] = "k:inline",
	[TOKEN_INT                    ] = "k:int",
	[TOKEN_LONG                   ] = "k:long",
	[TOKEN_REGISTER               ] = "k:register",
	[TOKEN_RESTRICT               ] = "k:restrict",
	[TOKEN_RETURN                 ] = "k:return",
	[TOKEN_SHORT                  ] = "k:short",
	[TOKEN_SIGNED                 ] = "k:signed",
	[TOKEN_SIZEOF                 ] = "k:sizeof",
	[TOKEN_STATIC                 ] = "k:static",
	[TOKEN_STRUCT                 ] = "k:struct",
	[TOKEN_SWITCH                 ] = "k:switch",
	[TOKEN_TYPEDEF                ] = "k:typedef",
	[TOKEN_UNION                  ] = "k:union",
	[TOKEN_UNSIGNED               ] = "k:union",
	[TOKEN_VOID                   ] = "k:void",
	[TOKEN_VOLATILE               ] = "k:volatile",
	[TOKEN_WHILE                  ] = "k:while",
	[TOKEN_BOOL                   ] = "k:_Bool",
	[TOKEN_COMPLEX                ] = "k:_Complex",
	[TOKEN_IMAGINARY              ] = "k:_Imaginary",
	[TOKEN_IDENTIFIER             ] = "i:",
	[TOKEN_STRING_LITERAL         ] = "s:",
	[TOKEN_INTEGER_CONSTANT       ] = "ic:",
	[TOKEN_FLOATING_CONSTANT      ] = "fc:",
	[TOKEN_ENUMERATION_CONSTANT   ] = "ec:",
	[TOKEN_CHARACTER_CONSTANT     ] = "cc:",
	[TOKEN_LSQUARE_BRACKET        ] = "p:[",
	[TOKEN_RSQUARE_BRACKET        ] = "p:]",
	[TOKEN_LBRACKET               ] = "p:(",
	[TOKEN_RBRACKET               ] = "p:)",
	[TOKEN_LCURLY_BRACE           ] = "p:{",
	[TOKEN_RCURLY_BRACE           ] = "p:}",
	[TOKEN_DOT                    ] = "p:.",
	[TOKEN_ARROW                  ] = "p:->",
	[TOKEN_INCREMENT              ] = "p:++",
	[TOKEN_DECREMENT              ] = "p:--",
	[TOKEN_AMPERSAND              ] = "p:&",
	[TOKEN_ASTERISK               ] = "p:*",
	[TOKEN_PLUS                   ] = "p:+",
	[TOKEN_MINUS                  ] = "p:-",
	[TOKEN_TILDE                  ] = "p:~",
	[TOKEN_BANG                   ] = "p:!",
	[TOKEN_FSLASH                 ] = "p:/",
	[TOKEN_PERCENT                ] = "p:%",
	[TOKEN_LSHIFT                 ] = "p:<<",
	[TOKEN_RSHIFT                 ] = "p:>>",
	[TOKEN_LOWER                  ] = "p:<",
	[TOKEN_GREATER                ] = "p:>",
	[TOKEN_LOWER_EQUAL            ] = "p:<=",
	[TOKEN_GREATER_EQUAL          ] = "p:>=",
	[TOKEN_EQUAL                  ] = "p:==",
	[TOKEN_NOT_EQUAL              ] = "p:!=",
	[TOKEN_CIRCUMFLEX             ] = "p:^",
	[TOKEN_BAR                    ] = "p:|",
	[TOKEN_AND                    ] = "p:&&",
	[TOKEN_OR                     ] = "p:||",
	[TOKEN_QUESTION               ] = "p:?",
	[TOKEN_COLON                  ] = "p::",
	[TOKEN_SEMICOLON              ] = "p:;",
	[TOKEN_ELLIPSIS               ] = "p:...",
	[TOKEN_ASSIGN                 ] = "p:=",
	[TOKEN_ASTERISK_ASSIGN        ] = "p:*=",
	[TOKEN_FSLASH_ASSIGN          ] = "p:/=",
	[TOKEN_PERCENT_ASSIGN         ] = "p:%=",
	[TOKEN_PLUS_ASSIGN            ] = "p:+=",
	[TOKEN_MINUS_ASSIGN           ] = "p:-=",
	[TOKEN_LSHIFT_ASSIGN          ] = "p:<<=",
	[TOKEN_RSHIFT_ASSIGN          ] = "p:>>=",
	[TOKEN_AMPERSAND_ASSIGN       ] = "p:&=",
	[TOKEN_CIRCUMFLEX_ASSIGN      ] = "p:^=",
	[TOKEN_BAR_ASSIGN             ] = "p:|=",
	[TOKEN_COMMA                  ] = "p:,",
};

const char *affix2str[CONSTANT_AFFIX_END] = {
	[CONSTANT_AFFIX_NONE] = "-",
	[CONSTANT_AFFIX_U   ] = "U",
	[CONSTANT_AFFIX_L   ] = "L",
	[CONSTANT_AFFIX_LL  ] = "LL",
	[CONSTANT_AFFIX_UL  ] = "UL",
	[CONSTANT_AFFIX_ULL ] = "ULL",
	[CONSTANT_AFFIX_F   ] = "F",
};

const char *op2str[OPERATOR_END] = {
	[OPERATOR_NONE          ] = ":<unknown>",
	[OPERATOR_INDEX         ] = ":[index]",
	[OPERATOR_CALL          ] = ":(call)",
	[OPERATOR_FIELD         ] = ":.field",
	[OPERATOR_ARROW         ] = ":->arrow",
	[OPERATOR_POST_INC      ] = ":post++",
	[OPERATOR_POST_DEC      ] = ":post--",
	[OPERATOR_COMPOUND      ] = ":(cpnd){}",
	[OPERATOR_PRE_INC       ] = ":++pre",
	[OPERATOR_PRE_DEC       ] = ":--pre",
	[OPERATOR_ADDRESS_OF    ] = ":&addr",
	[OPERATOR_DEREF         ] = ":*deref",
	[OPERATOR_UN_PLUS       ] = ":+pos",
	[OPERATOR_UN_MINUS      ] = ":-neg",
	[OPERATOR_BIT_NOT       ] = ":~not",
	[OPERATOR_LOGICAL_NOT   ] = ":!not",
	[OPERATOR_SIZEOF_EXPR   ] = ":sizeof",
	[OPERATOR_SIZEOF_TYPE   ] = ":(sizeof)",
	[OPERATOR_MUL           ] = ":mul*",
	[OPERATOR_DIV           ] = ":div/",
	[OPERATOR_MOD           ] = ":mod%",
	[OPERATOR_ADD           ] = ":add+",
	[OPERATOR_SUB           ] = ":sub-",
	[OPERATOR_LSHIFT        ] = ":lsh<<",
	[OPERATOR_RSHIFT        ] = ":rsh>>",
	[OPERATOR_LOWER         ] = ":lt<",
	[OPERATOR_GREATER       ] = ":gt>",
	[OPERATOR_LEQUAL        ] = ":leq<=",
	[OPERATOR_GEQUAL        ] = ":geq>=",
	[OPERATOR_EQUAL         ] = ":eq==",
	[OPERATOR_NEQUAL        ] = ":neq!=",
	[OPERATOR_BIT_AND       ] = ":and&",
	[OPERATOR_BIT_XOR       ] = ":xor^",
	[OPERATOR_BIT_OR        ] = ":or|",
	[OPERATOR_LOGICAL_AND   ] = ":and&&",
	[OPERATOR_LOGICAL_OR    ] = ":or||",
	[OPERATOR_TERNARY       ] = ":?ter:",
	[OPERATOR_ASSIGN        ] = ":as=",
	[OPERATOR_MUL_ASSIGN    ] = ":mas*=",
	[OPERATOR_DIV_ASSIGN    ] = ":das/=",
	[OPERATOR_MOD_ASSIGN    ] = ":mas%=",
	[OPERATOR_ADD_ASSIGN    ] = ":aas+=",
	[OPERATOR_SUB_ASSIGN    ] = ":sas-=",
	[OPERATOR_LSHIFT_ASSIGN ] = ":las<<=",
	[OPERATOR_RSHIFT_ASSIGN ] = ":ras>>=",
	[OPERATOR_BIT_AND_ASSIGN] = ":bas&=",
	[OPERATOR_BIT_XOR_ASSIGN] = ":bas^=",
	[OPERATOR_BIT_OR_ASSIGN ] = ":bas|=",
	[OPERATOR_COMMA         ] = ":comma,"
};

const char *declspec2str[DECLSPEC_END] = {
	[DECLSPEC_NONE             ] = ":<unknown>",
	[DECLSPEC_TYPEDEF          ] = ":typedef",
	[DECLSPEC_EXTERN           ] = ":extern",
	[DECLSPEC_STATIC           ] = ":static",
	[DECLSPEC_AUTO             ] = ":auto",
	[DECLSPEC_REGISTER         ] = ":register",
	[DECLSPEC_VOID             ] = ":void",
	[DECLSPEC_CHAR             ] = ":char",
	[DECLSPEC_SHORT            ] = ":short",
	[DECLSPEC_INT              ] = ":int",
	[DECLSPEC_LONG             ] = ":long",
	[DECLSPEC_FLOAT            ] = ":float",
	[DECLSPEC_DOUBLE           ] = ":double",
	[DECLSPEC_SIGNED           ] = ":signed",
	[DECLSPEC_UNSIGNED         ] = ":unsigned",
	[DECLSPEC_BOOL             ] = ":_Bool",
	[DECLSPEC_COMPLEX          ] = ":_Complex",
	[DECLSPEC_IMAGINARY        ] = ":_Imaginary",
	[DECLSPEC_STRUCT           ] = ":struct",
	[DECLSPEC_STRUCT_DEFINITION] = ":struct:",
	[DECLSPEC_UNION            ] = ":union",
	[DECLSPEC_UNION_DEFINITION ] = ":union:",
	[DECLSPEC_ENUM             ] = ":enum",
	[DECLSPEC_ENUM_DEFINITION  ] = ":enum:",
	[DECLSPEC_TYPEDEF_NAME     ] = ":$_t",
	[DECLSPEC_CONST            ] = ":const",
	[DECLSPEC_RESTRICT         ] = ":restrict",
	[DECLSPEC_VOLATILE         ] = ":volatile",
	[DECLSPEC_INLINE           ] = ":inline",
};

