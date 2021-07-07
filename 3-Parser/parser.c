/*
Author: Noelle Midkiff
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

void printtable();
void errorend(int x);
void getNextToken();
void expect(int tokenType, int error); // expect a given kind of token

/* Parsing functions */
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

}

void block()
{

}

void const_declaration()
{

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
