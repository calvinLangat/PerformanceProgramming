#include <stdio.h>
#include <stdlib.h>

typedef struct treenode{
	int val;
	struct treenode *left;
	struct treenode *right;
} treenode;

treenode* createNode(int value)
{
	treenode* node = (treenode* )malloc(sizeof(treenode));
	
	if(node != NULL)
	{
		node->val = value;
		node->left = NULL;
		node->right = NULL;
	}
	return node;
}

int insertNode(treenode** rootptr, int value)
{
	treenode* root = *rootptr; 
	if (root == NULL)
	{
		// Tree Empty
		(*rootptr) = createNode(value);
		return 1;
	}

	if (value == root->val)
	{
		// value already exists
		return 0;
	}

	if (value < root->val)
	{
		return insertNode(&root->left, value);
	}
	else
	{
		return insertNode(&root->right, value);
	}
}

void printTabs(int numtabs)
{
	for(int i=0; i<numtabs; i++)
		printf("\t");
}

void printTree_rec(treenode* root, int level)
{
	if(root == NULL)
	{
		printTabs(level);
		printf("---<EMPTY>---\n");
		return;
	}
	printTabs(level);
	printf("value = %d\n", root->val);

	printTabs(level);
	printf("left\n");
	printTree_rec(root->left, level+1);

	printTabs(level);
	printf("right\n");
	printTree_rec(root->right, level+1);

	printTabs(level);
	printf("done\n");

}

void printTree(treenode* root)
{
	printTree_rec(root, 0);
}

int searchTree(treenode* root, int value)
{
	if(root == NULL)
		return 0;

	if(root->val == value)
		return 1;

	if(value < root->val)
		return searchTree(root->left, value);
	else
		return searchTree(root->right, value);
}

int main(int argc, char* argv[])
{
	treenode* root;

	insertNode(&root, 5);
	insertNode(&root, 6);
	insertNode(&root, 9);
	insertNode(&root, 10);
	insertNode(&root, 2);
	insertNode(&root, 3);

	printTree(root);

	printf("%d {%d}\n", 5, searchTree(root, 5));
	printf("%d {%d}\n", 6, searchTree(root, 6));
	printf("%d {%d}\n", 9, searchTree(root, 9));
	printf("%d {%d}\n", 1, searchTree(root, 1));
	printf("%d {%d}\n", 100, searchTree(root, 100));

	return 0;
}