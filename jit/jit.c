#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>

void help() {
	printf("The function returns the subtraction of two integers, a and b (-128 <= b <= 127).\n");
}
int main(int argc, char ** argv) {
	if (argc != 3) {
		printf ("Error: wrong number of arguments, should be 2\n");
		help();
		exit(EXIT_FAILURE);
	}

	char* trash = '\0';
	errno = 0;
	int a = strtol(argv[1], &trash, 10);
	if ((errno == ERANGE && (a == LONG_MAX || a == LONG_MIN)) || (errno != 0 && a == 0) || *trash != '\0') {
		printf ("Error: first argument is not a valid number\n");
		help();
		exit(EXIT_FAILURE);
	}

	errno = 0;
	trash = '\0';
	int b = strtol(argv[2], &trash, 10);
	if ((errno == ERANGE && (b == LONG_MAX || b == LONG_MIN)) || (errno != 0 && b == 0) || *trash != '\0' || b > 127 || b < -128) {
		printf ("Error: second argument is not a valid number\n");
		help();
		exit(EXIT_FAILURE);
	}

	char code[] = {0x55, 			 // returns a - 1
				   0x48, 0x89, 0xe5,
				   0x89, 0x7d, 0xfc,
				   0x8b, 0x45, 0xfc,
				   0x83, 0xe8, 0x01, // <- replace 0x01 with needed number
				   0x5d,
				   0xc3};
	code[12] = b;
	void * func = mmap(NULL, sizeof(code), PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(func == MAP_FAILED) {
		fprintf(stderr, "An error occured: %s\n", strerror(errno));
	}
	memcpy(func, code, sizeof(code));


	int res = ((int(*)(int))func)(a);
	
	if(munmap(func, sizeof(code)) == -1) {
		printf("An error occured: %s\n", strerror(errno));
		help();
	}
	printf("The subtraction of %d and %d is %d\n", a, b, res);
	
	exit(EXIT_SUCCESS);
}