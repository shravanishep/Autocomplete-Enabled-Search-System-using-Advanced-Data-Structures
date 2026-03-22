#include <stdio.h>
#include "../include/tokenizer.h"

void readFile(const char *name){
    FILE *fp=fopen(name,"r");
    if(!fp){ printf("File error\n"); return;}

    char line[500];
    int id=1;
    while(fgets(line,sizeof(line),fp)){
        tokenize(line,id++);
    }
    fclose(fp);
}