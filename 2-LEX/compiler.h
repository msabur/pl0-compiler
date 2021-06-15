// This is the header file for the UCF Summer 2021 Systems Software Project
// Alter at your own risk.


typedef enum {
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
	int type;
} lexeme;

lexeme *lexanalyzer(char *input);
