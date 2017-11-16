#include <stdio.h>

#define NRW        17     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       17     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_CONST,		// const
	SYM_VAR,		// var
	SYM_PROCEDURE,	// procedure
	SYM_NUMBER,		// 0123456789
	SYM_PLUS,		// +
	SYM_MINUS,		// -
	SYM_TIMES,		// * 
	SYM_SLASH,		// /
	SYM_SHR,		// >>
	SYM_SHL,		// <<
	SYM_ODD,		// odd
	SYM_INC,		// ++
	SYM_DEC,		// --
	SYM_ADDTO,		// +=
	SYM_SUBTO,		// -=
	SYM_MULTO,		// *=
	SYM_DIVTO,		// /=
	SYM_EQU,		// ==
	SYM_NEQ,		// !=
	SYM_LES,		// <
	SYM_LEQ,		// <=
	SYM_GTR,		// >
	SYM_GEQ,		// >=
	SYM_LBRACKET,	// [
	SYM_RBRACKET,	// ]
	SYM_NOT,		// !
	SYM_AND,		// &&
	SYM_OR,			// ||
	SYM_BITAND,		// &
	SYM_BITOR,		// |
	SYM_XOR,		// ^
	SYM_BITNOT,		// ~
	SYM_MOD,		// %
	SYM_LPAREN,		// (
	SYM_RPAREN,		// )         
	SYM_COMMA,		// ,
	SYM_SEMICOLON,	// ;
	SYM_PERIOD,		// .
	SYM_BECOMES,	// :=
    SYM_BEGIN,		// begin     
	SYM_END,		// end
	SYM_IF,			// if
	SYM_THEN,		// then
	SYM_WHILE,		// while
	SYM_DO,			// do
	SYM_CALL,		// call

	SYM_ELSE,		// else
	SYM_ELIF,		// elif
	SYM_RETURN,		// return
	SYM_FOR,		// for
	SYM_BREAK,
	SYM_CONTINUE,
	SYM_AT,
};

enum idtype
{
	ID_CONSTANT,
	ID_VARIABLE,
	ID_PROCEDURE,
	ID_ARRAY, 
};

enum opcode
{
	LIT,
	OPR,
	LOD,
	LODS,
	STO,
	STOS,
	LDA,
	CAL,
	INT,
	JMP,
	JPC,
	MOV,
	POP
};

enum oprcode
{
	OPR_RET,
	OPR_NEG,
	OPR_ADD,
	OPR_MIN,
	OPR_MUL,
	OPR_DIV,
	OPR_ODD,
	OPR_EQU,
	OPR_NEQ,
	OPR_LES,
	OPR_LEQ,
	OPR_GTR,
	OPR_GEQ,
	OPR_AND,
	OPR_OR,
	OPR_NOT,
	OPR_BITAND,
	OPR_BITOR,
	OPR_XOR,
	OPR_MOD,
	OPR_BITNOT,
};
enum jmpcode
{
	J_Z = 1,
	J_NZ = -1,
	J_POS = 2,
	J_GRT = 10,
	J_GRE = 11,
	J_EQU = 12,
	J_LEQ = -10,
	J_LES = -11,
	J_NEQ  = -12,
};

typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "',' expected",
/* 27 */    "'(' expected",
/* 28 */    "'end' expected",
/* 29 */    "'begin' expected",
/* 30 */    "'[' expected",
/* 31 */    "']' expected",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
unsigned char arrayInfo[5]; //current array
char line[300];
int para_num;

instruction code[CXMAX];
//ZF note
//This array contains all reserved word
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while",
	"else", "elif", "return", "for","break","continue",
};
//ZF note
//This array contains all Symbols corresponding to the reserved word
//They must be in the same order
int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_ELSE, SYM_ELIF, SYM_RETURN, SYM_FOR,SYM_BREAK,SYM_CONTINUE,
};

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', ',', '.', ';',
	'[',']','!','&','|','^','%'
};

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LBRACKET,SYM_RBRACKET,SYM_NOT,
	SYM_BITAND,SYM_BITOR,SYM_XOR,SYM_MOD,
};



#define MAXINS   13
#define MAXD	 4
char* mnemonic[MAXINS] =
{
	"LIT",
	"OPR",
	"LOD",
	"LODS", 
	"STO",
	"STOS",
	"LDA",
	"CAL",
	"INT",
	"JMP",
	"JPC",
	"MOV",
	"POP",
};

typedef struct
{
	char name[MAXIDLEN + 1];
	unsigned char dimension;
	int  kind;
	int  value;
	unsigned char indices[MAXD];
} comtab;

comtab identifier_table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	unsigned char dimension;
	int   kind;
	short level;
	short address;
	unsigned char indices[MAXD];
} mask;

FILE* infile;

// EOF PL0.h
