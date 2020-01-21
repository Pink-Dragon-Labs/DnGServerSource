/*
	generic linked list class library
	author: Stephen Nichols
*/

#ifndef _LIST_HPP_
#define _LIST_HPP_

#include "listobj.hpp"
#include "new.hpp"
#include "malloc.hpp"
#include "memorypool.hpp"

extern MemoryAllocator gLinkedElementAllocator;
extern MemoryAllocator gLinkedListAllocator;

class LinkedList;

class LinkedElement 
{
	ListObject *_ptr;
	LinkedElement *_next, *_prev;
	LinkedList *m_pList;

public:

	void init ( void );

	/* access functions */
	inline ListObject *ptr ( void ) { return _ptr; };
	inline LinkedElement *next ( void ) { return _next; };
	inline LinkedElement *prev ( void ) { return _prev; };

	inline ListObject *ptr ( ListObject *value ) {
		_ptr = value;
		return value;
	}; 

	inline LinkedElement *next ( LinkedElement *value ) {
		_next = value;
		return value;
	};

	inline LinkedElement *prev ( LinkedElement *value ) {
		_prev = value;
		return value;
	};

	/* link this element after another element */
	void linkAfter ( LinkedElement *element );

	/* link this element before another element */
	void linkBefore ( LinkedElement *element );

	/* unlink this element from it's next and prev */
	void unlink ( void );

	// delete this element from it's owning list
	void DeleteFromList ( void );

	// set the owning list of this element
	inline void SetList ( LinkedList *pList ) {
		m_pList = pList;
	};
};

#undef new

class LinkedList : public ListObject
{
	LinkedElement *_head, *_tail;
	int _size, _disposeElements;

	/* private functions */
	void reset ( void );

public:

	LinkedList ( void );
	LinkedList ( LinkedList * );
	virtual ~LinkedList ( void );

	/* access functions */
	inline LinkedElement *head ( void ) { return _head; };
	inline LinkedElement *tail ( void ) { return _tail; };
	inline int size ( void ) { return _size; };
	inline int contains ( ListObject *ptr ) { return indexOf ( ptr ) != -1; };
	inline void disposeElements ( int val ) { _disposeElements = val; };

	/* add a pointer to the end of this list */
	LinkedElement *addToEnd ( ListObject *ptr );
	LinkedElement *add ( ListObject *ptr ) { return addToEnd ( ptr ); };

	/* add a pointer to the beginning of this list */
	void addToFront ( ListObject * ptr );

	/* add a pointer after another pointer in this list */
	void addAfter ( ListObject * ptr_a, ListObject * ptr_b );

	/* add a pointer before another pointer in this list */
	LinkedElement *addBefore ( ListObject * ptr_a, ListObject * ptr_b );
	LinkedElement *addBeforeElement ( LinkedElement *ptr_a, ListObject * ptr_b );

	/* delete a pointer from this list */
	void del ( ListObject * ptr );
	void delElement ( LinkedElement *ptr );

	/* return the pointer in this list at a particular index */
	ListObject * at ( int index );

	/* return the index of a pointer in this list */
	int indexOf ( ListObject * ptr );

	/* return the element of a pointer in this list */
	LinkedElement *elementOf ( ListObject * ptr );

	/* release the contents of this list */
	void release ( void );
	void dispose ( void );

	/* copy this list */
	LinkedList *copy ( void );

	/* copy another list */
	void copy ( LinkedList *list );

	/* block the calling thread until this object has a size */
	int waitForSize ( void );

	void* operator new ( size_t size, char* file, int nLine ) {
		return gLinkedListAllocator.allocate();
	}

	void operator delete ( void *ptr ) {
		gLinkedListAllocator.deallocate ( (char *)ptr );
	}

	inline int isValid ( void ) {
		return gLinkedListAllocator.isValidPtr ( (char *)this );
	}
};

#define new new( __FILE__, __LINE__ )


#endif
