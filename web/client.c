#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
int main (int argc, char** argv) {
	char* server_name = "localhost";
	int PORT = 1337;
	if (argc > 1) {
		server_name = argv[1];
	}
	if (argc > 2) {
		PORT = atoi(argv[2]);
	}
	char input[1024];
	printf ("Enter your message: ");
	scanf ("%s", input);
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_port = htons(PORT);
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, server_name, &server.sin_addr) != 1) {
		fprintf (stderr, "Can't convert network address: %s\n", strerror(errno));
		return 1;
	}
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf (stderr, "Can't create a sending socket: %s\n", strerror(errno));
		return 1;
	}
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
		fprintf (stderr, "Can't connect to server: %s\n", strerror(errno));
		return 1;
	}
	if (send(sock, input, strlen(input) + 1, 0) == -1) {
		fprintf (stderr, "Error while sending data to server: %s\n", strerror(errno));
		close(sock);
		return 1;
	}
	int rec = 0, length = 0, maxlength = 1024;
	char buffer[1024];
	char* pbuffer = buffer;
	while ((rec = recv(sock, pbuffer, maxlength, 0)) > 0) {
		pbuffer += rec;
		length += rec;
		maxlength -= rec;
	}
	if (length == 0) {
		fprintf (stderr, "Error: no data received: %s\n", strerror(errno));
	}
	if (length > maxlength) {
		fprintf (stderr, "Warning: too much data received, some data might be lost\n");
	}
	printf ("Received back: %s\n", buffer);
	close(sock);
	return 0;
}