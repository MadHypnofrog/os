#include <stdio.h>
char* static_lib(int a, int b, char* str) {
	sprintf(str, "I'm a function from a static library! I return the sum of %d and %d, it is %d", a, b, a+b);
	return str;
}