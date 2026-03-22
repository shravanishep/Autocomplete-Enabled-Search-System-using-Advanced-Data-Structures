#ifndef SUFFIX_H
#define SUFFIX_H

typedef struct {
	int* sa;
	int n;
} SuffixArray;

SuffixArray buildSuffixArray(const char* text);
void freeSuffixArray(SuffixArray* arr);
int suffixContains(const char* text, const char* pattern);

#endif
