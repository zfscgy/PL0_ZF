// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "pl0.h"
#include "set.h"
void mytest()
{
	int i = 2;
}

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
void error_s(char *msg)
{
	int i;
	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error Message: %s\n", msg);
	err++;
}
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
	else if (ch == '+')
	{
		getch();
		if (ch == '+')
		{
			sym = SYM_INC;
			getch();
		}
		else
		{
			sym = SYM_PLUS;
		}
	}
	else if (ch == '-')
	{
		getch();
		if (ch == '-')
		{
			sym = SYM_DEC;
			getch();
		}
		else
		{
			sym = SYM_MINUS;
		}
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
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
}
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;
	if (! in_symbol_set(sym, s1))
	{
		error(n);
		s = symset_unite(s1, s2);
		while(! in_symbol_set(sym, s))
			getsym();
		symset_destroy(s);
	}
}
void test_s(symset symset_leagal, symset symset_next, char *msg)
{
	symset s;
	if (!in_symbol_set(sym, symset_leagal))
	{
		error_s(msg);
		s = symset_unite(symset_leagal, symset_next);
		while (!in_symbol_set(sym, s))
			getsym();
		symset_destroy(s);
	}
}


int dx;  // data allocation index
// enter object(constant, variable or procedre) into table.
void identifier_enter(int kind)
{
	mask* mk;
	int offset = 1;
	tx++;
	printf("name=%s index=%d real_pos=%d\n", id, dx, tx);
	strcpy(identifier_table[tx].name, id);
	//table array contains a name and a int represents the kind of that identifier.
	//Register the identifier we have just read in.
	identifier_table[tx].kind = kind;
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
		identifier_table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &identifier_table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &identifier_table[tx];
		mk->level = level;
		break;
	case ID_ARRAY:
		mk = (mask*)&identifier_table[tx];
		mk->level = level;
		mk->address = dx;
		mk->dimension = arrayInfo[0];
		for (size_t i = 0; i < arrayInfo[0]; i++)
		{
			offset *= arrayInfo[i + 1];
			mk->indices[i] = arrayInfo[i + 1];
		}
		dx += offset;
	} // switch
} // enter

int identifier_position(char* id)
{
//	printf("Try Position: %s\n", id);
	int i;
	strcpy(identifier_table[0].name, id);
	i = tx + 1;
	while (strcmp(identifier_table[--i].name, id) != 0);
	return i;
} // position

void declaration_const()
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
				identifier_enter(ID_CONSTANT);
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
void declaration_var()
{
	if (sym == SYM_IDENTIFIER)
	{
		//ZFNote:
		//sym is a new var's name
		identifier_enter(ID_VARIABLE);
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

void declaration_array()
{
	int d = 0;
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym != SYM_LBRACKET)
		{
			error(30);
		}
		else
		{
			do {
				getsym();
				if (sym != SYM_NUMBER)
				{
				}
				else
				{
					arrayInfo[++d] = num;
				}
				getsym();
			} while (sym == SYM_COMMA);
			arrayInfo[0] = d;
			if (sym != SYM_RBRACKET)
			{
				error(31);
			}
			else
			{
				getsym();
			}
			identifier_enter(ID_ARRAY);
		}
	}
	else
	{
		error(4);
	}
}

void array_locate(int i)
{
	void expr_anyvalue(symset);
	mask* mk = (mask*)&identifier_table[i];
	if (sym != SYM_LBRACKET)
	{
		error(30);
	}
	gen(LIT, 0, 0);
	for (size_t d = 0; d < mk->dimension; d++)
	{
		getsym();
		expr_anyvalue(createset(SYM_COMMA, SYM_RBRACKET, SYM_NULL));
		for (size_t dl = d + 1; dl < mk->dimension; dl++)
		{
			gen(LIT, 0, mk->indices[dl]);
			gen(OPR, 0, OPR_MUL);
		}
		gen(OPR, 0, OPR_ADD);
	}
	if (sym != SYM_RBRACKET)
	{
		error(31);
	}
	getsym();
}

void expr_prime(symset fsys)
{
	void expr_function(int i);
	int i;
	mask* mk;
	if (sym == SYM_IDENTIFIER)
	{
		//			mask* mk;//Stores the JMP code
		if ((i = identifier_position(id)) == 0)
		{
			error_s("Undefined identifier");
		}
		else
		{
			switch (identifier_table[i].kind)
			{
			case ID_CONSTANT:
				gen(LIT, 0, identifier_table[i].value);
				getsym();
				break;
			case ID_VARIABLE:
				mk = (mask*)&identifier_table[i];
				gen(LOD, level - mk->level, mk->address);
				getsym();
				break;
				//ZF modified: Procedure can be a factor
			case ID_PROCEDURE:
				getsym();
				expr_function(i);
				break;
			case ID_ARRAY:
				mk = (mask*)&identifier_table[i];
				getsym();
				array_locate(i);
				gen(LODS, level - mk->level, mk->address);
				break;
			}
		}
	}
	else if (sym == SYM_NUMBER)
	{
		if (num > MAXADDRESS)
		{
			error_s("Number is too large, made it 0");
			num = 0;
		}
		gen(LIT, 0, num);
		getsym();
	}
}
//  ++,--
void expr_self(symset fsys)
{
	int selfsym;
	symset expr_prime_follow = symset_unite(fsys, end_symbols_self);
	if (in_symbol_set(sym, begin_symbols_self))
	{
		selfsym = sym;
		getsym();
		expr_self(fsys);
		if (code[cx - 1].f != LOD && code[cx - 1].f != LODS)
		{
			error_s("The expression is not a left value.");
		}
		else if (code[cx - 1].f == LODS)
		{
			code[cx - 1].f = LODSA;
		}
		switch (selfsym)
		{
		case SYM_DEC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			gen(code[cx - 3].f == LODSA ? STOS : STO, 0, code[cx - 3].a);
			gen(POP, 0, 1);
			gen(code[cx - 5].f == LODSA ? LODS : LOD, 0, code[cx - 5].a);
			break;
		case SYM_INC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_ADD);
			gen(code[cx - 3].f == LODSA ? STOS : STO, 0, code[cx - 3].a);
			gen(POP, 0, 1);
			gen(code[cx - 5].f == LODSA ? LODS : LOD, 0, code[cx - 5].a);
			break;
		case SYM_BITAND:
			if (code[cx - 1].f == LOD)
			{
				code[cx - 1].f = LDA;
			}
			else if (code[cx - 1].f == LODS)
			{
				cx--;
			}
			break;
		case SYM_TIMES:
			gen(LODS, 0, 0);
			break;
		}

	}
	else
	{
		expr_prime(expr_prime_follow);
	}
	if (in_symbol_set(sym, end_symbols_self))
	{
		if (code[cx - 1].f != LOD && code[cx - 1].f != LODS)
		{
			error_s("The expression is not a left value.");
		}
		if (code[cx - 1].f == LODS)
		{
			code[cx - 1].f = LODSA;
		}
		switch (sym)
		{
		case SYM_INC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_ADD);
			gen(code[cx - 3].f == LODSA ? STOS : STO, 0, code[cx - 3].a);
			if (code[cx - 3].f == LODSA)
			{
				gen(POP, 0, 1);
				gen(LODS, 0, 0);
			}
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			break;
		case SYM_DEC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			gen(code[cx - 3].f == LODSA ? STOS : STO, 0, code[cx - 3].a);
			if (code[cx - 3].f == LODSA)
			{
				gen(POP, 0, 1);
				gen(LODS, 0, 0);
			}
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_ADD);
			break;
		}
		getsym();
	}
}
void expr_factor(symset fsys)
{
	void expr_anyvalue(symset fsys);
	void expr_function(int i);
	int i;
	symset set;
	if (sym == SYM_LPAREN)
	{
		getsym();
		set = symset_unite(createset(SYM_RPAREN, SYM_NULL), fsys);
		expr_anyvalue(set);
		symset_destroy(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error_s("Expect ')' at the end of the expr.");
		}
	}
	else if (sym == SYM_NOT)
	{
		getsym();
		expr_factor(fsys);
		gen(OPR, 0, OPR_NOT);
	}
	else if (sym == SYM_BITNOT)
	{
		getsym();
		expr_factor(fsys);
		gen(OPR, 0, OPR_BITNOT);
	}
	else if (sym == SYM_MINUS)
	{
		getsym();
		expr_factor(fsys);
		gen(OPR, 0, OPR_NEG);
	}
	else
	{
		expr_self(fsys);
	}
}
// a*b,a/b
void expr_term(symset fsys)
{
	int mulop;
	symset set;	
	set = symset_unite(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD, SYM_NULL));
	expr_factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MOD)
	{
		mulop = sym;
		getsym();
		expr_factor(set);
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
	symset_destroy(set);
} // term
// a+b,a-b
void expr_arithmatic(symset fsys)
{
	int addop;
	symset set;

	set = symset_unite(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	expr_term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		expr_term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	}

	symset_destroy(set);
}
// a>b,a==b
void expr_comparation(symset fsys)
{
	int relop;
	symset set;
	set = symset_unite(symbols_relation, fsys);
	expr_arithmatic(set);
	while (in_symbol_set(sym, symbols_relation) && sym != SYM_NULL)
	{
		relop = sym;
		getsym();
		expr_arithmatic(set);
		switch (relop)
		{
		case SYM_EQU:
			gen(OPR, 0, OPR_EQU);
			break;
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
		}
	}
}
void expr_bit_and(symset fsys)
{
	expr_comparation(symset_unite(fsys, createset(SYM_BITAND, SYM_NULL)));
	while (sym == SYM_BITAND)
	{
		getsym();
		expr_comparation(symset_unite(fsys, createset(SYM_BITAND, SYM_NULL)));
		gen(OPR, 0, OPR_BITAND);
	}
}
void expr_bit_xor(symset fsys)
{
	expr_bit_and(symset_unite(fsys, createset(SYM_XOR, SYM_NULL)));
	while (sym == SYM_XOR)
	{
		getsym();
		expr_bit_and(symset_unite(fsys, createset(SYM_XOR, SYM_NULL)));
		gen(OPR, 0, OPR_XOR);
	}
}
void expr_bit_or(symset fsys)
{
	expr_bit_xor(symset_unite(fsys, createset(SYM_BITOR, SYM_NULL)));
	while (sym == SYM_BITOR)
	{
		getsym();
		expr_bit_xor(symset_unite(fsys, createset(SYM_BITOR, SYM_NULL)));
		gen(OPR, 0, OPR_BITOR);
	}
}
void check_branch()   
{
	int i;
	int d;
	if (code[cx - 1].f == JPC&&(code[cx - 1].l == -1 ||code[cx - 1].l == 1))
	{
		d = - code[cx - 1].l;
		if (code[cx - 2].f == OPR)
		{
			switch (code[cx - 2].a)
			{
			//Notice: if opr is >,<, must be reversed,but if opr is ==,!=,still the same
			//compares [top-1] with [top], in jpc, compares [top] with [top-1]
			case OPR_EQU:
				cx -= 2;
				gen(JPC, 12 * -d, code[cx + 1].a);
				break;
			case OPR_NEQ:
				cx -= 2;
				gen(JPC, -12 * -d, code[cx + 1].a);
				break;
			case OPR_GTR:
				cx -= 2;
				gen(JPC, 11 * d, code[cx + 1].a);
				break;
			case OPR_GEQ:
				cx -= 2;
				gen(JPC, 10 * d, code[cx + 1].a);
				break;
			case OPR_LES:
				cx -= 2;
				gen(JPC, -10 * d, code[cx + 1].a);
				break;
			case OPR_LEQ:
				cx -= 2;
				gen(JPC, -11 * d, code[cx + 1].a);
				break;
			}
		}
	}
}
void expr_condition_and(symset fsys,int *false_list,int *false_list_size,int *true_list, int *true_list_size)
{
	void expr_condition_or(symset,int*,int*,int*,int*);
	int i;
	if (1)//sym != SYM_LPAREN)
	{
		expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
		gen(JPC, J_Z, 0);
		check_branch();
		false_list[(*false_list_size)++] = cx - 1;
	}
	else
	{
		getsym();
		expr_condition_or(symset_unite(fsys, createset(SYM_RPAREN, SYM_NULL)), true_list, true_list_size, false_list, false_list_size);
		for (i = 0; i < *true_list_size; i++)
		{
			code[true_list[i]].a = cx;
		}
		getsym();
	}
	while (sym == SYM_AND)
	{
		if (*true_list_size > 1)
		{
			code[cx - 1].l = -code[cx - 1].l;
			false_list[(*false_list_size)++] = cx - 1;
		}
		*true_list_size = 0;
		getsym();
		if (1)//sym != SYM_LPAREN)
		{
			expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
			gen(JPC, J_Z, 0);
			check_branch();
			false_list[(*false_list_size)++] = cx - 1;
		}
		else
		{
			getsym();
			expr_condition_or(symset_unite(fsys, createset(SYM_RPAREN, SYM_NULL)), true_list, true_list_size, false_list, false_list_size);
			for (i = 0; i < *true_list_size; i++)
			{
				code[true_list[i]].a = cx;
			}
			getsym();
		} 
	};
}
void expr_condition_or(symset fsys,int *true_list,int *true_list_size, int *false_list, int *false_list_size)
{
	int i;
	expr_condition_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)), false_list, false_list_size, 
		true_list, true_list_size);
	check_branch();
	while (sym == SYM_OR)
	{
		for (i = 0; i < *false_list_size; i++)
		{
			code[false_list[i]].a = cx;
		}
		code[cx - 1].l = -code[cx - 1].l;
		true_list[(*true_list_size)++] = cx - 1;
		getsym();
		*false_list_size = 0;
		expr_condition_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)), false_list, false_list_size, true_list, true_list_size);
		check_branch();
	};
}
void expr_anyvalue(symset fsys)
{
	int true_list[30];
	int true_list_size = 0;
	int false_list[30];
	int false_list_size = 0;
	expr_condition_or(fsys, true_list, &true_list_size, false_list, &false_list_size);
	if (false_list_size == 1 && true_list_size == 0)
	{
		cx--;
	}
	else
	{
		int i;
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < false_list_size; i++)
		{
			code[false_list[i]].a = cx - 1;
		}
	}
}
void expr_logic_and(symset fsys)
{
	int JPCs[30];
	int i = 0;
	int j;
	expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
	JPCs[i++] = cx;
	gen(JPC, 0, 0);
	while(sym == SYM_AND)
	{
		getsym();
		expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
		gen(JPC, 0, 0);
		JPCs[i++] = cx - 1;
	}
	if (i != 1)
	{
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		for (j = 0; j < i; j++)
		{
			code[JPCs[j]].a = cx;
		}
		gen(LIT, 0, 0);
	}
	else
	{
		cx -= 1;
	}
}
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
void expr_logic_or(symset fsys)
{
	int JPCs[30];
	int i = 0;
	int j;
	expr_logic_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)));
	JPCs[i++] = cx;
	gen(JPC, 1, 0);
	while (sym == SYM_OR)
	{
		getsym();
		expr_logic_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)));
		gen(JPC, 1, 0);
		JPCs[i++] = cx - 1;
	}
	if (i != 1)
	{
		gen(LIT, 0, 0);
		gen(JMP, 0, cx + 2);
		for (j = 0; j < i; j++)
		{
			code[JPCs[j]].a = cx;
		}
		gen(LIT, 0, 1);
	}
	else
	{
		cx -= 1;
	}
}
void expr_assignment(symset fsys)
{
	int i,left_instruction_len;
	int cx0,cx1;
	instruction leftlod[10];
	cx0 = cx;
	expr_anyvalue(symset_unite(fsys,createset(SYM_BECOMES)));
	if (sym == SYM_BECOMES)
	{
		getsym();
		if (code[cx - 1].f == LOD || code[cx - 1].f == LODS)
		{
			leftlod[0] = code[cx - 1];
			cx--;
			expr_assignment(fsys);
			code[cx++] = leftlod[0];
			code[cx - 1].f += 2;
			if (code[cx - 1].f == STOS)
			{
				gen(POP, 0, 1);
				gen(LODS, 0, 0);
			}
		}
		else
		{
			error_s("Not a left value");
		}
	}
}
int function_paralist(symset fsys)
{
	int current_para_num = 0;
	int i;
	while (sym == SYM_IDENTIFIER)
	{
		identifier_enter(ID_VARIABLE);
		current_para_num++;
		getsym();
		if (sym == SYM_RPAREN)
		{
			break;
		}
		if (sym != SYM_COMMA)
		{
			error_s("Paralist encountered unexpected symbol.\nExpected ')' or ','.");
		}
		getsym();
	}
	dx -= current_para_num;
	for (i = 0; i < current_para_num; i++)
	{
		((mask*)&identifier_table[tx - i])->address = -i - 1;
	}
	return current_para_num;
}

void expr_function(int i) //i is the id of procedure identifier
{
	if (sym != SYM_LPAREN)
	{
		error(27);
	}
	//Following codes give the value to function arguments
	//Treat arguments like local vars!
	do {
		getsym();
		if (sym == SYM_RPAREN)
		{
			break;
		}
		expr_anyvalue(createset(SYM_COMMA, SYM_RPAREN, SYM_NULL));
	} while (sym == SYM_COMMA);
	if (sym != SYM_RPAREN)
	{
		error(22);
	}
	mask* mk = (mask*)&identifier_table[i];
	gen(CAL, level - mk->level, mk->address);
	getsym();
}
void statement(symset fsys,int *break_list,int *break_list_size,int *continue_list,int *continue_list_size)
{
	int i, cx1, cx2, cx3, cx4;
	symset inner_follow_symbols, set;
	if (sym == SYM_IF)
	{ // if statement
		int true_list[30];
		int true_list_size = 0;
		int false_list[30];
		int false_list_size = 0;
		getsym();
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		else
		{
			error_s("Expected '(' after 'if'");
		}
		set = createset(SYM_BEGIN,SYM_RPAREN ,SYM_NULL);
		inner_follow_symbols = symset_unite(set, fsys);
		expr_condition_or(set, true_list, &true_list_size, false_list, &false_list_size);
		symset_destroy(inner_follow_symbols);
		symset_destroy(set);
		for (i = 0; i < true_list_size; i++)
		{
			code[true_list[i]].a = cx;
		}
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error_s("Expected ')' after condition");
		}
		statement(symset_unite(fsys, createset(SYM_ELSE, SYM_NULL)), break_list, break_list_size, continue_list, continue_list_size);
		for (i = 0; i < false_list_size; i++)
		{
			code[false_list[i]].a = cx;
		}
		if (sym ==  SYM_ELSE)
		{
			for (i = 0; i < false_list_size; i++)
			{
				code[false_list[i]].a +=1;
			}
			cx1 = cx;
			gen(JMP, 0, 0);
			getsym();
			statement(fsys, break_list, break_list_size, continue_list, continue_list_size);
			code[cx1].a = cx;
		}
		else
		{

		}
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set = createset(SYM_END, SYM_NULL);
		inner_follow_symbols = symset_unite(set, begin_symbols_statement);
		statement(inner_follow_symbols, break_list, break_list_size, continue_list, continue_list_size);
		while (in_symbol_set(sym, begin_symbols_statement))
		{
			statement(inner_follow_symbols, break_list, break_list_size, continue_list, continue_list_size);
		}
		if (sym != SYM_END)
		{
			error_s("Expect 'end' after all statements.");
		}
		getsym();
		symset_destroy(set);
		symset_destroy(inner_follow_symbols);
	}
	else if (sym == SYM_WHILE)
	{ 
		int current_true_list[30];
		int current_true_list_size = 0;
		int current_false_list[30];
		int current_false_list_size = 0;
		int current_break_list[100];
		int current_break_list_size = 0;
		int current_continue_list[100];
		int current_continue_list_size = 0;
		getsym();
		if (sym != SYM_LPAREN)
		{
			error_s("Expect '(' after while.");
		}
		getsym();
		inner_follow_symbols = createset(SYM_RPAREN, SYM_NULL);
		cx1 = cx;
		expr_condition_or(inner_follow_symbols, current_true_list, &current_true_list_size, current_false_list, &current_false_list_size);
		symset_destroy(inner_follow_symbols);
		if (sym != SYM_RPAREN)
		{
			error_s("Expect ')' after while loop's condition.");
		}
		getsym();
		cx2 = cx;
		statement(fsys, current_break_list, &current_break_list_size, current_continue_list, &current_continue_list_size);
		gen(JMP, 0, cx1);
		for (i = 0; i < current_false_list_size; i++)
		{
			code[current_false_list[i]].a = cx;
		}
	}
	else if (sym == SYM_RETURN)
	{
		getsym();
		expr_anyvalue(fsys);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after return statement.");
		}
		getsym();
		gen(STO, 0, -para_num);
		gen(OPR,para_num, OPR_RET);
	}
	else if (sym == SYM_FOR)
	{
		int current_break_list[100];
		int current_break_list_size = 0;
		int current_continue_list[100];
		int current_continue_list_size = 0;
		int current_true_list[100];
		int current_true_list_size = 0;
		int current_false_list[100];
		int current_false_list_size = 0;
		//for (expr1; expr2; expr3)
		getsym();
		if (sym != SYM_LPAREN)
		{
			error_s("Expect '(' after for");
		}
		getsym();
		expr_assignment(fsys);
		gen(POP, 0, 1);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after for loop's expr1");
		}
		getsym();
		cx1 = cx;  //Here cx1 is the beginning of expr2
		expr_condition_or(fsys, current_true_list, &current_true_list_size, current_false_list, &current_false_list_size);
		for (i = 0; i < current_true_list_size; i++)
		{
			code[current_true_list[i]].a = cx;
		}
		gen(JMP, 0, 0);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after for loop's condition");
		}
		getsym();
		cx2 = cx; // expr3, mostly incremental
		expr_assignment(fsys);
		gen(POP, 0, 1);
		gen(JMP, 0, cx1);
		if (sym != SYM_RPAREN)
		{
			error_s("Expect ')' after 'for(expr1;expr2;expr3)'");
		}
		getsym();
		code[cx2 - 1].a = cx;
		statement(fsys, current_break_list, &current_break_list_size, current_continue_list, &current_continue_list_size);
		gen(JMP, 0, cx2);
		for (i = 0; i < current_false_list_size; i++)
		{
			code[current_false_list[i]].a = cx;
		}
		for (i = 0; i < current_break_list_size; i++)
		{
			code[current_break_list[i]].a = cx;
		}
	}
	else if (sym == SYM_BREAK)
	{
		if (break_list == NULL)
		{
			error_s("Cannot add 'break' outside for loop or while loop.");
		}
		else
		{
			break_list[(*break_list_size)++] = cx;
			gen(JMP, 0, 0);
			getsym();
		}
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after break.");
		}
		getsym();
	}
	else if (sym == SYM_CONTINUE)
	{
		if (continue_list == NULL)
		{
			error_s("Cannot add 'continue' outside for loop or while loop.");
		}
		else
		{
			continue_list[(*continue_list_size)++] = cx;
			gen(JMP, 0, 0);
			getsym();
		}
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after contnue.");
		}
		getsym();
	}
	else if (sym == SYM_SEMICOLON)
	{
		getsym();
		return;
	}
	else
	{
		expr_assignment(fsys);
		gen(POP, 0, 1);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' in the end of statement.");
		}
		getsym();
	}
}
void block(symset fsys)
{
	int cx0; // initial code index
	int block_para_num = 0;
	mask* mk;
	int block_dx;
	int savedTx;
	symset inner_follow_symbols, set;
	dx = 3;
	block_dx = dx;
	//ZF note:F
	//tx is the index of var in Identifier table
	mk = (mask*)&identifier_table[tx];
	//Current Instruction's index
	//!!!Begin index of the block!!!
	mk->address = cx;
	gen(JMP, 0, 0);	
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	if (sym == SYM_LPAREN)
	{
		getsym();
		block_para_num = function_paralist(fsys);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error_s("Expect ')' at the end of para_list.");
		}
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				declaration_const();
				while (sym == SYM_COMMA)
				{
					getsym();
					declaration_const();
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
		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			if (sym != SYM_LBRACKET)
			{
				do
				{
					declaration_var();
					while (sym == SYM_COMMA)
					{
						getsym();
						declaration_var();
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
			}
			else
			{
				getsym();
				if (sym != SYM_RBRACKET)
				{
					error(31);
				}
				getsym();
				do {
					declaration_array();
					while (sym == SYM_COMMA)
					{
						getsym();
						declaration_array();
					}
					if (sym == SYM_SEMICOLON)
					{
						getsym();
					}
					else
					{
						error(5);
					}
					
				} while (sym == SYM_IDENTIFIER);
			}
		} // if
	}
	while (in_symbol_set(sym, begin_symbols_declaration) && sym!=SYM_PROCEDURE);
	block_dx = dx; //save dx before handling procedure call!
	while (sym == SYM_PROCEDURE)
	{ // procedure declarations
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			identifier_enter(ID_PROCEDURE);
			getsym();
		}
		else
		{
			error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
		}
		level++;
		savedTx = tx;
		set = createset(SYM_BEGIN, SYM_NULL);
		inner_follow_symbols = symset_unite(set, fsys);
		block(inner_follow_symbols);
		symset_destroy(inner_follow_symbols);
		symset_destroy(set);
		tx = savedTx;
		level--;
	}
	dx = block_dx; //restore dx after handling procedure call!


	/*
	Following lines to check whether the next symbol is the
	begin of a statement...
	If not, error
	*/
	set = createset(SYM_BEGIN, SYM_NULL);
	test_s(set, set, "Expect 'begin' at the beginning of block.");
	symset_destroy(set);

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	//ZF note:
	//After all the declaration, then begin statement!
	//In this gen function, we see that i.a is set to block_dx, which is the end of local variable segment!!!
	gen(INT, 0, block_dx);
	set = createset(SYM_PERIOD,SYM_NULL);
	inner_follow_symbols= symset_unite(set, fsys);
	para_num = block_para_num;
	statement(inner_follow_symbols, NULL, NULL, NULL, NULL);
	symset_destroy(inner_follow_symbols);
	symset_destroy(set);
	gen(OPR, para_num, OPR_RET); // return
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
		printf("pc: %d\n", pc);
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
				static link
				dynamic link       <-- b
				return address
				return value temp  <-- top
				*/
				top = b + 2;
				pc = stack[top];
				top--;
				b = stack[top];
				top -= (i.l + 1);
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
			case OPR_BITAND:
				top--;
				stack[top] &= stack[top + 1];
				break;
			case OPR_BITOR:
				top--;
				stack[top] |= stack[top + 1];
				break;
			case OPR_BITNOT:
				stack[top] = ~stack[top];
				break;
			case OPR_XOR:
				top--;
				stack[top] ^= stack[top + 1];
				break;
			case OPR_MOD:
				top--;
				stack[top] %= stack[top + 1];
				break;
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
		case LODS:
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STO:
			printf("Store to Addr:%d   %d\n",i.a, base(stack, b, i.l) + i.a);
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			break;
		case STOS:
			printf("Store to Addr:%d   %d\n", i.a, base(stack, b, i.l) + i.a + stack[top - 1]);
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			printf("%d\n", stack[top]);
			break;
		case LODSA:
			stack[top + 1] = stack[base(stack, b, i.l) + i.a + stack[top]];
			top++;
			break;
		case LDA:
			stack[++top] = base(stack, b, i.l) + i.a;
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
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			switch (i.l)
			{
			case J_Z:
				if (stack[top] == 0)
					pc = i.a;
				top--;
				break;
			case J_NZ:
				if (stack[top] != 0)
					pc = i.a;
				top--;
				break;
			case J_POS:
				if (stack[top] > 0)
					pc = i.a;
				top--;
				break;
			case J_GRT:
				if (stack[top] > stack[top - 1])
					pc = i.a;
				top -= 2;
				break;
			case J_GRE:
				if (stack[top] >= stack[top - 1])
					pc = i.a;
				top -= 2;
				break;
			case J_LES:
				if (stack[top] < stack[top - 1])
					pc = i.a;
				top -= 2;
				break;
			case J_LEQ:
				if (stack[top] <= stack[top - 1])
					pc = i.a;
				top -= 2;
				break;
			case J_EQU:
				if (stack[top] == stack[top - 1])
					pc = i.a;
				top -= 2;
				break;
			case J_NEQ:
				if (stack[top] != stack[top - 1])
					pc = i.a;
				top -= 2;
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

void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;
	mytest();

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		system("pause");
		exit(1);
	}

	phi = createset(SYM_NULL);
	symbols_relation = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	begin_symbols_self = createset(SYM_NOT, SYM_BITNOT, SYM_INC, SYM_DEC,
		SYM_BITAND, SYM_TIMES, SYM_NULL);
	end_symbols_self = createset(SYM_INC, SYM_DEC, SYM_NULL);
	begin_symbols_declaration = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	begin_symbols_statement = createset(SYM_BEGIN, SYM_IDENTIFIER, SYM_BITAND, SYM_TIMES,
		SYM_IF, SYM_FOR, SYM_WHILE, SYM_SEMICOLON, SYM_BREAK, SYM_CONTINUE,
		SYM_INC, SYM_DEC, SYM_NULL);
	begin_symbols_primeexpr = createset(SYM_NUMBER,SYM_IDENTIFIER);
	begin_symbols_fact = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_ODD, SYM_NOT, SYM_NULL);
	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;
	getsym();
	//ZFNote:
	//set1 is the end symbols
	set1 = createset(SYM_PERIOD, SYM_NULL);
	//set2 is the start symbols
	set2 = symset_unite(begin_symbols_declaration, begin_symbols_statement);
	set = symset_unite(set1, set2);
	block(set);
	symset_destroy(set1);
	symset_destroy(set2);
	symset_destroy(set);
	symset_destroy(phi);
	symset_destroy(symbols_relation);
	symset_destroy(begin_symbols_declaration);
	symset_destroy(begin_symbols_statement);
	symset_destroy(begin_symbols_fact);	
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
		mask *m = (mask*)&identifier_table[i];
		printf("name=%s level=%d addr=%d\n",m->name, m->level, m->address);
	}
	system("pause");
}

