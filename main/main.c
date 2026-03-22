#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "include/file_handler.h"
#include "include/trie.h"
#include "include/hash.h"

static void normalize_query(char *s) {
    int j = 0;
    for (int i = 0; s[i]; i++) {
        if (isalnum((unsigned char)s[i])) {
            s[j++] = (char)tolower((unsigned char)s[i]);
        }
    }
    s[j] = '\0';
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("Usage: program file query\n");
        return 0;
    }

    readFile(argv[1]);

    char query[256];
    strncpy(query, argv[2], sizeof(query) - 1);
    query[sizeof(query) - 1] = '\0';
    normalize_query(query);

    if (query[0] == '\0') {
        printf("Invalid query\n");
        return 0;
    }

    searchPrefix(query);

    Node* res = searchHash(query);

    if(!res){
        printf("Not found\n");
        return 0;
    }

    while(res){
        printf("Doc %d (freq %d)\n", res->docID, res->freq);
        res = res->next;
    }

    return 0;
}