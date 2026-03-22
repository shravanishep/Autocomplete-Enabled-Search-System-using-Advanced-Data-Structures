#ifndef TRIE_H
#define TRIE_H

typedef struct TrieNode {
    struct TrieNode* child[26];
    int end;
} TrieNode;

void insertTrie(char *word);
void searchPrefix(char *prefix);

#endif