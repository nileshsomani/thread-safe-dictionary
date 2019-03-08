#include <stdlib.h>
#include <stdio.h>

#include "trie.h"

void insert(struct Node *trie, char *word) {
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
}

int search(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;

	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			return 0;
		}
	}
	if (iter->is_end == 1)
		return 1;
	return 0;
}

int del(struct Node *trie, char *word) {
	char c, i;
	struct Node *iter = trie;

	while (c = *word++) {
		i = INDEX(c);
		if (iter->children[i]) {
			iter = iter->children[i];
			continue;
		}
		else {
			return 1;
		}
	}
	// Soft Delete
	iter->is_end = 0;
	return 1;

}
struct Node *getNode() {
	int i;

	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	node->is_end = 0;

	for (i=0; i<NO_OF_CHARS; i++) {
		node->children[i] = NULL;
	}

	return node;
}

void free_trie(struct Node *trie) {
	int i;

	if (trie == NULL)
		return;

	for (i=0; i<NO_OF_CHARS; i++) {
		free_trie(trie->children[i]);
	}

	free(trie);
}

