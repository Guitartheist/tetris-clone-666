#include <SDL/SDL.h>
#include "Grid.h"
#include "Piece.h"

#ifndef PLAYER_H
#define PLAYER_H

enum Action{NOTHING,ROTATERIGHT,ROTATELEFT,MOVERIGHT,MOVELEFT,MOVEDOWN,DROP};

typedef struct Player
{
    int pieces;//number of pieces dropped so far
} Player;

void drawGame(Grid*,Piece*,enum Action,SDL_Surface*,int*,int);

#endif // PLAYER_H
