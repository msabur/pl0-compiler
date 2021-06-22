/*
 * COP 3402 - Systems Software
 * Summer 2021
 * Homework #2 (Lexical Analyzer)
 * Authors: Maahee, Grant Allan
 * Due: 6/25/2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"

#define THROW_ERROR(x) do{printerror(x); return NULL;}while(0)
#define MAX_IDENT_LENGTH 11
#define MAX_NUMBER_LENGTH 5

/* Global variables shared by different functions */
lexeme *list;
int lex_index = 0;   // next index to write to lexeme list
int read_index = 0;  // next index to read 'input[]' from

/* Functions to process different kinds of tokens */
int processSymbol(char * input);
int processNumber(char * input);
int processWord(char * input);

/* Helper functions */
bool isSymbolChar(int c);
int getSymbolType(char *s);
void addLexeme(char *name, int value, int type);

/* Output functions */
void printerror(int type);
void printtokens();

lexeme *lexanalyzer(char *input)
{
	list = malloc(500 * sizeof(lexeme));

	int error = 0;
	bool in_comment = false;

	// Processes the entire input
	while (!error && input[read_index] != '\0')
	{
		/* Skip invisible characters */
		if (iscntrl(input[read_index]) || isspace(input[read_index]))
			read_index++;

		/* Entering comments */
		else if (!in_comment && input[read_index] == '/' && input[read_index + 1] == '*')
		{
			in_comment = 1;
			read_index += 2; // skip the /*
		}

		/* Leaving comments */
		else if (in_comment && input[read_index] == '*' && input[read_index + 1] == '/')
		{
			in_comment = 0;
			read_index += 2; // skip the */
		}

		/* Do nothing if we are in a comment */
		else if (in_comment)
			read_index++;

		/* Processing a word (identifier or reserved word) */
		else if (isalpha(input[read_index]))
			error = processWord(input);

		/* Processing a number */
		else if (isdigit(input[read_index]))
			error = processNumber(input);

		/* Processing a symbol */
		else if (isSymbolChar(input[read_index]))
			error = processSymbol(input);

		// Characters not matched by other ways are invalid symbols
		else
			error = 1; // invalid symbol
	} // while

	if (error)
		THROW_ERROR(error);

	if(in_comment)
		THROW_ERROR(5); // neverending comment

	printtokens();
	return list;
}

int processSymbol(char * input) {
	char sym[] = {input[read_index], input[read_index + 1], '\0'};

	/* Look for valid symbols of length 2 or 1, storing the first found.
	 * This is for separating sequences like ==(
	 * We first store == of length 2, then ( of length 1.
	 */
	for (int n = (sym[1] == '\0') ? 1 : 2; n != 0; n--)
	{
		int symbol_type;
		sym[n] = '\0';
		if ((symbol_type = getSymbolType(sym)) != -1)
		{
			addLexeme(sym, 0, symbol_type);
			read_index += n;
			return 0;
		}
	}

	// If we didn't find any valid symbol, it must be invalid
	return 1; // invalid symbol
}

int processNumber(char * input) {
	// Reading the number into a buffer
	char tmp[501];
	int numberLength;
	sscanf(&input[read_index], "%500[0-9]%n", tmp, &numberLength);
	read_index += numberLength;

	// Validation
	if (numberLength > MAX_NUMBER_LENGTH)
		return 3; // number too long
	if(isalpha(input[read_index]))
		return 2; // invalid identifier

	addLexeme("", atoi(tmp), numbersym);
	return 0;
}

int processWord(char * input) {
	// Reading the word into a buffer
	char tmp[501];
	int wordLength;
	sscanf(&input[read_index], "%500[a-zA-Z0-9]%n", tmp, &wordLength);
	read_index += wordLength;

	// Validation
	if (wordLength > MAX_IDENT_LENGTH)
		return 4; // identifier too long

	// Check for reserved words and identifiers
	// Reserved Words
	if (strcmp(tmp, "const") == 0)
		addLexeme(tmp, 0, constsym);
	else if (strcmp(tmp, "var") == 0)
		addLexeme(tmp, 0, varsym);
	else if (strcmp(tmp, "procedure") == 0)
		addLexeme(tmp, 0, procsym);
	else if (strcmp(tmp, "call") == 0)
		addLexeme(tmp, 0, callsym);
	else if (strcmp(tmp, "if") == 0)
		addLexeme(tmp, 0, ifsym);
	else if (strcmp(tmp, "then") == 0)
		addLexeme(tmp, 0, thensym);
	else if (strcmp(tmp, "else") == 0)
		addLexeme(tmp, 0, elsesym);
	else if (strcmp(tmp, "while") == 0)
		addLexeme(tmp, 0, whilesym);
	else if (strcmp(tmp, "do") == 0)
		addLexeme(tmp, 0, dosym);
	else if (strcmp(tmp, "begin") == 0)
		addLexeme(tmp, 0, beginsym);
	else if (strcmp(tmp, "end") == 0)
		addLexeme(tmp, 0, endsym);
	else if (strcmp(tmp, "read") == 0)
		addLexeme(tmp, 0, readsym);
	else if (strcmp(tmp, "write") == 0)
		addLexeme(tmp, 0, writesym);
	else if (strcmp(tmp, "odd") == 0)
		addLexeme(tmp, 0, oddsym);
	// Identifiers
	else
		addLexeme(tmp, 0, identsym);

	return 0;
}

bool isSymbolChar(int c)
{
	static char symbolCharacters[] = "=<>%*/+-(),.;:";
	for (int i = 0; symbolCharacters[i] != '\0'; i++)
		if (c == symbolCharacters[i])
			return true;
	return false;
}

int getSymbolType(char *s)
{
	if (strcmp(s, "==") == 0)
		return eqlsym;
	else if (strcmp(s, "<>") == 0)
		return neqsym;
	else if (strcmp(s, "<") == 0)
		return lessym;
	else if (strcmp(s, "<=") == 0)
		return leqsym;
	else if (strcmp(s, ">") == 0)
		return gtrsym;
	else if (strcmp(s, ">=") == 0)
		return geqsym;
	else if (strcmp(s, "%") == 0)
		return modsym;
	else if (strcmp(s, "*") == 0)
		return multsym;
	else if (strcmp(s, "/") == 0)
		return slashsym;
	else if (strcmp(s, "+") == 0)
		return plussym;
	else if (strcmp(s, "-") == 0)
		return minussym;
	else if (strcmp(s, "(") == 0)
		return lparentsym;
	else if (strcmp(s, ")") == 0)
		return rparentsym;
	else if (strcmp(s, ",") == 0)
		return commasym;
	else if (strcmp(s, ".") == 0)
		return periodsym;
	else if (strcmp(s, ";") == 0)
		return semicolonsym;
	else if (strcmp(s, ":=") == 0)
		return becomessym;
	else
		return -1;
}

void addLexeme(char *name, int value, int type) {
	strcpy(list[lex_index].name, name);
	list[lex_index].type = type;
	list[lex_index].value = value;
	lex_index++;
}

void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
		case oddsym:
			printf("%11s\t%d", "odd", oddsym);
			break;
		case eqlsym:
			printf("%11s\t%d", "==", eqlsym);
			break;
		case neqsym:
			printf("%11s\t%d", "<>", neqsym);
			break;
		case lessym:
			printf("%11s\t%d", "<", lessym);
			break;
		case leqsym:
			printf("%11s\t%d", "<=", leqsym);
			break;
		case gtrsym:
			printf("%11s\t%d", ">", gtrsym);
			break;
		case geqsym:
			printf("%11s\t%d", ">=", geqsym);
			break;
		case modsym:
			printf("%11s\t%d", "%", modsym);
			break;
		case multsym:
			printf("%11s\t%d", "*", multsym);
			break;
		case slashsym:
			printf("%11s\t%d", "/", slashsym);
			break;
		case plussym:
			printf("%11s\t%d", "+", plussym);
			break;
		case minussym:
			printf("%11s\t%d", "-", minussym);
			break;
		case lparentsym:
			printf("%11s\t%d", "(", lparentsym);
			break;
		case rparentsym:
			printf("%11s\t%d", ")", rparentsym);
			break;
		case commasym:
			printf("%11s\t%d", ",", commasym);
			break;
		case periodsym:
			printf("%11s\t%d", ".", periodsym);
			break;
		case semicolonsym:
			printf("%11s\t%d", ";", semicolonsym);
			break;
		case becomessym:
			printf("%11s\t%d", ":=", becomessym);
			break;
		case beginsym:
			printf("%11s\t%d", "begin", beginsym);
			break;
		case endsym:
			printf("%11s\t%d", "end", endsym);
			break;
		case ifsym:
			printf("%11s\t%d", "if", ifsym);
			break;
		case thensym:
			printf("%11s\t%d", "then", thensym);
			break;
		case elsesym:
			printf("%11s\t%d", "else", elsesym);
			break;
		case whilesym:
			printf("%11s\t%d", "while", whilesym);
			break;
		case dosym:
			printf("%11s\t%d", "do", dosym);
			break;
		case callsym:
			printf("%11s\t%d", "call", callsym);
			break;
		case writesym:
			printf("%11s\t%d", "write", writesym);
			break;
		case readsym:
			printf("%11s\t%d", "read", readsym);
			break;
		case constsym:
			printf("%11s\t%d", "const", constsym);
			break;
		case varsym:
			printf("%11s\t%d", "var", varsym);
			break;
		case procsym:
			printf("%11s\t%d", "procedure", procsym);
			break;
		case identsym:
			printf("%11s\t%d", list[i].name, identsym);
			break;
		case numbersym:
			printf("%11d\t%d", list[i].value, numbersym);
			break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
	list[lex_index++].type = -1;
}

void printerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Excessive Number Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Excessive Identifier Length\n");
	else if (type == 5)
		printf("Lexical Analyzer Error: Neverending Comment\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");

	free(list);
	return;
}
