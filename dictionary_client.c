#include <stdio.h>
#include <stdlib.h>

#include "ipc_socket.h"

// Maximum word size in dictionary
#define WORD_MAX 50

int main(int argc, char *argv[]) {
	int client_fd, word_len;
	char result[10] = {0};

	if (argc < 3) {
		printf("Invalid Arguments\n");
		printf("Usage : dictionary {--insert <word> | --search <word> | --delete <word>}\n");
		return -1;
	}

	word_len = strlen(argv[2]);

	if (word_len > WORD_MAX) {
		printf("Invalid word length. Maximum supported length is 50\n");
		return -1;
	}

	// Create client connection with dictionary service
	if ((client_fd = open_clientfd()) < 0) {
		printf("Unable to connect to server\n");
		return -1;
	}

	// Send aruguments over socket connection
	if (strcmp(argv[1], "--search") == 0 || strcmp(argv[1], "--insert") == 0 || strcmp(argv[1], "--delete") == 0) {
		send(client_fd, argv[1], 8, 0);
		send(client_fd, argv[2], word_len, 0);
		read(client_fd, result, 10);
		printf("Action[%s] for Word[%s] : %s\n", (argv[1] + 2), argv[2], result);

	}
	else {
		printf("Invalid Action\n");
		printf("Usage : dictionary {--insert <word> | --search <word> | --delete <word>}\n");
		close(client_fd);
		return -1;
	}

	close(client_fd);
	return 0;
}
