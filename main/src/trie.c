#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/trie.h"

TrieNode* root = NULL;

TrieNode* create() {
    TrieNode* n = malloc(sizeof(TrieNode));
    n->end = 0;
    for (int i = 0; i < 26; i++) n->child[i] = NULL;
    return n;
}

void insertTrie(char *w) {
    if (!root) root = create();
    TrieNode* cur = root;

    for (int i = 0; w[i]; i++) {
        int idx = w[i] - 'a';
        if (idx < 0 || idx > 25) continue;
        if (!cur->child[idx]) cur->child[idx] = create();
        cur = cur->child[idx];
    }
    cur->end = 1;
}

void print(TrieNode* node, char *buf, int lvl) {
    if (node->end) {
        buf[lvl] = '\0';
        printf("%s\n", buf);
    }
    for (int i = 0; i < 26; i++) {
        if (node->child[i]) {
            buf[lvl] = i + 'a';
            print(node->child[i], buf, lvl+1);
        }
    }
}

void searchPrefix(char *p) {
    TrieNode* cur = root;
    for (int i = 0; p[i]; i++) {
        int idx = p[i]-'a';
        if (idx<0||idx>25||!cur->child[idx]) {
            printf("No suggestions\n");
            return;
        }
        cur = cur->child[idx];
    }
    char buf[100];
    strcpy(buf,p);
    printf("Suggestions:\n");
    print(cur,buf,strlen(p));
}