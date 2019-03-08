#define NO_OF_CHARS 26

#define INDEX(c) ((c) < 'Z'? c-'A':c-'a')

struct Node {
	int is_end;
	struct Node *children[NO_OF_CHARS];
};

struct Node *getNode();

void insert(struct Node *trie, char *word);

int search(struct Node *trie, char *word);

int del(struct Node *trie, char *word);

void free_trie(struct Node *trie);
