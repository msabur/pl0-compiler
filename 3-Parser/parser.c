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
		program();	  // We begin parsing!
		printtable(); // Print the symbol table
		return table;
	}
}

void program()
{
	// the main procedure is always the first item in our symbol table
	addSymbol("main", 0, prockind, 0);
	getToken();
	block();
	expect(periodsym, 3);
}

void block()
{
	if (token.type == constsym)
		const_declaration();
	if (token.type == varsym)
		var_declaration();
	while (token.type == procsym)
		procedure_declaration();

	statement();
}

void const_declaration()
{
	// Find the name of the constant
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = token;
	
	// Check if adding it to the symbol table will cause a conflict
	if (conflictingSymbol(ident.name, constkind))
		throw(1);

	// Find the value of the constant
	getToken();
	expect(becomessym, 5);
	getToken();
	expect(numbersym, 5);
	ident.value = token.value;

	
	addSymbol(ident.name, ident.value, constkind, 0);

	getToken();

	// If we see a comma, we have to parse another declaration
	if (token.type == commasym)
	{
		const_declaration();
	}
	// Otherwise we need semicolon to mark the end of the declaration
	else
	{
		expect(semicolonsym, 6); // need ; at end of declaration
		getToken();
	}
}

void var_declaration()
{
	int numVars = 0;
	do {
		// Find the name of the variable
		lexeme ident;
		getToken();
		expect(identsym, 4);
		ident = token;

		// Check if adding it to the symbol table will cause a conflict
		if (conflictingSymbol(ident.name, varkind))
			throw(1);
		addSymbol(ident.name, 0, varkind, numVars + 3);
		numVars++;

		getToken();

		// If we see a comma, we have to parse another declaration
	} while (token.type == commasym);

	// Otherwise we need semicolon to mark the end of the declaration
	expect(semicolonsym, 6); // need ; at end of declaration
	getToken();
}

void procedure_declaration()
{
	// Find the name of the procedure
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = token;

	// Check if adding it to the symbol table will cause a conflict
	if (conflictingSymbol(ident.name, prockind))
		throw(1);
	addSymbol(ident.name, 0, prockind, 0);

	getToken();
	expect(semicolonsym, 6); // need ';' at end of declaration
	getToken();

	// A procedure's body is its own scope. We track that with curLevel
	curLevel++;

	// Parse the body of the procedure
	block();

	// Make sure the procedure's local variables can't be used outside
	markSymbolsInScope();

	curLevel--;

	expect(semicolonsym, 6);
	getToken();
}

void statement()
{
	symbol *sym;
	switch (token.type)
	{
	case identsym:
		// Make sure that we only assign to variables
		sym = fetchSymbol(token.name, o_var);
		if (!sym)
			throw(7);
		getToken();
		expect(becomessym, 2);
		getToken();
		expression();
		if (token.type == lparentsym)
			throw(2);
		break;
	case callsym:
		getToken();
		expect(identsym, 14);
		// Make sure that we only call procedures
		sym = fetchSymbol(token.name, o_proc);
		if (!sym)
			throw(7);
		getToken();
		break;
	case beginsym:
		do
		{
			getToken();
			statement();
		} while (token.type == semicolonsym);
		expect(endsym, 10);
		getToken();
		break;
	case ifsym:
		getToken();
		condition();
		expect(thensym, 9);
		getToken();
		statement();
		if (token.type == elsesym)
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
		// Make sure to only read input into variables
		sym = fetchSymbol(token.name, o_var);
		if (!sym)
			throw(7);
		getToken();
		break;
	case writesym:
		getToken();
		expect(identsym, 2);
		// Make sure to only write constants or variables to the screen
		sym = fetchSymbol(token.name, o_const | o_var);
		if (!sym)
			throw(7);
		getToken();
		break;
	default:
		break;
	}
}

void condition()
{
	// We should throw an error when a condition is missing
	bool missing_condition = true;

	// Checking if the current token can be the start of a condition
	token_type first[] = {oddsym, plussym, minussym, identsym,
						  numbersym, lparentsym};
	for (int i = 0; i < sizeof(first) / sizeof(*first); i++)
		if (token.type == first[i])
			missing_condition = false;
	if (missing_condition)
		throw(11);

	if (token.type == oddsym)
	{
		getToken();
		expression();
	}
	else
	{
		expression();
		// Here we throw an error if there is no relational operator.
		// The relational operators are from eqlsym to geqsym
		if (token.type < eqlsym || token.type > geqsym)
			throw(12);
		getToken();
		expression();
	}
}

void expression()
{
	if (token.type == plussym || token.type == minussym)
		getToken();
	term();
	while (token.type == plussym || token.type == minussym)
	{
		getToken();
		term();
	}
}

void term()
{
	factor();
	while (token.type == multsym || token.type == slashsym || token.type == modsym)
	{
		getToken();
		factor();
	}
}

void factor()
{
	if (token.type == identsym)
	{
		// Only consts and vars are allowed in arithmetic expressions
		symbol *sym = fetchSymbol(token.name, o_const | o_var);
		if (!sym)
			throw(7);
		getToken();
	}
	else if (token.type == numbersym)
	{
		getToken();
	}
	else if (token.type == lparentsym)
	{
		getToken();
		expression();
		expect(rparentsym, 13);
		getToken();
	}
	else
	{
		// We shouldn't reach here
		throw(11);
	}
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
