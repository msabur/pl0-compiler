/* COP 3402 - Systems Software
 * Summer 2021
 * Homework #1 (P-Machine)
 * Authors: Maahee, Grant Allan
 * Due: 6/4/2021
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_pas_LENGTH 500
#define TRUE 0
#define FALSE 1

typedef struct instruction
{
	// Operation Code
	int op;

	// Lexicographical level
	int l;

	// For LIT and INC: A number
	// For JMP, JPC, and CAL: A program address
	// For LOD and STO: A data address
	// For 2: The identity of OPR
	int m;

} instruction;

/* Registers */
// Stack Pointer, Base Pointer, Program Counter
int sp, bp, pc;

// Instruction Register
instruction ir;

/* Memory */
// Process Address Space array
int pas[MAX_pas_LENGTH]; // global; automatically initialized to zeros

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

	// Helper arrays for printing names of actions
	const char *instruction[] = {
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

	const char *operator[] = {
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

		int address = pc - 3;

		// Use switch for the instructions
		switch (ir.op)
		{
		case 1: // LIT
			sp = sp + 1;
			pas[sp] = ir.m;
			break;

		case 2: // OPR
			switch (ir.m)
			{
			case 0: // RTN
				sp = bp - 1;
				isBorder[bp] = 0;
				bp = pas[sp + 2];
				pc = pas[sp + 3];
				break;

			case 1: // NEG
				pas[sp] = -1 * pas[sp];
				break;

			case 2: // ADD
				sp = sp - 1;
				pas[sp] = pas[sp] + pas[sp + 1];
				break;

			case 3: // SUB
				sp = sp - 1;
				pas[sp] = pas[sp] - pas[sp + 1];
				break;

			case 4: // MUL
				sp = sp - 1;
				pas[sp] = pas[sp] * pas[sp + 1];
				break;

			case 5: // DIV
				sp = sp - 1;
				pas[sp] = pas[sp] / pas[sp + 1];
				break;

			case 6: // ODD
				pas[sp] = (1 & pas[sp]) ? TRUE : FALSE;
				break;

			case 7: // MOD
				sp = sp - 1;
				pas[sp] = pas[sp] % pas[sp + 1];
				break;

			case 8: // EQL
				sp = sp - 1;
				pas[sp] = (pas[sp] == pas[sp + 1]) ? TRUE : FALSE;
				break;

			case 9: // NEQ
				sp = sp - 1;
				pas[sp] = (pas[sp] != pas[sp + 1]) ? TRUE : FALSE;
				break;

			case 10: // LSS
				sp = sp - 1;
				pas[sp] = (pas[sp] < pas[sp + 1]) ? TRUE : FALSE;
				break;

			case 11: // LEQ
				sp = sp - 1;
				pas[sp] = (pas[sp] <= pas[sp + 1]) ? TRUE : FALSE;
				break;

			case 12: // GTR
				sp = sp - 1;
				pas[sp] = (pas[sp] > pas[sp + 1]) ? TRUE : FALSE;
				break;

			case 13: // GEQ
				sp = sp - 1;
				pas[sp] = (pas[sp] >= pas[sp + 1]) ? TRUE : FALSE;
				break;

			default:
				printf("Defaulted in OPR: m=%d\n", ir.m);
				break;

			} // End of OPR switch
			break;

		case 3: // LOD
			sp = sp + 1;
			pas[sp] = pas[base(ir.l) + ir.m];
			break;

		case 4: // STO
			pas[base(ir.l) + ir.m] = pas[sp];
			sp = sp - 1;
			break;

		case 5: // CAL
			pas[sp + 1] = base(ir.l); // static link (SL)
			pas[sp + 2] = bp;         // dynamic link (DL)
			pas[sp + 3] = pc;         // return address (RA)
			bp = sp + 1;
			pc = ir.m;
			isBorder[sp + 1] = 1;
			break;

		case 6: // INC
			sp = sp + ir.m;
			break;

		case 7: // JMP
			pc = ir.m;
			break;

		case 8: // JPC
			if (pas[sp] == 1)
				pc = ir.m;
			sp = sp - 1;
			break;

		case 9: // SYS
			switch (ir.m)
			{
			case 1: // (write)
				printf("Output result is: %d\n", pas[sp]);
				sp = sp - 1;
				break;

			case 2: // (read)
				sp = sp + 1;
				printf("Please Enter an Integer: ");
				scanf("%d", &pas[sp]);
				printf("\n");
				break;

			case 3: // (halt)
				halt = 0;
				break;

			default:
				printf("Defaulted in SYS: m=%d.\n", ir.m);
				break;

			} // End of SYS switch
			break;

		default:
			printf("Defaulted in instructions: op=%d\n", ir.op);
			break;

		} // End of instructions switch

		// Get the name of the action
		const char *act = (ir.op == 2) ? operator[ir.m] : instruction[ir.op];

		/* Print to the output table */
		printf("%2d %s %-4d%-5d%-5d%-5d%-5d",
				address, act, ir.l, ir.m, pc, bp, sp);

		for (int i = initialBase; i <= sp; i++)
		{
			if (isBorder[i])
				printf("%s ", "|");
			printf("%d ", pas[i]);
		}

		puts("");
	}
}

/*
 * c) Student names should be written in the header comment of each source
 * code file, in the readme, and in the comments of the submission
 */
