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

int isSymbol(int c);
void printerror(int type);
void printtokens();

lexeme *lexanalyzer(char *input)
{
	list = malloc(500 * sizeof(lexeme));
	lex_index = 0;

	// on error, call printerror and return NULL
	char tmp[500]; *tmp = 0; // current token
	int tmp_index = 0; // next index to read 'tmp[]' from
	int read_index = 0; // next index to read 'input[]' from
	int error_number = -1;
	
	// Processes the entire input
	while(1)
	{
		// If it reaches the end of the input
		if(read_index >= 500)
			goto end;

		// Skip invisible characters
		while (iscntrl(input[read_index]) || isspace(input[read_index]))
		{
			read_index++;

			if (read_index >= 500)
				goto end;
		}

		// Tokenizing a word (identifier or reserved word)
		/*
			NOTE: Until we're told that numers are supposed to be
			part of the words, I'm going to assume they aren't.
		*/
		if(isalpha(input[read_index]))
		{
			// Read in the string of letters into temp
			while (isalpha(input[read_index]))
			{
				// Fill tmp with the input characters
				tmp[tmp_index++] = input[read_index++];

				// Break out of the loop if it reaches the end
				// of the input before finishing reading the string
				if (read_index >= 500)
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
			tmp_index = 0;		  // resetting this variable
			int letter = -1; // sentinel value


			// Check for reserved words and random words
			// TODO
		}

		// Tokenizing a number
		if(isdigit(input[read_index]))
		{
			// As long as the input continues to be numbers...
			while(isdigit(input[read_index]))
			{
				// Fill tmp with the input characters
				tmp[tmp_index++] = input[read_index++];

				if(read_index >= 500)
					// we reached the end of the file
					goto end;
				else if(tmp_index > MAX_NUMBER_LENGTH)
				{
					error_number = 3;
					goto error;
				} 
			}

			tmp[tmp_index++] = '\0'; // to end the string
			tmp_index = 0; // resetting this variable
			
			if(isalpha(input[read_index]))
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
		/*
								---Note---

			Currently, this doesn't properly account for comments.
			If it's something like in Appendix A, where it goes
			* / * (but without the spaces), the while loop would
			put all those in a single string. Which would break
			the if statements as they are now.

			Basically, we need a second checking function to catch
			stuff like that.

			Something we could do for that is check a block of two
			characters at once, to see if any of the double character
			strings are a match. If it's not, we go back and look for
			a match for the first character. Then we check the next
			two as a set, etc until we reach the end of the current
			tmp string.

			Ex:
			tmp = [/,/,/,*]

			We'd check elements 0 and 1 as a block ("//"). We wouldn't
			find a match, so we'd check to find a match for element 0.
			We'd find a match for that one and move onto checking
			elements 1 and 2 as a block ("//"). No match, so check element 1.
			Find a match for it, check 2 and 3 ("/*"). We find a match. We check
			to see if there's an element 5 and 6 block. There's not, so we check
			to see if there's an element 5. There's not, so we exit the loop.
		*/
		if(isSymbol(input[read_index]))
		{
			while(isSymbol(input[read_index]))
			{
				// Fill tmp with the string of symbols
				tmp[tmp_index++] = input[read_index++];

				// Break out of the loop if it reaches the end of the input
				if(read_index >= 500)
					break;
			}

			tmp[tmp_index++] = '\0';
			tmp_index = 0; // resetting this variable
			int symbol_type = -1; // sentinel value

			if(strcmp(tmp, "==") == 0)
				symbol_type = eqlsym;
			else if(strcmp(tmp, "<>") == 0)
				symbol_type = neqsym;
			else if(strcmp(tmp, "<") == 0)
				symbol_type = lessym;
			else if(strcmp(tmp, "<=") == 0)
				symbol_type = leqsym;
			else if(strcmp(tmp, ">") == 0)
				symbol_type = gtrsym;
			else if(strcmp(tmp, ">=") == 0)
				symbol_type = geqsym;
			else if(strcmp(tmp, "%") == 0)
				symbol_type = modsym;
			else if(strcmp(tmp, "*") == 0)
				symbol_type = multsym;
			else if(strcmp(tmp, "/") == 0)
				symbol_type = slashsym;
			else if(strcmp(tmp, "+") == 0)
				symbol_type = plussym;
			else if(strcmp(tmp, "-") == 0)
				symbol_type = minussym;
			else if(strcmp(tmp, "(") == 0)
				symbol_type = lparentsym;
			else if(strcmp(tmp, ")") == 0)
				symbol_type = rparentsym;
			else if(strcmp(tmp, ",") == 0)
				symbol_type = commasym;
			else if(strcmp(tmp, ".") == 0)
				symbol_type = periodsym;
			else if(strcmp(tmp, ";") == 0)
				symbol_type = semicolonsym;
			else if(strcmp(tmp, ":=") == 0)
				symbol_type = becomessym;
			// Check for if it's a comment
			else if (strcmp(tmp, "/*") == 0)
			{
				// If it is a comment, we want to skip everything
				// until we reach the end of the comment.
				while (strcmp(tmp, "*/") != 0)
				{
					// If tmp is "*/" after finishing the if statement,
					// we'll naturally break out of the while loop
					if (isSymbol(input[read_index]))
					{
						// Checks for symbols and fills tmp with
						// them if it finds them.
						while (isSymbol(input[read_index]))
						{
							// Fill tmp with the string of symbols
							tmp[tmp_index++] = input[read_index++];

							// Throw an error if it reaches the end of the
							// input without the comment ending
							if (read_index >= 500)
								error_number = 5;
								goto error;
						}

						tmp[tmp_index++] = '\0';
						tmp_index = 0; // resetting this variable
					}

					// If we reach the end of the input, goto end
					if (read_index >= 500)
						goto end;
				}
			}

			if (symbol_type == -1)
			{
				printf("Invalid symbol %s\n", tmp); // TODO delete this later
				error_number = 1;
				goto error;
			}

			list[lex_index].type = symbol_type;
			lex_index++;
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

int isSymbol(int c) {
	static char symbolCharacters[] = "=<>%*/+-(),.;:";
	static int symbolCharacters_length = sizeof(symbolCharacters) - 1;
	for(int i = 0; i < symbolCharacters_length; i++)
		if(c == symbolCharacters[i]) return 1;
	return 0;
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
