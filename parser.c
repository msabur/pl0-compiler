/*
Authors: Grant Allan, Maahee Abdus Sabur
Remaining emits: none
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "compiler.h"

// Toggleable workaround for our VM where each instruction takes 3 spaces
#define MULTIPLY_JUMP_INDEXES_BY_3

/* Constants */
// Symbol types
const int constkind = 1, varkind = 2, prockind = 3;
// Options for symbol table lookups
const int op_const = 0x2, op_var = 0x4, op_proc = 0x8, op_sameScope = 0x10;

/* Error management */
int error;
jmp_buf env;
#define catch() setjmp(env)
#define throw(value) longjmp(env, error = value)

enum Opcodes {LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SYS};
enum Oprcodes {RTN, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS,
	LEQ, GTR, GEQ};
enum Syscodes {WRT = 1, RED, HAL};

int codeIndex, sym_index, level;

symbol *table;
lexeme token, *list;
instruction *code;

/* Utilities */
void printErrorMessage(int x);
void getToken();
void expect(token_type type, int err);
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

instruction *parse(lexeme *tokens)
{
	code = malloc(MAX_FILE_LEN * sizeof(instruction));
	codeIndex = 0;

	table = malloc(MAX_FILE_LEN * sizeof(*table));
	list = tokens;

	if (catch())
	{
		printErrorMessage(error);
		free(code);
		code = NULL;
	}
	else
	{
		program();
	}

	free(table);
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

	// The program must end with a period
	expect(periodsym, 3);

	emit(SYS, 0, HAL);
}

/* Start other functions */
void block()
{
	int jmpIndex = codeIndex, space = 3;
	emit(JMP, 0, 0);

	if (token.type == constsym)
		const_declaration();
	if (token.type == varsym)
		space += var_declaration();
	while (token.type == procsym)
		procedure_declaration();

#ifdef MULTIPLY_JUMP_INDEXES_BY_3
	code[jmpIndex].m = codeIndex * 3;
#else
	code[jmpIndex].m = codeIndex;
#endif
	emit(INC, 0, space);

	printf("%d\n", level);
	statement();
}

/* Process a constant */
void const_declaration()
{
	// Will hold the name and value of the constant
	lexeme ident;

	// This token should have the constant name in it
	getToken();

	// If there's no name, throw an error
	expect(identsym, 4);

	// Save the constant's information
	ident = token;

	// This token should have the := in it
	getToken();

	// Make sure the constant is assigned a value
	expect(becomessym, 5);

	// This token should have the value of the constant in it
	getToken();

	// Make sure the constant is assigned a value
	expect(numbersym, 5);

	// Save the constant's value
	ident.value = token.value;

	// Check for conflicting symbols
	if(fetchSymbol(ident.name, op_const | op_var | op_sameScope))
		throw(1);

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
		// Symbol Declarations Must Be Followed By a Semicolon
		expect(semicolonsym, 6);

		// Get the next token
		getToken();
	}
}

/* Process a variable */
int var_declaration()
{
	int numVars = 0;
	do {
		// Will hold the name and value of the variable
		lexeme ident;

		// This token should hold the name of the variable
		getToken();

		// Symbols Must Be Declared with an Identifier
		expect(identsym, 4);

		// Save the variable's information
		ident = token;

		// Check for conflicting symbols
		if(fetchSymbol(ident.name, op_const | op_var | op_sameScope))
			throw(1);

		// Add the variable to the symbol table	
		addSymbol(ident.name, 0, varkind, numVars + 3);
		numVars++;

		// Get the next token to check it
		getToken();

		// If we see a comma, we have to parse another declaration
	} while (token.type == commasym);

	// Otherwise we need semicolon to mark the end of the declaration
	expect(semicolonsym, 6);

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

	// Symbols Must Be Declared with an Identifier
	expect(identsym, 4);

	// Save the procedure's information
	ident = token;

	// Check for conflicting symbols
	if(fetchSymbol(ident.name, op_proc | op_sameScope))
		throw(1);

	// Add the procedure to the symbol table
	addSymbol(ident.name, 0, prockind, codeIndex);

	// This token should have a semicolon in it
	getToken();

	// Make sure the declaration is followed by ;
	expect(semicolonsym, 6);

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

	// Check to make sure the whole thing ends with a ";"
	expect(semicolonsym, 6);

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
		// If we don't find the variable, throw an error
		if (!sym)
			throw(7);

		// This should hold :=
		getToken();

		// Assignment expressions require the := symbol
		expect(becomessym, 2);

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
		// If we don't find the procedure, throw an error
		if (!sym)
			throw(7);

		// Move onto the next token to be processed
		getToken();
#ifdef MULTIPLY_JUMP_INDEXES_BY_3
		emit(CAL, level - sym->level, sym->addr * 3);
#else
		emit(CAL, level - sym->level, sym->addr);
#endif
		break;

	case beginsym:
		do
		{
			// Get the first token of the statement
			getToken();

			// Process the statement
			statement();

			// Process all statements in the procedure
		} while (token.type == semicolonsym);

		// Make sure the procedure ends with end
		// begin Must Be Followed By end
		expect(endsym, 10);

		// Move onto the next token to be processed
		getToken();
		break;

	case ifsym:
		// Get the first token of the condition to be processed
		getToken();

		// Process the condition
		condition();

		jpcIndex = codeIndex;
		emit(JPC, 0, 0);

		// Make sure the next symbol is then
		expect(thensym, 9);

		// Get the first symbol of the statement inside the if
		getToken();

		// Process the statement
		statement();

		// If there's an else...
		if (token.type == elsesym)
		{
			// Get the first token of the statement
			getToken();

			jmpIndex = codeIndex;
			emit(JMP, 0, 0);
#ifdef MULTIPLY_JUMP_INDEXES_BY_3
			code[jpcIndex].m = codeIndex * 3;
#else
			code[jpcIndex].m = codeIndex;
#endif

			// Process the statement
			statement();

			// The "else" block is skipped if the condition passes
#ifdef MULTIPLY_JUMP_INDEXES_BY_3
			code[jmpIndex].m = codeIndex * 3;
#else
			code[jmpIndex].m = codeIndex;
#endif
		}
		// If there is no "else", it jumps to the end on fail condition
		else
		{
#ifdef MULTIPLY_JUMP_INDEXES_BY_3
			code[jpcIndex].m = codeIndex * 3;
#else
			code[jpcIndex].m = codeIndex;
#endif
		}
		break;

	case whilesym:
		// Get the first token of the condition
		getToken();

		jmpIndex = codeIndex;

		// Process the condition
		condition();

		// Make sure the next symbol is do
		expect(dosym, 8);

		// Get the first symbol of the statement inside the while
		getToken();

		jpcIndex = codeIndex;
		emit(JPC, 0, 0);

		// Process the statement
		statement();

		// We recheck the condition after the loop
		// We skip the loop if the condition is false
#ifdef MULTIPLY_JUMP_INDEXES_BY_3
		emit(JMP, 0, jmpIndex * 3);
		code[jpcIndex].m = codeIndex * 3;
#else
		emit(JMP, 0, jmpIndex);
		code[jpcIndex].m = codeIndex;
#endif
		break;

	case readsym:
		// This should have an identity in it
		getToken();

		// Make sure read is followed by an identifier
		expect(identsym, 14);

		sym = fetchSymbol(token.name, op_var);
		// Make sure we're reading a variable
		if (!sym)
			throw(7);

		// First we get the new value on top of the stack. Then we store it.
		emit(SYS, 0, RED);
		emit(STO, level - sym->level, sym->addr);

		// Get the next token to be processed
		getToken();
		break;

	case writesym:
		// This should have an identity in it
		getToken();

		// Make sure write is followed by an identifier
		expect(identsym, 2);

		sym = fetchSymbol(token.name, op_const | op_var);
		// Make sure to only write constants or variables to the screen
		if (!sym)
			throw(7);

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

		// Here we throw an error if there is no relational operator.
		// The relational operators are from eqlsym to geqsym
		if (token.type < eqlsym || token.type > geqsym)
			throw(12);

		int rel = token.type;

		// Get the first token of the expression
		getToken();

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
	{
		if(token.type == minussym)
			emit(OPR, 0, NEG);
		getToken();
	}

	// Process the term
	term();

	// As long as there's another plus or minus, keep processing
	// the expression
	while (token.type == plussym || token.type == minussym)
	{
		int operator = token.type;
		getToken();
		term();
		if (operator == plussym)
			emit(OPR, 0, ADD);
		else if (operator == minussym)
			emit(OPR, 0, SUB);
	}
}

/* Process multiplication, division, and/or modulus */
void term()
{
	// Process the factor
	factor();

	// As long as the current token is * / or %, keep processing
	while (token.type == multsym || token.type == slashsym || token.type ==
			modsym)
	{
		int operator = token.type;
		getToken();
		factor();
		if (operator == multsym)
			emit(OPR, 0, MUL);
		else if (operator == slashsym)
			emit(OPR, 0, DIV);
		else if (operator == modsym)
			emit(OPR, 0, MOD);
	}
}

/* Process the factor */
void factor()
{
	if (token.type == identsym)
	{
		symbol *sym = fetchSymbol(token.name, op_const | op_var);

		// Only consts and vars are allowed in arithmetic expressions.
		// If it's not a constant or a variable, throw an error
		if (!sym)
			throw(7);

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

		// Make sure to close the parentheses
		expect(rparentsym, 13);

		// Get the next token to be processed
		getToken();
	}
	else
	{
		// We get here when an if/while statement is missing a condition
		throw(11);
	}
}

/* Throws an error if the current token isn't of the expected type */
void expect(token_type type, int err)
{
	if (token.type != type)
		throw(err);
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
	table[sym_index] = (symbol) {kind, "", val, level, address, 0};
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
		// If done marking the current scope, break
		if (table[i].level != level)
			break;

		// Mark the current symbol
		table[i].mark = 1;
	}
}

void emit(int opr, int l, int m)
{
	code[codeIndex++] = (instruction) {opr, l, m};
}

/* Print the error thrown */
void printErrorMessage(int x)
{
	const char *errors[] =
		{
			[1] = "Competing Symbol Declarations",
			[2] = "Unrecognized Statement Form",
			[3] = "Programs Must Close with a Period",
			[4] = "Symbols Must Be Declared with an Identifier",
			[5] = "Constants Must Be Assigned a Value at Declaration",
			[6] = "Symbol Declarations Must Be Followed By a Semicolon",
			[7] = "Undeclared Symbol",
			[8] = "while Must Be Followed By do",
			[9] = "if Must Be Followed By then",
			[10] = "begin Must Be Followed By end",
			[11] = "while and if Statements Must Contain Conditions",
			[12] = "Conditions Must Contain a Relational-Operator",
			[13] = "( Must Be Followed By )",
			[14] = "call and read Must Be Followed By an Identifier"};

	if (x < 1 || x > 14)
		printf("Implementation Error: Unrecognized Error Code\n");
	else
		printf("Parser Error: %s\n", errors[x]);
}
