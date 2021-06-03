/* COP 3402 - Systems Software
 * Summer 2021
 * Homework #1 (P-Machine)
 * Authors: Maahee, Grant Allan
 * Due: 6/4/2021
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_pas_LENGTH 500

typedef struct instruction
{
	// Operation Code
	int op;

	// Lexicographical
	int l;

	// For LIT and INC: A number
	// For JMP, JPC, and CAL: A program address
	// For LOD and STO: A data address
	// For OPR: The identity of OPR
	int m;

} instruction;

/* Registers */
// Stack Pointer, Base Pointer, Program Counter
int sp, bp, pc;

// Instruction Register
instruction ir;

/* Memory */
// Process Address Space array
int pas[MAX_pas_LENGTH]; // global, automatically initialized to zeros

int base(int L)
{
	// arb = activation record base
	int arb = bp;

	// Find base L levels down
	while (L > 0)
	{
		arb = pas[arb];
		L--;
	}
	
	return arb;
}

int main(int argc, char **argv)
{
	/* Load instructions into memory and initialize registers */
	// Open input file to read
	FILE *inputFile = fopen(argv[1], "r");

	// Output an error message if the file couldn't be opened
	if (inputFile == NULL)
	{
		perror("Opening file");
		exit(EXIT_FAILURE);
	}

	// Create an array to take input from the input file
	char line[1000];

	// Initialize the stack pointer
	sp = -1;

	// Run through the entire input file. Store the values in
	// pas in sets of three [op],[l],[m], [op],[l],[m],...
	while ((fgets(line, sizeof(line) - 1, inputFile)) != NULL)
	{
		int op, l, m;
		int n = sscanf(line, "%d %d %d", &op, &l, &m);

		// ignore empty lines
		if (n != 3)
			continue;
		
		pas[++sp] = op;
		pas[++sp] = l;
		pas[++sp] = m;
	}

	// Close the input file, since we're done with it now.
	fclose(inputFile);

	// Initialize base pointer
	bp = sp + 1;

	/* Finished with loading instructions and initializing registers */

	// Set up the table to display the output
	puts("                PC   BP   SP   stack");
	printf("%-16s%-5d%-5d%-5d\n", "Initial values:", pc, bp, sp);

	/* This is all to make it easy to refer to instructions by their name or number */
	enum Opcodes
	{
		LIT = 1,
		OPR,
		LOD,
		STO,
		CAL,
		INC,
		JMP,
		JPC,
		SYS
	};

	enum Oprcodes
	{
		RTN,
		NEG,
		ADD,
		SUB,
		MUL,
		DIV,
		ODD,
		MOD,
		EQL,
		NEQ,
		LSS,
		LEQ,
		GTR,
		GEQ
	};

	char *opName[] = {
		[1] = "LIT",
		"OPR",
		"LOD",
		"STO",
		"CAL",
		"INC",
		"JMP",
		"JPC",
		"SYS",
	};

	char *oprName[] = {
		"RTN",
		"NEG",
		"ADD",
		"SUB",
		"MUL",
		"DIV",
		"ODD",
		"MOD",
		"EQL",
		"NEQ",
		"LSS",
		"LEQ",
		"GTR",
		"GEQ"
	};

	// Helper variables to help in printing the stack
	int curLevel = 0;
	int isBorder[MAX_pas_LENGTH] = {0};
	int initialBase = bp;

	// Halt is set to zero at the end of a successfully run program
	int halt = 1;

	/* Now that we have everything up, run the instructions */
	while (halt != 0)
	{
		/* Fetch Cycle */
		ir.op = pas[pc++];
		ir.l = pas[pc++];
		ir.m = pas[pc++];


		/* Execute Cycle */

		// Note: lineNumber is triple the actual line number in the file
		int lineNumber = pc - 3;

		// Use switch for the instructions
		switch (ir.op)
		{
		case LIT:
			sp = sp + 1;
			pas[sp] = ir.m;
			break;

		case OPR:
			switch (ir.m)
			{
			case RTN:
				sp = bp - 1;
				isBorder[bp] = 0;
				bp = pas[sp + 2];
				pc = pas[sp + 3];
				curLevel--;
				break;

			case NEG:
				pas[sp] = -1 * pas[sp];
				break;

			case ADD:
				sp = sp - 1;
				pas[sp] = pas[sp] + pas[sp + 1];
				break;

			case SUB:
				sp = sp - 1;
				pas[sp] = pas[sp] - pas[sp + 1];
				break;

			case MUL:
				sp = sp - 1;
				pas[sp] = pas[sp] * pas[sp + 1];
				break;

			case DIV:
				sp = sp - 1;
				pas[sp] = pas[sp] / pas[sp + 1];
				break;

			case ODD:
				pas[sp] = pas[sp] % 2;
				break;

			case MOD:
				sp = sp - 1;
				pas[sp] = pas[sp] % pas[sp + 1];
				break;

			case EQL:
				sp = sp - 1;
				pas[sp] = pas[sp] == pas[sp + 1];
				break;

			case NEQ:
				sp = sp - 1;
				pas[sp] = pas[sp] != pas[sp + 1];
				break;

			case LSS:
				sp = sp - 1;
				pas[sp] = pas[sp] < pas[sp + 1];
				break;

			case LEQ:
				sp = sp - 1;
				pas[sp] = pas[sp] <= pas[sp + 1];
				break;

			case GTR:
				sp = sp - 1;
				pas[sp] = pas[sp] > pas[sp + 1];
				break;

			case GEQ:
				sp = sp - 1;
				pas[sp] = pas[sp] >= pas[sp + 1];
				break;

			default:
				printf("Defaulted in OPR.\n");
				break;

			} // End of OPR switch
			break;

		case LOD:
			sp = sp + 1;
			pas[sp] = pas[base(ir.l) + ir.m];
			break;

		case STO:
			pas[base(ir.l) + ir.m] = pas[sp];
			sp = sp - 1;
			break;

		case CAL:
			pas[sp + 1] = base(ir.l); // static link (SL)
			pas[sp + 2] = bp;		  // dynamic link (DL)
			pas[sp + 3] = pc;		  // return address (RA)
			bp = sp + 1;
			pc = ir.m;
			curLevel++;
			isBorder[sp + 1] = 1;
			break;

		case INC:
			sp = sp + ir.m;
			break;

		case JMP:
			pc = ir.m;
			break;

		case JPC:
			if (pas[sp] == 1)
				pc = ir.m;
			sp = sp - 1;
			break;

		case SYS:
			switch (ir.m)
			{
			case 1:
				printf("Output result is: %d\n", pas[sp]);
				sp = sp - 1;
				break;

			case 2:
				sp = sp + 1;
				printf("Please Enter an Integer: ");
				scanf("%d", &pas[sp]);
				printf("\n");
				break;

			case 3:
				halt = 0;
				break;

			default:
				printf("Defaulted in SYS.\n");
				break;

			} // End of SYS switch
			break;

		default:
			printf("Defaulted in instructions.\n");
			break;

		} // End of instructions switch

		char *act = (ir.op == OPR) ? oprName[ir.m] : opName[ir.op];
		
		/* Print to the output table */
		printf("%2d %s %-4d%-5d%-5d%-5d%-5d",
			   lineNumber, act, ir.l, ir.m, pc, bp, sp);

		for (int i = initialBase; i <= sp; i++)
		{
			if (isBorder[i])
				printf("%s", "|");
			printf("%-2d ", pas[i]);
		}

		puts("");
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
     - c) Student names should be written in the header comment of each source code file, in the readme, and in the comments of the submission
*/
