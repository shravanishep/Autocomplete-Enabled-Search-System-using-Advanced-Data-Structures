#ifndef AVL_H
#define AVL_H

typedef struct AVLNode {
	char key[64];
	int count;
	int height;
	struct AVLNode* left;
	struct AVLNode* right;
} AVLNode;

AVLNode* avlInsert(AVLNode* root, const char* key);
int avlSearchCount(AVLNode* root, const char* key);
void avlFree(AVLNode* root);

#endif
