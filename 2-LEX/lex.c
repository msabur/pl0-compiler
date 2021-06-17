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
	int IN_COMMENT = 0;
	for(int i = 0; i + 1 < input_len; i++) {
		if(IN_COMMENT) {
			input[i] = ' ';
		}
		if(input[i] == '/' &&
				input[i + 1] == '*')
		{
			input[i] = input[i + 1] = ' ';
			IN_COMMENT = 1;
			i++;
		}
		if(input[i] == '*' &&
				input[i + 1] == '/') {
			input[i] = input[i + 1] = ' ';
			IN_COMMENT = 0;
			i++;
		}
	}
	if(IN_COMMENT) {
		error_number = 5;
		goto error;
	}

	// on error, call printerror and return NULL
	char tmp[500]; *tmp = 0; // current token
	int tmp_index = 0; // next index to read 'tmp[]' from
	int read_index = 0; // next index to read 'input[]' from

	for(read_index = 0; read_index < input_len;)
	{
		if(read_index >= 500) // when at end of input
			goto end;

		// skip any whitespace
		while(iscntrl(input[read_index]))
			read_index++;
		
		// tokenizing a word (identifier or reserved word)
		/*
		 * The number tokenization part depends on this to ignore
		 * words that have numbers in them.
		 */
		if(isalpha(input[read_index]))
		{
			// TODO do stuff
			while(isalnum(input[read_index]))
			{
				// TODO do stuff
				read_index++;
			}
			// TODO do stuff
		}

		// tokenizing a number
		else if(isdigit(input[read_index]))
		{
			while(isdigit(input[read_index]))
			{
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
				// a number followed by a letter
				error_number = 2; // invalid identifier
				goto error;
			}
			list[lex_index].type = numbersym;
			list[lex_index].value = atoi(tmp);
			lex_index++;
		}

				

		// tokenizing a symbol
		else if(isSymbolChar(input[read_index]))
		{
			while(isSymbolChar(input[read_index]))
			{
				tmp[tmp_index++] = input[read_index++];
				if(read_index >= 500)
					break;
			}
			tmp[tmp_index++] = '\0';
			char *curSymbol;
			int symbol_type = -1; // sentinel value

			/*
			 * Processing each symbol individually so we can
			 * distinguish between symbols clumped together, such
			 * as "(((x-1))*7)"
			 * The code is ugly but this is the essence of it:
			 * > foreach char in tmp:
			 * >     if it is the start of a duo:
			 * >         if the next character completes the duo:
			 * >             record the duo
			 * >             skip the next character
			 * >             continue
			 * >     record the char as a symbol, or throw error
			 */
			for(int i = 0; i < tmp_index - 1; i++)
			{
				switch(tmp[i])
				{
					case '=':
					case '<':
					case '>':
					case ':':
						curSymbol = (char []){tmp[i], 
							tmp[i + 1], '\0'};
						if(i + 1 < tmp_index - 1 &&
								isSymbol(curSymbol)) {
							symbol_type = getSymbolType(curSymbol);
							if(symbol_type != -1) {
								list[lex_index++].type = symbol_type;
								i++;
								continue;
							}
						}
					default:
						char *zi = (char []){tmp[i], '\0'};
						if((symbol_type = getSymbolType(zi)) != -1) {
							list[lex_index++].type = symbol_type;
						} else {
							error_number = 1;
							goto error;
						}
				}
			}
			tmp_index = 0; // resetting this variable
			/*
			tmp_index = 0; // resetting this variable
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

			if(symbol_type == -1) {
				printf("Invalid symbol %s\n", tmp); // TODO delete this later
				error_number = 1;
				goto error;
			}

			list[lex_index].type = symbol_type;
			lex_index++;
			*/
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

int isSymbol(char *s) {
	static char *symbols[] = {"==", "<>", "<", "<-", ">", ">=", "%", "*",
		"/", "+", "-", "(", ")", ",", ".", ";", ":="};
	static int symbols_length = sizeof(symbols) - 1;
	for (int i = 0; i < symbols_length; i++)
		if(strcmp(s, symbols[i])) return 1;
	return 0;
}

int isSymbolChar(int c) {
	static char symbolCharacters[] = "=<>%*/+-(),.;:";
	static int symbolCharacters_length = sizeof(symbolCharacters) - 1;
	for(int i = 0; i < symbolCharacters_length; i++)
		if(c == symbolCharacters[i]) return 1;
	return 0;
}

int getSymbolType(char *s) {
	printf("in getSymbolType, %s\n", s);
	if(strcmp(s, "==") == 0)
		return eqlsym;
	else if(strcmp(s, "<>") == 0)
		return neqsym;
	else if(strcmp(s, "<") == 0)
		return lessym;
	else if(strcmp(s, "<=") == 0)
		return leqsym;
	else if(strcmp(s, ">") == 0)
		return gtrsym;
	else if(strcmp(s, ">=") == 0)
		return geqsym;
	else if(strcmp(s, "%") == 0)
		return modsym;
	else if(strcmp(s, "*") == 0)
		return multsym;
	else if(strcmp(s, "/") == 0)
		return slashsym;
	else if(strcmp(s, "+") == 0)
		return plussym;
	else if(strcmp(s, "-") == 0)
		return minussym;
	else if(strcmp(s, "(") == 0)
		return lparentsym;
	else if(strcmp(s, ")") == 0)
		return rparentsym;
	else if(strcmp(s, ",") == 0)
		return commasym;
	else if(strcmp(s, ".") == 0)
		return periodsym;
	else if(strcmp(s, ";") == 0)
		return semicolonsym;
	else if(strcmp(s, ":=") == 0)
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
