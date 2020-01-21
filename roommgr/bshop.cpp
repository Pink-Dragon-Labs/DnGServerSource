/*
	BShop class
	author: Stephen Nichols
*/

#include "bshop.hpp"
#include "roommgr.hpp"

// 
// ShopItem: This class represents one entry in a shop's inventory.  
//

ShopItem::ShopItem()
{
	type = _SHOP_ITEM;
	cost = 0;
	icon = -1;
	clutStart = 0;
	color = 0;
	
	name = 0;
	owner = 0;
	shop = 0;
}

ShopItem::~ShopItem()
{
	setName ( 0 );
}

void ShopItem::setName ( char *text )
{
	if ( name ) {
		free ( name );
		name = 0;
	}

	if ( text ) 
		name = strdup ( text );
}

void ShopItem::setIcon ( int theIcon )
{
	icon = theIcon;
}

ShopItem *ShopItem::clone ( BShop *owner )
{
	return 0;
}

int ShopItem::price ( void )
{
	return shop? (int)((double)cost * ((double)shop->sellMarkup / 100.0)) : cost;
}

void ShopItem::buildPacket ( PackedData *packet, WorldObject *context )
{
	packet->putByte ( type );
	packet->putWord ( icon );
	packet->putByte ( clutStart );
	packet->putByte ( color );
	packet->putLong ( price() );
	packet->putString ( name? name : (char*) "" );
}

void ShopItem::buildWeightPacket ( PackedData *pPacket ) 
{
	pPacket->putWord ( 0 );
}

const char *ShopItem::description ( void )
{
	return "You see a ShopItem.  If you see this message, the programmers have some more work to do!";
}

// 
// ShopCategory: This class represents a category of ShopItems in a shop's
// inventory.
//

ShopCategory::ShopCategory() : ShopItem()
{
	type = _SHOP_CATEGORY;
}

ShopCategory::~ShopCategory()
{
}

// add an item to this category
void ShopCategory::addItem ( ShopItem *item )
{
	elements.add ( item );
	item->shop = shop;
	item->owner = this;
}

// delete an item from this category
void ShopCategory::deleteItem ( ShopItem *item )
{
	elements.del ( item );
}

ShopItem *ShopCategory::clone ( BShop *owner )
{
	LinkedElement *element = elements.head();
	ShopCategory *clone = new ShopCategory;

	clone->icon = icon;
	clone->cost = cost;
	clone->clutStart = clutStart;
	clone->color = color;
	clone->setName ( name );
	clone->shop = owner;

	while ( element ) {
		clone->addItem ( ((ShopItem *)element->ptr())->clone ( owner ) );		
		element = element->next();
	}

	return (ShopItem *)clone;
}

void ShopCategory::buildPacket ( PackedData *packet, WorldObject *context )
{
	ShopItem::buildPacket ( packet );
	packet->putWord ( elements.size() );

	LinkedElement *element = elements.head();

	while ( element ) {
		ShopItem *item = (ShopItem *)element->ptr();
		item->buildPacket ( packet, context );
		element = element->next();
	}
}

void ShopCategory::buildWeightPacket ( PackedData *pPacket ) 
{
	pPacket->putWord ( elements.size() );

	LinkedElement *element = elements.head();

	while ( element ) {
		ShopItem *item = (ShopItem *)element->ptr();
		item->buildWeightPacket ( pPacket );
		element = element->next();
	}
}

//
// ShopObject: This class represents a particular physical item that is for
// sale in a shop.
//

ShopObject::ShopObject ( WorldObject *obj ) : ShopItem()
{
	type = _SHOP_OBJECT;

	if ( obj->view == _MANA_VIEW )
		type = _SHOP_CRYSTALS;

	if ( obj ) {
		object = obj;
		icon = obj->view + _WOA_INVENTORY;

		if ( !IsThisATestServer() )
			cost = obj->netWorth();
		else
			cost = 0;

		clutStart = obj->clutStart;
		color = obj->color;

		setName ( obj->name );
	}
}

ShopObject::~ShopObject()
{
	object = 0;
}

ShopItem *ShopObject::clone ( BShop *owner )
{
	ShopObject *clone = new ShopObject ( object );

	clone->shop = owner;
	clone->setName ( name );
	clone->cost = cost;

	return clone;
}

void ShopObject::buildPacket ( PackedData *packet, WorldObject *context )
{
	packet->putByte ( type );
	packet->putWord ( icon );
	packet->putByte ( clutStart );
	packet->putByte ( color );
	packet->putLong ( price() );

	packet->putString ( name? name : (char*) "" );
}

void ShopObject::buildWeightPacket ( PackedData *pPacket ) 
{
	if ( type == _SHOP_CRYSTALS ) {
		pPacket->putWord ( 0 );
	} else {
		BCarryable *pCarry = (BCarryable *)object->getBase ( _BCARRY ); 

		if ( pCarry ) {
			pPacket->putWord ( pCarry->weight );
		} else {
			pPacket->putWord ( 0 );
		}
	}
}

const char *ShopObject::description ( void )
{
	BDescribed *bdescribed = (BDescribed *)object->getBase ( _BDESCRIBED );
	const char *retVal = 0;

	if ( bdescribed ) {
		if ( bdescribed->shopText )
			retVal = bdescribed->shopText;
		else
			retVal = bdescribed->text;
	}

	return retVal;
}

// 
// BShop: This is the base class that represents a shop.
//

BShop::BShop ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BSHOP;
	inventory.setName ( self->name );
	inventory.shop = this;
	buyMarkup = 50;
	sellMarkup = 110;
	currency = Gold;
}

BShop::~BShop()
{
	items.release();
}

void BShop::copy ( WorldObjectBase *theBase )
{
	BShop *base = (BShop *)theBase;

	currency = base->currency;
	sellMarkup = base->sellMarkup;
	buyMarkup = base->buyMarkup;
	inventory.shop = this;

	LinkedElement *element = base->inventory.elements.head();

	while ( element ) {
		ShopItem *item = (ShopItem *)element->ptr();
		ShopItem *clone = item->clone ( this );

		inventory.addItem ( clone );

		element = element->next();
	}

	items.copy ( &base->items );
}

void BShop::buildShopPacket ( PackedData *packet, WorldObject *context )
{
	inventory.buildPacket ( packet, context );
}

void BShop::buildWeightPacket ( PackedData *pPacket )
{
	inventory.buildWeightPacket ( pPacket );
}

void BShop::addItem ( ShopItem *item )
{
	items.add ( item );
	item->shop = this;
}

ShopItem *BShop::at ( int index )
{
	return (ShopItem *)items.at ( index );
}

int BShop::price ( WorldObject *object )
{
	int retVal = 0;

	if ( object ) {
		if ( object->physicalState & _STATE_WORTHLESS ) 
			return 0;

		if ( object->physicalState & _STATE_WHOLESALE ) {
			retVal = object->netWorth();
		} else {
			int value = object->netWorth ( buyMarkup );

			int theValue = (object->value * buyMarkup) / 100;

			BContainer *base = (BContainer *)object->getBase ( _BCONTAIN );

			if ( base && base->contents.size() )
				value -= theValue;

			retVal = value;
		}
	}

	return retVal;
}

int BShop::buy ( WorldObject *object, WorldObject *seller )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !buyMarkup )
		return _ERR_TOO_EXPENSIVE;

	if ( object && seller ) {
		if ( object->objectWornOn == -1 && object->objectWieldedOn == -1 ) {
			int price = this->price ( object );

			if ( !seller->player->checkAccess( _ACCESS_BUY_STORE ) )
				seller->value += price;

			BContainer *bcontain = (BContainer *)object->getBase ( _BCONTAIN );

			if ( bcontain && bcontain->contents.size() ) {
				LinkedElement *element = bcontain->contents.head();

				while ( element ) {
					WorldObject *obj = (WorldObject *)element->ptr();
					element = element->next();

					if ( !(obj->physicalState & _STATE_WORTHLESS) )
						roomMgr->destroyObj ( obj, TRUE, __FILE__, __LINE__ );
				}
			} else {
				roomMgr->destroyObj ( object, TRUE, __FILE__, __LINE__ );
			}

			retVal = _WO_ACTION_HANDLED;

		} else {
			retVal = _ERR_MUST_REMOVE;
		}
	}

	return retVal;
}

int BShop::toss ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( object ) {

		BContainer *bcontain = (BContainer *)object->getBase ( _BCONTAIN );
		
		if ( bcontain && bcontain->contents.size() ) {
			LinkedElement *element = bcontain->contents.head();
		
			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();
//				if ( !(obj->physicalState & _STATE_WORTHLESS) )
  				roomMgr->destroyObj ( obj, TRUE, __FILE__, __LINE__ );
			}
		} else {
				roomMgr->destroyObj ( object, TRUE, __FILE__, __LINE__ );
		}
		retVal = _WO_ACTION_HANDLED;
	}
	return retVal;
}

void BShop::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BShop)\n" );
}
