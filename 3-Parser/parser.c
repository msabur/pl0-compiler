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
const int op_const = 0x2, op_var = 0x4, op_proc = 0x8, op_sameScope = 0x10;

/* Error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

symbol *table;
int sym_index;

// Token is the current token
// List is the global we'll use to move through the input
lexeme token, *list;

// Tracks our lexicographical level
int level;

/* Utilities */
void printtable();
void printErrorMessage(int x);
void getToken();
void expect(int token_type, int err);
void addSymbol(char *name, int val, int kind, int address);
symbol *fetchSymbol(char *name, int kinds);
void markSymbolsInScope();
bool conflictingSymbol(char *name, int kind);

/* Parsing Functions */
void program();
void block();
void const_declaration();
void var_declaration();
void procedure_declaration();
void statement();
void condition();
void expression();
void term();
void factor();

/* Main Function */
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
		// We begin parsing!
		program();
		
		// Print the symbol table
		printtable();

		return table;
	}
}

/* Start the program */
void program()
{
	// Load main into the symbol table
	addSymbol("main", 0, prockind, 0);

	// Get the first token of the program
	getToken();

	// This will be the main block. That is, the entire program.
	block();

	// The program must end with a period
	expect(periodsym, 3);
}

/* Start other functions */
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

/* Process a constant */
void const_declaration()
{
	// Use ident to hold the name and value of the constant
	lexeme ident;

	// This token should have the constant name in it
	getToken();

	// If there's no name, throw an error
	// Symbols Must Be Declared with an Identifier
	expect(identsym, 4);

	// Load the constant name into the ident
	ident = token;

	// This token should have the := in it
	getToken();

	// Equals (":="")
	// Constants Must Be Assigned a Value at Declaration
	expect(becomessym, 5);

	// This token should have the value of the constant in it
	getToken();

	// Make sure there's a number it's being set equal to
	// Constants Must Be Assigned a Value at Declaration
	expect(numbersym, 5);

	// Load the constant value into the ident
	ident.value = token.value;

	// Check for conflicting symbols
	// Parser Error: Competing Symbol Declarations
	if (conflictingSymbol(ident.name, 1))
		throw(1);
	
	// Add the constant to the symbol table
	addSymbol(ident.name, ident.value, constkind, 0);

	// Get the next token
	getToken();

	// If we see a comma, we have to parse another declaration
	if (token.type == commasym)
		const_declaration();
	// Otherwise we need semicolon to mark the end of the declaration
	else
	{
		// Symbol Declarations Must Be Followed By a Semicolon
		expect(semicolonsym, 6);

		// Get the next token
		getToken();
	}
}

/* Process a constant */
void var_declaration()
{
	int numVars = 0;
	do {
		// Use ident to hold the name and value of the variable
		lexeme ident;

		// This token should hold the name of the variable
		getToken();

		// Symbols Must Be Declared with an Identifier
		expect(identsym, 4);

		// Fill ident with the variable's token information
		ident = token;

		// We add it to the symbol table only if it wasn't already there
		if (conflictingSymbol(ident.name, varkind))
			throw(1);

		// Add the variable to the symbol table	
		addSymbol(ident.name, 0, varkind, numVars + 3);
		numVars++;

		// Get the next token to check it
		getToken();

		// If we see a comma, we have to parse another declaration
	} while (token.type == commasym);

	// Otherwise we need semicolon to mark the end of the declaration
	expect(semicolonsym, 6);
	getToken();
}

/* Process a procedure */
void procedure_declaration()
{
	// Use ident to hold the name and value of the procedure
	lexeme ident;

	// This token should hold the name of the procedure
	getToken();

	// Symbols Must Be Declared with an Identifier
	expect(identsym, 4);

	// Fill ident with the procedure's information
	ident = token;

	// We add it to the symbol table only if it wasn't already there
	if (conflictingSymbol(ident.name, prockind))
		throw(1);
	
	// Add the procedure to the symbol table
	addSymbol(ident.name, 0, prockind, 0);

	// This token should have a semicolon in it
	getToken();

	// Symbol Declarations Must Be Followed By a Semicolon
	expect(semicolonsym, 6);

	// Grab the next token so the block can process properly
	getToken();

	// Increase the level for everything inside the procedure.
	level++;

	// Parse the body of the procedure
	block();

	// Make sure the procedure's local variables can't be used outside
	markSymbolsInScope();

	// Decrease level now that we're outside the procedure
	level--;

	// Check to make sure the whole thing ends with a ";"
	// Symbol Declarations Must Be Followed By a Semicolon
	expect(semicolonsym, 6);

	// Grab the next token for processing
	getToken();
}

/* Process a statement */
void statement()
{
	symbol *sym;
	switch (token.type)
	{
	case identsym:
		// Make sure that we only assign to variables
		sym = fetchSymbol(token.name, op_var);

		// If it's not a variables, throw an error
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);
		
		// This should hold :=
		getToken();

		// Unrecognized Statement Form
		expect(becomessym, 2);
		
		// This should hold the first item in the expression
		getToken();
		
		// Process the expression
		expression();

		// Parser Error: Unrecognized Statement Form
		if (token.type == lparentsym)
			throw(2);

		break;
	
	case callsym:
		// This should hold the name of a procedure
		getToken();

		// call and read Must Be Followed By an Identifien
		expect(identsym, 14);
		
		// Make sure that we only call procedures
		sym = fetchSymbol(token.name, op_proc);

		// If it's not a procedure, throw an error
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);
		
		// Move onto the next token to be processed
		getToken();
		break;
	
	case beginsym:
		do
		{
			// Get the first token of the statement; should be
			// the variable whose value is being changed.
			getToken();

			// Process the statement
			statement();

		// Process all statements in the procedure
		} while (token.type == semicolonsym);

		// Make sure the procedure ends with end
		// begin Must Be Followed By end
		expect(endsym, 10);

		// Move onto the next token to be processed
		getToken();
		break;
	
	case ifsym:
		// Get the first token of the condition to be processed
		getToken();

		// Process the condition
		condition();

		// Make sure the next symbol is then
		// if Must Be Followed By then
		expect(thensym, 9);

		// Get the first symbol of the statement inside the if
		getToken();

		// Process the statement
		statement();

		// If there's an else...
		if (token.type == elsesym)
		{
			// Get the first token of the statement
			getToken();

			// Process the statement
			statement();
		}
		break;
	
	case whilesym:
		// Get the first token of the condition
		getToken();

		// Process the condition
		condition();

		// Make sure the next symbol is do
		// while Must Be Followed By do
		expect(dosym, 8);

		// Get the first symbol of the statement inside the while
		getToken();

		// Process the statement
		statement();
		break;
	
	case readsym:
		// This should have an identity in it
		getToken();

		// call and read Must Be Followed By an Identifien
		expect(identsym, 14);

		// Make sure to only read input into variables
		sym = fetchSymbol(token.name, op_var);
		
		// Make sure we're reading a variable
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);
		
		// Get the next token to be processed
		getToken();
		break;
	
	case writesym:
		// This should have an identity in it
		getToken();

		// Parser Error: Unrecognized Statement Form
		expect(identsym, 2);
		
		// Make sure to only write constants or variables to the screen
		sym = fetchSymbol(token.name, op_const | op_var);

		// Make sure we're writing a constant or a variable
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);

		// Get the next token to be processed
		getToken();
		break;
	
	default:
		break;
	}
}

/* Process a formula */
void condition()
{
	if (token.type == oddsym)
	{
		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();
	}
	else
	{
		// Process the expression
		expression();

		// Here we throw an error if there is no relational operator.
		// The relational operators are from eqlsym to geqsym
		// Conditions Must Contain a Relational-Operator
		if (token.type < eqlsym || token.type > geqsym)
			throw(12);
		
		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();
	}
}

/* Process Plus and/or Minus */
void expression()
{
	// If it's a plus or minus, move to the next token
	if (token.type == plussym || token.type == minussym)
		getToken();
	
	// Process the term
	term();

	// As long as there's another plus or minus, keep processing
	// the expression
	while (token.type == plussym || token.type == minussym)
	{
		getToken();
		term();
	}
}

/* Process multiplication, division, and/or modulus */
void term()
{
	// Process the factor
	factor();

	// As long as the current token is * / or %, keep processing
	while (token.type == multsym || token.type == slashsym || token.type == modsym)
	{
		getToken();
		factor();
	}
}

/* Process the factor */
void factor()
{
	if (token.type == identsym)
	{
		// Only consts and vars are allowed in arithmetic expressions
		symbol *sym = fetchSymbol(token.name, op_const | op_var);

		// If it's not a constant or a variable, throw an error
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);

		// Get the next token to be processed
		getToken();
	}
	else if (token.type == numbersym)
	{
		// Get the next token to be processed
		getToken();
	}
	else if (token.type == lparentsym)
	{
		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();

		// Make sure to close the parentheses
		// ( Must Be Followed By )
		expect(rparentsym, 13);

		// Get the next token to be processed
		getToken();
	}
	else
	{
		// We get here when an if/while statement is missing a condition
		throw(11);
	}
}

/* Throws an error if the current token isn't of the expected type */
void expect(int token_type, int err)
{
	if (token.type != token_type)
		throw(err);
}

/* Fetches the next token and assigns it to the global variable token */
void getToken()
{
	static int tokenIndex = 0;
	token = list[tokenIndex++];
}

/* Adds a symbol to the symbol table */
void addSymbol(char *name, int val, int kind, int address)
{
	// Adding this symbol to the table
	table[sym_index].addr = address;
	table[sym_index].kind = kind;
	table[sym_index].val = val;
	table[sym_index].mark = 0;
	table[sym_index].level = level;
	strcpy(table[sym_index].name, name);

	// Increment the index
	sym_index++;
}

/* Returns the requested symbol, or NULL if it's not found.
 * The options parameter tells the kind of symbol to fetch.
 * If multiple results are found, the first one is returned.
 */
symbol *fetchSymbol(char *name, int kinds)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		// If searching on the current scope only, stop when we leave it
		if ((kinds & op_sameScope) && table[i].level != level)
			break;

		// Ignore marked symbols
		if (table[i].mark)
			continue;
		
		// Ignore symbols whose names don't match what we want
		if (strcmp(table[i].name, name) != 0)
			continue;
		
		// If the current symbol is of a requested kind, return it
		if (kinds & (1 << table[i].kind))
			return &table[i];
	}
	return NULL;
}

/* Checks if a symbol was already declared in the current scope */
bool conflictingSymbol(char *name, int kind)
{
	symbol *sym;

	// Switch for checking if it's a const, var, or proc
	switch (kind)
	{
	case 1: // Constant
	case 2: // Variable
		// Look for a const or var with the same name in the current scope
		sym = fetchSymbol(name, op_const | op_var | op_sameScope);
		// If we find it, then it conflicts
		if (sym)
			return true;
		break;

	case 3: // Procedure
		// Look for a procedure with the same name in the current scope
		sym = fetchSymbol(name, op_proc | op_sameScope);
		// If we find it, then it conflicts
		if (sym)
			return true;
		break;

	default:
		break;
	}

	// If it reaches here, it never conflicted, so return false
	return false;
}

/* Marks symbols in the current scope before we exit it */
void markSymbolsInScope()
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		// Should only be true when marking main
		if (table[i].mark == 1)
			continue;

		// If it's done marking the current scope, break
		if (table[i].level != level)
			break;
		
		// Mark the current symbol
		table[i].mark = 1;
	}
}

/* Print the error thrown */
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

/* Print the Symbol Table */
void printtable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address\n");
	printf("--------------------------------------------\n");
	for (i = 0; i < sym_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr);
}
