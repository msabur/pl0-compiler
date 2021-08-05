/*
 * COP 3402 - Systems Software
 * Summer 2021
 * Homework #2 (Lexical Analyzer)
 * Authors: msabur, Grant-Allan
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
size_t input_index = 0;

// Set up tmp
char tmp[MAX_FILE_LEN];	   // tmp array
int tmp_index = 0; // tmp index

// 0 for if there's no error found in the comments,
// 1 for if there is.
int comment_error = 0;

// error_checker to assist in the main while loop
int error_checker = 0;

void printerror(int type);

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
		if (input[input_index] == '\0')
			// If we are, then this comment never ended, so return 1
			return 1;

		// Increment the input_index, then recursively call itself
		input_index++;
		comment_processor(input, 0);
	}
	return 0;
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

		// Throw an error if the string is longer than the max
		// allowed length.
		if (tmp_index > MAX_IDENT_LENGTH)
		{	error_processor(4); // Excessive Identifier Length
			return;
		}	
	}

	// End the string
	tmp[tmp_index] = '\0';

	// Reset tmp_index
	tmp_index = 0;

	// Check for reserved words and identifiers
	// Reserved Words
	if (strcmp(tmp, "const") == 0)
		list[lex_index].type = constsym;
	else if (strcmp(tmp, "var") == 0)
		list[lex_index].type = varsym;
	else if (strcmp(tmp, "procedure") == 0)
		list[lex_index].type = procsym;
	else if (strcmp(tmp, "call") == 0)
		list[lex_index].type = callsym;
	else if (strcmp(tmp, "return") == 0)
		list[lex_index].type = returnsym;
	else if (strcmp(tmp, "if") == 0)
		list[lex_index].type = ifsym;
	else if (strcmp(tmp, "then") == 0)
		list[lex_index].type = thensym;
	else if (strcmp(tmp, "else") == 0)
		list[lex_index].type = elsesym;
	else if (strcmp(tmp, "while") == 0)
		list[lex_index].type = whilesym;
	else if (strcmp(tmp, "do") == 0)
		list[lex_index].type = dosym;
	else if (strcmp(tmp, "begin") == 0)
		list[lex_index].type = beginsym;
	else if (strcmp(tmp, "end") == 0)
		list[lex_index].type = endsym;
	else if (strcmp(tmp, "read") == 0)
		list[lex_index].type = readsym;
	else if (strcmp(tmp, "write") == 0)
		list[lex_index].type = writesym;
	else if (strcmp(tmp, "odd") == 0)
		list[lex_index].type = oddsym;
	// Identifiers
	else
	{
		list[lex_index].type = identsym;

		// As this is an identifier, we copy the
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
	{
		error_processor(3); // Excessive Number Length
		return;
	}

	// End the string
	tmp[tmp_index] = '\0';

	// Reset tmp_index
	tmp_index = 0;

	// Checks to see if the next character is a letter
	if (isalpha(input[input_index]))
	{
		error_processor(2); // Invalid Identifier
		return;
	}

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
	{
		error_processor(1); // Invalid Symbol
		return;
	}

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
	list = malloc(MAX_FILE_LEN * sizeof(lexeme));
	lex_index = 0;

	size_t input_len = strlen(input);

	// While there's still input to be processed and no errors
	while (input_index < input_len && error_checker == 0)
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

		// Unrecognized characters are invalid symbols
		else
			error_processor(1);
	}

	// Check to see if there was a comment error
	if (comment_error == 1)
		error_processor(5); // Neverending Comment

	// Return NULL if there was an error, otherwise return the list
	// This prevents double-freeing the list.
	return (error_checker) ? NULL : list;
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
