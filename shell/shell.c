#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define DELIMS " \t\n\a\r"

char ** split(char * s) {
	int buf = 8;
	int pos = 0;
	char ** tokens = (char **)malloc(buf * sizeof(char *));
	char * token;

	token = strtok(s, DELIMS);

	while(token) { 
		tokens[pos] = token;
		pos++;
		if(pos >= buf) {
			buf += 8;
			tokens = (char **)realloc(tokens, buf * sizeof(char *));
			if(!tokens) {
				fprintf(stderr, "Error occured while reallocing the memory for tokens: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, DELIMS);
	}

	tokens[pos] = NULL;
	return tokens;
}

int execute(char ** args, char * prog) {
	extern char ** environ;
	pid_t pid;
	int status;
	if(args[0] == NULL) {
		return 1;
	}
	pid = fork();
	if(pid == 0) {
		if(execve(args[0], args, environ) == -1) {
			fprintf(stderr, "Error occured while running the program %s: %s\n", prog, strerror(errno));
		}
		free(args);
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		fprintf(stderr, "Failed to fork at program %s\n", prog);
	} else {
		do {
			if(wait(&status) == -1) {
				fprintf(stderr, "Error occured while waiting in program %s: %s\n", prog, strerror(errno));
			}
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	printf("Exit code for program %s: %d\n", prog, status); 
	return 1;
}


void help() {
	printf ("This program takes a path to a file as the only argument and runs all the programs listed in it.\n");
}


int main(int argc, char ** argv) {
	if (argc != 2) {
		printf ("Error: wrong number of arguments, should be 1\n");
		help();
		exit(EXIT_FAILURE);
	}
	FILE *f;
	if ((f = fopen (argv[1], "r")) == NULL) {
		fprintf (stderr, "Error: cannot find the file %s\n", argv[1]);
		return 1;
	}
	size_t buffer_s = 0;
	char * inp;
	char ** tokens;
	while (getline(&inp, &buffer_s, f) != EOF) {
		tokens = split(inp);
		if(tokens[0] != NULL) {
			execute(tokens, inp);
		}
		free(tokens);
	}
	fclose(f);
	return 0;
}