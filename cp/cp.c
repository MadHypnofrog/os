#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>

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
	char *temp = (char*)malloc((strlen(path) - j) * sizeof(char));
	for (u_int i = 0; i < strlen(path) - j - 1; ++i) {
		temp[i] = path[i + j + 1];
	}
	temp[strlen(path) - j - 1] = '\0';
	return temp;
}

int get_permissions(const char* path) {
	struct stat statbuf;
	if (stat(path, &statbuf) == -1) {
		return S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	} else {
		return statbuf.st_mode;
	}
}

int read_and_write(const char *from, const char *to) {
	int fd_from, fd_to;

	if ((fd_from = open(from, O_RDONLY)) == -1) {
		fprintf(stderr, "Failed to open file: \'%s\'\nError code: %d\n", from, errno);
		return 1;
	}

	if ((fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, get_permissions(from))) == -1) {
		fprintf(stderr, "Failed to open file: \'%s\'\nError code: %d\n", to, errno);
		if (close(fd_from) == -1) {
			fprintf(stderr, "Failed to close file: \'%s\'\nError code: %d\n", from, errno);
		}
		return 1;	
	}

	char buffer[4096];
	ssize_t buffered;
	for (; (buffered = read(fd_from, buffer, sizeof(buffer))) > 0;) {
		if (buffered == -1) {
			if (errno != EINTR) {
				fprintf(stderr, "Failed to write the full file \'%s\'\nError code: %d\n", to, errno);
				return 1;	
			} else {
				continue;
			}
		}
		char *to_write = buffer;
		ssize_t written;

		do {
			if ((written = write(fd_to, to_write, buffered)) >= 0) {
				buffered -= written;
				to_write += written;
			} else {
				fprintf(stderr, "Failed to write the full file \'%s\'\nError code: %d\n", to, errno);
				return 1;
			}
		} while (buffered > 0);
	}
	if (close(fd_from) == -1) {
		fprintf(stderr, "Failed to close file: \'%s\'\nError code: %d\n", from, errno);
		return 1;
	}
	if (close(fd_to) == -1) {
		fprintf(stderr, "Failed to close file: \'%s\'\nError code: %d\n", to, errno);
		return 1;
	}
	return 0;
}

int file_type(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) == -1) {
		return -1;
	}
	if (S_ISREG(statbuf.st_mode)) {
		return 0;
	}
	if (S_ISDIR(statbuf.st_mode)) {
		return 1;
	}
	return 0;
}


int copy_folder(const char *from, const char *to) {
	if (strcmp(from, to) == 0) {
		return 0;
	}
	DIR *directory;
	if ((directory = opendir(from)) != NULL) {
		struct dirent *dir;
		while ((dir = readdir(directory)) != NULL) {
			char new_from[1024];				
			char new_to[1024];
			if (dir->d_type == DT_DIR) {
				snprintf(new_from, sizeof(new_from), "%s%s/", from, dir->d_name);
				snprintf(new_to, sizeof(new_to), "%s%s/", to, dir->d_name);
				if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
					errno = 0;
					if (mkdir(new_to, get_permissions(from)) == -1 && errno != EEXIST) {
						fprintf(stderr, "Failed to create a directory: \'%s\'\nError code: %d\n", new_to, errno);
						if (closedir(directory) == -1) {
							fprintf(stderr, "Failed to close directory: \'%s\'\nError code: %d\n", from, errno);
						}
						return 1;
					}
					copy_folder(new_from, new_to);
				}
			} else {
				snprintf(new_from, sizeof(new_from), "%s%s", from, dir->d_name);
				snprintf(new_to, sizeof(new_to), "%s%s", to, dir->d_name);
				read_and_write(new_from, new_to);
			}
		}
		if (closedir(directory) == -1) {
			fprintf(stderr, "Failed to close directory: \'%s\'\nError code: %d\n", from, errno);
		}
	} else {
		fprintf(stderr, "Failed to open directory: \'%s\'\nError code: %d\n", from, errno);
		return 1;
	}
	return 0;
}

char* add_slash (char* str) {
	char* folder = get_folder(str);
	if (strcmp(folder, "") == 0) {
		free(folder);
		return str;
	}
	char* result = (char*)malloc(1024 * sizeof(char));
	snprintf(result, 1024, "%s/", str);
	free(folder);
	return result;
}


int cp(const char *from, const char *to) {
	char canon_from[1024], canon_to[1024];
	if (realpath(from, canon_from) == NULL) {
		fprintf(stderr, "An error has occured: \'%s\' is not a valid path\n", canon_from);
		return 1;
	}
	if (realpath(to, canon_to) == NULL) {
		fprintf(stderr, "An error has occured: \'%s\' is not a valid path\n", canon_to);
		return 1;
	}
	int type = file_type(from);
	int to_type = file_type(to);
	if (to_type == -1) {
		fprintf(stderr, "Failed to access the file: %s\nError code: %d\n", canon_to, errno);
		return 1;
	}
	if (type == 0) {
		if (to_type == 0) {
			read_and_write(from, to);
		} else if (to_type == 1) {
			char new_to[1024];
			char* folder = get_folder(from);
			snprintf(new_to, sizeof(new_to), "%s/%s", to, folder);
			free(folder);
			read_and_write(from, new_to);
		}
	} else if (type == 1) {
		if (to_type == 0) {
			fprintf(stderr, "An error has occured: can't copy directory \'%s\' to file \'%s\'\n", from, to);
			return 1;
		}
		char* new_from = add_slash(canon_from);
		char* new_to = add_slash(canon_to);
		if (strlen(new_from) <= strlen(new_to)) {
			char prefix_to[1024];
			memcpy(prefix_to, new_to, sizeof(char) * strlen(new_from));
			if (strcmp(new_from, prefix_to) == 0) {
				fprintf(stderr, "An error has occured: \'%s\' is a parent of \'%s\'\n", new_from, new_to);
				free(new_from);
				free(new_to);
				return 1;
			}
		}
		char* folder_from = get_folder(from);
		if (strcmp(folder_from, "") != 0) {
			free(folder_from);
			char* folder = get_folder(canon_from);
			char added_to[1024];
			snprintf(added_to, sizeof(added_to), "%s%s/", new_to, folder);
			free(folder);
			errno = 0;
			if (mkdir(added_to, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1 && errno != EEXIST) {
				fprintf(stderr, "Failed to create a directory: \'%s\'\nError code: %d\n", new_to, errno);
				free(new_from);
				free(new_to);
				return 1;
			}
			copy_folder(new_from, added_to);
		} else {
			free(folder_from);
			copy_folder(new_from, new_to);
		}
		free(new_from);
		free(new_to);
	} else {
		fprintf(stderr, "Failed to access the file: %s\nError code: %d\n", from, errno);
		return 1;
	}
	return 0;
}
	

void help() {
	printf("Usage of \'cp\' program:\n");
	printf("./cp [name1] [name2]\n");
	printf("where\n");
	printf("\t-name1 is a file or a folder you want to copy from;\n");
	printf("\t-name2 is a file or a folder you want to copy to;\n");
	printf("You can copy a file to a file, a file to a directory and a directory to a directory.\n");
	printf("If a trailing slash is added to name1, it copies the contents of this directory. For example, those are the same:\n");
	printf ("\t./cp /src/foo /dest\n");
	printf ("\t./cp /src/foo/ /dest/foo\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		help();
	}
	cp(argv[1], argv[2]);
	return EXIT_SUCCESS;
}