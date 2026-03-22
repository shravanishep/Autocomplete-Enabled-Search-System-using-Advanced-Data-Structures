#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 1000

typedef struct Node {
    int docID;
    int freq;
    struct Node* next;
} Node;

typedef struct Entry {
    char word[50];
    Node* list;
    struct Entry* next;
} Entry;

void insertHash(char *word, int docID);
Node* searchHash(char *word);

#endif