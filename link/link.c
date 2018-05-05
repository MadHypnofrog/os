#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* static_lib(int a, int b, char* str);
char* dynamic_lib(int a, int b, char* str);

char *get_folder(const char *path) {
	int j = -1;
	for (u_int i = 0; i < strlen(path); ++i) {
		if (path[i] == '/') {
			j = i;
		}
	} 
	if (j == -1) {
		return (char*)path;
	}
	char *temp = (char*)malloc((j + 2) * sizeof(char));
	for (u_int i = 0; i < j + 1; ++i) {
		temp[i] = path[i];
	}
	temp[j + 1] = '\0';
	return temp;
}

int main(int argc, char ** argv) {
	char str1[1024];
	printf("%s\n", static_lib(2, 5, str1));
	printf("%s\n", dynamic_lib(6, 4, str1));
	
	void *dynlib;
	char * (*func)(int, int, char*);
	char path[1024];
	char* folder = get_folder(argv[0]);
	snprintf(path, sizeof(path), "%sdynamic_linked_lib.so", folder);
	free(folder);
	dynlib = dlopen(path, RTLD_LAZY);
	if (!dynlib){
		fprintf(stderr,"Error: can't open dynamic_linked_lib.so: %s\n", dlerror());
		return 0;
	};
	func = dlsym(dynlib, "dynamic_linked_lib");
	printf ("%s\n", (*func)(0, 9, str1));
	dlclose(dynlib);
	return 0;
}