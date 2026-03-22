#include "../include/heap.h"

void swap(HeapNode *a, HeapNode *b) {
    HeapNode t = *a;
    *a = *b;
    *b = t;
}

void heapify(HeapNode arr[], int n, int i) {
    int largest = i;
    int l = 2*i+1, r = 2*i+2;

    if (l < n && arr[l].score > arr[largest].score)
        largest = l;
    if (r < n && arr[r].score > arr[largest].score)
        largest = r;

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}

void buildHeap(HeapNode arr[], int n) {
    for (int i = n/2 - 1; i >= 0; i--)
        heapify(arr, n, i);
}

void sortResults(HeapNode arr[], int n) {
    buildHeap(arr, n);
    for (int i = n-1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}