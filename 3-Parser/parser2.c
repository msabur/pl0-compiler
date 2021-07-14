/*
 * COP 3402 - Systems Software
 * Summer 2021
 * Homework #3 (Parser - Code Generator)
 * Authors: Maahee, Grant Allan
 * Due: 7/15/2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include "compiler.h"

/* Constants */
// Symbol types
const int constkind = 1, varkind = 2, prockind = 3;
// Options for symbol table lookups 
const int o_const = 1<<1, o_var = 1<<2, o_proc = 1<<3;

/* Error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

symbol *table;
int sym_index;
lexeme token, *list;
int curLevel; // tracks our lexicographical level

/* Utilities */
void printtable();
void printErrorMessage(int x);
void getToken();
void expect(int token_type, int err);
void addSymbol(char *name, int val, int kind, int addr);
symbol *fetchSymbol(char *name, int kinds);
void markSymbolsInScope();
bool conflictingSymbol(char *name, int kind);

/* Parsing functions */

void X();
void B();
void C();
void C_PRIME();
void V();
void V_PRIME();
void P();
void S();
void G();
void L();
void D();
void R();
void E();
void E_PRIME();
void T();
void T_PRIME();
void F();

symbol *parse(lexeme *input)
{
	// Initializing symbol table and token list
	table = malloc(1000 * sizeof(symbol));
	list = input; // so we can access input from a global variable

	if (catch() != 0)
	{
		// We jump here when an error is thrown
		printErrorMessage(error);
		free(table);
		return NULL;
	}
	else
	{
		X();	  // We begin parsing!
		printtable(); // Print the symbol table
		return table;
	}
}

void X() {

	if (cur == constsym || cur == varsym || cur == proceduresym || cur == identsym || cur == writesym || cur == readsym || cur == callsym || cur == beginsym || cur == ifsym || cur == whilesym || cur == periodsym)
	{
		B(); accept(periodsym);
	}
	else

		throw();

}
void B() {

	if (cur == constsym || cur == varsym || cur == proceduresym || cur == identsym || cur == writesym || cur == readsym || cur == callsym || cur == beginsym || cur == ifsym || cur == whilesym)
	{
		C(); V(); P(); S();
	}
	else if (cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void C() {

	if (cur == constsym)
	{
		accept(constsym); accept(identsym); accept(becomessym); accept(numbersym); C_PRIME(); accept(semicolonsym);
	}
	else if (cur == varsym || cur == procsym || cur == identsym || cur == writesym || cur == readsym || cur == callsym || cur == beginsym || cur == ifsym || cur == whilesym || cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void C_PRIME() {

	if (cur == commasym)
	{
		accept(commasym); accept(identsym); accept(becomessym); accept(numbersym); C_PRIME();
	}
	else if (cur == semicolonsym)

		return;

	else

		throw();

}
void V() {

	if (cur == varsym)
	{
		accept(varsym); accept(identsym); V_PRIME(); accept(semicolonsym);
	}
	else if (cur == procsym || cur == identsym || cur == writesym || cur == readsym || cur == callsym || cur == beginsym || cur == ifsym || cur == whilesym || cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void V_PRIME() {

	if (cur == commasym)
	{
		accept(commasym); accept(identsym); V_PRIME();
	}
	else if (cur == semicolonsym)

		return;
	else

		throw();

}
void P() {

	if (cur == procsym)
	{
		accept(procsym); accept(identsym); accept(semicolonsym); B(); accept(semicolonsym); P();
	}
	else if (cur == identsym || cur == callsym || cur == writesym || cur == readsym || cur == beginsym || cur == ifsym || cur == whilesym || cur == periodsym || cur ==  || cur == semicolonsym)

		return;

	else

		throw();

}
void S() {

	if (cur == identsym)
	{
		accept(identsym); accept(becomessym); E();
	}
	else if (cur == callsym)
	{
		accept(callsym); accept(identsym);
	}
	else if (cur == writesym)
	{
		accept(writesym); E();
	}
	else if (cur == readsym)
	{
		accept(readsym); accept(identsym);
	}
	else if (cur == beginsym)
	{
		accept(beginsym); S(); G(); accept(endsym);
	}
	else if (cur == ifsym)
	{
		accept(ifsym); D(); accept(thensym); S(); L();
	}
	else if (cur == while)
	{
		accept(whilesym); D(); accept(dosym); S();
	}
	else if (cur == endsym, elsesym, periodsym, semicolonsym)

		return;

	else

		throw();

}
void G() {

	if (cur == semicolonsym)
	{
		accept(semicolonsym); S(); G();
	}
	else if (cur == endsym)

		return;

	else

		throw();

}
void L() {

	if (cur == elsesym)
	{
		accept(elsesym); S();
	}
	else if (cur == endsym || cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void D() {

	if (cur == oddsym)
	{
		accept(oddsym); E();
	}
	else if (cur == plussym || cur == minussym || cur == identsym || cur == numbersym || cur == lparentsym)
	{
		E(); R(); E();
	}
	else

		throw();

}
void R() {

	if (cur == eqlsym)

		accept(eqlsym);

	else if (cur == neqsym)

		accept(neqsym);

	else if (cur == lessym)

		accept(lessym);

	else if (cur == leqsym)

		accept(leqsym);

	else if (cur == gtrsym)

		accept(gtrsym);

	else if (cur == geqsym)

		accept(geqsym);

	else

		throw();

}
void E() {

	if (cur == plussym)
	{
		accept(plussym); T(); E_PRIME();
	}
	else if (cur == minussym)
	{
		accept(minussym); T(); E_PRIME();
	}
	else if (cur == identsym || cur == numbersym || cur == lparentsym)
	{
		T(); E_PRIME();
	}
	else

		throw();;

}
void E_PRIME() {

	if (cur == plussym)
	{
		accept(plussym); T(); E_PRIME();
	}
	else if (cur == minussym)
	{
		accept(minussym); T(); E_PRIME();
	}
	else if (cur == endsym || cur == elsesym || cur == dosym || cur == thensym || cur == eqlsym || cur == neqsym || cur == lessym || cur == leqsym || cur == gtrsym || cur == geqsym || cur == rparentsym || cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void T() {

	if (cur == identsym || cur == numbersym || cur == lparentsym)
	{
		F(); T_PRIME();
	}
	else

		throw();

}
void T_PRIME() {
	if (cur == multsym)
	{
		accept(multsym); F(); T_PRIME();
	}
	else if (cur == slashsym)
	{
		accept(slashsym); F(); T_PRIME();
	}
	else if (cur == modsym)
	{
		accept(modsym); F(); T_PRIME();
	}
	else if (cur == endsym || cur == elsesym || cur == dosym || cur == thensym || cur == eqlsym || cur == neqsym lessym || cur == leqsym || cur == gtrsym || cur == geqsym rparentsym || cur == periodsym || cur == semicolonsym)

		return;

	else

		throw();

}
void F() {

	if (cur == identsym)

		accept(identsym);

	else if (cur == numbersym)

		accept(numbersym);

	else if (cur == lparentsym) {

		accept(lparentsym); E(); accept(rparentsym);
	}

	else

		throw();
}

// Throws an error if the current token isn't of the expected type
void expect(int token_type, int err)
{
	if (token.type != token_type)
		throw(err);
}

// Fetches the next token and assigns it to the global variable token
void getToken()
{
	static int tokenIndex = 0;
	token = list[tokenIndex++];
}

// Adds a symbol to the symbol table
void addSymbol(char *name, int val, int kind, int addr)
{
	// Now, we add this symbol to the table
	table[sym_index].addr = addr;
	table[sym_index].kind = kind;
	table[sym_index].val = val;
	table[sym_index].mark = 0;
	table[sym_index].level = curLevel;
	strcpy(table[sym_index].name, name);

	// Increment the index
	++sym_index;
}

// Returns the requested symbol, or NULL if it's not found.
// The options parameter tells the kind of symbol to fetch.
// If multiple results are found, the first one is returned.
symbol *fetchSymbol(char *name, int kinds)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		// ignore marked symbols
		if (table[i].mark)
			continue;
		// ignore symbols whose names don't match what we want
		if (strcmp(table[i].name, name) != 0)
			continue;
		// if the current symbol is of a requested kind, return it
		if (kinds & (1 << table[i].kind))
			return &table[i];
	}
	// at this point we didn't find the symbol
	return NULL;
}

// Checks if a symbol was already declared in the current scope
bool conflictingSymbol(char *name, int kind)
{
	int i;
	// Switch for checking if it's a const, var, or proc
	switch (kind)
	{
	case 1: // Constant
	case 2: // Variable
		// Run through the table
		for (i = sym_index - 1; i != -1; i--)
		{
			// Only entries on our current level cause conflicts
			if (table[i].level != curLevel)
				break;

			// If it has the same name and kind == 1 or 2
			if (strcmp(table[i].name, name) == 0 && kind == 1)
				// Then yes, it conflicts
				return true;
			else if (strcmp(table[i].name, name) == 0 && kind == 2)
				// Then yes, it conflicts
				return true;
		}
		break;

	case 3: // Procedure
		// Run through the table
		for (i = sym_index - 1; i != -1; i--)
		{
			// Only entries on our current level cause conflicts
			if (table[i].level != curLevel)
				break;

			// If it has the same name and kind == 3
			if ((strcmp(table[i].name, name) == 0) && kind == 3)
				// Then yes, it conflicts
				return true;
		}
		break;

	default:
		break;
	}

	// If it reaches here, it never conflicted, so return false (0)
	return false;
}

// Marks symbols in the current scope before we exit it
void markSymbolsInScope()
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (table[i].mark == 1) // should only be true when marking main
			continue;
		if (table[i].level != curLevel) // done marking the current scope
			break;
		table[i].mark = 1;
	}
}

void printErrorMessage(int x)
{
	const char *errors[] =
		{
			[1] = "Competing Symbol Declarations",
			[2] = "Unrecognized Statement Form",
			[3] = "Programs Must Close with a Period",
			[4] = "Symbols Must Be Declared with an Identifier",
			[5] = "Constants Must Be Assigned a Value at Declaration",
			[6] = "Symbol Declarations Must Be Followed By a Semicolon",
			[7] = "Undeclared Symbol",
			[8] = "while Must Be Followed By do",
			[9] = "if Must Be Followed By then",
			[10] = "begin Must Be Followed By end",
			[11] = "while and if Statements Must Contain Conditions",
			[12] = "Conditions Must Contain a Relational-Operator",
			[13] = "( Must Be Followed By )",
			[14] = "call and read Must Be Followed By an Identifier"};

	if (x < 1 || x > 14)
		printf("Implementation Error: Unrecognized Error Code\n");
	else
		printf("Parser Error: %s\n", errors[x]);
}

void printtable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address\n");
	printf("--------------------------------------------\n");
	for (i = 0; i < sym_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr);
}
