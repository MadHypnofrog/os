#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/epoll.h>
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
	if ((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
		fprintf (stderr, "Can't create a sending socket: %s\n", strerror(errno));
		return 1;
	}
	int epoll_fd = epoll_create(5);
	if (epoll_fd == -1) {
		fprintf (stderr, "Can't create an epoll instance: %s\n", strerror(errno));
		close(sock);
		return 1;
	}
	struct epoll_event events[1024];
	struct epoll_event event;
	event.data.fd = sock;
	event.events = EPOLLIN | EPOLLOUT;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1) {
		fprintf (stderr, "Can't add a socket to epoll: %s\n", strerror(errno));
		close(sock);
		close(epoll_fd);
		return 1;
	}
	bool connected = true;
	event.events = EPOLLIN;
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
		if (errno == EINPROGRESS) {
			errno = 0;
			connected = false;
		} else {
			fprintf (stderr, "Can't connect to server: %s\n", strerror(errno));
			close(sock);
			close(epoll_fd);
			return 1;
		}
	}
	char input[1024];
	printf ("Enter your message: ");
	scanf ("%s", input);
	char* sbuffer = input;
	bool waiting = true;
	while (waiting) {
		int ready = epoll_wait(epoll_fd, events, 1024, -1);
		if (ready == -1) {
			fprintf (stderr, "An error occured while waiting: %s\n", strerror(errno));
			close(sock);
			close(epoll_fd);
			return 1;
		}
		int cur = 0;
		for (cur = 0; cur < ready; cur++) {
			if ((events[cur].events & EPOLLOUT) != 0) {
				if (!connected) {
					int err = 0;
					socklen_t len = sizeof(int);
					if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) != -1 && err == 0) {
						connected = true;
					} else {
						fprintf (stderr, "Can't connect to the server: %s\n", strerror(errno));
						close(sock);
						close(epoll_fd);
						return 1;
					}
				}
				int sent = 0, length_s = 0, needed = strlen(input) + 1;
				while (needed > 0 && (sent = send(sock, sbuffer, needed, 0)) > 0) {
					sbuffer += sent;
					length_s += sent;
					needed -= sent;
				}
				if (length_s == 0) {
					fprintf (stderr, "Error: no data sent: %s\n", strerror(errno));
					close(sock);
					close(epoll_fd);
					return 1;
				}
				if (needed > 0) {
					fprintf (stderr, "Warning: not all data was sent to the server\n");
				}
				waiting = false;
			}
		}
	}
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock, &event) == -1) {
		fprintf (stderr, "Can't replace a socket in epoll: %s\n", strerror(errno));
		close(sock);
		close(epoll_fd);
		return 1;
	}
	int ready = epoll_wait(epoll_fd, events, 1024, -1);
	if (ready == -1) {
		fprintf (stderr, "An error occured while waiting: %s\n", strerror(errno));
		close(sock);
		close(epoll_fd);
		return 1;
	}
	int cur = 0; 
	for (cur = 0; cur < ready; cur++) {
		if ((events[cur].events & EPOLLIN) != 0) {
			int rec = 0, length = 0, maxlength = 1024, sock_r = events[cur].data.fd;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_r, NULL) == -1) {
				fprintf (stderr, "Can't delete a socket from epoll: %s\n", strerror(errno));
				close(sock_r);
				continue;
			}
			char buffer[1024];
			char* pbuffer = buffer;
			while ((rec = recv(sock_r, pbuffer, maxlength, 0)) > 0) {
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
				close(sock_r);
				close(epoll_fd);
				close(sock);
				return 1;
			}
			if (maxlength < 0) {
				fprintf (stderr, "Warning: too much data received, some data might be lost\n");
			}
			printf ("Received back: %s\n", buffer);
		}
	}
	close(sock);
	close(epoll_fd);
	return 0;
}