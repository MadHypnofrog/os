#include <stdio.h>
char* dynamic_lib(int a, int b, char* str) {
	sprintf(str, "I'm a function from a dynamic library! I return the difference of %d and %d, it is %d", a, b, a-b);
	return str;
}