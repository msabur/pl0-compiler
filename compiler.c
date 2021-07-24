/*
	Based on the driver for the UCF Summer 2021 Systems Software Project
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

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

int main(int argc, char **argv)
{
	char *outputfile = "output.pm0";
	FILE *ifp, *ofp;
	char *inputfile;
	int c;
	lexeme *list;
	instruction *code;
	int i;
	
	int showCode = 0;

	if (argc == 1)
	{
		fprintf(stderr, "Error : please include the file name\n");
		return 1;
	}

	if (argc > 2 && strcmp(argv[2], "-c") == 0)
		showCode = 1;

	ifp = fopen(argv[1], "r");
	if (!ifp)
	{
		perror("Opening input file");
		return 1;
	}
	inputfile = malloc(MAX_FILE_LEN * sizeof(char));

	i = 0;
	while ((c = fgetc(ifp)) != EOF)
	{
		inputfile[i++] = c;
	}
	inputfile[i] = '\0';
	fclose(ifp);
	
	list = lexanalyzer(inputfile);
	if (list == NULL)
	{
		free(inputfile);
		return 1;
	}
	
	code = parse(list);
	if (code == NULL)
	{
		free(inputfile);
		free(list);
		return 1;
	}

	if (showCode)
		printcode(code);

	ofp = fopen(outputfile, "w");
	if (!ofp)
	{
		perror("Opening output file");
		return 1;
	}

	writeCodeToFile(code, ofp);
	fclose(ofp);
	
	free(list);
	free(inputfile);
	free(code);
}
