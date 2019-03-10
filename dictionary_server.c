#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "dictionary_server.h"
#include "ipc_socket.h"

static int cur_size = 0;
pthread_mutex_t cur_size_mutex;
static struct Node *trie;

static int insert(struct Node *trie, char *word) {
	char c, i;
	struct Node *node = NULL;
	struct Node *iter = trie;

	pthread_mutex_lock(&trie->lock);
	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			pthread_mutex_lock(&cur_size_mutex);
			if (cur_size < MAX_SIZE) {
				node = getNode();
				iter->children[i] = node;
				cur_size += 1;
				pthread_mutex_unlock(&cur_size_mutex);
				iter = iter->children[i];
			}
			else {
				pthread_mutex_unlock(&cur_size_mutex);
				pthread_mutex_unlock(&trie->lock);
				return FAILURE;

			}
		}
	}
	iter->is_end = 1;
	pthread_mutex_unlock(&trie->lock);
	return SUCCESS;
}

static int search(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;

	pthread_mutex_lock(&trie->lock);
	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			pthread_mutex_unlock(&trie->lock);
			return FAILURE;
		}
	}
	if (iter->is_end == 1) {
		pthread_mutex_unlock(&trie->lock);
		return SUCCESS;
	}
	pthread_mutex_unlock(&trie->lock);
	return FAILURE;
}

static int del(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;
	pthread_t tid;
	struct del_arguments *args;
	char *tmp = word;

	pthread_mutex_lock(&trie->lock);
	while (c = *tmp++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			pthread_mutex_unlock(&trie->lock);
			return SUCCESS;
		}
	}
	// Soft Delete
	iter->is_end = 0;
	pthread_mutex_unlock(&trie->lock);

	if (has_children(iter) == 0) {
		args = (struct del_arguments *)malloc(sizeof(struct del_arguments));
		args->trie = trie;
		args->word = (char *)malloc(sizeof(char) * strlen(word));
		strncpy(args->word, word, strlen(word));
		pthread_create(&tid, NULL, del_thread, (void *)args);
	}

	return SUCCESS;
}

static int has_children(struct Node *trie) {
	int i;

	for (i=0; i<NO_OF_CHARS; i++) {
		if (trie->children[i] != NULL)
			return 1;
	}

	return 0;
}

static void recursive_del(struct Node *trie, char *word) {
	char c;
	int i;
	struct Node *iter;

	if (c = *word++) {
		i = INDEX(c);
		if (trie->children[i]) {
			iter = trie->children[i];
			recursive_del(iter, word);
		}
		else
			return;
	}
	if (!c)
		return;
	if (trie->children[i]->is_end == 0 && has_children(trie->children[i]) == 0) {
		free_trie(trie->children[i]);
		trie->children[i] = NULL;
		pthread_mutex_lock(&cur_size_mutex);
		cur_size -= 1;
		pthread_mutex_unlock(&cur_size_mutex);
	}
}

static struct Node *getNode() {
	int i;

	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	node->is_end = 0;

	for (i=0; i<NO_OF_CHARS; i++) {
		node->children[i] = NULL;
	}
	pthread_mutex_init(&node->lock, NULL);
	return node;
}

static void free_trie(struct Node *trie) {
	int i;

	if (trie == NULL)
		return;

	for (i=0; i<NO_OF_CHARS; i++) {
		free_trie(trie->children[i]);
	}
	pthread_mutex_destroy(&trie->lock);
	free(trie);
}

static void *del_thread(void *arg) {
	struct del_arguments *args = (struct del_arguments *)arg;

	pthread_detach(pthread_self());

	pthread_mutex_lock(&args->trie->lock);
	recursive_del(args->trie, args->word);
	pthread_mutex_unlock(&args->trie->lock);

	free(args->word);
	free(args);
	args = NULL;
	return NULL;
}

static void *thread(void *arg) {
	int conn_fd = *(int *)arg;
	free(arg);
	char action[10] = {0};
	char word[WORD_MAX] = {0};
	int res = 0;

	pthread_detach(pthread_self());
	read(conn_fd, action, 8);
	read(conn_fd, word, 50);
	printf("Spawning thread for %s [%s]\n", action, word);
	if (strcmp(action, "--search") == 0) {
		res = search(trie, word);
	}
	else if (strcmp(action, "--insert") == 0) {
		res = insert(trie, word);
	}
	else if (strcmp(action, "--delete") == 0) {
		res = del(trie, word);
	}
	if (res == SUCCESS)
		send(conn_fd, "SUCCESS", 7, 0);
	else
		send(conn_fd, "FAILURE", 7, 0);

	close(conn_fd);
	return NULL;
}

static void interrupt_handler(int sig) {
	free_trie(trie);
	pthread_mutex_destroy(&cur_size_mutex);
	printf("Stopping server..\n");
	exit(0);
}

int main() {
	int listen_fd, *conn_fd=NULL;
	pthread_t tid;
	struct sockaddr_in clientaddr;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	trie = getNode();

	printf("Starting server..\n");
	if ((listen_fd = open_listenfd()) < 0) {
		printf("Unable to create socket\n");
		return -1;
	}
	pthread_mutex_init(&cur_size_mutex, NULL);

	signal(SIGINT, interrupt_handler);

	while(1) {
		conn_fd = (int *)malloc(sizeof(int));
		if ((*conn_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen)) < 0) {
			return -1;
		}
		else {
			pthread_create(&tid, NULL, thread, (void *)conn_fd);
		}
	}

	close(listen_fd);
	return 0;
}
