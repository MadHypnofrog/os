#include <stdio.h>
char* dynamic_linked_lib(int a, int b, char* str) {
	sprintf(str, "I'm a function from a dynamically linked library! I return the product of %d and %d, it is %d", a, b, a*b);
	return str;
}