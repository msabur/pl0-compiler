/*
   This is the lex skeleton for the UCF Summer 2021 Systems Software Project
   Implement the function lexanalyzer, add as many functions and global
   variables as desired, but do not alter printerror or printtokens.
   Include your name (and your partner's name) in this comment in order to 
   earn the points for compiling
Names: Grant Allan, Maahee Abdus Sabur
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

int isSymbolChar(int c);
int isSymbol(char *s);
int getSymbolType(char *s);
void printerror(int type);
void printtokens();

lexeme *lexanalyzer(char *input)
{
	list = malloc(500 * sizeof(lexeme));
	lex_index = 0;
	int input_len = strlen(input);
	int error_number = -1;

	/* Replacing comments with whitespace so that they are ignored */
	// Run through the entire input
	for (int i = 0; i + 1 < input_len; i++)
	{
		// Enter the comment
		if (input[i] == "/" && input[i + 1] == "*")
		{
			// Until the comment is closed
			while (input[i] != "*" && input[i + 1] != "/")
			{
				// Replace the comment with whitespace
				input[i] = " ";
				i++;

				// Throws an error if the comment never ends
				if (i > input_len)
				{
					error_number = 5; // neverending comment
					goto error;
				}
			}

			// Set the "*/" to whitespace
			input[i] == " ";
			input[i++] == " ";
		}
	}

	// On error, call printerror and return NULL
	char tmp[500];
	*tmp = 0;			// current token
	int tmp_index = 0;	// next index to read 'tmp[]' from
	int read_index = 0; // next index to read 'input[]' from

	// Processes the entire input
	while (read_index < input_len)
	{
		// Skip invisible characters
		while (iscntrl(input[read_index]) || isspace(input[read_index]))
		{
			read_index++;

			if (read_index >= input_len)
				goto end;
		}

		// Tokenizing a word (identifier or reserved word)
		if (isalpha(input[read_index]))
		{
			// Read in the string of letters and numbers into tmp
			while (isalnum(input[read_index]))
			{
				// Fill tmp with the input characters
				tmp[tmp_index++] = input[read_index++];

				// Break out of the loop if it reaches the end
				// of the input before finishing reading the string
				if (read_index >= input_len)
					break;

				// Throw an error if the string is longer than the max
				// allowed length.
				if (tmp_index > MAX_IDENT_LENGTH)
				{
					error_number = 4; // invalid identifier
					goto error;
				}
			}

			tmp[tmp_index++] = '\0';
			tmp_index = 0;	 // resetting this variable
			int word_type = -1; // sentinel value

			// Check for reserved words and identifiers
			// Reserved Words
			if (strcmp(tmp, "const") == 0)
				word_type = constsym; // 29
			else if (strcmp(tmp, "var") == 0)
				word_type = varsym; // 30
			else if (strcmp(tmp, "preocedure") == 0)
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
		}

		// Tokenizing a number
		else if (isdigit(input[read_index]))
		{
			// As long as the input continues to be numbers...
			while (isdigit(input[read_index]))
			{
				// Fill tmp with the input characters
				tmp[tmp_index++] = input[read_index++];

				if (read_index >= input_len)
					// we reached the end of the file
					goto end;
				else if (tmp_index > MAX_NUMBER_LENGTH)
				{
					error_number = 3; // number too long
					goto error;
				}
			}
			tmp[tmp_index++] = '\0'; // to end the string
			tmp_index = 0;			 // resetting this variable

			if (isalpha(input[read_index]))
			{
				// A number followed by a letter
				error_number = 2; // invalid identifier
				goto error;
			}
			list[lex_index].type = numbersym;
			list[lex_index].value = atoi(tmp);
			lex_index++;
		}

		// Tokenizing a symbol
		else if (isSymbolChar(input[read_index]))
		{
			while (isSymbolChar(input[read_index]))
			{
				tmp[tmp_index++] = input[read_index++];
				if (read_index >= input_len)
					break;
			}
			tmp[tmp_index++] = '\0';
			char *curSymbol;
			int symbol_type = -1; // sentinel value

			/*
			 * Processing each symbol individually so we can
			 * distinguish between symbols clumped together, such
			 * as "(((x-1))*7)". To test it: <symbolsWithoutWhitespace.txt>.
			 * The code is ugly but this is the essence of it:
			 * > for each char in tmp:
			 * >     if it is the start of a duo:
			 * >         if the next character completes the duo:
			 * >             record the duo
			 * >             skip the next character
			 * >             continue
			 * >     record the char as a symbol, or throw error
			 * 
			 */
			// Run through the string
			for (int i = 0; i < tmp_index - 1; i++)
			{
				switch (tmp[i])
				{
				case '=':
				case '<':
				case '>':
				case ':':
					curSymbol = (char[])
					{
						tmp[i],
						tmp[i + 1],
						'\0'
					};
					if (i <= tmp_index - 1 && isSymbol(curSymbol))
					{
						symbol_type = getSymbolType(curSymbol);
						
						if (symbol_type != -1)
						{
							list[lex_index++].type = symbol_type;
							i++;
							continue;
						}
					}
				default:
					curSymbol = (char[]){tmp[i], '\0'};
					if ((symbol_type = getSymbolType(curSymbol)) != -1)
					{
						list[lex_index++].type = symbol_type;
					}
					else
					{
						error_number = 1;
						goto error;
					}
				}
			}
			tmp_index = 0; // resetting this variable
		}

		else
			read_index++;
	}
end:
	printtokens();
	return list;
error:
	printerror(error_number);
	return NULL;
}

int isSymbol(char *s)
{
	static char *symbols[] = {"==", "<>", "<", "<-", ">", ">=", "%", "*",
							  "/", "+", "-", "(", ")", ",", ".", ";", ":="};
	static int symbols_length = sizeof(symbols) - 1;
	for (int i = 0; i < symbols_length; i++)
		if (strcmp(s, symbols[i]))
			return 1;
	return 0;
}

int isSymbolChar(int c)
{
	static char symbolCharacters[] = "=<>%*/+-(),.;:";
	static int symbolCharacters_length = sizeof(symbolCharacters) - 1;
	for (int i = 0; i < symbolCharacters_length; i++)
		if (c == symbolCharacters[i])
			return 1;
	return 0;
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
