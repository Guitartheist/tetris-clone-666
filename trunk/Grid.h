#include "Block.h"
#include <SDL/SDL.h>

#ifndef GRID_H
#define GRID_H

#define GRIDXSIZE 10
#define GRIDYSIZE 22

/**
Top of grid is 0, bottom line is GRIDYSIZE-1
Left of grid is 0, far right is GRIDXSIZE-1
*/

typedef struct Grid
{
    Block grid[GRIDXSIZE][GRIDYSIZE];
} Grid;

void drawGrid(Grid*,SDL_Surface*);
int scoreLines(Grid*); //clear completey filled x-axis lines, return number of lines cleared
int battleLines(Grid *grid, Grid *attackBuffer); //clear lines and store them in attackBuffer, return number of lines cleared
void pushLine(Grid *target, Grid *source); //Remove the bottom line from source and push it up into target
void clearGrid(Grid*);
Block dropBlock(Block);
Block raiseBlock(Block);

#endif // GRID_H
