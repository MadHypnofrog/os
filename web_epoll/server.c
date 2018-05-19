#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/epoll.h>
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
	int epoll_fd = epoll_create(1024);
	if (epoll_fd == -1) {
		fprintf (stderr, "Can't create an epoll instance: %s\n", strerror(errno));
		close(listen_socket);
		return 1;
	}
	struct epoll_event events[1024];
	struct epoll_event event_listen;
	event_listen.data.fd = listen_socket;
	event_listen.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &event_listen) == -1) {
		fprintf (stderr, "Can't add a listening socket to epoll: %s\n", strerror(errno));
		close(listen_socket);
		close(epoll_fd);
		return 1;
	}
	bool running = true;
	while (running) {
		printf ("Waiting...\n");
		int ready = epoll_wait(epoll_fd, events, 1024, -1);
		if (ready == -1) {
			fprintf (stderr, "An error occured while waiting: %s\n", strerror(errno));
			continue;
		}
		int cur = 0;
		for (cur = 0; cur < ready; cur++) {
			int sock = events[cur].data.fd;
			if (sock == listen_socket) {
				int sock_r;
				if ((sock_r = accept(listen_socket, NULL, NULL)) < 0) {
					fprintf (stderr, "Can't open a socket: %s\n", strerror(errno));
					continue;
				}
				struct epoll_event event;
				event.data.fd = sock_r;
				event.events = EPOLLIN;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_r, &event) == -1) {
					fprintf (stderr, "Can't add a socket to epoll: %s\n", strerror(errno));
					close(sock_r);
					continue;
				}
				printf ("A client connected...\n");
			} else {
				int rec = 0, length = 0, maxlength = 1024;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL) == -1) {
					fprintf (stderr, "Can't delete a socket from epoll: %s\n", strerror(errno));
					close(sock);
					continue;
				}
				char buffer[1024];
				char* pbuffer = buffer;
				while ((rec = read(sock, pbuffer, maxlength)) > 0) {
					pbuffer += rec;
					length += rec;
					maxlength -= rec;
					if (strlen(buffer) != length) {
						break;
					}
				}
				if (length == 0) {
					if (errno != 0) {
						fprintf (stderr, "Error: no data received: %s\n", strerror(errno));
					} else {
						fprintf (stderr, "Error: the client disconnected whlie waiting\n");
					}
					close(sock);
					continue;
				}
				if (maxlength < 0) {
					fprintf (stderr, "Warning: too much data received, some data might be lost\n");
				}
				printf ("Received: %s\n", buffer);
				char* sbuffer = buffer;
				int sent = 0, length_s = 0, needed = length;
				while (needed > 0 && (sent = send(sock, sbuffer, needed, 0)) > 0) {
					sbuffer += sent;
					length_s += sent;
					needed -= sent;
				}
				if (length_s == 0) {
					fprintf (stderr, "Error: no data sent: %s\n", strerror(errno));
				}
				if (needed > 0) {
					fprintf (stderr, "Warning: not all data was sent to the client\n");
				}
				close(sock);
				if (strcmp(buffer, "exit") == 0) {
					printf ("Shutting down...\n");
					close(sock);
					running = false;
					break;
				}
			}
		}
	}
	shutdown(epoll_fd, SHUT_RDWR);
	close(epoll_fd);
	close(listen_socket);
	return 0;
}