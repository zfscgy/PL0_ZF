#ifndef SET_H
#define SET_H

typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;

symset phi, begin_symbols_declaration, begin_symbols_statement, begin_symbols_primeexpr, begin_symbols_fact;
symset  begin_symbols_self,end_symbols_self, symbols_relation;
symset createset(int data, .../* SYM_NULL */);
void destroyset(symset s);
symset uniteset(symset s1, symset s2);
int in_symbol_set(int elem, symset s);

#endif
// EOF set.h
