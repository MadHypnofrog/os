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
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_port = htons(PORT);
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, server_name, &server.sin_addr) == -1) {
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
	char input[1024];
	printf ("Enter your message: ");
	scanf ("%s", input);
	char* sbuffer = input;
	int sent = 0, length_s = 0, needed = strlen(input) + 1;
	while (needed > 0 && (sent = send(sock, sbuffer, needed, 0)) > 0) {
		sbuffer += sent;
		length_s += sent;
		needed -= sent;
	}
	if (length_s == 0) {
		fprintf (stderr, "Error: no data sent: %s\n", strerror(errno));
		close(sock);
		return 1;
	}
	if (needed > 0) {
		fprintf (stderr, "Warning: not all data was sent to the server\n");
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
		if (errno != 0) {
			fprintf (stderr, "Error: no data received: %s\n", strerror(errno));
		} else {
			fprintf (stderr, "Error: server is offline\n");
		}
		close(sock);
		return 1;
	}
	if (maxlength < 0) {
		fprintf (stderr, "Warning: too much data received, some data might be lost\n");
	}
	printf ("Received back: %s\n", buffer);
	close(sock);
	return 0;
}