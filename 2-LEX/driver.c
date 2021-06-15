/*
	This is the driver for the UCF Summer 2021 Systems Software Project
	Alter at your own risk.
*/

#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"

int main(int argc, char **argv)
{
	FILE *ifp;
	char *inputfile;
	char c;
	lexeme *list;
	int i;
	
	if (argc < 2)
	{
		printf("Error : please include the file name");
		return 0;
	}
	
	ifp = fopen(argv[1], "r");
	inputfile = malloc(500 * sizeof(char));
	i = 0;
	
	c = fgetc(ifp);
	while (1)
	{
		inputfile[i++] = c;
		c = fgetc(ifp);
		if (c == EOF)
			break;
	}
	inputfile[i] = '\0';
	
	list = lexanalyzer(inputfile);
	if (list == NULL)
	{
		free(inputfile);
		return 0;
	}
	
	free(list);
	free(inputfile);
	return 0;
}