/*
Author: Noelle Midkiff
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "compiler.h"

/* Set up error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

symbol *table;
int sym_index;

void printtable();
void errorend(int x);

symbol *parse(lexeme *input)
{
	table = malloc(1000 * sizeof(symbol));
	sym_index = 0;
	error = 0;

	if (catch() != 0)
	{
		errorend(error);
		free(table);
		return NULL;
	}
	else
	{
		printtable();
		return table;
	}
}

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
