/*
 * COP 3402 - Systems Software
 * Summer 2021
 * Homework #3 (Parser - Code Generator)
 * Authors: Maahee, Grant Allan
 * Due: 7/15/2021
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "compiler.h"


enum
{
	op_const = 1 << 1,
	op_var = 1 << 2,
	op_proc = 1 << 3
};


/* Error Management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)


symbol *table, *sym;
symbol *fetch_symbol(char *name, int kind);
lexeme *list, token, const_token;
int list_index;
int level;
int address;
int sym_index;
int error;


/* Professor Provided Function */
void printtable();


/* Utility Functions */
void enter(int kind, char *name, int value);
void address_checker(int kind);
void is_valid(int token_type, int error_type);
int conflicting_symbol(char *name, int kind);
void get_token();
void mark_symbols();
void printErrorMessage(int x);


/* Parsing Functions */
void program();
void block();
void const_declaration();
void var_declaration();
void procedure_declaration();
void statement();
void condition();
int rel_op();
void expression();
void term();
void factor();


/* Returns the requested symbol, or NULL if it's not found.
 * The options parameter tells the kind of symbol to fetch.
 * If multiple results are found, the first one is returned.
 */
symbol *fetchSymbol(char *name, int kinds)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		// Ignore marked symbols
		if (table[i].mark)
			continue;

		// Ignore symbols whose names don't match what we want
		if (strcmp(table[i].name, name) != 0)
			continue;

		if (kinds & (1 << table[i].kind))
			return &table[i];
	}
	return NULL;
}


/* Store a new symbol and its parameters into the Symbol Table */
void enter(int kind, char *name, int value)
{
	// Determine address
	address_checker(kind);

	table[sym_index].kind = kind;
	strcpy(table[sym_index].name, name);
	table[sym_index].val = value;
	table[sym_index].level = level;
	table[sym_index].addr = address;
	table[sym_index].mark = 0;
	sym_index++;
}


/* Determine the current address */
void address_checker(int kind)
{
	// For procedures and constants
	if (kind == 3 || kind == 1)
		address = 0;
	
	// Checks to see if the previous address value is a 0.
	else if (table[sym_index - 1].addr == 0)
		address = 3;
	
	// If it's not a procedure or a constant and comes after a
	// another variable, then we add one to the previous address
	// and store it.
	else
		address = table[sym_index - 1].addr + 1;
}


/* Check for errors */
void is_valid(int token_type, int error_type)
{
	if (token.type != token_type && error == 0)
		throw(error_type);
}


/* Checks if a symbol was already declared in the current scope */
int conflicting_symbol(char *name, int kind)
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
			// Only entries on our current level are valid
			if (table[i].level != level)
				break;

			// If it has the same name and kind == 1 or 2
			if (strcmp(table[i].name, name) == 0 && kind == 1)
			{
				// Then yes, it conflicts
				error = 1;
				return 1;
			}
			else if (strcmp(table[i].name, name) == 0 && kind == 2)
			{
				// Then yes, it conflicts
				error = 1;
				return 1;
			}
		}
		break;

	case 3: // Procedure
		// Run through the table
		for (i = sym_index - 1; i != -1; i--)
		{
			// Only entries on our current level are valid
			if (table[i].level != level)
				break;

			// If it has the same name and kind == 3
			if ((strcmp(table[i].name, name) == 0) && kind == 3)
			{
				// Then yes, it conflicts
				error = 1;
				return 1;
			}
		}
		break;

	default:
		break;
	}

	// If it reaches here, it never conflicted, so return false (0)
	return 0;
}


/* Get the current token */
void get_token()
{
	token = list[list_index++];
}


/* Marks the symbols in a procedure */
void mark_symbols()
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


/* Start the program */
void program()
{
	// Load main into the symbol table
	enter(3, "main", 0);

	// Grab the first token to look at
	get_token();

	// This will be the main block. That is, the entire program.
	block();

	// Program must end with a period
	is_valid(periodsym, 3);
}


/* Start other functions */
void block()
{
	if (token.type == constsym)
		const_declaration();

	if (token.type == varsym)
		var_declaration();
	
	if (token.type == procsym)
		procedure_declaration();
	
	statement();
}


/* Process a constant */
void const_declaration()
{
	do
	{
		// This token should have the constant name in it
		get_token();

		// If there's no name, throw an error
		// Symbols Must Be Declared with an Identifier
		is_valid(identsym, 4);

		// Load the constant name into const_token
		const_token = token;

		// This token should have the := in it
		get_token();

		// Equals (":="")
		// Constants Must Be Assigned a Value at Declaration
		is_valid(becomessym, 5);

		// This token should have the value of the constant in it
		get_token();

		// Make sure there's a number it's being set equal to
		// Constants Must Be Assigned a Value at Declaration
		is_valid(numbersym, 5);

		// Load the constant value into the const_token
		const_token.value = token.value;

		// Check for conflicting symbols
		// Parser Error: Competing Symbol Declarations
		if (conflicting_symbol(const_token.name, 1))
			throw(1);

		// Constants are a kind of 1
		enter(1, const_token.name, const_token.value);

		// This should either have a comma or a semicolon in it
		get_token();
	
		// If there's a comma, there's another constant being set,
		// so we want to loop through this again.
	} while (token.type == commasym);

	// Check to make sure the whole thing ends with a ";"
	// Symbol Declarations Must Be Followed By a Semicolon
	is_valid(semicolonsym, 6);

	// Move to the next token
	get_token();
}


/* Process a variable */
void var_declaration()
{
	do
	{
		// This should have the variable in it
		get_token();

		// Symbols Must Be Declared with an Identifier
		is_valid(identsym, 4);

		// Check for conflicting symbols
		// Parser Error: Competing Symbol Declarations
		if (conflicting_symbol(token.name, 2))
			throw(1);

		// Variable is a kind of 2
		// We don't declare the value of the variables,
		// so we just put 0 for their value.
		enter(2, token.name, 0);

		// This should have the comma in it, if there is one.
		// If not, it will have the semicolon.
		get_token();

		// If there's a comma, there's another variable being set,
		// so we want to loop through this again.
	} while (token.type == commasym);

	// Check to make sure the whole thing ends with a ";"
	// Symbol Declarations Must Be Followed By a Semicolon
	is_valid(semicolonsym, 6);

	// Move to the next token
	get_token();
}


/* Process a procedure */
void procedure_declaration()
{
	do
	{
		// This should have the name of the procedure
		get_token();

		// The procedure must have a name
		// Symbols Must Be Declared with an Identifier
		is_valid(identsym, 4);

		// Check for conflicting symbols
		// Competing Symbol Declarations
		if (conflicting_symbol(token.name, 3))
			throw(1);

		// Procedure is a kind of 3 and always has a value of 0.
		enter(3, token.name, 0);

		// This should have the semicolon in it
		get_token();

		// Check to make sure a ";" comes after the name
		// Symbol Declarations Must Be Followed By a Semicolon
		is_valid(semicolonsym, 6);

		// This has the first thing of whatever is in the procedure.
		get_token();

		// Call block again, but this time we have a higher lexicographical level due
		// to being in a procedure.
		// This block is the block inside this procedure, rather than the main procedure.
		level++;
		block();

		// Mark every symbol in the procedure
		mark_symbols();

		// Decrease level now that we're out of the procedure
		level--;

		// Check to make sure the whole thing ends with a ";"
		// Symbol Declarations Must Be Followed By a Semicolon
		is_valid(semicolonsym, 6);

		// Get the next token
		get_token();
	} while (token.type == procsym);
}


/* Process a statement */
void statement()
{
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
		get_token();

		// Unrecognized Statement Form
		is_valid(becomessym, 2);

		// This should hold the first item in the expression
		get_token();

		// Process the expression
		expression();

		// Parser Error: Unrecognized Statement Form
		if (token.type == lparentsym)
			throw(2);
		break;

	case callsym:
		// This should hold the name of a procedure
		get_token();

		// call and read Must Be Followed By an Identifien
		is_valid(identsym, 14);

		// Make sure that we only call procedures
		sym = fetchSymbol(token.name, op_proc);

		// If it's not a procedure, throw an error
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);

		// Move onto the next token to be processed
		get_token();
		break;

	case beginsym:
		do
		{
			// Get the first token of the statement; should be
			// the variable whose value is being changed.
			get_token();

			// Process the statement
			statement();

			// Process all statements in the procedure
		} while (token.type == semicolonsym);

		// Make sure the procedure ends with end
		// begin Must Be Followed By end
		is_valid(endsym, 10);

		// Move onto the next token to be processed
		get_token();
		break;

	case ifsym:
		// Get the first token of the condition to be processed
		get_token();

		// Process the condition
		condition();

		// Make sure the next symbol is then
		// if Must Be Followed By then
		is_valid(thensym, 9);

		// Get the first symbol of the statement inside the if
		get_token();

		// Process the statement
		statement();

		// If there's an else...
		if (token.type == elsesym)
		{
			// Get the first token of the statement
			get_token();

			// Process the statement
			statement();
		}
		break;

	case whilesym:
		// Get the first token of the condition
		get_token();

		// Process the condition
		condition();

		// Make sure the next symbol is do
		// while Must Be Followed By do
		is_valid(dosym, 8);

		// Get the first symbol of the statement inside the while
		get_token();

		// Process the statement
		statement();
		break;

	case readsym:
		// This should have an identity in it
		get_token();

		// call and read Must Be Followed By an Identifien
		is_valid(identsym, 14);

		// Make sure to only read input into variables
		sym = fetchSymbol(token.name, op_var);

		// Make sure we're reading a variable
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);

		// Get the next token to be processed
		get_token();
		break;

	case writesym:
		// This should have an identity in it
		get_token();

		// Parser Error: Unrecognized Statement Form
		is_valid(identsym, 2);

		// Make sure to only write constants or variables to the screen
		sym = fetchSymbol(token.name, op_const | op_var);

		// Make sure we're writing a constant or a variable
		// Parser Error: Undeclared Symbol
		if (!sym)
			throw(7);

		// Get the next token to be processed
		get_token();
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
		get_token();

		// Process the expression
		expression();
	}
	else
	{
		// Process the expression
		expression();

		// Make sure it's a relation. If it isn't, throw an error
		// Conditions Must Contain a Relational-Operator
		if (!rel_op())
			throw(12);

		// Get the first token of the expression
		get_token();

		// Process the expression
		expression();
	}
}


/* Relative Operators */
int rel_op()
{
	switch (token.type)
	{
	case eqlsym:
	case neqsym:
	case lessym:
	case leqsym:
	case gtrsym:
	case geqsym:
		// Return true if it is a relative operator
		return 1;
		break;
	default:
		// Return false if it isn't
		return 0;
		break;
	}
}


/* Process plus and/or minus */
void expression()
{
	// If it's a plus or minus, move to the next token
	if (token.type == plussym || token.type == minussym)
		get_token();

	// Process the term
	term();

	// As long as there's another plus or minus, keep processing
	// the expression
	while (token.type == plussym || token.type == minussym)
	{
		get_token();
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
		get_token();
		factor();
	}
}


/* Process the factor */
void factor()
{
	switch (token.type)
	{
	case identsym:
		get_token();
		break;

	case numbersym:
		get_token();
		break;

	case lparentsym:
		get_token();

		// Read the expression in the parentheses
		expression();

		// Make sure to close the parentheses
		// ( Must Be Followed By )
		is_valid(rparentsym, 13);
		
		get_token();
		break;

	default:
		throw(0); // Not sure what error this is
		break;
	}
}


/* Main Parser */
symbol *parse(lexeme *input)
{
	// So we can access input from a global variable
	list = input;

	table = malloc(1000 * sizeof(symbol));
	sym_index = 0;
	list_index = 0;
	level = 0;
	address = 0;
	error = 0;

	if (catch() != 0)
	{
		// We jump here when an error is thrown
		printErrorMessage(error);
		free(table);
		return NULL;
	}
	else
	{
		// Run the program
		program();
		
		// Only prints if there's no error
		if (!error)
			printtable();
		
		// Return the symbol table
		return table;
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


/* Print the Output */
void printtable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address\n");
	printf("------------------------------------------------------\n");
	for (i = 0; i < sym_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr);
}
