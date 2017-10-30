// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "pl0.h"
#include "set.h"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
int inBlockComment = 0;
void getch(void)
{
	//ZF note:
	//ll is the length of current reading line, cc is the output char's index in that line.
	//once cc == ll, means that you have to read a new line; otherwise, we buffered the current line, so just output line[++cc]

	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		//ZF note: 
		//Add the last char in the line, that is ' '
		line[++ll] = ' ';
		//Modified by ZF: to skip "//"
		int i, inLineComment = 0;
		for (i = 1; i <= ll; i++)
		{
			if (inBlockComment == 0 && line[i] == '/'&&line[i + 1] == '*')
			{
				inBlockComment = 1;
				line[i] = ' ';
			}
			else if(inBlockComment==1)
			{
				if (line[i] == '*'&&line[i+1] == '/')
				{
					line[i] = line[i+1] = ' ';
					inBlockComment = 0;
				}
				else
				{
					line[i] = ' ';
				}
			}
			if (inLineComment != 1 && inBlockComment != 1 && line[i] == '/'&& line[i + 1] == '/')
			{
				inLineComment = 1;
			}
			if (inLineComment == 1)
			{
				line[i] = ' ';
			}
		}
	}	

	//
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];
	//ZF note
	//Skip White Space and Tab keys
	while (ch == ' '||ch == '\t')
		getch();
	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		//ZFNote
		//Read in all the chars
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		//ZFNote
		//If is not a reserved word, in the end i will be -1
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		//ZFNote
		//Read in the number as integer
		//The int value will be stored in var num;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		//ZFNote:
		//Although in else case,we already get the next char,
		//Don't worry,Getsym() will not invoke getch() in the beginning if ch is not space/tab...
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '=')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_EQU;
			getch();
		}
		else
		{
			sym = SYM_NULL;
		}
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else
		{
			sym = SYM_BITAND;
			//getch();
		}
	}
	else if(ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else
		{
			sym = SYM_BITOR;
			//getch();
		}
	}
	else if(ch == '!')
	{
		sym = SYM_NOT;
		getch();
	}
	else if (ch == '^')
	{
		sym = SYM_XOR;
		getch();
	}
	//ZF note:
	//Sym is an operator(add, minus...)
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	//ZF note:
	//Code array is to store generated code
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	//ZD add: a new var entered
	///
	printf("name=%s index=%d real_pos=%d\n", id, dx, tx);

	strcpy(table[tx].name, id);
	//ZF note:
	//table array contains a name and a int represents the kind of that identifier.
	//Register the identifier we have just read in.
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		//ZF note:
		//Then we put the value in
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
//	printf("Try Position: %s\n", id);
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	//ZF note:
	//If sym is a identifier, then we must receive a "=" and a number
	//( for you cannot use a var to initialize a const)
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		//ZFNote:
		//sym is a new var's name
		enter(ID_VARIABLE);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration



//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
//ZF note:
//The most prior expr
//A signal constant/var
//and expression in parens
void factor(symset fsys)
{
//	void expression(symset fsys);
	void logic_or(symset fsys);
	int i;
	symset set;
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
//			mask* mk;//Stores the JMP code
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
					//ZF modified: Procedure can be a factor
				case ID_PROCEDURE:
					getsym();
					if (sym != SYM_LPAREN)
					{
						error(27);
					}
					//ZF add:
					//Following codes give the value to function arguments
					//Treat arguments like local vars!

					//mask* mk

					int j = 3;
					//		int para_dx = 3;
					do {
						getsym();
						if (sym == SYM_RPAREN)
						{
							break;
						}
						//mk = (mask*)&table[++j];
						logic_or(uniteset(fsys, createset(SYM_COMMA, SYM_RPAREN, SYM_NULL)));
						gen(MOV, 0, j++);
						//getsym();
					} while (sym == SYM_COMMA);
					if (sym != SYM_RPAREN)
					{
						error(22);
					}
					mk = (mask*)&table[i];
					gen(CAL, level - mk->level, mk->address);
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			logic_or(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NOT)
		{
			getsym();
			factor(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		test(fsys, createset(SYM_RPAREN, SYM_COMMA, SYM_NULL), 23);
	} // while
	//test(fsys, createset(SYM_RPAREN, SYM_COMMA, SYM_NULL), 23);
} // factor

//////////////////////////////////////////////////////////////////////
//ZF note:
//Generates a multiply/divide instruction
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH ||sym==SYM_MOD)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if(mulop == SYM_SLASH)
		{
			gen(OPR, 0, OPR_DIV);
		}
		else if (mulop == SYM_MOD)
		{
			gen(OPR, 0, OPR_MOD);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
//		destroyset(set);
/*		if (! inset(sym, relset))
		{
			error(20);
		}*/
		while (inset(sym, relset) && sym != SYM_NULL)
		{
			relop = sym;
			getsym();
			expression(set);
			switch (relop)
			{
			//Equal
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			//Not Equal
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition


//ZF add:
void bit_and(symset fsys)
{
	condition(uniteset(fsys, createset(SYM_BITAND, SYM_NULL)));
	while (sym == SYM_BITAND)
	{
		getsym();
		condition(uniteset(fsys, createset(SYM_BITAND, SYM_NULL)));
		gen(OPR, 0, OPR_BITAND);
	}
}
void bit_xor(symset fsys)
{
	bit_and(uniteset(fsys, createset(SYM_XOR, SYM_NULL)));
	while (sym == SYM_XOR)
	{
		getsym();
		bit_and(uniteset(fsys, createset(SYM_XOR, SYM_NULL)));
		gen(OPR, 0, OPR_XOR);
	}
}
void bit_or(symset fsys)
{
	bit_xor(uniteset(fsys, createset(SYM_BITOR, SYM_NULL)));
	while (sym == SYM_BITOR)
	{
		getsym();
		bit_xor(uniteset(fsys, createset(SYM_BITOR, SYM_NULL)));
		gen(OPR, 0, OPR_BITOR);
	}
}
//ZF add:
//Store the JPC codes
int JPCs[30];
//And expression
void logic_and(symset fsys)
{
	int i = 0;
	int j;
	bit_or(uniteset(fsys, createset(SYM_AND, SYM_NULL)));
	JPCs[i++] = cx;
	gen(JPC, 0, 0);
	gen(POP, 0, -1);
	while(sym == SYM_AND)
	{
		getsym();
		bit_or(uniteset(fsys, createset(SYM_AND, SYM_NULL)));
		gen(OPR, 0, OPR_AND);
		JPCs[i++] = cx;
		gen(JPC, 0, 0);
		gen(POP, 0, -1);
	}
	if (i != 1)
	{
		gen(JMP, 0, cx + 2);
		for (j = 0; j < i; j++)
		{
			code[JPCs[j]].a = cx;
		}
		gen(LIT, 0, 0);
	}
	else
	{
		cx -= 2;
	}
}
//ZF add:
//Or expression
//Realize short-out:
/*
	lit a
	lit b
	opr_or
	jpc(if result is 1,then jump to end)
	top++(prior instruction using JPC, causes top--)
	lit c
	opr_or
	jpc end
	lit d
	...
	...
	opr_or
	jpc end
	jmp end+1
	lit 0,1       <---end

*/
void logic_or(symset fsys)
{
	int i = 0;
	int j;
	logic_and(uniteset(fsys, createset(SYM_OR, SYM_NULL)));
	JPCs[i++] = cx;
	gen(JPC, 1, 0);
	gen(POP, 0, -1);
	while (sym == SYM_OR)
	{
		getsym();
		logic_and(uniteset(fsys, createset(SYM_OR, SYM_NULL)));
		gen(OPR, 0, OPR_OR);
		JPCs[i++] = cx;
		gen(JPC, 1, 0);
		gen(POP, 0, -1);
	}
	if (i != 1)
	{
		gen(JMP, 0, cx + 2);
		for (j = 0; j < i; j++)
		{
			code[JPCs[j]].a = cx;
		}
		gen(LIT, 0, 1);
	}
	else
	{
		cx -= 2;
	}
}

void para_list(symset fsys)
{
	while (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
		if (sym == SYM_RPAREN)
		{
			return;
		}
		if (sym != SYM_COMMA)
		{
			error(26);
		}
		getsym();
	}
	//getsym();
}
//////////////////////////////////////////////////////////////////////
void functioncall(int i)
{
	if (sym != SYM_LPAREN)
	{
		error(27);
	}
	//ZF add:
	//Following codes give the value to function arguments
	//Treat arguments like local vars!

	//mask* mk

	int j = 3;
	//		int para_dx = 3;
	do {
		getsym();
		if (sym == SYM_RPAREN)
		{
			break;
		}
		//mk = (mask*)&table[++j];
		logic_or(createset(SYM_COMMA, SYM_RPAREN, SYM_NULL));
		gen(MOV, 0, j++);
		//getsym();
	} while (sym == SYM_COMMA);
	if (sym != SYM_RPAREN)
	{
		error(22);
	}
	mask* mk = (mask*)&table[i];
	gen(CAL, level - mk->level, mk->address);
	getsym();
}
//////////////////////////////////////////////////////////////////////
//ZF note:
//It is a statement which like "i := 1"
//
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;
	//ZF note:
	//A declaration 
	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		logic_or(fsys);
		mk = (mask*) &table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		mask* mk;//Stores the JMP code
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
		/*
		if (sym != SYM_LPAREN)
		{
			error(27);
		}
		//ZF add:
		//Following codes give the value to function arguments
		//Treat arguments like local vars!

		//mask* mk

		int j = 3;
//		int para_dx = 3;
		do {
			getsym();
			if (sym == SYM_RPAREN)
			{
				break;
			}
			//mk = (mask*)&table[++j];
			logic_or(uniteset(fsys, createset(SYM_COMMA,SYM_RPAREN, SYM_NULL)));
			gen(MOV, 0, j++);
			//getsym();
		} while (sym == SYM_COMMA);
		if (sym != SYM_RPAREN)
		{
			error(22);
		}
		mk = (mask*)&table[i];
		gen(CAL, level - mk->level, mk->address);
		getsym();*/
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		else
		{
			error(27);
		}
		set1 = createset(SYM_BEGIN,SYM_RPAREN ,SYM_NULL);
		set = uniteset(set1, fsys);
		logic_or(set);
		destroyset(set1);
		destroyset(set);
		cx1 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(28);
		}
		statement(uniteset(fsys,createset(SYM_ELSE,SYM_NULL)));
		code[cx1].a = cx;//cx1 is the index of JPC instruction

		//ZF add 
		//ZF add:
		//
		if (sym != SYM_SEMICOLON)
		{
			error(17);
		}
		else
		{
			getsym();
		}
		if (sym == SYM_ELSE)
		{
			code[cx1].a += 1;
			cx1 = cx;
			gen(JMP, 0, 0);
			getsym();
			statement(fsys);
			code[cx1].a = cx;
		}

	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				getsym();
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if (sym == SYM_RETURN)
	{
		getsym();
		//getchar();
		logic_or(fsys);
		gen(OPR, 0, OPR_RET);
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	//ZF note:F
	//tx is the index of var in Identifier table
	mk = (mask*)&table[tx];
	//Current Instruction's index
	//!!!Begin index of the block!!!
	mk->address = cx;
	gen(JMP, 0, 0);	
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	//ZF add:
	//If block is Procedure, then check the para_list....
	if (sym == SYM_LPAREN)
	{
		getsym();
		para_list(fsys);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22);// Missing ')'
		}
	}


	//ZF note:
	//This loop seems that we have to do all the declaration at the beginning of the program
	do
	{
		//If first sym is "const", then we do a const declaration
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				//ZF note:
				//";" means the end of a line
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		//ZF note:
		//If symbol is a variable
		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
	}
	while (inset(sym, declbegsys) && sym!=SYM_PROCEDURE);
	block_dx = dx; //save dx before handling procedure call!
	while (sym == SYM_PROCEDURE)
	{ // procedure declarations
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			enter(ID_PROCEDURE);
			getsym();
		}
		else
		{
			error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
		}


		/*			if (sym == SYM_SEMICOLON)
					{
						getsym();
					}
					else
					{
						error(5); // Missing ',' or ';'.
					}*/

		level++;
		savedTx = tx;
		set1 = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset(set1, fsys);
		block(set);
		destroyset(set1);
		destroyset(set);
		tx = savedTx;
		level--;
		//ZF note:
		//If syms is ";"
		if (sym == SYM_SEMICOLON)
		{
			getsym();
			set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
			set = uniteset(statbegsys, set1);
			test(set, fsys, 6);
			destroyset(set1);
			destroyset(set);
		}
		else
		{
			error(5); // Missing ',' or ';'.
		}
	} // while
	dx = block_dx; //restore dx after handling procedure call!


	/*
	Following lines to check whether the next symbol is the
	begin of a statement...
	If not, error
	*/
	set1 = createset(SYM_IDENTIFIER, SYM_NULL);
	set = uniteset(statbegsys, set1);
	test(set, declbegsys, 7);
	destroyset(set1);
	destroyset(set);

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	//ZF note:
	//After all the declaration, then begin statement!
	//In this gen function, we see that i.a is set to block_dx, which is the end of local variable segment!!!
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
//To trace down along the static chain..
//Find out where the identifier locates at
//For instance   p0 defined p1, p1 defined p2
// Procedure  local_vars     level    dx
// p0         a              0        3 
// p1         b,c            1        3,4
// p2         d              2        3
// and p2 will visit a
// Then the level diff is 2-0=2, so p2 will go up for 2 layers
// and find the nearest var with name a
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 0;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		//for declaring const,vars,procedures?
		//ZF add:
		case POP:
			top -= i.a;
			break;
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				//stack[top] is the return value!
				/*

				static link(return value)(top-after)
				dynamic link
				return address
				return value(top-before)
				*/
				stack[b] = stack[top];
				top = b;
				//top = b - 1;
				pc = stack[top + 2];
				b = stack[top + 1];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_NOT:
				//top--;
				stack[top] = !stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			//ZF add:
			case OPR_BITAND:
				top--;
				stack[top] &= stack[top + 1];
				break;
			case OPR_BITOR:
				top--;
				stack[top] |= stack[top + 1];
				break;
			case OPR_XOR:
				top--;
				stack[top] ^= stack[top + 1];
				break;
			case OPR_MOD:
				top--;
				stack[top] %= stack[top + 1];
				break;

			//end ZF add
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			//ZF add:
			//"and" and "or" expressions
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			//ZF debug:
			printf("Store to Addr:%d   %d\n",i.a, base(stack, b, i.l) + i.a);

			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			//For find the actual mother function's base
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			// It's only for return  
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		//ZF add:
		//Use this to pass function arguments to the local_var segment 
		//Before CAL 
		case MOV:
			stack[top + i.a] = stack[top];
			top--;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			switch (i.l)
			{
			case 0:
				if (stack[top] == 0)
					pc = i.a;
				top--;
				break;
			case 1:
				if (stack[top] != 0)
					pc = i.a;
				top--;
				break;
			case 2:
				if (stack[top] > 0)
					pc = i.a;
				top--;
				break;
			}
			break;
		} // switch
		printf("%s %d %d\n", mnemonic[i.f], i.l, i.a);
		printf("----------Stack-----------\n");
		for (size_t si = 0; si < 15; si++)
		{
			printf(" %8d", stack[si]);
			if (top == si) printf("   <--top");
			if (b == si) printf("   <--base");
			printf("\n");
		}
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	//Relation symbols
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	//ZF note:
	//Set of declarations
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	//ZF note:
	//Set of Control flow
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_ELSE,SYM_WHILE, SYM_NULL);
	//ZF note:
	//A variable or A number or Left Parentheis or Minus or NULL
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_ODD, SYM_NOT, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();
	//ZFNote:
	//set1 is the end symbols
	set1 = createset(SYM_PERIOD, SYM_NULL);
	//set2 is the start symbols
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);

	printf("====Program Ends=====\n");
	for (size_t i = 0; i <= 20; i++)
	{
		mask *m = (mask*)&table[i];
		printf("name=%s level=%d addr=%d\n",m->name, m->level, m->address);
	}
	//ZF Modified:
	system("pause");
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
