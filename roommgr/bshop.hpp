/*
	BShop class
	author: Stephen Nichols
*/

#ifndef _BSHOP_HPP_
#define _BSHOP_HPP_

#include "../global/system.hpp"
#include "wobjectbase.hpp"

class BShop;

// enumerate the types of shop objects
enum {
	_SHOP_ITEM,
	_SHOP_CATEGORY,
	_SHOP_OBJECT,
	_SHOP_SPELL,
	_SHOP_SKILL,
	_SHOP_CRYSTALS,
	_SHOP_NEW_SKILL,
	_SHOP_STAT,
};


// 
// ShopItem: This class represents one entry in a shop's inventory.  
//

class ShopItem : public ListObject
{
public:
	int type, cost, icon, clutStart, color;
	char *name;
	BShop *shop;

	ShopItem();
	virtual ~ShopItem();

	// this method is called to set the name of this ShopItem
	void setName ( char *text );

	// this method is called to set the icon of this ShopItem
	void setIcon ( int theIcon );

	// this method is called to clone this object
	virtual ShopItem *clone ( BShop *owner );

	// put my binary information into a PackedData structure
	virtual void buildPacket ( PackedData *packet, WorldObject *context = NULL );

	// put my weight information into a PackedData structure
	virtual void buildWeightPacket ( PackedData *pPacket );

	// get my text description
	virtual const char *description ( void );

	// get my price -- yee ha
	virtual int price ( void );

	// pointer to my owner
	ShopItem *owner;
};

//
// ShopCartItem: This class represents a ShopItem / quantity pair that is used for mass buy.
//

class ShopCartItem : public ListObject
{
public:
	// the number of items...
	int m_nQuantity;

	// the pointer to the item to buy...
	ShopItem *m_pItem;

public:
	ShopCartItem() { m_nQuantity = 0; m_pItem = NULL; };
	virtual ~ShopCartItem() {};
};

// 
// ShopCategory: This class represents a category of ShopItems in a shop's
// inventory.
//

class ShopCategory : public ShopItem
{
public:
	// list of ShopItems in this category
	LinkedList elements;

	ShopCategory();
	virtual ~ShopCategory();

	// add an item to this category
	void addItem ( ShopItem *item );

	// delete an item from this category
	void deleteItem ( ShopItem *item );

	// this method is called to clone this object
	virtual ShopItem *clone ( BShop *owner );

	// put my binary information into a PackedData structure
	virtual void buildPacket ( PackedData *packet, WorldObject *context = NULL );

	// put my weight information into a PackedData structure
	virtual void buildWeightPacket ( PackedData *pPacket );
};

//
// ShopObject: This class represents a particular physical item that is for
// sale in a shop.
//

class ShopObject : public ShopItem
{
public:
	// object that this represents
	WorldObject *object;

	ShopObject ( WorldObject *obj );
	virtual ~ShopObject();

	// this method is called to clone this object
	virtual ShopItem *clone ( BShop *owner );
	virtual void buildPacket ( PackedData *packet, WorldObject *context = NULL );

	// put my weight information into a PackedData structure
	virtual void buildWeightPacket ( PackedData *pPacket );

	// get my text description
	virtual const char *description ( void );
};

class BShop : public WorldObjectBase
{
public:
	// types of currency accepted
	enum {
		Gold,
		Copper
	};

	BShop ( WorldObject *obj );
	virtual ~BShop();

	void copy ( WorldObjectBase *base );
	void buildShopPacket ( PackedData *packet, WorldObject *context = NULL );
	void buildWeightPacket ( PackedData *pPacket );
	virtual void writeSCIData ( FILE *file );

	// return the ShopItem at a particular index
	ShopItem *at ( int index );

	// add a new item to this shop
	void addItem ( ShopItem *item );

	// buy an object from an object
	int buy ( WorldObject *object, WorldObject *owner );

	// sell an item to an object
	int sell ( int item, WorldObject *buyer );

	// return the price this shop will pay to buy an object
	int price ( WorldObject *object );

	// get rid of objects (bought by a shop)
	int toss ( WorldObject *object );

	// this is the inventory of this shop
	ShopCategory inventory;

	// this is the list of all shop items
	LinkedList items;

	// this is the selling price markup percentage; 100 == same as value
	int sellMarkup;

	// this is the buying price markup percentage; 100 == same as value
	int buyMarkup;

	int currency;
};

#endif
