#include <stdlib.h>
#include <string.h>
#include "../include/suffix.h"

static const char* gText = NULL;

static int cmpSuffix(const void* a, const void* b) {
	int i = *(const int*)a;
	int j = *(const int*)b;
	return strcmp(gText + i, gText + j);
}

SuffixArray buildSuffixArray(const char* text) {
	SuffixArray out;
	out.sa = NULL;
	out.n = 0;

	if (!text) return out;
	out.n = (int)strlen(text);
	if (out.n == 0) return out;

	out.sa = (int*)malloc(sizeof(int) * out.n);
	if (!out.sa) {
		out.n = 0;
		return out;
	}

	for (int i = 0; i < out.n; i++) out.sa[i] = i;
	gText = text;
	qsort(out.sa, out.n, sizeof(int), cmpSuffix);
	return out;
}

void freeSuffixArray(SuffixArray* arr) {
	if (!arr) return;
	free(arr->sa);
	arr->sa = NULL;
	arr->n = 0;
}

int suffixContains(const char* text, const char* pattern) {
	if (!text || !pattern || !pattern[0]) return 0;

	SuffixArray sa = buildSuffixArray(text);
	if (!sa.sa) return 0;

	int left = 0;
	int right = sa.n - 1;
	int found = 0;
	int pLen = (int)strlen(pattern);

	while (left <= right) {
		int mid = (left + right) / 2;
		int cmp = strncmp(text + sa.sa[mid], pattern, pLen);
		if (cmp == 0) {
			found = 1;
			break;
		}
		if (cmp < 0) left = mid + 1;
		else right = mid - 1;
	}

	freeSuffixArray(&sa);
	return found;
}
