#ifndef RECIPEITEM_H
#define RECIPEITEM_H

class RecipeItem
{
public:
    String seasoningID;
    int amount;
    int color[3] = {0, 0, 0};
    RecipeItem() {}
    RecipeItem(String x, int y, int z[3])
    {
        seasoningID = x;
        amount = y;
        for (int i = 0; i < 3; i++)
        {
            color[i] = z[i];
        }
    }
};

#endif