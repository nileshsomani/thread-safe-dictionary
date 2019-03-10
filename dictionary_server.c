#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "dictionary_server.h"
#include "ipc_socket.h"

static volatile int keepRunning = 1;

static int insert(struct Node *trie, char *word) {
	char c, i;
	struct Node *node = NULL;
	struct Node *iter = trie;

	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			node = getNode();
			iter->children[i] = node;
			iter = iter->children[i];
		}
	}
	iter->is_end = 1;
	return SUCCESS;
}

static int search(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;

	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			return FAILURE;
		}
	}
	if (iter->is_end == 1)
		return SUCCESS;
	return FAILURE;
}

static int del(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;

	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			return SUCCESS;
		}
	}
	// Soft Delete
	iter->is_end = 0;
	return SUCCESS;
}

static struct Node *getNode() {
	int i;

	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	node->is_end = 0;

	for (i=0; i<NO_OF_CHARS; i++) {
		node->children[i] = NULL;
	}
	return node;
}

static void free_trie(struct Node *trie) {
	int i;

	if (trie == NULL)
		return;

	for (i=0; i<NO_OF_CHARS; i++) {
		free_trie(trie->children[i]);
	}
	free(trie);
}

static void *thread(void *arg) {
	int conn_fd = *(((struct Arguments *)arg)->conn_fd);
	struct Node *trie = ((struct Arguments *)arg)->trie;
	char action[10] = {0};
	char word[50] = {0};
	pthread_detach(pthread_self());
	free(((struct Arguments *)arg)->conn_fd);
	// Read and Write to socket
	read(conn_fd, action, 8);
	read(conn_fd, word, 50);
	//printf("Server rec[action] : %s\n", action);
	//printf("Server rec[word] : %s\n", word);
	if (strcmp(action, "--search") == 0) {
		if (search(trie, word) == SUCCESS)
			send(conn_fd, "SUCCESS", 7, 0);
		else
			send(conn_fd, "FAILURE", 7, 0);
	}
	else if (strcmp(action, "--insert") == 0) {
		if (insert(trie, word) == SUCCESS)
			send(conn_fd, "SUCCESS", 7, 0);
		else
			send(conn_fd, "FAILURE", 7, 0);
	}
	else if (strcmp(action, "--delete") == 0) {
		if (del(trie, word) == SUCCESS)
			send(conn_fd, "SUCCESS", 7, 0);
		else
			send(conn_fd, "FAILURE", 7, 0);
	}

	close(conn_fd);
	return NULL;
}

static void interrupt_handler(int sig) {
	keepRunning = 0;
}

int main() {
	int listen_fd, *conn_fd;
	pthread_t tid;
	struct sockaddr_in clientaddr;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	struct Node *trie = getNode();
	struct Arguments args;
	args.trie = trie;

	printf("Starting server..\n");
	if ((listen_fd = open_listenfd()) < 0) {
		printf("Unable to create socket\n");
		return -1;
	}

	// TODO This is not enough. Need to make socket connection non-blocking
	signal(SIGINT, interrupt_handler);

	while(keepRunning) {
		conn_fd = (int *)malloc(sizeof(int));
		if ((*conn_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen)) < 0)
			return -1;
		args.conn_fd = conn_fd;
		pthread_create(&tid, NULL, thread, (void *)&args);
	}

	free_trie(trie);
	printf("Stopping server..\n");
	return 0;
}
