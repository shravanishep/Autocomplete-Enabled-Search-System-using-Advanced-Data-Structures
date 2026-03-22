#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/hash.h"
#include "../include/trie.h"

void clean(char *w) {
    int j=0;
    char t[100];
    for(int i=0;w[i];i++)
        if(isalnum(w[i])) t[j++]=tolower(w[i]);
    t[j]='\0';
    strcpy(w,t);
}

void tokenize(char *line, int docID) {
    char *tok = strtok(line," \n\t");
    while(tok){
        clean(tok);
        if(strlen(tok)>0){
            insertTrie(tok);
            insertHash(tok,docID);
        }
        tok = strtok(NULL," \n\t");
    }
}