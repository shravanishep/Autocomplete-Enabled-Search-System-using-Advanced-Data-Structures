#include <stdio.h>
#include <string.h>
#include "include/hash.h"
#include "include/trie.h"

void readFile(char*);

int main(){
    char filename[100];

    printf("Enter file path: ");
    scanf("%s", filename);

    readFile(filename);

    char q[50];

    while(1){
        printf("\nSearch: ");
        scanf("%s", q);

        for(int i=0;q[i];i++) q[i]=tolower(q[i]);

        searchPrefix(q);

        Node* res = searchHash(q);

        if(!res){
            printf("No exact match\n");
            continue;
        }

        // collect for heap
        HeapNode arr[100];
        int n=0;

        while(res){
            arr[n].docID = res->docID;
            arr[n].score = res->freq;
            res=res->next;
            n++;
        }

        sortResults(arr,n);

        printf("Ranked Results:\n");
        for(int i=n-1;i>=0;i--){
            printf("Doc %d (score %d)\n",arr[i].docID,arr[i].score);
        }
    }
}