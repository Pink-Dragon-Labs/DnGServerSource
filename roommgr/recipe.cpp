//
// recipe.cpp
//
// Recipe code.
//
// author: Stephen Nichols
//

#include "roommgr.hpp"

RecipeList gRecipeList;

RecipeList::RecipeList()
{
}

RecipeList::~RecipeList()
{
}

Recipe *RecipeList::matches ( WorldObject *container )
{
	BContainer *bcontain = (BContainer *)container->getBase ( _BCONTAIN );

	if ( !bcontain )
		return NULL;

	LinkedElement *element = head();

	while ( element ) {
		Recipe *recipe = (Recipe *)element->ptr();

		if ( recipe->matches ( container ) )
			return recipe;

		element = element->next();
	}

	return NULL;
}

Recipe::Recipe()
{
	difficulty = 1;
	minManaCost = 0;
	size = 0;
	memset ( ingredients, 0, sizeof ( ingredients ) );
	object = NULL;
}

Recipe::~Recipe()
{
	for ( int i=0; i<size; i++ )
		free ( ingredients[i] ); 
}

int Recipe::matches ( WorldObject *container )
{
	BContainer *bcontain = (BContainer *)container->getBase ( _BCONTAIN );

	if ( !bcontain )
		return 0;

	LinkedElement *element = bcontain->contents.head();
	int matched[_MAX_INGREDIENTS];
	memset ( matched, 0, sizeof ( matched ) ); 

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();

		for ( int i=0; i<size; i++ ) {
			if ( !strcmp ( obj->classID, ingredients[i] ) ) {
				if ( matched[i] )
					return 0;

				matched[i] = 1;	
			}
		}

		element = element->next();
	}

	for ( int j=0; j<size; j++ ) {
		if ( !matched[j] )
			return 0;
	}

	return 1;
}

int Recipe::doit ( WorldObject *ego )
{
	if ( object ) {
		WorldObject *obj = ego->addObject ( object->classID );
		obj->makeVisible ( 1 );
	}

	return 1;
}

void Recipe::addIngredient ( char *str )
{
	ingredients[size] = strdup ( str );
	size++;
}
