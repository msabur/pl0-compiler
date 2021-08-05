// This is the header file for the UCF Summer 2021 Systems Software Project
// Alter at your own risk.


typedef enum token_type {
	oddsym = 1, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, 
	modsym, multsym, slashsym, plussym, minussym,
	lparentsym, rparentsym, commasym, periodsym, semicolonsym, 
	becomessym, beginsym, endsym, ifsym, thensym, elsesym,
	whilesym, dosym, callsym, writesym, readsym, constsym, 
	varsym, procsym, identsym, numbersym,
} token_type;

typedef struct lexeme {
	char name[12];
	int value;
	token_type type;
} lexeme;

typedef struct symbol {
	int kind;
	char name[12];
	int val;
	int level;
	int addr;
	int mark;
} symbol;

lexeme *lexanalyzer(char *input);
symbol *parse(lexeme *input);
