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
	//char* server_name = argv[0];
	int PORT = 1337;
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_port = htons(PORT);
	server.sin_family = AF_INET;
	inet_pton(AF_INET, server_name, &server.sin_addr);
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf (stderr, "Can't create a socket: %s\n", strerror(errno));
		return 1;
	}
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
		fprintf (stderr, "Can't connect to server: %s\n", strerror(errno));
		return 1;
	}
	char input[1024];
	scanf ("%s", input);
	send(sock, input, strlen(input) + 1, 0);
	int rec = 0, length = 0, maxlength = 1024;
	char buffer[1024];
	char* pbuffer = buffer;
	while ((rec = recv(sock, pbuffer, maxlength, 0)) > 0) {
		pbuffer += rec;
		length += rec;
		maxlength -= rec;
		printf ("Received: %s\n", buffer);
	}
	close(sock);
	return 0;
}