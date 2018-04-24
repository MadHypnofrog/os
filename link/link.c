#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

char* static_lib(int a, int b, char* str);
char* dynamic_lib(int a, int b, char* str);

int main() {
	char str1[1024];
	printf("%s\n", static_lib(2, 5, str1));
	printf("%s\n", dynamic_lib(6, 4, str1));
	
	void *dynlib;
	char * (*func)(int, int, char*);
	dynlib = dlopen("dynamic_linked_lib.so", RTLD_LAZY);
	if (!dynlib){
		fprintf(stderr,"Error: can't open dynamic_linked_lib.so: %s\n", dlerror());
		return 0;
	};
	func = dlsym(dynlib, "dynamic_linked_lib");
	printf ("%s\n", (*func)(0, 9, str1));
	dlclose(dynlib);
	return 0;
}