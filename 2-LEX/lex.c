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

lexeme *list;
int lex_index;       // next index to write to lexeme list
char tmp[500] = {0}; // buffer to store current token
int tmp_index = 0;	 // next index to write to 'tmp[]'
int read_index = 0;  // next index to read 'input[]' from

/* Functions to process different kinds of tokens */
int processSymbol(char * input);
int processNumber(char * input);
int processWord(char * input);

/* Helper functions to check symbols */
bool isSymbolChar(int c);
bool isSymbol(char *s);
int getSymbolType(char *s);

/* Output functions */
void printerror(int type);
void printtokens();

lexeme *lexanalyzer(char *input)
{
	list = malloc(500 * sizeof(lexeme));
	lex_index = 0;
	int input_len = strlen(input);

	int error_number = 0;    // 0 means no error, 1-5 tell different errors
	bool IN_COMMENT = false; // tells if we are in a comment or not

	// Processes the entire input
	while (error_number == 0 && read_index < input_len)
	{
		/* Skip invisible characters */
		if (iscntrl(input[read_index]) || isspace(input[read_index]))
			read_index++;

		/* Entering comments */
		else if (!IN_COMMENT && input[read_index] == '/' && input[read_index + 1] == '*')
		{
			IN_COMMENT = 1;
			read_index += 2; // skip the /*
		}

		/* Leaving comments */
		else if (IN_COMMENT && input[read_index] == '*' && input[read_index + 1] == '/')
		{
			IN_COMMENT = 0;
			read_index += 2; // skip the */
		}

		/* Do nothing if we are in a comment */
		else if (IN_COMMENT) {
			read_index++;
		}

		/* Tokenizing a word (identifier or reserved word) */
		else if (isalpha(input[read_index]))
			error_number = processWord(input);

		/* Tokenizing a number */
		else if (isdigit(input[read_index]))
			error_number = processNumber(input);

		/* Tokenizing a symbol */
		else if (isSymbolChar(input[read_index]))
			error_number = processSymbol(input);

		// Characters not matched by other ways are invalid symbols
		else
			error_number = 1; // invalid symbol
	}

	if (error_number != 0)
		THROW_ERROR(error_number);

	if(IN_COMMENT)
		THROW_ERROR(5); // neverending comment

	printtokens();
	return list;
}

int processSymbol(char * input) {
	char *curSymbol;
	int symbol_type;

	// First try to read a two-character symbol
	curSymbol = (char[]){input[read_index], input[read_index + 1], '\0'};
	if ((symbol_type = getSymbolType(curSymbol)) != -1)
	{
		list[lex_index++].type = symbol_type;
		read_index += 2;
		return 0;
	}

	// If that failed, try to read a single-character symbol
	curSymbol = (char[]){input[read_index], '\0'};
	if ((symbol_type = getSymbolType(curSymbol)) != -1)
	{
		list[lex_index++].type = symbol_type;
		read_index++;
		return 0;
	}

	// If that also fails, it's an invalid symbol.
	return 1;
}


int processNumber(char * input) {
	// Store the number in tmp
	while (isdigit(input[read_index]))
		tmp[tmp_index++] = input[read_index++];

	// Throw an error if the number is longer than the max
	// allowed length.
	if (tmp_index > MAX_NUMBER_LENGTH)
		return 3; // number too long

	tmp[tmp_index++] = '\0'; // ends the string for atoi to work
	tmp_index = 0;			 // resetting this variable

	// a number followed by a letter is an error
	if (isalpha(input[read_index]))
		return 2; // invalid identifier

	// add the number to the lexeme list
	list[lex_index].type = numbersym;
	list[lex_index].value = atoi(tmp);
	lex_index++;
	return 0;
}


int processWord(char * input) {
	// Read in the string of letters and numbers into tmp
	while (isalnum(input[read_index]))
	{
		// Fill tmp with the input characters
		tmp[tmp_index++] = input[read_index++];

		// Throw an error if the string is longer than the max
		// allowed length.
		if (tmp_index > MAX_IDENT_LENGTH)
			return 4; // invalid identifier
	}

	tmp[tmp_index++] = '\0';
	tmp_index = 0;	 // resetting this variable
	int word_type;

	// Check for reserved words and identifiers
	// Reserved Words
	if (strcmp(tmp, "const") == 0)
		word_type = constsym; // 29
	else if (strcmp(tmp, "var") == 0)
		word_type = varsym; // 30
	else if (strcmp(tmp, "procedure") == 0)
		word_type = procsym; // 31
	else if (strcmp(tmp, "call") == 0)
		word_type = callsym; // 26
	else if (strcmp(tmp, "if") == 0)
		word_type = ifsym; // 21
	else if (strcmp(tmp, "then") == 0)
		word_type = thensym; // 22
	else if (strcmp(tmp, "else") == 0)
		word_type = elsesym; // 23
	else if (strcmp(tmp, "while") == 0)
		word_type = whilesym; // 24
	else if (strcmp(tmp, "do") == 0)
		word_type = dosym; // 25
	else if (strcmp(tmp, "begin") == 0)
		word_type = beginsym; // 19
	else if (strcmp(tmp, "end") == 0)
		word_type = endsym; // 20
	else if (strcmp(tmp, "read") == 0)
		word_type = readsym; // 28
	else if (strcmp(tmp, "write") == 0)
		word_type = writesym; // 27
	else if (strcmp(tmp, "odd") == 0)
		word_type = oddsym; // 1
	// Identifiers
	else
		word_type = identsym; // 32

	list[lex_index].type = word_type;
	if (word_type == identsym)
		strcpy(list[lex_index].name, tmp);
	lex_index++;
	return 0;
}


bool isSymbol(char *s)
{
	static char *symbols[] = {"==", "<>", "<", "<=", ">", ">=", "%", "*",
		"/", "+", "-", "(", ")", ",", ".", ";", ":="};
	static int symbols_length = sizeof(symbols) - 1;
	for (int i = 0; i < symbols_length; i++)
		if (strcmp(s, symbols[i]))
			return true;
	return false;
}

bool isSymbolChar(int c)
{
	static char symbolCharacters[] = "=<>%*/+-(),.;:";
	static int symbolCharacters_length = sizeof(symbolCharacters) - 1;
	for (int i = 0; i < symbolCharacters_length; i++)
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
