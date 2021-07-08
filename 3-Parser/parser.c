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

/* Error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

symbol *table;
int sym_index;
lexeme curToken, *list;

/* Utilities */
void printtable();
void errorend(int x);
void getToken();
void expect(int token_type, int err); // expect a given kind of token
void addSymbol(int kind, char *name, int val);

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
	getToken();
	expect(identsym, 4);
	lexeme ident = curToken;
	getToken();
	expect(becomessym, 5); 
	getToken();
	expect(numbersym, 5);
	int value = curToken.value;

	// printf("New constant: name %s, value %d\n", ident.name, value);
	// TODO add this symbol to the symbol table
	
	getToken();
	if (curToken.type == commasym)
	{
		const_declaration();
	}
	else
	{
		expect(semicolonsym, 6); // need ';' at end of const declaration
		getToken();
	}	
}

void var_declaration()
{

}

void procedure_declaration()
{

}

void statement()
{

}

void condition()
{

}

void expression()
{

}

void term()
{

}

void factor()
{

}


void expect(int token_type, int err) {
	if(curToken.type != token_type)
		throw(err);
}

void getToken() {
	static int tokenIndex = 0;
	curToken = list[tokenIndex++];
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
	printf("------------------------------------------------------\n");
	for (i = 0; i < sym_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr); 
}
