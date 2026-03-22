#include <stdlib.h>
#include <string.h>
#include "../include/avl.h"

static int maxVal(int a, int b) { return (a > b) ? a : b; }

static int height(AVLNode* n) { return n ? n->height : 0; }

static AVLNode* newNode(const char* key) {
	AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
	if (!n) return NULL;
	strncpy(n->key, key, sizeof(n->key) - 1);
	n->key[sizeof(n->key) - 1] = '\0';
	n->count = 1;
	n->height = 1;
	n->left = NULL;
	n->right = NULL;
	return n;
}

static AVLNode* rightRotate(AVLNode* y) {
	AVLNode* x = y->left;
	AVLNode* t2 = x->right;
	x->right = y;
	y->left = t2;
	y->height = maxVal(height(y->left), height(y->right)) + 1;
	x->height = maxVal(height(x->left), height(x->right)) + 1;
	return x;
}

static AVLNode* leftRotate(AVLNode* x) {
	AVLNode* y = x->right;
	AVLNode* t2 = y->left;
	y->left = x;
	x->right = t2;
	x->height = maxVal(height(x->left), height(x->right)) + 1;
	y->height = maxVal(height(y->left), height(y->right)) + 1;
	return y;
}

static int balance(AVLNode* n) {
	return n ? height(n->left) - height(n->right) : 0;
}

AVLNode* avlInsert(AVLNode* root, const char* key) {
	if (!key || !key[0]) return root;
	if (!root) return newNode(key);

	int cmp = strcmp(key, root->key);
	if (cmp == 0) {
		root->count++;
		return root;
	}

	if (cmp < 0) root->left = avlInsert(root->left, key);
	else root->right = avlInsert(root->right, key);

	root->height = 1 + maxVal(height(root->left), height(root->right));
	int b = balance(root);

	if (b > 1 && strcmp(key, root->left->key) < 0) return rightRotate(root);
	if (b < -1 && strcmp(key, root->right->key) > 0) return leftRotate(root);
	if (b > 1 && strcmp(key, root->left->key) > 0) {
		root->left = leftRotate(root->left);
		return rightRotate(root);
	}
	if (b < -1 && strcmp(key, root->right->key) < 0) {
		root->right = rightRotate(root->right);
		return leftRotate(root);
	}

	return root;
}

int avlSearchCount(AVLNode* root, const char* key) {
	AVLNode* cur = root;
	while (cur) {
		int cmp = strcmp(key, cur->key);
		if (cmp == 0) return cur->count;
		cur = (cmp < 0) ? cur->left : cur->right;
	}
	return 0;
}

void avlFree(AVLNode* root) {
	if (!root) return;
	avlFree(root->left);
	avlFree(root->right);
	free(root);
}
