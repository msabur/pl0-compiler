/*
 * Authors: Maahee, Grant Allan
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_pas_LENGTH 500

typedef struct instruction {
	int opcode;
	int l;
	int m;
} instruction;

/* Registers */
int sp, bp, pc;
instruction ir;

/* Memory */
int pas[MAX_pas_LENGTH]; // global, automatically initialized to zeros

int base(int L) {
	int arb = bp;	// arb = activation record base
	while ( L > 0) { //find base L levels down
		arb = pas[arb];
		L--;
	}
	return arb;
}

int main(int argc, char **argv) {
	// load instructions into memory and initialize registers
	FILE *inputFile = fopen(argv[1], "r");
	char line[1000];
	sp = -1;
	while((fgets(line, sizeof(line) - 1, inputFile)) != NULL) {
		int op, l, m;
		sscanf(line, "%d %d %d", &op, &l, &m);
		pas[++sp] = op;
		pas[++sp] = l;
		pas[++sp] = m;
	}
	puts("");
	fclose(inputFile);
	bp = sp + 1;
	// finished loading instructions and initializing registers
	
	puts("                PC   BP   SP   stack");
	printf("%-16s%-5d%-5d%-5d\n", "Initial values:", pc, bp, sp);
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
