/*
 * Authors: Maahee, 
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_PAS_LENGTH 500

typedef struct instruction {
	int opcode;
	int l;
	int m;
} instruction;

/* Registers for the CPU */
int SP, BP, PC;
instruction *IR;

/* Memory area */
int PAS[MAX_PAS_LENGTH] = {0}; // initialized to zeroes

int base(int L) {
	int base = 0; // TODO: remove this line
	int arb = BP;	// arb = activation record base
	while ( L > 0) { //find base L levels down
		arb = PAS[base];
		L--;
	}
	return arb;
}

int main(int argc, char **argv) {
	FILE *inputFile = fopen(argv[1], "r");
	char ch;
	char line[1000];
	while((fgets(line, sizeof(line) - 1, inputFile)) != NULL) {
		// TODO: store the input file into PAS array
		printf("%s", line);
	}
	puts("");
	fclose(inputFile);
}

/*
Rubric:
-100 – Does not compile
10   – Compiles
20   – Produces lines of meaningful execution before segfaulting or looping infinitely
5    – Follows IO specifications (takes command line argument for input file name and prints output to console)
5    – README.txt containing author names
5    – Fetch cycle is implemented correctly
15   – Instructions are not implemented with individual functions
5    – Well commented source code
5    – Arithmetic instructions are implemented correctly
5    – Read and write instructions are implemented correctly
10   – Load and store instructions are implemented correctly
10   – Call and return instructions are implemented correctly
5    – Follows formatting guidelines correctly, source code is named vm.c
*/

/*
c) Student names should be written in the header comment of each source code file, in the readme, and in the comments of the submission
*/
