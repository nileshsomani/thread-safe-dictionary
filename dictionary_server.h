#include<pthread.h>

#define WORD_MAX 50
#define NO_OF_CHARS 26
#define FAILURE 0
#define SUCCESS 1
#define INDEX(c) ((c) < 'Z'? c-'A':c-'a')
#define MAX_SIZE (1024*1024)/sizeof(struct Node)

struct Node {
	pthread_mutex_t lock;
	int is_end;
	struct Node *children[NO_OF_CHARS];
};

struct Arguments {
	struct Node *trie;
	int *conn_fd;
};

static struct Node *getNode();

static int insert(struct Node *trie, char *word);

static int search(struct Node *trie, char *word);

static int del(struct Node *trie, char *word);

static void free_trie(struct Node *trie);

static void *thread(void *args);
