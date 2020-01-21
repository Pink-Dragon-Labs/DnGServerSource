//
// tree.cpp
//
// binary tree structure
//
// author: Stephen Nichols
//

#include "system.hpp"

MemoryAllocator gTreeNodeAllocator ( sizeof ( TreeNode ), 10000 );

//
// TreeNode
//

TreeNode::TreeNode()
{
	left = right = NULL;
	name = NULL;
	data = NULL;
}

TreeNode::~TreeNode()
{
	if ( name ) {
		free ( name );
		name = NULL;
	}
}

//
// BinaryTree
//

BinaryTree::BinaryTree()
{
	root = NULL;
	size = 0;
	m_bCaseSensitive = TRUE;
}

BinaryTree::~BinaryTree()
{
	delete root;
	root = NULL;
	size = 0;
}

TreeNode *BinaryTree::add ( const char *name, void *ptr )
{
	//
	// if no root node, add is braindead
	//

	if ( !root ) {
		root = new TreeNode;
		root->name = strdup ( name );
		root->data = ptr;
		size++;

		return root;
	}

	//
	// scan from the root until we find a node that has a left or a right that
	// is larger than our name
	//

	TreeNode *node = root;

	while ( node ) {
		int compare = m_bCaseSensitive? strcmp ( name, node->name ) : strcasecmp ( name, node->name );

		// already in the tree, skip out
		if ( compare == 0 ) {
			node->data = ptr;
			return node;
		}
			
		// if less than current node, try adding to the left
		if ( compare < 0 ) {
			if ( node->left ) {
				node = node->left;
				continue;
			} 

			TreeNode *newNode = new TreeNode;
			newNode->name = strdup ( name );
			newNode->data = ptr;
			node->left = newNode;
			size++;

			return newNode;
		}

		// if greater than current node, try adding to the right
		if ( compare > 0 ) {
			if ( node->right ) {
				node = node->right;
				continue;
			}

			TreeNode *newNode = new TreeNode;
			newNode->name = strdup ( name );
			newNode->data = ptr;
			node->right = newNode;
			size++;

			return newNode;
		}
	}

	return NULL;
}

void BinaryTree::del ( const char *name )
{
	//
	// look for the node we want to toss
	//

	TreeNode *nodeToDel = root;
	TreeNode *parent = NULL;

	while ( nodeToDel ) {
		int compare = m_bCaseSensitive? strcmp ( name, nodeToDel->name ) : strcasecmp ( name, nodeToDel->name );

		if ( compare == 0 )
			break;

		if ( compare < 0 ) {
			parent = nodeToDel;
			nodeToDel = nodeToDel->left;
			continue;
		}

		if ( compare > 0 ) {
			parent = nodeToDel;
			nodeToDel = nodeToDel->right;
			continue;
		}
	}

	if ( !nodeToDel ) {
		return;
	}

	TreeNode *node = nodeToDel;

	if ( !nodeToDel->right ) {
		node = node->left;
	}

	else if ( nodeToDel->right->left == NULL ) {
		node = node->right;
		node->left = nodeToDel->left;
	}

	else {
		TreeNode *c = node->right;

		while ( c->left->left != NULL )
			c = c->left;

		node = c->left;
		c->left = node->right;
		node->left = nodeToDel->left;
		node->right = nodeToDel->right;
	}

	if ( parent ) {
		int compare = m_bCaseSensitive? strcmp ( nodeToDel->name, parent->name ) : strcasecmp ( nodeToDel->name, parent->name );

		if ( compare < 0 ) {
			parent->left = node;
		} else {
			parent->right = node;
		}
	} else {
		root = node;
	}

	size--;
	delete nodeToDel;
}

TreeNode *BinaryTree::find ( const char* name )
{
	//
	// scan the tree from the top
	//

	TreeNode *node = root;

	while ( node ) {
		int compare = m_bCaseSensitive? strcmp ( name, node->name ) : strcasecmp ( name, node->name );

		// found it, all done
		if ( compare == 0 ) {
			return node;
		}
			
		// if less than current node, go left 
		if ( compare < 0 ) {
			if ( node->left ) {
				node = node->left;
				continue;
			} 

			return NULL;
		}

		// if greater than current node, go right
		if ( compare > 0 ) {
			if ( node->right ) {
				node = node->right;
				continue;
			}

			return NULL;
		}
	}

	return NULL;
}

// set the case sensitive flag
bool BinaryTree::SetCaseSensitive ( bool bCaseSensitive )
{
	bool bRetVal = m_bCaseSensitive;
	m_bCaseSensitive = bCaseSensitive;

	return bRetVal;
}
