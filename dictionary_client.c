#include <stdio.h>
#include <stdlib.h>

#include "ipc_socket.h"

int main() {
	int client_fd;
	char buffer[10] = {0};

	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}
	send(client_fd, "--search", 8, 0);
	send(client_fd, "hello", 5, 0);
	read(client_fd, buffer, 10);
	printf("Client rec : %s\n", buffer);
	close(client_fd);

	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}
	send(client_fd, "--insert", 8, 0);
	send(client_fd, "hello", 5, 0);
	read(client_fd, buffer, 10);
	printf("Client rec : %s\n", buffer);
	close(client_fd);

	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}
	send(client_fd, "--search", 8, 0);
	send(client_fd, "hello", 5, 0);
	read(client_fd, buffer, 10);
	printf("Client rec : %s\n", buffer);
	close(client_fd);

	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}
	send(client_fd, "--delete", 8, 0);
	send(client_fd, "hello", 5, 0);
	read(client_fd, buffer, 10);
	printf("Client rec : %s\n", buffer);
	close(client_fd);
	
	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}
	send(client_fd, "--search", 8, 0);
	send(client_fd, "hello", 5, 0);
	read(client_fd, buffer, 10);
	printf("Client rec : %s\n", buffer);
	close(client_fd);

	return 0;
}
