#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main (int argc, char** argv) {
	int PORT = 1337;
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_port = htons(PORT);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	//server.sin_addr.s_addr = htonl(atoi(argv[0]));
	int listen_socket;
	if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf (stderr, "Can't create a socket: %s\n", strerror(errno));
		return 1;
	}
	if (bind(listen_socket, (struct sockaddr*) &server, sizeof(server)) < 0) {
		fprintf (stderr, "Can't bind a socket: %s\n", strerror(errno));
		return 1;
	}
	if (listen(listen_socket, 100) < 0) {
		fprintf (stderr, "Can't create a listening socket: %s\n", strerror(errno));
		return 1;
	}
	while (true) {
		int socket;
		printf ("Waiting...\n");
		if ((socket = accept(listen_socket, NULL, NULL)) < 0) {
			fprintf (stderr, "Can't open a socket: %s\n", strerror(errno));
			continue;
		}
		int rec = 0, length = 0, maxlength = 1024;
		char buffer[1024];
		char* pbuffer = buffer;
		if ((rec = recv(socket, pbuffer, maxlength, 0)) > 0) {
			//printf ("Received %d characters\n", rec);
			pbuffer += rec;
			length += rec;
			maxlength -= rec;
			printf ("Received: %s\n", buffer);
			if (strcmp(buffer, "exit") == 0) {
				printf ("GG\n");
				close(socket);
				break;
			}
			send(socket, buffer, length, 0);
			//printf ("Sent\n");
		}
		//printf ("Not closed yet..\n");
		close(socket);
		//printf ("Closed!\n");
	}
	close(listen_socket);
	return 0;
}