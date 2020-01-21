/*
	linked list class library
	author: Stephen Nichols
*/

#include <stdlib.h>
#include <stdio.h>

#include "list.hpp"
#include "types.hpp"

MemoryAllocator gLinkedElementAllocator ( sizeof ( LinkedElement ), 100000 );
MemoryAllocator gLinkedListAllocator ( sizeof ( LinkedList ), 10000 );

/* LinkedElement */

void LinkedElement::init ( void )
{
	_ptr = NULL;
	_next = NULL;
	_prev = NULL;
	m_pList = NULL;
}

/* link this element after another element */

void LinkedElement::linkAfter ( LinkedElement *left )
{
	/* you cannot link to a NULL LinkedElement */
	if ( left != NULL ) {
		LinkedElement *right = left->next();

		/* insert this between left and right */
		_prev = left;
		_next = right;

		left->next ( this );

		if ( right != NULL )
			right->prev ( this );
	}
}

/* link this element before another element */

void LinkedElement::linkBefore ( LinkedElement *right )
{
	/* you cannot link to a NULL LinkedElement */
	if ( right != NULL ) {
		LinkedElement *left = right->prev();

		/* insert this between left and right */
		_prev = left;
		_next = right;

		right->prev ( this );

		if ( left )
			left->next ( this );
	}
}

/* unlink this element from it's prev and next */

void LinkedElement::unlink ( void )
{
	if ( _prev )
		_prev->next ( _next );

	if ( _next )
		_next->prev ( _prev );

	_next = NULL;
	_prev = NULL;
	m_pList = NULL;
}

// delete this element from the owning list
void LinkedElement::DeleteFromList ( void )
{
	if ( m_pList ) {
		m_pList->delElement ( this );
	}
}

/* LinkedList */

/* default constructor */

LinkedList::LinkedList ( void )
{
	reset();
}

/* copy constructor */

LinkedList::LinkedList ( LinkedList *list )
{
	reset();

	LinkedElement *element = list->head();

	while ( element != NULL ) {
		LinkedElement *next = element->next();
		add ( element->ptr() );
		element = next;
	}
}

/* destructor */

LinkedList::~LinkedList ( void ) 
{
	if ( _disposeElements ) {
		dispose();
	}

	release();
	reset();
}

void LinkedList::dispose ( void )
{
	LinkedElement *element = head();

	while ( element != NULL ) {
		LinkedElement *next = element->next();
		delete element->ptr();
		element = next;
	}

	release();
}

/* reset the members of this class */

void LinkedList::reset ( void )
{
	_head = NULL;
	_tail = NULL;
	_size = 0;
	_disposeElements = 1;
}

/* add a pointer to the end of this list */

LinkedElement *LinkedList::addToEnd ( ListObject *ptr )
{
	LinkedElement *element = (LinkedElement *)gLinkedElementAllocator.allocate();

	element->init();
	
	if ( _tail ) {
		element->linkAfter ( _tail );
	} else {
		_head = element;
	}

	element->SetList ( this );
	_tail = element;
	_size++;

	element->ptr ( ptr );

	return element;
}

/* add a pointer to the front of this list */

void LinkedList::addToFront ( ListObject *ptr )
{
	LinkedElement *element = (LinkedElement *)gLinkedElementAllocator.allocate();

	element->init();
	
	if ( _head ) {
		_head->prev ( element );
		element->next ( _head );
	}

	_size++;
	_head = element;
	element->ptr ( ptr );
	element->SetList ( this );
}

/* add a pointer after another pointer in this list */

void LinkedList::addAfter ( ListObject *ptr_a, ListObject *ptr_b )
{
	LinkedElement *element_a = elementOf ( ptr_a );

	if ( element_a != NULL ) {
		LinkedElement *element_b = (LinkedElement *)gLinkedElementAllocator.allocate();

		element_b->init();

		element_b->linkAfter ( element_a );

		if ( element_b->prev() == NULL )
			_head = element_b;

		if ( element_b->next() == NULL )
			_tail = element_b;

		element_b->ptr ( ptr_b );
		element_b->SetList ( this );
		_size++;
	}
}	

/* add a pointer before another pointer in this list */

LinkedElement *LinkedList::addBefore ( ListObject *ptr_a, ListObject *ptr_b )
{
	LinkedElement *element_a = elementOf ( ptr_a );
	return addBeforeElement ( element_a, ptr_b );
}

LinkedElement *LinkedList::addBeforeElement ( LinkedElement *element_a, ListObject *ptr_b ) 
{
	LinkedElement *element_b = NULL; 

	if ( element_a != NULL ) {
		LinkedElement *element_b = (LinkedElement *)gLinkedElementAllocator.allocate();

		element_b->init();

		element_b->linkBefore ( element_a );

		if ( element_b->prev() == NULL )
			_head = element_b;

		if ( element_b->next() == NULL )
			_tail = element_b;

		element_b->ptr ( ptr_b );
		element_b->SetList ( this );
		_size++;
	}

	return element_b;
}

/* delete a pointer from this list */

void LinkedList::del ( ListObject *ptr )
{
	LinkedElement *element = elementOf ( ptr );
	delElement ( element );
}

void LinkedList::delElement ( LinkedElement *element )
{
	if ( element != NULL ) {
		if ( element == _head ) {
			_head = element->next();
		}

		if ( element == _tail ) {
			_tail = element->prev();
		}

		element->unlink();

		gLinkedElementAllocator.deallocate ( (char *)element );

		_size--;
	}
}	

/* return the pointer in this list at a particular index */

ListObject *LinkedList::at ( int index )
{
	LinkedElement *element = _head;
	int count = 0;

	while ( element != NULL && count++ < index )
		element = element->next();

	ListObject *retVal = NULL;

	if ( element != NULL )
		retVal = element->ptr();

	return retVal;
}

/* return the index of a pointer in this list */

int LinkedList::indexOf ( ListObject *ptr )
{
	int retVal = -1;

	LinkedElement *element = _head;
	int index = 0;

	while ( element != NULL && element->ptr() != ptr ) {
		element = element->next();
		index++;
	}

	if ( element != NULL )
		retVal = index;

	return retVal;
}

/* return the element of a pointer in this list */

LinkedElement *LinkedList::elementOf ( ListObject *ptr )
{
	LinkedElement *element = _head;

	while ( element != NULL && element->ptr() != ptr )
		element = element->next();

	return element;
}

/* release the contents of this list */

void LinkedList::release ( void )
{
	while ( _tail )
		delElement ( _tail );
}

/* return a copy of this list */
LinkedList *LinkedList::copy ( void )
{
	LinkedList *ret = new LinkedList;

	LinkedElement *element = head();

	while ( element ) {
		ret->add ( element->ptr() );
		element = element->next();
	}

	return ret;
}

/* copy another list */
void LinkedList::copy ( LinkedList *list )
{
	LinkedElement *element = list->head();

	while ( element ) {
		add ( element->ptr() );
		element = element->next();
	}
}

/* wait for this list's size semaphore */

int LinkedList::waitForSize ( void )
{
	return FALSE;
}
