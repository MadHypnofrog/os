#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
int main (int argc, char** argv) {
	int PORT = 1337;
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (argc > 1) {
		if (inet_aton(argv[1], &server.sin_addr) == 0) {
			fprintf(stderr, "Invalid IP-address: %s\n", strerror(errno));
			server.sin_addr.s_addr = htonl(INADDR_ANY);
		}
	}
	if (argc > 2) {
		PORT = atoi(argv[2]);
	}
	server.sin_port = htons(PORT);
	server.sin_family = AF_INET;
	int listen_socket;
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
		int sock;
		printf ("Waiting...\n");
		if ((sock = accept(listen_socket, NULL, NULL)) < 0) {
			fprintf (stderr, "Can't open a socket: %s\n", strerror(errno));
			continue;
		}
		int rec = 0, length = 0, maxlength = 1024;
		char buffer[1024];
		char* pbuffer = buffer;
		while ((rec = recv(sock, pbuffer, maxlength, 0)) > 0) {
			pbuffer += rec;
			length += rec;
			maxlength -= rec;
			printf ("Received: %s\n", buffer);
			if (strlen(buffer) != length) {
				break;
			}
		}
		if (length == 0) {
			fprintf (stderr, "Error: no data received: %s\n", strerror(errno));
			close(sock);
			continue;
		}
		if (maxlength < 0) {
			fprintf (stderr, "Warning: too much data received, some data might be lost\n");
		}
		if (strcmp(buffer, "exit") == 0) {
			printf ("Shutting down...\n");
			close(sock);
			break;
		}
		if (send(sock, buffer, length, 0) == -1) {
			fprintf (stderr, "Error while sending data to client: %s\n", strerror(errno));
		}
		close(sock);
	}
	close(listen_socket);
	return 0;
}