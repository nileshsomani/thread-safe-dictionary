#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "dictionary_server.h"
#include "ipc_socket.h"

// Current number of allocated trie nodes
static int cur_size = 0;

// Mutex lock for updating/accessing cur_size variable
static pthread_mutex_t cur_size_mutex;

// Mutex lock for updating/accessing dictioanry
static pthread_mutex_t trie_mutex;

// Root node of the dictionary
static struct Node *trie;

/*
 * This method is used to insert new word in
 * the dictionary
*/
static int insert(char *word) {
	char c, i;
	struct Node *node = NULL;
	struct Node *iter = trie;

	// acquire global lock
	pthread_mutex_lock(&trie_mutex);

	// iterate through characters
	while (c = *word++) {
		i = INDEX(c);
		// if already present continue
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			// if not present, check memory footprint
			pthread_mutex_lock(&cur_size_mutex);
			if (cur_size < MAX_SIZE) {
				// create new node and attach
				node = getNode();
				iter->children[i] = node;
				cur_size += 1;
				pthread_mutex_unlock(&cur_size_mutex);
				iter = iter->children[i];
			}
			else {
				// release locks
				pthread_mutex_unlock(&cur_size_mutex);
				pthread_mutex_unlock(&trie_mutex);
				return NOMEM;

			}
		}
	}
	// mark last node as end of word
	iter->is_end = 1;
	// release global lock
	pthread_mutex_unlock(&trie_mutex);
	return SUCCESS;
}

/*
 * This method is used to search the word in
 * the dictionary
*/
static int search(char *word) {
	char c, i;
	struct Node *iter = trie;

	// acquire global lock
	pthread_mutex_lock(&trie_mutex);

	// iterate through characters
	while (c = *word++) {
		i = INDEX(c);
		// if present, continue
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			// word not found. release lock and return
			pthread_mutex_unlock(&trie_mutex);
			return FAILURE;
		}
	}

	// release lock
	pthread_mutex_unlock(&trie_mutex);

	// after complete iteration check if last node is marked end of word
	if (iter->is_end == 1) {
		return SUCCESS;
	}
	return FAILURE;
}

/*
 * This method is used to soft delete word in
 * the dictionary
*/
static int del(char *word) {
	char c, i;
	struct Node *iter = trie;
	pthread_t tid;
	char *tmp = word;

	// acquire global lock
	pthread_mutex_lock(&trie_mutex);

	// iterate through characters
	while (c = *tmp++) {
		i = INDEX(c);
		// if already present, continue
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			// word does not exist to delete
			pthread_mutex_unlock(&trie_mutex);
			return NEXISTS;
		}
	}
	// Soft Delete
	iter->is_end = 0;
	// release lock
	pthread_mutex_unlock(&trie_mutex);

	// if last node has no children, spawn a thread for
	// recursive delete of nodes
	if (has_children(iter) == FAILURE) {
		tmp = (char *)malloc(sizeof(char) * strlen(word));
		strncpy(tmp, word, strlen(word));
		pthread_create(&tid, NULL, del_thread, (void *)tmp);
	}

	return SUCCESS;
}

/*
 * This method check is the current node has any
 * child alphabets
*/
static int has_children(struct Node *trie) {
	int i;

	for (i=0; i<NO_OF_CHARS; i++) {
		if (trie->children[i] != NULL)
			return SUCCESS;
	}

	return FAILURE;
}

/*
 * This method is used to delete word from
 * the dictionary
*/
static void recursive_del(struct Node *trie, char *word) {
	char c;
	int i;
	struct Node *iter;

	// recursive iteration through word
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

	// start freeing up memory in recursive manner and update children
	if (trie->children[i]->is_end == 0 && has_children(trie->children[i]) == 0) {
		free_trie(trie->children[i]);
		trie->children[i] = NULL;
		pthread_mutex_lock(&cur_size_mutex);
		cur_size -= 1;
		pthread_mutex_unlock(&cur_size_mutex);
	}
}

/*
 * This method creates a new node of trie DS
 * intiliazes the is_end with false and all the
 * children pointers with NULL
*/
static struct Node *getNode() {
	int i;

	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	node->is_end = 0;

	for (i=0; i<NO_OF_CHARS; i++) {
		node->children[i] = NULL;
	}
	//pthread_mutex_init(&node->lock, NULL);
	return node;
}

/*
 * This method is used to free up the dictionary recursively
*/
static void free_trie(struct Node *trie) {
	int i;

	if (trie == NULL)
		return;

	for (i=0; i<NO_OF_CHARS; i++) {
		free_trie(trie->children[i]);
	}
	//pthread_mutex_destroy(&trie->lock);
	free(trie);
}

/*
 * This method is used to spawn a worker thread to hard
 * delete word from the dictionary once soft delete is done
*/
static void *del_thread(void *arg) {
	char *word = (char *)arg;

	pthread_detach(pthread_self());

	pthread_mutex_lock(&trie_mutex);
	recursive_del(trie, word);
	pthread_mutex_unlock(&trie_mutex);

	free(arg);
	return NULL;
}

/*
 * This method spawns a new worker thread for operation
 * on the dictionary
*/
static void *thread(void *arg) {
	int conn_fd = *(int *)arg;
	free(arg);
	char action[10] = {0};
	char word[WORD_MAX] = {0};
	int res = 0;

	pthread_detach(pthread_self());
	// read action from client
	read(conn_fd, action, 8);

	// read word from client
	read(conn_fd, word, 50);

	printf("Spawning thread for %s [%s]\n", action, word);

	// perform action based on user provided keyword
	if (strcmp(action, "--search") == 0) {
		res = search(word);
	}
	else if (strcmp(action, "--insert") == 0) {
		res = insert(word);
	}
	else if (strcmp(action, "--delete") == 0) {
		res = del(word);
	}
	if (res == SUCCESS)
		send(conn_fd, "SUCCESS", 7, 0);
	else if (res == FAILURE)
		send(conn_fd, "FAILURE", 7, 0);
	else if (res == NEXISTS)
		send(conn_fd, "DOES NOT EXISTS", 15, 0);
	else if (res == NOMEM)
		send(conn_fd, "MEMORY IS FULL", 14, 0);

	close(conn_fd);
	return NULL;
}

/*
* Interrupt handler to capture Ctrl-C and free up dictionary
* and destroy locks before stopping dictionary service
*/
static void interrupt_handler(int sig) {
	pthread_mutex_destroy(&trie_mutex);
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

	// Create a dictionary service that listens on socket
	if ((listen_fd = open_listenfd()) < 0) {
		printf("Unable to create socket\n");
		return -1;
	}

	// init global locks
	pthread_mutex_init(&trie_mutex, NULL);
	pthread_mutex_init(&cur_size_mutex, NULL);

	// init interrupt handler
	signal(SIGINT, interrupt_handler);

	// start accept connections and spawn worker threads for each new connection
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
