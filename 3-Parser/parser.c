#include <stdio.h>
#include <setjmp.h>

/* Set up error management */
int error_number;
jmp_buf env;
#define catch() setjmp(env)
#define throw(error) longjmp(env, error_number = error)

int main() {
	if(catch() == 0) {
		// code that can throw an error
		throw(10);
	} else {
		// handle error
		printf("Error %d\n", error_number);
	}
}
