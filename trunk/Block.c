#include <SDL/SDL.h>
#include "Block.h"

void setBlock(Block *block, int x, int y, SDL_PixelFormat *format, enum Color color)
{
    block->position.x=x;
    block->position.y=y;
    block->position.w=BLOCKSIZE;
    block->position.h=BLOCKSIZE;
    switch (color)
    {
    case RED:
        block->color = SDL_MapRGB(format,255,0,0);
        break;
    case BLUE:
        block->color = SDL_MapRGB(format,0,0,255);
        break;
    case GREEN:
        block->color = SDL_MapRGB(format,0,255,0);
        break;
    case YELLOW:
        block->color = SDL_MapRGB(format,255,255,0);
        break;
    case CYAN:
        block->color = SDL_MapRGB(format,0,255,255);
        break;
    case ORANGE:
        block->color = SDL_MapRGB(format,255,140,0);
        break;
    case PURPLE:
        block->color = SDL_MapRGB(format,255,0,255);
        break;
    }
}

void setBlockFromBlock(Block* dest, Block* const source)
{
    dest->position=source->position;
    dest->color=source->color;
}

void drawBlock(Block *block,SDL_Surface *screen)
{
    SDL_Rect rect;
    rect.x=block->position.x+1;
    rect.y=block->position.y+1;
    rect.h=block->position.h-1;
    rect.w=block->position.w-1;
    SDL_FillRect(screen,&rect,block->color);
}

void drawBlockOutline(Block* block,SDL_Surface* screen)
{
    SDL_Rect rect;
    rect.x=block->position.x+1;
    rect.y=block->position.y+1;
    rect.h=block->position.h-1;
    rect.w=block->position.w-1;
    SDL_FillRect(screen,&rect,SDL_MapRGB(screen->format,255,255,255));
}
