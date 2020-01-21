//
// recipe.hpp
//
// Recipe code.
//
// author: Stephen Nichols
//

#ifndef _RECIPE_HPP_
#define _RECIPE_HPP_

#include "../global/system.hpp"
#undef new
class Recipe;



class RecipeList : public LinkedList {
public:
	RecipeList();
	virtual ~RecipeList();

	// return the Recipe, if any, that matches the passed container
	Recipe *matches ( WorldObject *container );

	void *operator new ( size_t size, char* file, int nLine ) { return db_malloc ( size, file, nLine ); }

	void operator delete ( void *ptr ) { free ( ptr ); }
};

#define new new( __FILE__, __LINE__ )

#define _MAX_INGREDIENTS	10

class Recipe : public ListObject
{
public:
	Recipe();
	virtual ~Recipe();

	// do the objects in the given container match our recipe requirements?
	int matches ( WorldObject *container );

	// do whatever this recipe requires
	int doit ( WorldObject *ego );

	// add an ingredient
	void addIngredient ( char *str );

	// difficulty level of this recipe
	int difficulty;

	// minimum mana cost 
	int minManaCost;

	// ingredient properties
	int size;
	char *ingredients[_MAX_INGREDIENTS];

	// properties for Object making recipe
	WorldObject *object;
};

extern RecipeList gRecipeList;

#endif
