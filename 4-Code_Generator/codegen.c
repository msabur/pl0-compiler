/*
 * COP 3402 - Systems Software
 * Summer 2021
 * Homework #4 (PL/0 Compiler)
 * Authors: Maahee, Grant Allan
 * Due: 6/25/2021
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"

/* Constants */
// Symbol types
const int constkind = 1, varkind = 2, prockind = 3;
// Options for symbol table lookups
const int op_const = 0x2, op_var = 0x4, op_proc = 0x8, op_sameScope = 0x10;
instruction *code;

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
enum Syscodes
{
	WRT = 1,
	RED,
	HAL
};

int code_index, sym_index, level;

symbol *table;
lexeme token, *list;

void printcode();

/* Utilities */
void printtable();
void printErrorMessage(int x);
void getToken();
void addSymbol(char *name, int val, int kind, int address);
symbol *fetchSymbol(char *name, int options);
void markSymbolsInScope();
bool conflictingSymbol(char *name, int kind);
void emit(int opr, int l, int m);

/* Parsing Functions */
void program();
void block();
void const_declaration();
int var_declaration();
void procedure_declaration();
void statement();
void condition();
void expression();
void term();
void factor();

instruction *generate_code(lexeme *tokens, symbol *symbols)
{
	code = malloc(500 * sizeof(instruction));
	code_index = 0;

	table = malloc(1000 * sizeof(*table));
	list = tokens;

	program();

	printcode();
	return code;
}

/* Start the program */
void program()
{
	// Load main into the symbol table
	addSymbol("main", 0, prockind, 0);

	// Get the first token of the program
	getToken();

	// This will be the main block. That is, the entire program.
	block();

	emit(SYS, 0, HAL);
}

/* Start other functions */
void block()
{
	int jmpIndex = code_index, space = 3;
	emit(JMP, 0, 0);

	if (token.type == constsym)
		const_declaration();

	if (token.type == varsym)
		space += var_declaration();

	while (token.type == procsym)
		procedure_declaration();

	code[jmpIndex].m = code_index * 3;
	emit(INC, 0, space);

	statement();
}

/* Process a constant */
void const_declaration()
{
	// Will hold the name and value of the constant
	lexeme ident;

	// This token should have the constant name in it
	getToken();

	// Save the constant's information
	ident = token;

	// This token should have the := in it
	getToken();

	// This token should have the value of the constant in it
	getToken();

	// Save the constant's value
	ident.value = token.value;

	// Add the constant to the symbol table
	addSymbol(ident.name, ident.value, constkind, 0);

	// Get the next token
	getToken();

	// If we see a comma, we have to parse another declaration
	if (token.type == commasym)
		const_declaration();
	// Otherwise we need semicolon to mark the end of the declaration
	else
	{
		// Get the next token
		getToken();
	}
}

/* Process a variable */
int var_declaration()
{
	int numVars = 0;
	do
	{
		// Will hold the name and value of the variable
		lexeme ident;

		// This token should hold the name of the variable
		getToken();

		// Save the variable's information
		ident = token;

		// Add the variable to the symbol table
		addSymbol(ident.name, 0, varkind, numVars + 3);
		numVars++;

		// Get the next token to check it
		getToken();

		// If we see a comma, we have to parse another declaration
	} while (token.type == commasym);
	getToken();

	return numVars;
}

/* Process a procedure */
void procedure_declaration()
{
	// Will hold the name and value of the procedure
	lexeme ident;

	// This token should hold the name of the procedure
	getToken();

	// Save the procedure's information
	ident = token;

	// Add the procedure to the symbol table
	addSymbol(ident.name, 0, prockind, code_index);

	// This token should have a semicolon in it
	getToken();

	// Grab the next token so the block can process properly
	getToken();

	// Increase the level for everything inside the procedure.
	level++;

	// Parse the body of the procedure
	block();

	// Make sure the procedure's local variables can't be used outside
	markSymbolsInScope();

	// Decrease level now that we're outside the procedure
	level--;

	// Grab the next token for processing
	getToken();

	emit(OPR, 0, RTN);
}

/* Process a statement */
void statement()
{
	symbol *sym;
	int jmpIndex, jpcIndex;
	switch (token.type)
	{
	case identsym:
		sym = fetchSymbol(token.name, op_var);

		// This should hold :=
		getToken();

		// This should hold the first item in the expression
		getToken();

		// Process the expression
		expression();

		// The instruction to do the assignment
		emit(STO, level - sym->level, sym->addr);
		break;

	case callsym:
		// This should hold the name of a procedure
		getToken();

		// Make sure call is followed by an identifier
		expect(identsym, 14);

		sym = fetchSymbol(token.name, op_proc);

		// Move onto the next token to be processed
		getToken();

		emit(CAL, level - sym->level, sym->addr * 3);
		break;

	case beginsym:
		do
		{
			// Get the first token of the statement; should be
			// the variable whose value is being changed.
			getToken();

			// Process the statement
			statement();

			// Process all statements in the procedure
		} while (token.type == semicolonsym);

		// Move onto the next token to be processed
		getToken();
		break;

	case ifsym:
		// Get the first token of the condition to be processed
		getToken();

		// Process the condition
		condition();

		jpcIndex = code_index;
		emit(JPC, 0, 0);

		// Get the first symbol of the statement inside the if
		getToken();

		// Process the statement
		statement();

		// If there's an else...
		if (token.type == elsesym)
		{
			// Get the first token of the statement
			getToken();

			jmpIndex = code_index;
			emit(JMP, 0, 0);
			code[jpcIndex].m = code_index * 3;

			// Process the statement
			statement();

			// The "else" block is skipped if the condition passes
			code[jmpIndex].m = code_index * 3;
		}
		// If there is no "else", it jumps to the end on fail condition
		else
		{
			code[jpcIndex].m = code_index * 3;
		}
		break;

	case whilesym:
		// Get the first token of the condition
		getToken();

		jmpIndex = code_index;

		// Process the condition
		condition();

		// Get the first symbol of the statement inside the while
		getToken();

		jpcIndex = code_index;
		emit(JPC, 0, 0);

		// Process the statement
		statement();

		// We recheck the condition after the loop
		// We skip the loop if the condition is false
		emit(JMP, 0, jmpIndex * 3);
		code[jpcIndex].m = code_index * 3;
		break;

	case readsym:
		// This should have an identity in it
		getToken();

		sym = fetchSymbol(token.name, op_var);

		// First we get the new value on top of the stack. Then we store it.
		emit(SYS, 0, RED);
		emit(STO, level - sym->level, sym->addr);

		// Get the next token to be processed
		getToken();
		break;

	case writesym:
		// This should have an identity in it
		getToken();

		sym = fetchSymbol(token.name, op_const | op_var);

		// First we load the value on top of the stack. Then we write it.
		if (sym->kind == varkind)
			emit(LOD, level - sym->level, sym->addr);
		else
			emit(LIT, 0, sym->val);
		
		emit(SYS, 0, WRT);

		// Get the next token to be processed
		getToken();
		break;

	default:
		break;
	}
}

/* Process a formula */
void condition()
{
	if (token.type == oddsym)
	{
		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();

		emit(OPR, 0, ODD);
	}
	else
	{
		// Process the expression
		expression();

		int rel = token.type;

		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();
		// Process the expression
		expression();
		switch (rel)
		{
		case eqlsym:
			emit(OPR, 0, EQL);
			break;
		case neqsym:
			emit(OPR, 0, NEQ);
			break;
		case lessym:
			emit(OPR, 0, LSS);
			break;
		case leqsym:
			emit(OPR, 0, LEQ);
			break;
		case gtrsym:
			emit(OPR, 0, GTR);
			break;
		case geqsym:
			emit(OPR, 0, GEQ);
			break;
		}
	}
}

/* Process Plus and/or Minus */
void expression()
{
	// If it's a plus or minus, move to the next token
	if (token.type == plussym || token.type == minussym)
		getToken();

	// Process the term
	term();

	// As long as there's another plus or minus, keep processing
	// the expression
	while (token.type == plussym || token.type == minussym)
	{
		int operator= token.type;
		getToken();
		term();
		if (operator== plussym)
			emit(OPR, 0, ADD);
		else if (operator== minussym)
			emit(OPR, 0, SUB);
	}
}

/* Process multiplication, division, and/or modulus */
void term()
{
	// Process the factor
	factor();

	// As long as the current token is * / or %, keep processing
	while (token.type == multsym || token.type == slashsym || token.type == modsym)
	{
		int operator= token.type;
		getToken();
		factor();
		if (operator== multsym)
			emit(OPR, 0, MUL);
		else if (operator== slashsym)
			emit(OPR, 0, DIV);
		else if (operator== modsym)
			emit(OPR, 0, MOD);
	}
}

/* Process the factor */
void factor()
{
	if (token.type == identsym)
	{
		symbol *sym = fetchSymbol(token.name, op_const | op_var);

		// instruction generation
		if (sym->kind == constkind)
			emit(LIT, 0, sym->val);
		else if (sym->kind == varkind)
			emit(LOD, level - sym->level, sym->addr);

		// Get the next token to be processed
		getToken();
	}
	else if (token.type == numbersym)
	{
		// instruction generation
		emit(LIT, 0, token.value);
		// Get the next token to be processed
		getToken();
	}
	else if (token.type == lparentsym)
	{
		// Get the first token of the expression
		getToken();

		// Process the expression
		expression();

		// Get the next token to be processed
		getToken();
	}
}

/* Fetches the next token and assigns it to the global variable token */
void getToken()
{
	static int tokenIndex = 0;
	token = list[tokenIndex++];
}

/* Adds a symbol to the symbol table */
void addSymbol(char *name, int val, int kind, int address)
{
	table[sym_index] = (symbol){kind, "", val, level, address};
	strcpy(table[sym_index].name, name);

	sym_index++;
}

/* Returns the requested symbol, or NULL if it's not found.
 * The options parameter specifies the kind of symbol to search for,
 * and whether to only search in the current scope.
 * Options can be combined with '|'
 * Possible options: op_const, op_proc, op_var, op_sameScope
 */
symbol *fetchSymbol(char *name, int options)
{
	for (int i = sym_index - 1; i >= 0; i--)
	{
		// If searching in the current scope only, stop when we leave it
		if ((options & op_sameScope) && table[i].level != level)
			break;

		// Ignore marked symbols
		if (table[i].mark)
			continue;

		// Ignore symbols whose names don't match what we want
		if (strcmp(table[i].name, name) != 0)
			continue;

		// If the current symbol is of a requested kind, return it
		if (options & (1 << table[i].kind))
			return &table[i];
	}
	return NULL;
}

/* Marks symbols in the current scope before we exit it */
void markSymbolsInScope()
{
	for (int i = sym_index - 1; i != -1; i--)
	{
		// If it's done marking the current scope, break
		if (table[i].level != level)
			break;

		// Mark the current symbol
		table[i].mark = 1;
	}
}

void emit(int opr, int l, int m)
{
	code[code_index++] = (instruction){opr, l, m};
}

void printcode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < code_index; i++)
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
	}
}
