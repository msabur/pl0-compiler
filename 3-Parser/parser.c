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

/* Error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

symbol *table;
int sym_index;
lexeme curToken, *list;
int curLevel; // tracks our lexicographical level

/* Utilities */
void printtable();
void errorend(int x);
void getToken();
void expect(int token_type, int err); // expect a given kind of token
void addSymbol(char *name, int val, int type);
symbol *fetchSymbol(char *name);
bool containsSymbol(char *name);
void markSymbolsInScope();
bool conflictingSymbol(char *name);

/* Parsing functions */
void program(); // starting symbol
void block();
void const_declaration();
void var_declaration();
void procedure_declaration();
void statement();
void condition();
void expression();
void term();
void factor();

symbol *parse(lexeme *input)
{
	table = malloc(1000 * sizeof(symbol));
	list = input; // so we can access input from a global variable
	sym_index = 0;
	error = 0;

	if (catch() != 0)
	{
		// we jump here when an error is thrown
		errorend(error);
		free(table);
		return NULL;
	}
	else
	{
		// code that can throw an error
		program();
		printtable();
		return table;
	}
}

void program()
{
	addSymbol("main", 0, procsym);
	getToken();
	block();
	expect(periodsym, 3);
}

void block()
{
	if (curToken.type == constsym)
		const_declaration();
	if (curToken.type == varsym)
		var_declaration();
	if (curToken.type == procsym)
		procedure_declaration();

	statement();
}

void const_declaration()
{
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = curToken;
	getToken();
	expect(becomessym, 5); 
	getToken();
	expect(numbersym, 5);
	ident.value = curToken.value;

	if (conflictingSymbol(ident.name))
		throw(1);
	addSymbol(ident.name, ident.value, constsym);

	getToken();
	if (curToken.type == commasym)
	{
		const_declaration();
	}
	else
	{
		expect(semicolonsym, 6); // need ';' at end of declaration
		getToken();
	}	
}

void var_declaration()
{
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = curToken;

	if (conflictingSymbol(ident.name))
		throw(1);
	addSymbol(ident.name, 0, varsym);

	getToken();
	if (curToken.type == commasym)
	{
		var_declaration();
	}
	else
	{
		expect(semicolonsym, 6); // need ';' at end of declaration
		getToken();
	}
}

void procedure_declaration()
{
	while (curToken.type == procsym)
	{
		lexeme ident;
		getToken();
		expect(identsym, 4);
		ident = curToken;

		if (conflictingSymbol(ident.name))
			throw(1);
		addSymbol(ident.name, 0, procsym);

		getToken();
		expect(semicolonsym, 6); // need ';' at end of declaration
		getToken();

		curLevel++;
		block();
		markSymbolsInScope();
		curLevel--;

		expect(semicolonsym, 6); // TODO make sure this is the right error
		getToken();
	}
}

void statement()
{
	symbol *sym;
	switch (curToken.type)
	{
	case identsym:
		sym = fetchSymbol(curToken.name);
		if (!sym || sym->kind != 2)
			throw(7);
		getToken();
		expect(becomessym, 2);
		getToken();
		expression();
		break;
	case callsym:
		getToken();
		expect(identsym, 14);
		sym = fetchSymbol(curToken.name);
		if (!sym || sym->kind != 3)
			throw(7);
		getToken();
		break;
	case beginsym:
		do
		{
			getToken();
			statement();
		} while (curToken.type == semicolonsym);
		expect(endsym, 10);
		getToken();
		break;
	case ifsym:
		getToken();
		condition();
		expect(thensym, 9);
		getToken();
		statement();
		if (curToken.type == elsesym)
		{
			getToken();
			statement();
		}
		break;
	case whilesym:
		getToken();
		condition();
		expect(dosym, 8);
		getToken();
		statement();
		break;
	case readsym:
		getToken();
		expect(identsym, 14);
		sym = fetchSymbol(curToken.name);
		if (!sym || sym->kind != 2)
			throw(7);
		getToken();
		break;
	case writesym:
		getToken();
		expect(identsym, 2);
		sym = fetchSymbol(curToken.name);
		if (!sym || (sym->kind != 2 && sym->kind != 1))
			throw(7);
		getToken();
		break;
	default:
		break;
	}
}

void condition()
{
	if (curToken.type == oddsym)
	{
		getToken();
		expression();
	}
	else {
		expression();
		// here we throw an error if there is no relational operator.
		// the relational operators are from eqlsym to geqsym
		if (curToken.type < eqlsym || curToken.type > geqsym)
			throw(12);
		getToken();
		expression();
	}
}

/*
 * expression ::= term
 *              | <add/subtract> term
 *              | expression <add/subtract> term 
 */
void expression()
{
	if (curToken.type == plussym || curToken.type == minussym)
		getToken();
	term();
	while (curToken.type == plussym || curToken.type == minussym)
	{
		getToken();
		term();
	}
}

void term()
{
	factor();
	while (curToken.type == multsym
			|| curToken.type == slashsym
			|| curToken.type == modsym)
	{
		getToken();
		factor();
	}
}

void factor()
{
	if (curToken.type == identsym)
	{
		symbol *sym = fetchSymbol(curToken.name);
		if (!sym || (sym->kind != 2 && sym->kind != 1))
			throw(7);
		getToken();
	}
	else if (curToken.type == numbersym)
	{
		getToken();
	}
	else if (curToken.type == lparentsym)
	{
		getToken();
		expression();
		expect(rparentsym, 13);
		getToken();
	}
	else
	{
		/*
		 * XXX Not sure whether to throw an error here.
		 * The recitation slide throws 'identifier, (, or number expected',
		 * but that doesn't match any of the errors in the instructions.
		 */
	}

}


void expect(int token_type, int err) {
	if(curToken.type != token_type)
		throw(err);
}

void getToken() {
	static int tokenIndex = 0;
	curToken = list[tokenIndex++];
}

void addSymbol(char *name, int val, int type)
{
	/*
	 * For a new entry to the symbol table,
	 * we need the values for name, val, kind, and addr. 
	 * We got name and val from arguments, and now 
	 * we determine what kind and addr should be.
	 */

	int kind, addr; // addr stands for address

	// Determine kind
	if (type == constsym)
		kind = 1;
	else if (type == varsym)
		kind = 2;
	else // procedures
		kind = 3;

	// Determine addr
	if (kind == 3 || kind == 1) 
		addr = 0; // for procedures and constants
	else if (table[sym_index - 1].addr == 0)
		addr = 3; // first symbol has offset 3
	else
		addr = table[sym_index - 1].addr + 1; // for normal variables

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

// Returns NULL if the symbol isn't found
symbol *fetchSymbol(char *name)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (strcmp(table[i].name, name) == 0 && table[i].mark == 0)
			return &table[i];
	}
	return NULL;
}

// Returns true if the symbol is in the table, otherwise false
bool containsSymbol(char *name)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (strcmp(table[i].name, name) == 0 && table[i].mark == 0)
			return true;
	}
	return false;
}

// Checks if a symbol was already declared in the current scope
bool conflictingSymbol(char *name)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (table[i].level != curLevel)
			break;
		if (strcmp(table[i].name, name) == 0)
			return true;
	}
	return false;
}

// Marks symbols in the current level (scope).
// Used when leaving a scope, such as when exiting a procedure.
// Note: Should be called before decrementing the curLevel variable
void markSymbolsInScope() {
	for (int i = sym_index - 1; i != -1; i--) {
		if (table[i].mark == 1) // should only be true when marking main
			continue;
		if (table[i].level != curLevel) // done marking the current scope
			break;
		table[i].mark = 1;
	}
}

void errorend(int x)
{
	// Misspellings are from the provided skeleton
	const char *errors[] = 
	{
		[ 1] = "Competing Symbol Declarations",
		[ 2] = "Unrecognized Statement Form",
		[ 3] = "Programs Must Close with a Period",
		[ 4] = "Symbols Must Be Declared with an Identifier",
		[ 5] = "Constants Must Be Assigned a Value at Declaration",
		[ 6] = "Symbol Declarations Must Be Followed By a Semicolon",
		[ 7] = "Undeclared Symbol",
		[ 8] = "while Must Be Followed By do",
		[ 9] = "if Must Be Followed By then",
		[10] = "begin Must Be Followed By end",
		[11] = "while and if Statements Must Contain Conditions",
		[12] = "Conditions Must Contain a Relational-Operator",
		[13] = "( Must Be Followed By )",
		[14] = "call and read Must Be Followed By an Identifien"
	};

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
