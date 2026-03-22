#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/hash.h"

Entry* table[TABLE_SIZE] = {NULL};

int hashFunc(char *word) {
    int sum = 0;
    for (int i = 0; word[i]; i++)
        sum += word[i];
    return sum % TABLE_SIZE;
}

void insertHash(char *word, int docID) {
    int idx = hashFunc(word);
    Entry* temp = table[idx];

    while (temp) {
        if (strcmp(temp->word, word) == 0) {
            Node* n = temp->list;
            while (n) {
                if (n->docID == docID) {
                    n->freq++;
                    return;
                }
                n = n->next;
            }
            Node* newNode = malloc(sizeof(Node));
            newNode->docID = docID;
            newNode->freq = 1;
            newNode->next = temp->list;
            temp->list = newNode;
            return;
        }
        temp = temp->next;
    }

    Entry* e = malloc(sizeof(Entry));
    strcpy(e->word, word);

    Node* n = malloc(sizeof(Node));
    n->docID = docID;
    n->freq = 1;
    n->next = NULL;

    e->list = n;
    e->next = table[idx];
    table[idx] = e;
}

Node* searchHash(char *word) {
    int idx = hashFunc(word);
    Entry* temp = table[idx];

    while (temp) {
        if (strcmp(temp->word, word) == 0)
            return temp->list;
        temp = temp->next;
    }
    return NULL;
}