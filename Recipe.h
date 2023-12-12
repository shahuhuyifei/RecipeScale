#ifndef RECIPE_H
#define RECIPE_H

#include "RecipeItem.h"

class Recipe
{
public:
    String recipeID;
    RecipeItem recipeItem_list[SIZE];

    Recipe() {}

    Recipe(String x, RecipeItem y[SIZE])
    {
        recipeID = x;
        // *recipeItem_list = *y;
        for (int i = 0; i < SIZE; i++)
        {
            recipeItem_list[i] = y[i];
        }
    }
};

#endif