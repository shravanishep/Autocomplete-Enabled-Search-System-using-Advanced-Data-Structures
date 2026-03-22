#include <stdlib.h>
#include <string.h>
#include "../include/edit_distance.h"

static int min3(int a, int b, int c) {
	int m = (a < b) ? a : b;
	return (m < c) ? m : c;
}

int levenshteinDistance(const char* a, const char* b) {
	if (!a || !b) return -1;

	int n = (int)strlen(a);
	int m = (int)strlen(b);

	int* prev = (int*)malloc((m + 1) * sizeof(int));
	int* curr = (int*)malloc((m + 1) * sizeof(int));
	if (!prev || !curr) {
		free(prev);
		free(curr);
		return -1;
	}

	for (int j = 0; j <= m; j++) prev[j] = j;

	for (int i = 1; i <= n; i++) {
		curr[0] = i;
		for (int j = 1; j <= m; j++) {
			int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
			curr[j] = min3(
				prev[j] + 1,
				curr[j - 1] + 1,
				prev[j - 1] + cost
			);
		}

		int* tmp = prev;
		prev = curr;
		curr = tmp;
	}

	int result = prev[m];
	free(prev);
	free(curr);
	return result;
}
