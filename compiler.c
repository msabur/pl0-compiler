/*
	Based on the driver for the UCF Summer 2021 Systems Software Project
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "getopt.h"

extern int lex_index;

int show_code, show_lex, no_output;
char outputfile[256] = "output.pm0", inputfile[256] = {0};

void printcode(instruction *code);
void printlexemes(lexeme *lexemes);
void writeCodeToFile(instruction *code, FILE *fp);
void parseArgs(int argc, const char ** argv);
static void print_help_string(getopt_context_t ctx);

static const getopt_option_t option_list[] =
{
	{ "help",         'h', GETOPT_OPTION_TYPE_NO_ARG,   0x0,        'h', "print this help text",    0x0 },
	{ "show-code",    'c', GETOPT_OPTION_TYPE_FLAG_SET, &show_code, 'c', "print generated code",    0x0 },
	{ "show-lexemes", 'l', GETOPT_OPTION_TYPE_FLAG_SET, &show_lex,  'l', "print lexeme table",      0x0 },
	{ "output",       'o', GETOPT_OPTION_TYPE_REQUIRED, 0x0,        'o', "the output file",      "FILE" },
	{ "no-output",    'n', GETOPT_OPTION_TYPE_FLAG_SET, &no_output, 'n', "don't make output file",  0x0 },
	GETOPT_OPTIONS_END
};


int main(int argc, const char **argv)
{
	parseArgs(argc, argv);

	FILE *ifp, *ofp;
	char *input;
	int c;
	lexeme *list;
	instruction *code;
	int i;
	
	if (inputfile[0] == '\0')
	{
		fprintf(stderr, "Error : please include the file name\n");
		return 1;
	}

	ifp = fopen(inputfile, "r");
	if (!ifp)
	{
		perror("Opening input file");
		return 1;
	}
	input = malloc(MAX_FILE_LEN * sizeof(char));

	i = 0;
	while ((c = fgetc(ifp)) != EOF)
	{
		input[i++] = c;
	}
	input[i] = '\0';
	fclose(ifp);
	
	list = lexanalyzer(input);
	if (list == NULL)
	{
		free(input);
		return 1;
	}
	
	if (show_lex)
		printlexemes(list);

	code = parse(list);
	if (code == NULL)
	{
		free(input);
		free(list);
		return 1;
	}

	if (show_code)
		printcode(code);

	if (!no_output)
	{
		ofp = fopen(outputfile, "w");
		if (!ofp)
		{
			perror("Opening output file");
			return 1;
		}
		writeCodeToFile(code, ofp);
		fclose(ofp);
	}
	
	free(list);
	free(input);
	free(code);
}

static void print_help_string(getopt_context_t ctx)
{
	puts("Usage: ./compiler [options] filename");
	puts("Options:");
	char buffer[2048];
	printf("%s\n", getopt_create_help_string(&ctx, buffer, sizeof(buffer)));
}

void parseArgs(int argc, const char ** argv) {
    getopt_context_t ctx;
    if(getopt_create_context(&ctx, argc, argv, option_list) < 0)
    {
        printf("error while creating getopt ctx, bad options-list?");
        exit(1);
    }
    
    int opt;
    
    while((opt = getopt_next(&ctx)) != -1)
    {
        switch(opt)
        {
            case '+': strncpy(inputfile,                     ctx.current_opt_arg, 255); break;
            case '?': printf("unknown option %s\n",               ctx.current_opt_arg); break;
            case '!': printf("argument required for option %s\n", ctx.current_opt_arg); break;
            case 'o': strncpy(outputfile, ctx.current_opt_arg, 255); break;
            case 'h': print_help_string(ctx); break;
            default:  break;
        }
    }
}


void writeCodeToFile(instruction *code, FILE *fp)
{
	int i = 0, halted = 0;
	while (!halted)
	{
		fprintf(fp, "%d %d %d\n", code[i].opcode, code[i].l, code[i].m);
		if (code[i].opcode == 9 && code[i].l == 0 && code[i].m == 3)
			halted = 1;
		i++;
	}
}

void printcode(instruction *code)
{
	puts("Generated code:");
	int i, halted = 0;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; !halted; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RTN\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("ODD\t");
						break;
					case 7:
						printf("MOD\t");
						break;
					case 8:
						printf("EQL\t");
						break;
					case 9:
						printf("NEQ\t");
						break;
					case 10:
						printf("LSS\t");
						break;
					case 11:
						printf("LEQ\t");
						break;
					case 12:
						printf("GTR\t");
						break;
					case 13:
						printf("GEQ\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
		if (code[i].opcode == 9 && code[i].l == 0 && code[i].m == 3)
			halted = 1;
	}
}

void printlexemes(lexeme *lexemes)
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (lexemes[i].type)
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
		case returnsym:
			printf("%11s\t%d", "return", returnsym);
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
			printf("%11s\t%d", lexemes[i].name, identsym);
			break;
		case numbersym:
			printf("%11d\t%d", lexemes[i].value, numbersym);
			break;
		}
		printf("\n");
	}
	printf("\n");
}
