/*
 * Authors: Maahee, Grant Allan
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_pas_LENGTH 500

typedef struct instruction {
	int op;
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
	
	/* make it easy to refer to opcodes by their name or number */
	enum Opcodes{LIT=1, OPR, LOD, STO, CAL, INC, JMP, JPC, SYS};
	enum Oprcodes{RTN, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS,
		LEQ, GTR, GEQ};
	// TODO make oprcodeName
	const char *opName[] = {
		[LIT] = "LIT",
		[OPR] = "OPR",
		[LOD] = "LOD",
		[STO] = "STO",
		[CAL] = "CAL",
		[INC] = "INC",
		[JMP] = "JMP",
		[JPC] = "JPC",
		[SYS] = "SYS",
		// adding some extra elements to debug out-of-bounds accesses
		"NO1","NO2","NO3","NO4","NO5","NO6","NO7","NO8","NO9","NO0"
	};
	// run the instructions
	int halt = 1; // is set to 0 at end of program
	while(halt != 0) {
		// fetch cycle
		ir.op = pas[pc++];
		ir.l = pas[pc++];
		ir.m = pas[pc++];
		
		// execute cycle
		int lineNumber = pc - 3;
		if(ir.op == JMP) ir.m *= 3;
		
		if(ir.op == LIT) {
			sp = sp + 1;
			pas[sp] = ir.m;
		} else if(ir.op == OPR) {
			if(ir.m == RTN) {
				sp = bp - 1;
				bp = pas[sp + 2];
				pc = pas[sp + 3];
			} else if(ir.m == NEG) {
				pas[sp] = -1 * pas[sp];
			} else if(ir.m == ADD) {
				sp = sp - 1;
				pas[sp] = pas[sp] + pas[sp + 1];
			} else if(ir.m == SUB) {
				sp = sp - 1;
				pas[sp] = pas[sp] - pas[sp + 1];
			} else if(ir.m == MUL) {
				sp = sp - 1;
				pas[sp] = pas[sp] * pas[sp + 1];
			} else if(ir.m == DIV) {
				sp = sp - 1;
				pas[sp] = pas[sp] / pas[sp + 1];
			} else if(ir.m == ODD) {
				pas[sp] = pas[sp] % 2;
			} else if(ir.m == MOD) {
				sp = sp - 1;
				pas[sp] = pas[sp] % pas[sp + 1];
			} else if(ir.m == EQL) {
				sp = sp - 1;
				pas[sp] = pas[sp] == pas[sp + 1];
			} else if(ir.m == NEQ) {
				sp = sp - 1;
				pas[sp] = pas[sp] != pas[sp + 1];
			} else if(ir.m == LSS) {
				sp = sp - 1;
				pas[sp] = pas[sp] < pas[sp + 1];
			} else if(ir.m == LEQ) {
				sp = sp - 1;
				pas[sp] = pas[sp] <= pas[sp + 1];
			} else if(ir.m == GTR) {
				sp = sp - 1;
				pas[sp] = pas[sp] > pas[sp + 1];
			} else if(ir.m == GEQ) {
				sp = sp - 1;
				pas[sp] = pas[sp] >= pas[sp + 1];
			}
		} else if(ir.op == LOD) {
			sp = sp + 1;
			pas[sp] = pas[base(ir.l) + ir.m];
		} else if(ir.op == STO) {
			pas[base(ir.l) + ir.m] = pas[sp];
			sp = sp - 1;
		} else if(ir.op == CAL) {
			pas[sp + 1] = base(ir.l); // static link (SL)
			pas[sp + 2] = bp;        // dynamic link (DL)
			pas[sp + 3] = pc;       // return address (RA)
			bp = sp + 1;
			pc = ir.m;
		} else if(ir.op == INC) {
			sp = sp + ir.m;
		} else if(ir.op == JMP) {
			pc = ir.m; // m was already multiplied by 3, in line 78
		} else if(ir.op == JPC) {
			if(pas[sp] == 0) pc = ir.m;
			sp = sp - 1;
		} else if(ir.op == SYS) {
			if(ir.m == 1) {
				printf("Output result is: %d\n", pas[sp]);
				sp = sp - 1;
			} else if(ir.m == 2) {
				sp = sp + 1;
				printf("Please Enter an Integer: ");
				scanf("%d", &pas[sp]);
			} else if(ir.m == 3) {
				halt = 0;
			}
		}
		printf("%2d %s  %d %d %d %d %d\n",
			lineNumber, opName[ir.op], ir.l, ir.m, pc, bp, sp);
	}
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
