#ifndef HEAP_H
#define HEAP_H

typedef struct {
    int docID;
    int score;
} HeapNode;

void buildHeap(HeapNode arr[], int n);
void sortResults(HeapNode arr[], int n);

#endif