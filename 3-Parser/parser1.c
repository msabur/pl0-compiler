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
#include "compiler.h"


symbol *table, *sym;
symbol *fetch_symbol(char *name);
lexeme *list, token, const_token;
int list_index;
int level;
int address;
int sym_index;
int error;


/* Professor Provided Functions */
void printtable();
void errorend(int x);


/* Utility Functions */
void enter(int kind, char *name, int value);
void address_checker(int kind);
void is_valid(int token_type, int error_type);
int conflicting_symbol(char *name);
void get_token();
void mark_symbols();


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
		errorend(error_type);
}


/* Checks if a symbol was already declared in the current scope */
int conflicting_symbol(char *name)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (table[i].level != level)
			break;

		if (strcmp(table[i].name, name) == 0)
			return 1;
	}
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
	int i = sym_index;

	// Decrease i until we're at the start of the procedure
	while (table[i].kind != 3)
		i--;

	// Incease i once so we're at the first symbol after the
	// procedure entry in the table
	i++;

	// Mark every symbol in the procedure
	while (i != sym_index)
	{
		table[i].mark = 1;
		i++;
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


/* Create a constant */
void const_declaration()
{
	do
	{
		// This token should have the constant name in it
		get_token();
		const_token = token;

		// If there's no name, throw an error
		// Symbols Must Be Declared with an Identifier
		is_valid(identsym, 4);

		// This token should have the := in it
		get_token();

		// Equals (":="")
		// Constants Must Be Assigned a Value at Declaration
		is_valid(becomessym, 5);

		// This token should have the value of the constant in it
		get_token();

		// Change the value of const_token to be the value it's supposed to be
		const_token.value = token.value;

		// Make sure there's a number it's being set equal to
		// Constants Must Be Assigned a Value at Declaration
		is_valid(numbersym, 5);

		// Check for conflicting symbols
		// Competing Symbol Declarations
		if (conflicting_symbol(const_token.name))
			errorend(1);

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


/* Create a procedure */
void var_declaration()
{
	do
	{
		// This should have the variable in it
		get_token();

		// Symbols Must Be Declared with an Identifier
		is_valid(identsym, 4);

		// Check for conflicting symbols
		// Competing Symbol Declarations
		if (conflicting_symbol(token.name))
			errorend(1);

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


/* Create a procedure */
// NOTE: Something in here is borked
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
		if (conflicting_symbol(token.name))
			errorend(1);

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


/* Create a statement */
void statement()
{
	switch (token.type)
	{
	case identsym:
		get_token();

		// Make sure that after the variable is the becomes symbol
		// Unrecognized Statement Form
		is_valid(becomessym, 2);

		get_token();
		expression();
		break;

	case callsym:
		get_token();

		// Make sure that after call is the name of the procedure
		// call and read Must Be Followed By an Identifien
		is_valid(identsym, 14);

		get_token();
		break;

	case beginsym:
		do
		{
			get_token();
			statement();
		} while (token.type == semicolonsym);

		// Procedure must end with end
		// begin Must Be Followed By end
		is_valid(endsym, 10);

		get_token();
		break;

	case ifsym:
		get_token();
		condition();

		// if Must Be Followed By then
		is_valid(thensym, 9);

		get_token();
		statement();
		break;

	case whilesym:
		get_token();
		condition();

		// while Must Be Followed By do
		is_valid(dosym, 8);

		get_token();
		statement();
		break;

	case readsym:
		get_token();

		// call and read Must Be Followed By an Identifien
		is_valid(identsym, 14);

		sym = fetch_symbol(token.name);

		if (!sym || sym->kind != 2)
			errorend(7);

		get_token();
		break;

	case writesym:
		get_token();

		// Unrecognized Statement Form
		is_valid(identsym, 2);

		sym = fetch_symbol(token.name);

		if (!sym || (sym->kind != 2 && sym->kind != 1))
			errorend(7);

		get_token();
		break;

	default:
		break;
	}
}


/* Formulas */
void condition()
{
	if (token.type == oddsym)
	{
		get_token();
		expression();
	}
	else
	{
		expression();

		// Make sure it's a relation. If it isn't, throw an error
		// Conditions Must Contain a Relational-Operator
		if (!rel_op())
			errorend(12);

		get_token();
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


/* Add an expression */
void expression()
{
	if (token.type == plussym || token.type == minussym)
		get_token();

	term();

	while (token.type == plussym || token.type == minussym)
	{
		get_token();
		term();
	}
}


/* Multiplication, Division, and Modulus */
void term()
{
	factor();

	while (token.type == multsym || token.type == slashsym || token.type == modsym)
	{
		get_token();
		factor();
	}
}


/* Factor Checker */
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
		errorend(0); // Not sure what error this is
		break;
	}
}


/* Returns NULL if the symbol isn't found */
symbol *fetch_symbol(char *name)
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		if (strcmp(table[i].name, name) == 0 && table[i].mark == 0)
			return &table[i];
	}

	return NULL;
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

	if (error)
	{
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


/* Print the error */
void errorend(int x)
{
	switch (x)
	{
		case 1:
			printf("Parser Error: Competing Symbol Declarations\n");
			break;
		case 2:
			printf("Parser Error: Unrecognized Statement Form\n");
			break;
		case 3:
			printf("Parser Error: Programs Must Close with a Period\n");
			break;
		case 4:
			printf("Parser Error: Symbols Must Be Declared with an Identifier\n");
			break;
		case 5:
			printf("Parser Error: Constants Must Be Assigned a Value at Declaration\n");
			break;
		case 6:
			printf("Parser Error: Symbol Declarations Must Be Followed By a Semicolon\n");
			break;
		case 7:
			printf("Parser Error: Undeclared Symbol\n");
			break;
		case 8:
			printf("Parser Error: while Must Be Followed By do\n");
			break;
		case 9:
			printf("Parser Error: if Must Be Followed By then\n");
			break;
		case 10:
			printf("Parser Error: begin Must Be Followed By end\n");
			break;
		case 11:
			printf("Parser Error: while and if Statements Must Contain Conditions\n");
			break;
		case 12:
			printf("Parser Error: Conditions Must Contain a Relational-Operator\n");
			break;
		case 13:
			printf("Parser Error: ( Must Be Followed By )\n");
			break;
		case 14:
			printf("Parser Error: call and read Must Be Followed By an Identifier\n");
			break;
		default:
			printf("Implementation Error: Unrecognized Error Code\n");
			break;
	}

	// Set error to 1
	error = 1;
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