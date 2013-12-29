#include <SDL/SDL.h>
#include "Grid.h"
#include "Piece.h"

#ifndef PLAYER_H
#define PLAYER_H

typedef struct Player
{
    int score;
    int lines;
    int pieces;//number of pieces dropped so far
    Piece active;
    Piece held;
    Grid grid;
    Uint8 swapped; //set to 1 when hold is used for the first time
    Uint8 swappable; //set to 0 when hold is used, 1 when hold piece is locked into the grid
} Player;

void drawGame(Player,SDL_Surface*);
void initPlayer(Player*);

#endif // PLAYER_H
