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

enum {constkind = 1, varkind = 2, prockind = 3};
enum {op_const = 1<<1, op_var = 1<<2, op_proc = 1<<3};

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
void printErrorMessage(int x);
void getToken();
void expect(int token_type, int err);
void addSymbol(char *name, int val, int type);
symbol *fetchSymbol(char *name, int kinds);
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
	// Initializing symbol table and token list
	table = malloc(1000 * sizeof(symbol));
	list = input; // so we can access input from a global variable

	if (catch() != 0)
	{
		// we jump here when an error is thrown
		printErrorMessage(error);
		free(table);
		return NULL;
	}
	else
	{
		program(); // We begin parsing!
		printtable(); // print the symbol table
		return table;
	}
}

void program()
{
	// the main procedure is always the first item in our symbol table
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
	while (curToken.type == procsym)
		procedure_declaration();

	statement();
}

void const_declaration()
{
	// Gather information about the constant
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = curToken;
	getToken();
	expect(becomessym, 5); 
	getToken();
	expect(numbersym, 5);
	ident.value = curToken.value;

	// TODO allow declaring constants with same name as procedures
	
	// We add it to the symbol table only if it wasn't already there
	if (conflictingSymbol(ident.name))
		throw(1);
	addSymbol(ident.name, ident.value, constsym);

	getToken();

	// If we see a comma, we have to parse another declaration
	if (curToken.type == commasym)
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
	// Gather information about the variable
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = curToken;

	// TODO allow declaring variables with same name as procedures
	
	// We add it to the symbol table only if it wasn't already there
	if (conflictingSymbol(ident.name))
		throw(1);
	addSymbol(ident.name, 0, varsym);

	getToken();

	// If we see a comma, we have to parse another declaration
	if (curToken.type == commasym)
	{
		var_declaration();
	}

	// Otherwise we need semicolon to mark the end of the declaration
	else
	{
		expect(semicolonsym, 6); // need ; at end of declaration
		getToken();
	}
}

void procedure_declaration()
{
	// Gather information about the procedure
	lexeme ident;
	getToken();
	expect(identsym, 4);
	ident = curToken;
	
	// TODO allow declaring procedures with same name as consts, vars
	
	// We add it to the symbol table only if it wasn't already there
	if (conflictingSymbol(ident.name))
		throw(1);
	addSymbol(ident.name, 0, procsym);

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

	expect(semicolonsym, 6); // TODO make sure this is the right error
	getToken();
}

void statement()
{
	symbol *sym;
	switch (curToken.type)
	{
	case identsym:
		// Make sure that we only assign to variables
		sym = fetchSymbol(curToken.name, op_var);
		if (!sym)
			throw(7);
		getToken();
		expect(becomessym, 2);
		getToken();
		expression();
		break;
	case callsym:
		getToken();
		expect(identsym, 14);
		// Make sure that we only call procedures
		sym = fetchSymbol(curToken.name, op_proc);
		if (!sym)
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
		// Make sure to only read input into variables
		sym = fetchSymbol(curToken.name, op_var);
		if (!sym)
			throw(7);
		getToken();
		break;
	case writesym:
		getToken();
		expect(identsym, 2);
		// Make sure to only write constants or variables to the screen
		sym = fetchSymbol(curToken.name, op_const | op_var);
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
	for (int i = 0; i < sizeof(first)/sizeof(*first); i++)
		if (curToken.type == first[i])
			missing_condition = false;
	if (missing_condition)
		throw(11);

	if (curToken.type == oddsym)
	{
		getToken();
		expression();
	}
	else {
		expression();
		// Here we throw an error if there is no relational operator.
		// The relational operators are from eqlsym to geqsym
		if (curToken.type < eqlsym || curToken.type > geqsym)
			throw(12);
		getToken();
		expression();
	}
}

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
		// Only consts and vars are allowed in arithmetic expressions
		symbol *sym = fetchSymbol(curToken.name, op_const | op_var);
		if (!sym)
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
		fprintf(stderr, "Here we are!");
	}

}

// Throws an error if the current token isn't of the expected type
void expect(int token_type, int err) {
	if(curToken.type != token_type)
		throw(err);
}

// Fetches the next token and assigns it to the global variable curToken
void getToken() {
	static int tokenIndex = 0;
	curToken = list[tokenIndex++];
}

// Adds a symbol to the symbol table
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
		kind = constkind;
	else if (type == varsym)
		kind = varkind;
	else
		kind = prockind;

	// Determine addr
	if (kind == prockind || kind == constkind) 
		addr = 0;
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
		if (kinds & (1 << table[i].kind))
			return &table[i];
	}
	return NULL;
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

// Marks symbols in the current scope before we exit it
void markSymbolsInScope() {
	for (int i = sym_index - 1; i != -1; i--) {
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
		[14] = "call and read Must Be Followed By an Identifier"
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
