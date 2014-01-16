#ifndef BLOCK_H
#define BLOCK_H

#include <SDL/SDL.h>

#define BLOCKSIZE 16

enum Color{RED,BLUE,CYAN,PURPLE,YELLOW,ORANGE,GREEN};

typedef struct Block
{
    SDL_Rect position;
    Uint32 color;
} Block;

void setBlock(Block*,int,int,SDL_PixelFormat*,enum Color);
void setBlockFromBlock(Block*,Block* const);
void drawBlock(Block*,SDL_Surface*);
void drawBlockOutline(Block*,SDL_Surface*);

#endif // BLOCK_H
