//
// tree.hpp
//
// binary tree structure
//
// author: Stephen Nichols
//

#ifndef _TREE_HPP_
#define _TREE_HPP_

extern MemoryAllocator gTreeNodeAllocator;

#undef new

class TreeNode 
{
public:
	TreeNode();
	virtual ~TreeNode();

	TreeNode *left, *right;
	char *name;
	void *data;

	void* operator new ( size_t size, char* file, int nLine ) {
		return gTreeNodeAllocator.allocate();
	};

	void operator delete ( void *ptr ) {
		gTreeNodeAllocator.deallocate ( (char *)ptr );
	};
};

#define new new( __FILE__, __LINE__ )

class BinaryTree 
{
protected:
	// flag for case comparison of names...
	bool m_bCaseSensitive;

public:
	BinaryTree();
	virtual ~BinaryTree();

	TreeNode *add ( const char* name, void *ptr );
	void del ( const char *name );
	TreeNode *find ( const char* name );

	// call to set case sensitivity
	bool SetCaseSensitive ( bool bCaseSensitive );

	TreeNode *root;
	int size;
};

#endif
