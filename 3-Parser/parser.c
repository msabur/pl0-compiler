#include <stdio.h>
#include <setjmp.h>

/* Set up error management */
#define catch() setjmp(error_buffer)
#define throw(error) longjmp(error_buffer, error)
jmp_buf error_buffer;

int main() {
	int error_number;
	if((error_number = catch()) == 0) {
		// code that can throw an error
		throw(10);
	} else {
		// handle error
		printf("Error %d\n", error_number);
	}
}
