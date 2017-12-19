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
	int a[10];
	int *p = &a;
	//*(p++)--;
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
		line_count++;
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
			line_code[line_count][ll - 1] = ch;
		} // while
		//strcpy(line_code[line_count], &line[1]);
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
			sym = SYM_COLON;       // illegal?
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
		else if (ch == '>')
		{
			sym = SYM_SHR;
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
		else if (ch == '<')
		{
			sym = SYM_SHL;
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
			sym = SYM_BECOMES;
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

	cx_line[cx] = line_count;
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
void skip(symset _end_symbols)
{
	while (!in_symbol_set(sym, _end_symbols))
	{
		getsym();
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
	case ID_PTR:
		mk = (mask*)&identifier_table[tx];
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
		break;
	case ID_TAG:
		mk = (mask*)&identifier_table[tx];
		mk->address = cx;
		break;
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
int function_position(char *id, int para_num)
{
	int i;
	strcpy(identifier_table[0].name, id);
	i = tx;
	while (strcmp(identifier_table[i].name, id) != 0 || 
		(identifier_table[i].dimension != para_num && identifier_table[i].kind != ID_PTR))
	{
		i--;
	}
	return i;
}


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
	void expr_assignment(symset fsys);
	if (sym == SYM_IDENTIFIER)
	{
		//ZFNote:
		//sym is a new var's name
		identifier_enter(ID_VARIABLE);
		getsym();
		if (sym == SYM_BECOMES)
		{
			symset inner_follow_symbols = createset(SYM_COMMA, SYM_SEMICOLON, SYM_NULL);
			getsym();
			expr_assignment(inner_follow_symbols);
			symset_destroy(inner_follow_symbols);
			gen(STO, 0, dx - 1);
			gen(POP, 0, 1);
		}
	}
	else
	{
		error_s("Expect an identifier after 'var'.");
		skip(end_symbols);
	}
} // vardeclaration
void declaration_pointer()
{
	int d = 0;
	if (sym == SYM_IDENTIFIER)
	{ 
		identifier_enter(ID_PTR);
		getsym();
		if (sym == SYM_LBRACKET)
		{
			do {
				getsym();
				if (sym != SYM_NUMBER)
				{
					error_s("Expect number in '[...]' in pointer declaration.");
				}
				else
				{
					identifier_table[tx].indices[d++] = num;
				}
				getsym();
			} while (sym == SYM_COMMA);
			if (sym != SYM_RBRACKET)
			{
				error_s("Expect ']' after dimension in pointer declaration.");
				skip(end_symbols); return;
			}
			getsym();
		}
		if (sym == SYM_BECOMES)
		{
			symset inner_follow_symbols = createset(SYM_COMMA, SYM_SEMICOLON, SYM_NULL);
			getsym();
			expr_assignment(inner_follow_symbols);
			gen(STO, 0, dx - 1);
			gen(POP, 0, 1);
			symset_destroy(inner_follow_symbols);
		}
		identifier_table[tx].dimension = d > 1 ? d : 1;
	}
	else
	{
		error_s("Expect an identifier after 'ptr'.");
		skip(end_symbols); return;
	}
}
void declaration_array()
{
	int d = 0;
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym != SYM_LBRACKET)
		{
			error_s("Expect '[' after identifier in array declaration.");
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
				error_s("Expect ']' at the end of dimensions in array declaration.");
			}
			getsym();
			int dx_begin = dx;
			identifier_enter(ID_ARRAY);
			if (sym == SYM_BECOMES)
			{
				int dimCount[5] = { 0,0,0,0,0 };
				int current_dimension = 0;
				int current_index = 0;
				int current_offset = 0;
				getsym();
				symset inner_follow_symbols = createset(SYM_COMMA, SYM_RBRACE, SYM_NULL);
				while (sym == SYM_LBRACE)
				{
					current_dimension++;
					getsym();
					if (current_dimension > arrayInfo[0])
					{
						error_s("Array initialization error.Dimension not match!");
						skip(end_symbols); return;
					}
					while (sym != SYM_RBRACE && sym!= SYM_LBRACE && sym!= SYM_SEMICOLON)
					{
						expr_assignment(inner_follow_symbols);
						gen(STO, 0, dx_begin + current_offset);
						current_offset++;
						dimCount[current_dimension]++;
						if (sym == SYM_RBRACE)
						{
							break;
						}
						else if (sym != SYM_COMMA)
						{
							error_s("Expect ',' between numbers in array declaration.");
						}
						getsym();
					}
					while (sym == SYM_RBRACE)
					{
						if (dimCount[current_dimension] < arrayInfo[current_dimension])
						{
							int unFilledNumbers = 1;
							int d;
							for (d = current_dimension; d < arrayInfo[0]; d++)
							{
								unFilledNumbers *= arrayInfo[d];
							}
							for (d = 0; d < unFilledNumbers; d++)
							{
								gen(LIT, 0, 0);
								gen(STO, 0, dx_begin + current_offset);
								current_offset++;
							}
						}
						else if (dimCount[current_dimension] > arrayInfo[current_dimension])
						{
							error_s("Dimension not match in array's initialization!");
							skip(end_symbols); return;
						}
						getsym();
						if (sym == SYM_COMMA)
						{
							getsym();
						}
						dimCount[current_dimension] = 0;
						current_dimension--;
						dimCount[current_dimension]++;
					}
				}
				gen(POP, 0, current_offset);
				symset_destroy(inner_follow_symbols);
			}
		}
		int offset = 1;
	}
	else
	{
		error_s("Expect identifer after 'var[]' in array declaration.");
	}
}

int array_locate(int i)
{
	void expr_anyvalue(symset);
	mask* mk = (mask*)&identifier_table[i];
	gen(LIT, 0, 0);
	if (sym != SYM_LBRACKET)
	{
		return 1;
	}
	symset separate_symbols = createset(SYM_COMMA, SYM_RBRACKET, SYM_NULL);
	for (size_t d = 0; d < mk->dimension; d++)
	{
		if (sym == SYM_RBRACKET)
		{
			getsym();
			return 1;
		}
		getsym();
		expr_anyvalue(separate_symbols);
		for (size_t dl = d + 1; dl < mk->dimension; dl++)
		{
			gen(LIT, 0, mk->indices[dl]);
			gen(OPR, 0, OPR_MUL);
		}
		gen(OPR, 0, OPR_ADD);
	}
	symset_destroy(separate_symbols);
	if (sym != SYM_RBRACKET)
	{
		error_s("Expect ']' after array's dimension.");
	}
	getsym();
	return 0;
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
			error_s("Undefined identifier.");
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
				mk = (mask*)&identifier_table[i];
				getsym();
				if (sym != SYM_LPAREN)
				{
					gen(LDFUN, level - mk->level, mk->address);
				}
				else
				{
					expr_function(i);
				}
				break;
			case ID_ARRAY:
				mk = (mask*)&identifier_table[i];
				getsym();
				if (array_locate(i) == 0)
				{
					gen(LODS, level - mk->level, mk->address);
				}
				else
				{
					gen(LDA, 0, mk->address);//Only parse function in the same layer!
					gen(OPR, 0, OPR_ADD);
				}
				break;
			case ID_PTR:
				mk = (mask*)&identifier_table[i];
				gen(LOD, level - mk->level, mk->address);
				getsym();
				if (sym == SYM_LBRACKET)
				{
					if (array_locate(i) == 0)
					{
						gen(OPR, 0, OPR_ADD);
						gen(LODS, level, -1);
					}
					else
					{
						gen(OPR, 0, OPR_ADD);
					}
				}
				else if (sym == SYM_LPAREN)
				{
					instruction lod = code[cx - 1];
					cx--;
					expr_function(i);
					cx--;
					code[cx++] = lod;
					gen(CALS, 0, 0);
				}
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
	else
	{
		error_s("Incorrect symbol in expr_prime.");
		skip(end_symbols); return;
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
			gen(code[cx - 3].f == LODSA ? STOS : STO, code[cx - 3].l, code[cx - 3].a);
			gen(POP, 0, 1);
			gen(code[cx - 5].f == LODSA ? LODS : LOD, code[cx - 5].l, code[cx - 5].a);
			break;
		case SYM_INC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_ADD);
			gen(code[cx - 3].f == LODSA ? STOS : STO, code[cx - 3].l, code[cx - 3].a);
			gen( POP, 0, 1);
			gen(code[cx - 5].f == LODSA ? LODS : LOD, code[cx - 5].l, code[cx - 5].a);
			break;
		case SYM_BITAND:
			if (code[cx - 1].f == LOD)
			{
				code[cx - 1].f = LDA;
			}
			else if (code[cx - 1].f == LODS || code[cx - 1].f == LODSA)
			{
				code[cx - 1].f = LDA;
				gen(OPR, 0, OPR_ADD);
			}
			break;
		}

	}
	else
	{
		if (sym == SYM_LPAREN)
		{
			getsym();
			symset inner_follow_symbols = createset(SYM_RPAREN,SYM_NULL);
			expr_assignment(inner_follow_symbols);
			symset_destroy(inner_follow_symbols);
			if (sym != SYM_RPAREN)
			{
				error_s("Expect ')' in expr_self.");
				skip(end_symbols); return;
			}
			getsym();
		}
		else
		{
			expr_prime(expr_prime_follow);
		}
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
			gen(code[cx - 3].f == LODSA ? STOS : STO, code[cx - 3].l, code[cx - 3].a);
			if (code[cx - 4].f == LODSA)
			{
				gen(POP, 0, 1);
				gen(LODS, code[cx - 5].l, code[cx - 5].a);
			}
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			break;
		case SYM_DEC:
			gen(LIT, 0, 1);
			gen(OPR, 0, OPR_MIN);
			gen(code[cx - 3].f == LODSA ? STOS : STO, code[cx - 3].l, code[cx - 3].a);
			if (code[cx - 4].f == LODSA)
			{
				gen(POP, 0, 1);
				gen(LODS, code[cx - 5].l, code[cx - 5].a);
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
	void expr_assignment(symset fsys);
	void expr_function(int i);
	int i;
	symset set;
	if (sym == SYM_NOT)
	{
		getsym();
		expr_factor(fsys);
		if (code[cx - 1].f == OPR)
		{
			switch (code[cx-1].a)
			{
			case OPR_EQU:code[cx - 1].f = OPR_NEQ; break;
			case OPR_NEQ:code[cx - 1].f = OPR_EQU; break;
			case OPR_LEQ:code[cx - 1].f = OPR_GTR; break;
			case OPR_LES:code[cx - 1].f = OPR_GEQ; break;
			case OPR_GEQ:code[cx - 1].f = OPR_LES; break;
			case OPR_GTR:code[cx - 1].f = OPR_LEQ; break;
			default:
				gen(OPR, 0, OPR_NOT);
				break;
			}
		}
		else if (code[cx - 1].f == LIT
			&& code[cx - 2].f == JMP && code[cx - 2].a == cx
			&& code[cx - 3].f == LIT)
		{
			int temp = code[cx - 1].a;
			code[cx - 1].a = code[cx - 3].a;
			code[cx - 3].a = temp;
		}
		else
		{
			gen(OPR, 0, OPR_NOT);
		}
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
	else if (sym == SYM_TIMES)
	{
		getsym();
		expr_factor(fsys);
		gen(LODS, level, -1);
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
void expr_shift(symset fsys)
{
	int shiftop;
	symset set1 = createset(SYM_SHL, SYM_SHR, SYM_NULL);
	symset inner_follow_symbols = symset_unite(fsys, set1);
	expr_arithmatic(inner_follow_symbols);
	while (sym == SYM_SHL || sym == SYM_SHR)
	{
		shiftop = sym;
		getsym();
		expr_arithmatic(inner_follow_symbols);
		if (shiftop == SYM_SHL)
		{
			gen(OPR, 0, OPR_SHL);
		}
		else
		{
			gen(OPR, 0, OPR_SHR);
		}
	}
	symset_destroy(set1);
	symset_destroy(inner_follow_symbols);
}
// a>b,a==b
void expr_comparation(symset fsys)
{
	int relop;
	symset set;
	set = symset_unite(symbols_relation, fsys);
	expr_shift(set);
	while (in_symbol_set(sym, symbols_relation) && sym != SYM_NULL)
	{
		relop = sym;
		getsym();
		expr_shift(set);
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
void expr_condition_and(symset fsys, int *false_list, int *false_list_size, int *true_list, int *true_list_size)
{
	void expr_condition_or(symset, int*, int*, int*, int*);
	int i;
	int inner_true_list[30];
	int inner_true_list_size = 0;
	int cx0, cx1, cx2;
	cx0 = cx;
	expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
	if (code[cx - 1].f == LIT && code[cx - 1].a == 0
		&& code[cx - 2].f == JMP && code[cx - 2].a == cx
		&& code[cx - 3].f == LIT && code[cx - 3].a == 1)
	{
		for (i = cx0; i < cx - 3; i++)
		{
			if (code[i].f == JPC)
			{
				if (code[i].a == cx - 3)
				{
					inner_true_list[inner_true_list_size++] = i;
				}
				if (code[i].a == cx - 1)
				{
					false_list[(*false_list_size)++] = i;
				}
			}
		}
		cx -= 3;
	}
	else if (code[cx - 1].f == LIT && code[cx - 1].a == 1
		&& code[cx - 2].f == JMP && code[cx - 2].a == cx
		&& code[cx - 3].f == LIT && code[cx - 3].a == 0)
	{
		for (i = cx0; i < cx - 4; i++)
		{
			if (code[i].f == JPC)
			{
				if (code[i].a == cx - 1)
				{
					inner_true_list[inner_true_list_size++] = i;
				}
				if (code[i].a == cx - 3)
				{
					false_list[(*false_list_size)++] = i;
				}
			}
		}
		cx -= 3;
	}
	else
	{
		gen(JPC, J_Z, 0);
		check_branch();
		false_list[(*false_list_size)++] = cx - 1;
	}
	while (sym == SYM_AND)
	{
		for (i = 0; i < inner_true_list_size; i++)
		{
			code[inner_true_list[i]].a = cx;
		}
		inner_true_list_size = 0;
		getsym();
		cx0 = cx;
		expr_bit_or(symset_unite(fsys, createset(SYM_AND, SYM_NULL)));
		if (code[cx - 1].f == LIT &&code[cx - 1].a == 0
			&& code[cx - 2].f == JMP && code[cx - 2].a == cx
			&& code[cx - 3].f == LIT && code[cx - 3].a == 1)
		{
			for (i = cx0; i < cx - 3; i++)
			{
				if (code[i].f == JPC)
				{
					if (code[i].a == cx - 3)
					{
						inner_true_list[inner_true_list_size++] = i;
					}
					else if (code[i].a == cx - 1)
					{
						false_list[(*false_list_size)++] = i;
					}
				}
			}
			cx -= 3;
		}
		else if (code[cx - 1].f == LIT && code[cx - 1].a == 1
			&& code[cx - 2].f == JMP && code[cx - 2].a == cx
			&& code[cx - 3].f == LIT && code[cx - 3].a == 0)
		{
			for (i = cx0; i < cx - 4; i++)
			{
				if (code[i].f == JPC)
				{
					if (code[i].a == cx - 1)
					{
						inner_true_list[inner_true_list_size++] = i;
					}
					if (code[i].a == cx - 3)
					{
						false_list[(*false_list_size)++] = i;
					}
				}
			}
			cx -= 3;
		}
		else
		{
			gen(JPC, J_Z, 0);
			check_branch();
			false_list[(*false_list_size)++] = cx - 1;
		}
	}
	for (i = 0; i < inner_true_list_size; i++)
	{
		true_list[(*true_list_size)++] = inner_true_list[i];
	}
}
void expr_condition_or(symset fsys,int *true_list,int *true_list_size, int *false_list, int *false_list_size)
{
	int i;
	int inner_false_list[30];
	int inner_false_list_size = 0;
	expr_condition_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)),inner_false_list,&inner_false_list_size, 
		true_list, true_list_size);
	check_branch();
	while (sym == SYM_OR)
	{
		if (inner_false_list_size > 0)
		{
			inner_false_list_size--;
			code[cx - 1].l = -code[cx - 1].l;
			true_list[(*true_list_size)++] = cx - 1;
		}
		for (i = 0; i < inner_false_list_size; i++)
		{
			code[inner_false_list[i]].a = cx;
		}
		getsym();
		inner_false_list_size = 0;
		expr_condition_and(symset_unite(fsys, createset(SYM_OR, SYM_NULL)), inner_false_list, &inner_false_list_size, true_list, true_list_size);
		check_branch();
	}
	for (i = 0; i < inner_false_list_size; i++)
	{
		false_list[(*false_list_size)++] = inner_false_list[i];
	}
}
void expr_anyvalue(symset fsys)
{
	int true_list[30];
	int true_list_size = 0;
	int false_list[30];
	int false_list_size = 0;
	expr_condition_or(fsys, true_list, &true_list_size, false_list, &false_list_size);
	if (sym == SYM_QUESTION)
	{
		int i;
		getsym();
		for (i = 0; i < true_list_size; i++)
		{
			code[true_list[i]].a = cx;
		}
		symset inner_follow_symbols = createset(SYM_COLON, SYM_NULL);
		expr_assignment(inner_follow_symbols);
		symset_destroy(inner_follow_symbols);
		int cx_jmp = cx;
		gen(JMP, 0, 0);
		if (sym != SYM_COLON)
		{
			error_s("Expect : in '..?..:..' expr.");
			skip(end_symbols); return;
		}
		getsym();
		for (i = 0; i < false_list_size; i++)
		{
			code[false_list[i]].a = cx;
		}
		expr_assignment(fsys);
		code[cx_jmp].a = cx;
	}
	else
	{
		if (false_list_size == 1 && true_list_size == 0)
		{
			cx--;
		}
		else
		{
			int i;
			for (i = 0; i < true_list_size; i++)
			{
				code[true_list[i]].a = cx;
			}
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < false_list_size; i++)
			{
				code[false_list[i]].a = cx - 1;
			}
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
				gen(LODS, code[cx - 2].l, code[cx - 2].a);
			}
		}
		else
		{
			error_s("Not a left value, cannot be the left of '='.");
		}
	}
}
int function_paralist(symset fsys)
{
	int current_para_num = 0;
	int i;
	int typesym;
	while (sym == SYM_VAR||sym == SYM_PTR || sym == SYM_PROCEDURE)
	{
		typesym = sym;
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			switch (typesym)
			{
			case SYM_VAR: declaration_var(); break;
			case SYM_PTR: declaration_pointer(); break;
			case SYM_PROCEDURE: declaration_pointer(); break; //Function pointer!
			}
			current_para_num++;
		}
		else
		{
			error_s("Expect identifier after 'var' in para list.");
		}
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
	int paraNum = 0;
	do {
		getsym();
		if (sym == SYM_RPAREN)
		{
			break;
		}
		expr_anyvalue(createset(SYM_COMMA, SYM_RPAREN, SYM_NULL));
		paraNum++;
	} while (sym == SYM_COMMA);
	if (sym != SYM_RPAREN)
	{
		error_s("Expect ')' after function's paralist.");
		skip(end_symbols); return;
	}
	mask* mk = (mask*)&identifier_table[i];
	if (mk->dimension != paraNum)
	{
		i = function_position(mk->name, paraNum);
	}
	mk = (mask*)&identifier_table[i];
	if (i == 0)
	{
		error_s("Number of parameters do not match with the function.");
		skip(end_symbols); return;
	}
	gen(CAL, level - mk->level, mk->address);
	getsym();
}
void statement(symset fsys, int *break_list, int *break_list_size, int *continue_list, int *continue_list_size)
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
		set = createset(SYM_BEGIN, SYM_RPAREN, SYM_NULL);
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
		statement(symset_unite(fsys, createset(SYM_ELSE, SYM_ELIF, SYM_NULL)), break_list, break_list_size, continue_list, continue_list_size);
		for (i = 0; i < false_list_size; i++)
		{
			code[false_list[i]].a = cx;
		}
		if (sym == SYM_ELSE || sym == SYM_ELIF)
		{
			for (i = 0; i < false_list_size; i++)
			{
				code[false_list[i]].a += 1;
			}
			cx1 = cx;
			gen(JMP, 0, 0);
			if (sym == SYM_ELSE)
			{
				getsym();
			}
			else
			{
				sym = SYM_IF;
			}
			statement(fsys, break_list, break_list_size, continue_list, continue_list_size);
			code[cx1].a = cx;
		}
		else
		{

		}
	}
	else if (sym == SYM_BEGIN)
	{ // block
		int outter_tx = tx;
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
		tx = outter_tx;
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
		for (i = 0; i < current_true_list_size; i++)
		{
			code[current_true_list[i]].a = cx2;
		}
		for (i = 0; i < current_false_list_size; i++)
		{
			code[current_false_list[i]].a = cx;
		}
		for (i = 0; i < current_break_list_size; i++)
		{
			code[current_break_list[i]].a = cx;
		}
		for (i = 0; i < current_continue_list_size; i++)
		{
			code[current_continue_list[i]].a = cx1;
		}
	}
	else if (sym == SYM_DO)
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
		symset inner_follow_symbols = createset(SYM_WHILE);
		cx1 = cx; //begin of statement in while
		statement(inner_follow_symbols, current_break_list, &current_break_list_size, current_continue_list, &current_continue_list_size);
		symset_destroy(inner_follow_symbols);
		if (sym != SYM_WHILE)
		{
			error_s("Expect while after statements in 'do block'.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
		if (sym != SYM_LPAREN)
		{
			error_s("Expect '(' after while.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
		inner_follow_symbols = createset(SYM_RPAREN, SYM_NULL);
		cx2 = cx; //begin of condition
		expr_condition_or(inner_follow_symbols, current_true_list, &current_true_list_size, current_false_list, &current_false_list_size);
		symset_destroy(inner_follow_symbols);
		gen(JMP, 0, cx1);
		if (sym != SYM_RPAREN)
		{
			error_s("Expect ')' after while loop's condition.");
		}
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after do..while().");
			skip(end_symbols); return;
		}
		for (size_t i = 0; i < current_true_list_size; i++)
		{
			code[current_true_list[i]].a = cx1;
		}
		for (size_t i = 0; i < current_false_list_size; i++)
		{
			code[current_false_list[i]].a = cx;
		}
		for (size_t i = 0; i < current_break_list_size; i++)
		{
			code[current_break_list[i]].a = cx;
		}
		for (size_t i = 0; i < current_continue_list_size; i++)
		{
			code[current_continue_list[i]].a = cx2;
		}

	}
	else if (sym == SYM_SWITCH)
	{
		int current_break_list[100];
		int current_break_list_size = 0;
		getsym();
		if (sym != SYM_LPAREN)
		{
			error_s("Expect '(' after 'switch");
		}
		getsym();
		inner_follow_symbols = createset(SYM_RPAREN);
		expr_assignment(inner_follow_symbols);
		if (sym != SYM_RPAREN)
		{
			error_s("Expect ')' after switch's .");
		}
		getsym();
		if (sym != SYM_BEGIN)
		{
			error_s("Expect 'begin' after 'switch''s condition.");
		}
		int prior_case_cx = -1;
		int prior_jmp_cx = -1;
		getsym();
		while (sym == SYM_CASE)
		{
			getsym();
			inner_follow_symbols = createset(SYM_COLON);
			if (prior_case_cx != -1)
			{
				gen(POP, 0, -1);
				code[prior_case_cx].a = cx;
			}
			expr_assignment(inner_follow_symbols);
			prior_case_cx = cx;
			gen(JPC, J_NEQS, 0);
			if (prior_jmp_cx != -1)
			{
				code[prior_jmp_cx].a = cx;
			}
			if (sym != SYM_COLON)
			{
				error_s("Expect ':' after 'case''s value.");
			}
			getsym();
			symset_destroy(inner_follow_symbols);
			inner_follow_symbols = createset(SYM_CASE, SYM_NULL);
			while (in_symbol_set(sym, begin_symbols_statement))
			{
				statement(inner_follow_symbols, current_break_list, &current_break_list_size, NULL, NULL);
			}
			prior_jmp_cx = cx;
			gen(JMP, 0, 0);
			symset_destroy(inner_follow_symbols);
		}
		if (sym == SYM_DEFAULT)
		{
			if (prior_case_cx != -1)
			{
				gen(POP, 0, -1);
				code[prior_case_cx].a = cx;
			}
			if (prior_jmp_cx != -1)
			{
				code[prior_jmp_cx].a = cx;
			}
			getsym();
			if (sym != SYM_COLON)
			{
				error_s("Expect ':' after default.");
			}
			getsym();
			inner_follow_symbols = createset(SYM_END, SYM_NULL);
			while (in_symbol_set(sym, begin_symbols_statement))
			{
				statement(inner_follow_symbols, current_break_list, &current_break_list_size, NULL, NULL);
			}
		}
		if (sym != SYM_END)
		{
			error_s("Expect 'end' after switch's statements.");
		}
		getsym();
		for (i = 0; i < current_break_list_size; i++)
		{
			code[current_break_list[i]].a = cx;
		}
		gen(POP, 0, 1);

	}
	else if (sym == SYM_RETURN)
	{
		getsym();
		expr_anyvalue(fsys);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after return statement.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
		gen(STO, 0, -para_num);
		gen(RET, para_num, 0);
	}
	else if (sym == SYM_FOR)
	{
		int outter_tx = tx;
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
		if (sym == SYM_VAR)
		{
			getsym();
			declaration_var();
		}
		else if (sym == SYM_PTR)
		{
			getsym();
			declaration_pointer();
		}
		else if (sym == SYM_SEMICOLON)
		{
		}
		else
		{
			expr_assignment(fsys);
			gen(POP, 0, 1);
		}

		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after for loop's expr1");
		}
		getsym();
		cx1 = cx;  //Here cx1 is the beginning of expr2
		if (sym != SYM_SEMICOLON)
		{
			expr_condition_or(fsys, current_true_list, &current_true_list_size, current_false_list, &current_false_list_size);
		}
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
		if (sym != SYM_RPAREN)
		{
			expr_assignment(fsys);
		}
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
		for (i = 0; i < current_continue_list_size; i++)
		{
			code[current_continue_list[i]].a = cx2;
		}
	}
	else if (sym == SYM_BREAK)
	{
		if (break_list == NULL)
		{
			error_s("Cannot add 'break' outside for loop or while loop.");
			skip(end_symbols); getsym(); return;
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
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
	else if (sym == SYM_CONTINUE)
	{
		if (continue_list == NULL)
		{
			error_s("Cannot add 'continue' outside for loop or while loop.");
			skip(end_symbols); getsym(); return;
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
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
	else if (sym == SYM_SEMICOLON)
	{
		getsym();
		return;
	}
	else if (sym == SYM_EXIT)
	{
		gen(EXIT, 0, 0);
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' after exit.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
	else if (sym == SYM_PRINT)
	{
		getsym();
		if (sym != SYM_LPAREN)
		{
			error_s("Expect '(' after 'print'.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
		if (sym != SYM_RPAREN)
		{
			int printnum = 0;
			expr_anyvalue(fsys);
			printnum++;
			while (sym == SYM_COMMA)
			{
				getsym();
				expr_anyvalue(fsys);
				printnum++;
			}
			if (sym != SYM_RPAREN)
			{
				error_s("Expect ')' at the end of param list.");
				skip(end_symbols); getsym(); return;
			}
			gen(PRT, 0, printnum);
			gen(POP, 0, printnum);
		}
		else
		{
			gen(PRT, -1, 0);
		}
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' at the end of print statement.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
	else if (in_symbol_set(sym, begin_symbols_declaration))
	{
		int dx0 = dx;
		switch (sym)
		{
		case SYM_VAR:
			getsym();
			if (sym != SYM_LBRACKET)
			{
				do
				{
					if (sym == SYM_COMMA)
					{
						getsym();
					}
					declaration_var();
				} while (sym == SYM_COMMA);
			}
			else
			{
				getsym();
				if (sym != SYM_RBRACKET)
				{
					error_s("Expect 'var[]' but only get 'var['");
					skip(end_symbols); return;
				}
				do {
					getsym();
					declaration_array();
				} while (sym == SYM_COMMA);
			}
			break;
		case SYM_PTR:
			do {
				getsym();
				declaration_pointer();
			} while (sym == SYM_COMMA);
			break;
		default:
			error_s("Cannot declare a function in statement.");
		}
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expected ';' at the end of declaration in statement.");
			skip(end_symbols); getsym();
		}
		getsym();
	}
	else if (sym == SYM_IDENTIFIER && !identifier_position(id))
	{
		identifier_enter(ID_TAG);
		getsym();
		if (sym != SYM_COLON)
		{
			error_s("This is not a defined identifier and isnot followed by ':'.");
			tx--;
			skip(end_symbols); getsym(); return;
		}
		getsym();
		statement(fsys, break_list, break_list_size, continue_list, continue_list_size);
	}
	else if (sym == SYM_GOTO)
	{
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error_s("Expect a tag after 'goto'.");
			skip(end_symbols); getsym(); return;
		}
		i = identifier_position(id);
		mask* mk = (mask*)&identifier_table[i];
		if (mk->kind != ID_TAG)
		{
			error_s("This identifier after 'goto' is not a tag.");
			skip(end_symbols); getsym(); return;
		}
		gen(JMP, 0, mk->address);
		getsym();
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' in the end of assignment statement.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
	else
	{
		expr_assignment(fsys);
		gen(POP, 0, 1);
		if (sym != SYM_SEMICOLON)
		{
			error_s("Expect ';' in the end of assignment statement.");
			skip(end_symbols); getsym(); return;
		}
		getsym();
	}
}

int current_block_int_cx;
void block(symset fsys)
{
	int cx0,cx_int,cx_jmp; // initial code index
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
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	cx_int = cx;
	gen(INT, 0, dx);
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
	mk->dimension = block_para_num;
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
					break;
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		}
		if (sym == SYM_VAR)
		{
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
						break;
					}
					else
					{
						error_s("Expect ';' after declaration.");
						skip(end_symbols); getsym();
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
						break;
					}
					else
					{
						error_s("Expect ';' after declaration.");
						skip(end_symbols); getsym();
					}
					
				} while (sym == SYM_IDENTIFIER);
			}
		} // if
		if (sym == SYM_PTR)
		{
			getsym();
			while (sym == SYM_IDENTIFIER)
			{
				declaration_pointer();
				if (sym == SYM_SEMICOLON)
				{
					getsym();
					break;
				}
			}
		}
	}
	while (in_symbol_set(sym, begin_symbols_declaration) && sym!=SYM_PROCEDURE);
	cx_jmp = cx;
	gen(JMP, 0, 0);
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
	if (sym != SYM_BEGIN)
	{
		error_s("Expect 'begin' after all declarations.");
	}
	cx0 = cx;
	//ZF note:
	//After all the declaration, then begin statement!
	//In this gen function, we see that i.a is set to block_dx, which is the end of local variable segment!!!
	//gen(INT, 0, block_dx);
	set = createset(SYM_PERIOD,SYM_NULL);
	inner_follow_symbols= symset_unite(set, fsys);
	para_num = block_para_num;
	code[cx_jmp].a = cx;
	statement(inner_follow_symbols, NULL, NULL, NULL, NULL);
	code[cx_int].a = dx;
	symset_destroy(inner_follow_symbols);
	symset_destroy(set);
	gen(RET, para_num, 0); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block


//System pre-defined functions
void init()
{
	mask* mk = (mask*) &identifier_table[++tx];
	strcpy(mk->name, "random");
	mk->kind = ID_PROCEDURE;
	mk->dimension = 0;
	mk->address = 0;
	//random();			cx = 0
	gen(INT, 0, 3);
	gen(SYS, 0, SYS_RAN);
	gen(STO, 0, 0);
	gen(RET, 0, 0);
	mk = (mask*)&identifier_table[++tx];
	//random(var n)		cx = 4
	strcpy(mk->name, "random");
	mk->kind = ID_PROCEDURE;
	mk->dimension = 1;
	mk->address = 4;
	gen(INT, 0, 3);
	gen(SYS, 0, SYS_RAN);
	gen(LOD, 0, -1);
	gen(LIT, 0, 1);
	gen(OPR, 0, OPR_ADD);
	gen(OPR, 0, OPR_MUL);
	gen(LIT, 0, RAND_MAX + 1);
	gen(OPR, 0, OPR_DIV);
	gen(STO, 0, -1);
	gen(RET, 1, 0);
	//callstack()		cx = 14
	mk = (mask*)&identifier_table[++tx];
	strcpy(mk->name, "callstack");
	mk->kind = ID_PROCEDURE;
	mk->dimension = 0;
	mk->address = 14;
	gen(INT, 0, 3);
	gen(SYS, 0, SYS_CALSTK);
	gen(RET, 0, 0);

	tx++;
}


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
int callstack(int stack[], int current_b, int current_pc)
{
	printf("Call stack:\n");
	while (current_pc != start_cx)
	{
		if (current_pc < start_cx)
		{
			printf("In system code segment.\n");
		}
		else
		{
			printf("line entered:%8d  %s\n", cx_line[current_pc], line_code[cx_line[current_pc]]);
		}
		current_pc = stack[current_b + 2];
		current_b = stack[current_b + 1];
	}
}
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
	for (size_t i = 0; i < STACKSIZE; i++)
	{
		stack[i] = 0;
	}
	printf("Begin executing PL/0 program.\n");
	srand(time(0));
	pc = start_cx;
	b = 1;
	top = 0;
	stack[1] = stack[2] = 0;
	stack[3] = start_cx;
	do
	{
		i = code[pc];
		//for debug
		if (is_debug)
		{
			printf("\n%s %d %d\n", mnemonic[i.f], i.l, i.a);
			printf("----------Stack-----------\n");
			for (int si = -10; si < 10; si++)
			{
				if (top + si < 0 || si + cx_line[pc] < 0)
				{
					continue;
				}
				if (b == top + si)
				{
					if (si == 0)
					{
						printf("-->base-> ");
					}
					printf("  base->  ");
				}
				else if (si == 0)
				{
					printf("     ---> ");
				}
				else
				{
					printf("          ");
				}
				printf("%3d:%6d   %3d:%s", top + si, stack[top + si], cx_line[pc] + si, line_code[cx_line[pc] + si]);
				if (si == 0)
				{
					printf("<----------------------");
				}
				printf("\n");
			}
			getchar();
		}
		pc++;
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
		case RET:
			top = b + 2;
			pc = stack[top];
			top--;
			b = stack[top];
			top -= (i.l + 1);
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
				*/
				top = b + 2;
				pc = stack[top];
				top--;
				b = stack[top];
				top -= (i.l + 1);
//				printf("\nretu rn: %d\n", stack[top]);
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
			case OPR_SHR:
				top--;
				stack[top] >>= stack[top + 1];
				break;
			case OPR_SHL:
				top--;
				stack[top] <<= stack[top + 1];
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
			//printf("Store to Addr:%d   %d\n",i.a, base(stack, b, i.l) + i.a);
			stack[base(stack, b, i.l) + i.a] = stack[top];
			//printf("%d\n", stack[top]);
			break;
		case STOS:
			//printf("Store to Addr:%d   %d\n", i.a, base(stack, b, i.l) + i.a + stack[top - 1]);
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			//printf("%d\n", stack[top]);
			break;
		case LODSA:
			stack[top + 1] = stack[base(stack, b, i.l) + i.a + stack[top]];
			top++;
			break;
		case LDA:
			stack[++top] = base(stack, b, i.l) + i.a;
			break;
		case LDFUN:
			stack[++top] = (i.a & 0xffffff) + (i.l << 24);
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
		case CALS:
			stack[top + 2] = pc;
			pc = stack[top] & 0xffffff;
			top--;
			stack[top + 1] = base(stack, b, stack[top + 1] >> 24);
			stack[top + 2] = b;
			b = top + 1;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case EXIT:
			pc = 0;
			break;
		case PRT:
			if (i.l == -1)
			{
				printf("\n");
			}
			else
			{
				int printcount = i.a;
				for (; printcount > 0; printcount--)
				{
					printf("%d\t", stack[top - printcount + 1]);
				}
			}
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
			case J_NEQS:
				if (stack[top] != stack[top - 1])
					pc = i.a;
				top -= 1;
				break;
			}
			break;
		case SYS:
			switch (i.a)
			{
			case SYS_PRT:
				if (i.l == -1)
				{
					printf("\n");
				}
				else
				{
					int printcount = i.a;
					for (; printcount > 0; printcount--)
					{
						printf("%d\t", stack[top - printcount + 1]);
					}
				}
				break;
			case SYS_RAN:
				stack[++top] = rand();
				break;
			case SYS_CALSTK:
				callstack(stack, b, pc);
				break;
			}
			break;
		} // switch
	}
	while (pc!=start_cx);

	printf("\nEnd executing PL/0 program.\n");
} // interpret



void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;
	mytest();
	cx = 0;
	init();
	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		system("pause");
		exit(1);
	}
	printf("If debug, input any unzero value.");
	scanf("%d", &is_debug);
	getchar();
	phi = createset(SYM_NULL);
	symbols_relation = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	begin_symbols_self = createset(SYM_NOT, SYM_BITNOT, SYM_INC, SYM_DEC,
		SYM_BITAND, SYM_TIMES, SYM_NULL);
	end_symbols_self = createset(SYM_INC, SYM_DEC, SYM_NULL);
	begin_symbols_declaration = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_PTR, SYM_NULL);
	begin_symbols_statement = createset(SYM_BEGIN, SYM_IDENTIFIER, SYM_LPAREN,SYM_BITAND, SYM_TIMES,
		SYM_IF, SYM_FOR, SYM_DO, SYM_WHILE, SYM_SWITCH, SYM_SEMICOLON, SYM_BREAK, SYM_CONTINUE,
		SYM_INC, SYM_DEC,SYM_RETURN, SYM_PRINT, SYM_VAR, SYM_PTR,SYM_NULL);
	begin_symbols_primeexpr = createset(SYM_NUMBER,SYM_IDENTIFIER);
	begin_symbols_fact = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_ODD, SYM_NOT, SYM_NULL);
	end_symbols = createset(SYM_SEMICOLON);
	err = cc = cx = ll = 0; // initialize global variables
	line_count = 0;
	ch = ' ';
	kk = MAXIDLEN;
	getsym();
	//ZFNote:
	//set1 is the end symbols
	set1 = createset(SYM_PERIOD, SYM_NULL);
	//set2 is the start symbols
	set2 = symset_unite(begin_symbols_declaration, begin_symbols_statement);
	set = symset_unite(set1, set2);
	cx = start_cx;
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
		printf("name=%10s level=%8d addr=%8d dimension=%8d\n",m->name, m->level, m->address, m->dimension);
	}
	system("pause");
}

