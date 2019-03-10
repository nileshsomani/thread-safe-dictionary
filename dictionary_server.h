#include<pthread.h>

// Current maximum word length
#define WORD_MAX 50
// Supported only a-z(upper/lower) character
#define NO_OF_CHARS 26

#define FAILURE 0
#define SUCCESS 1

#define INDEX(c) ((c) < 'Z'? c-'A':c-'a')

// Maximum number of nodes to restrict memory foot print to 1M
#define MAX_SIZE (1024*1024)/sizeof(struct Node)

struct Node {
	//pthread_mutex_t lock; // TODO for more granular locking
	int is_end;	// To specify end of word
	struct Node *children[NO_OF_CHARS];
};

/*
 * This method creates a new node of trie DS
 * intiliazes the is_end with false and all the
 * children pointers with NULL
*/
static struct Node *getNode();

/*
 * This method is used to insert new word in
 * the dictionary
*/
static int insert(char *word);

/*
 * This method is used to search the word in
 * the dictionary
*/
static int search(char *word);

/*
 * This method is used to soft delete word in
 * the dictionary
*/
static int del(char *word);

/*
 * This method is used to delete word from
 * the dictionary
*/
static void recursive_del(struct Node *trie, char *word);

/*
 * This method is used to free up the dictionary
*/
static void free_trie(struct Node *trie);

/*
 * This method spawns a new worker thread for operation
 * on the dictionary
*/
static void *thread(void *args);

/*
 * This method is used to spawn a worker thread to hard
 * delete word from the dictionary once soft delete is done
*/
static void *del_thread(void *args);

/*
 * This method check is the current node has any
 * child alphabets
*/
static int has_children(struct Node *trie);

