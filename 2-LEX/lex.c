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
#include "compiler.h"

#define MAX_IDENT_LENGTH 11
#define MAX_NUMBER_LENGTH 5

lexeme *list;
int lex_index;

// To keep track of our position in the input
int input_index = 0;

// Set up tmp
char tmp[500];	   // tmp array
int tmp_index = 0; // tmp index

// 0 for if there's no error found in the comments,
// 1 for if there is.
int comment_error = 0;

// error_checker to assist in the main while loop
int error_checker = 0;

void printerror(int type);
void printtokens();

// Method to see if an input character is a symbol
int is_symbol(char input_char);

// Methods to process everything
int comment_processor(char *input, int count);
void invisible_char_processor(char *input);
void word_processor(char *input);
void number_processor(char *input);
void symbol_processor(char *input);
void error_processor(int error_number);

/* Check to see if the input is a symbol */
int is_symbol(char input_char)
{
	switch (input_char)
	{
	case '=':
	case '<':
	case '>':
	case '%':
	case '*':
	case '/':
	case '+':
	case '-':
	case '(':
	case ')':
	case ',':
	case '.':
	case ';':
	case ':':
		return 1;
	default:
		return 0;
	}
}

/* Process the comment */
int comment_processor(char *input, int first_loop)
{
	// Only runs during the first loop
	// Skips the "/*"
	if (first_loop == 1)
		input_index += 2;

	// Checks to see if the comment ends.
	if (input[input_index] == '*' && input[input_index + 1] == '/')
	{
		// If it does end, skip "*/"
		input_index += 2;

		// Return 0 since the comment ended without causing an error
		return 0;
	}
	else
	{
		// Check to see if we're past the input.
		if (input_index >= strlen(input))
			// If we are, then this comment never ended, so return 1
			return 1;

		// Increment the input_index, then recursively call itself
		input_index++;
		comment_processor(input, 0);
	}
}

/* Process the invisible characters */
void invisible_char_processor(char *input)
{
	// If we're on an invisible character, recursive call
	// while skipping that input_index space.
	if (iscntrl(input[input_index]) || isspace(input[input_index]))
	{
		input_index++;
		invisible_char_processor(input);
	}
}

/* Process the words */
void word_processor(char *input)
{
	// Read in the string of letters and numbers into tmp
	while (isalnum(input[input_index]))
	{
		// Add the input character to tmp, then increment
		// both indexes
		tmp[tmp_index++] = input[input_index++];

		// Break out of the loop if it reaches the end
		// of the input
		if (input_index >= strlen(input))
			break;

		// Throw an error if the string is longer than the max
		// allowed length.
		if (tmp_index > MAX_IDENT_LENGTH)
			error_processor(4); // Excessive Identifier Length
	}

	// End the string
	tmp[tmp_index] = '\0';

	// Reset tmp_index
	tmp_index = 0;

	// Check for reserved words and identifiers
	// Reserved Words
	if (strcmp(tmp, "const") == 0)
		list[lex_index].type = constsym; // 29
	else if (strcmp(tmp, "var") == 0)
		list[lex_index].type = varsym; // 30
	else if (strcmp(tmp, "procedure") == 0)
		list[lex_index].type = procsym; // 31
	else if (strcmp(tmp, "call") == 0)
		list[lex_index].type = callsym; // 26
	else if (strcmp(tmp, "if") == 0)
		list[lex_index].type = ifsym; // 21
	else if (strcmp(tmp, "then") == 0)
		list[lex_index].type = thensym; // 22
	else if (strcmp(tmp, "else") == 0)
		list[lex_index].type = elsesym; // 23
	else if (strcmp(tmp, "while") == 0)
		list[lex_index].type = whilesym; // 24
	else if (strcmp(tmp, "do") == 0)
		list[lex_index].type = dosym; // 25
	else if (strcmp(tmp, "begin") == 0)
		list[lex_index].type = beginsym; // 19
	else if (strcmp(tmp, "end") == 0)
		list[lex_index].type = endsym; // 20
	else if (strcmp(tmp, "read") == 0)
		list[lex_index].type = readsym; // 28
	else if (strcmp(tmp, "write") == 0)
		list[lex_index].type = writesym; // 27
	else if (strcmp(tmp, "odd") == 0)
		list[lex_index].type = oddsym; // 1
	// Identifiers
	else
	{
		list[lex_index].type = identsym; // 32

		// As this is an indentifier, we copy the
		// identifier onto the list as well.
		strcpy(list[lex_index].name, tmp);
	}

	// Increment the list
	lex_index++;
}

/* Process the numbers */
void number_processor(char *input)
{
	// As long as the input continues to be numbers...
	while (isdigit(input[input_index]))
		tmp[tmp_index++] = input[input_index++];

	if (tmp_index > MAX_NUMBER_LENGTH)
		error_processor(3); // Excessive Number Length

	// End the string
	tmp[tmp_index] = '\0';

	// Reset tmp_index
	tmp_index = 0;

	// Checks to see if the next character is a letter
	if (isalpha(input[input_index]))
		error_processor(2); // Invalid Identifier

	// Add the number symbol to the list along with the actual
	// string of numbers, then increment the list index.
	list[lex_index].type = numbersym;
	list[lex_index].value = atoi(tmp);
	lex_index++;
}

/* Process the symbols */
void symbol_processor(char *input)
{
	// Initialize symbol_type operator
	int symbol_type = -1;

	// A series of switch statements that ape a trie
	switch (input[input_index])
	{
	case '=':
		switch (input[input_index + 1])
		{
		case '=':
			// Set the symbol for "=="
			symbol_type = eqlsym;

			// Move forward two input_index spaces
			input_index += 2;
			break; // Break case "=="
		}
		break; // Break outer case '='

	case '<':
		switch (input[input_index + 1])
		{
		case '>':
			// Set the symbol for "<>"
			symbol_type = neqsym;

			// Move forward two input_index spaces
			input_index += 2;
			break; // Break case "<>"

		case '=':
			// Set the symbol for "<="
			symbol_type = leqsym;

			// Move forward two input_index spaces
			input_index += 2;
			break; // Break case "<="

		default:
			// Set the symbol for "<"
			symbol_type = lessym;

			// Move forward one input_index space
			input_index++;
			break; // Break default case "<"
		}
		break; // Break outer case '<'

	case '>':
		switch (input[input_index + 1])
		{
		case '=':
			// Set the symbol for ">="
			symbol_type = geqsym;

			// Move forward two input_index spaces
			input_index += 2;
			break; // Break case ">="

		default:
			// Set the symbol for ">"
			symbol_type = gtrsym;

			// Move forward one index space
			input_index++;
			break; // Break default case '>'
		}
		break; // Break outer case '>'

	case ':':
		switch (input[input_index + 1])
		{
		case '=':
			// Set the symbol for ":="
			symbol_type = becomessym;

			// Move forward two input_index spaces
			input_index += 2;
			break; // Break case ":="
		}
		break; // Break outer case ':'

	case '/':
		switch (input[input_index + 1])
		{
		case '*':
			// Call the comment processor
			comment_error = comment_processor(input, 1);

			// Set the symbol to -2 so as not to
			// throw an error
			symbol_type = -2;
			break; // Break case "/*"

		default:
			// Set the symbol for "/"
			symbol_type = slashsym;

			// Move forward one index space
			input_index++;
			break; // Break default case "/"
		}
		break; // Break outer case '/'

	case '%':
		// Set the symbol for "%"
		symbol_type = modsym;

		// Move forward one index space
		input_index++;
		break; // Break outer case '%'

	case '*':
		// Set the symbol for "*"
		symbol_type = multsym;

		// Move forward one index space
		input_index++;
		break; // Break outer case '*'

	case '+':
		// Set the symbol for "+"
		symbol_type = plussym;

		// Move forward one index space
		input_index++;
		break; // Break outer case '+'

	case '-':
		// Set the symbol for "-"
		symbol_type = minussym;

		// Move forward one index space
		input_index++;
		break; // Break outer case '-'

	case '(':
		// Set the symbol for "("
		symbol_type = lparentsym;

		// Move forward one index space
		input_index++;
		break; // Break outer case '('

	case ')':
		// Set the symbol for ")"
		symbol_type = rparentsym;

		// Move forward one input_index space
		input_index++;
		break; // Break outer case ')'

	case ',':
		// Set the symbol for ","
		symbol_type = commasym;

		// Move forward one index space
		input_index++;
		break; // Break outer case ','

	case '.':
		// Set the symbol for "."
		symbol_type = periodsym;

		// Move forward one input_index space
		input_index++;
		break; // Break outer case '.'

	case ';':
		// Set the symbol for ";"
		symbol_type = semicolonsym;

		// Move forward one index space
		input_index++;
		break; // Break outer case ';'
	}		   // Close outer switch statement

	// Check to see if an error should be thrown
	if (symbol_type == -1)
		error_processor(1); // Invalid Symbol

	// Append symbol to the list
	list[lex_index].type = symbol_type;
	lex_index++;
}

/* Process errors */
void error_processor(int error_number)
{
	// Call printerror
	printerror(error_number);

	// Set error_checker to true
	error_checker = 1;
}

/* Fill the lexeme list */
lexeme *lexanalyzer(char *input)
{
	// Allocate space for the list of lexemes and initialize
	// the list index at 0
	list = malloc(500 * sizeof(lexeme));
	lex_index = 0;

	// While there's still input to be processed and no errors
	while (input_index < strlen(input) && error_checker == 0)
	{
		// Check for comments
		if (input[input_index] == '/' && input[input_index + 1] == '*')
			comment_error = comment_processor(input, 1);

		// Check for invisible characters
		else if (iscntrl(input[input_index]) || isspace(input[input_index]))
			invisible_char_processor(input);

		// Check for words
		else if (isalpha(input[input_index]))
			word_processor(input);

		// Check for numbers
		else if (isdigit(input[input_index]))
			number_processor(input);

		// Check for symbols
		else if (is_symbol(input[input_index]))
			symbol_processor(input);
	}

	// Check to see if there was a comment error
	if (comment_error == 1)
		error_processor(5); // Neverending Comment

	// Print out the table, but only if there's no errors.
	if (error_checker == 0)
		printtokens();

	// Return NULL if there was an error, otherwise return the list
	// This prevents double-freeing the list.
	return error_checker ? NULL : list;
}

/* Print out the table using the list */
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

/* Print out what error was thrown */
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
