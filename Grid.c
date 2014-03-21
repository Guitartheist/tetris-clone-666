#include "Block.h"
#include "Grid.h"
#include <SDL/SDL.h>

void drawGrid(Grid* toDraw,SDL_Surface* screen)
{
    int x,y;
    SDL_Rect gridLine;
    gridLine.y=0;
    gridLine.w=1;
    gridLine.h=GRIDYSIZE*BLOCKSIZE;

    SDL_Rect screenRect = {0,0,GRIDXSIZE*BLOCKSIZE,GRIDYSIZE*BLOCKSIZE};
    //draw background

    SDL_FillRect(screen,&screenRect,SDL_MapRGB(screen->format,0,0,0));

    //draw blocks
    for (x=0; x<GRIDXSIZE; x++)
    {
        gridLine.x=x*BLOCKSIZE;
        //draw a vertical line
        SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
        for (y=0; y<GRIDYSIZE; y++)
            if (toDraw->grid[x][y].position.x>=0)
                drawBlock( &toDraw->grid[x][y], screen);
    }

    //Draw horizontal lines
    gridLine.x=GRIDXSIZE*BLOCKSIZE;
    SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
    gridLine.x=0;
    gridLine.y=GRIDYSIZE*BLOCKSIZE;
    gridLine.w=GRIDXSIZE*BLOCKSIZE;
    gridLine.h=1;
    SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
    gridLine.y=2*BLOCKSIZE;
    SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
}

//Function needs to count completed lines, clear Blocks from those lines,
//and collapse grid to fill space vacated by completed lines.
int scoreLines(Grid* grid)
{
    int x,y;
    int numLines=0;

    for (y=GRIDYSIZE-1; y>=0; y--)
    {
        //check current horizontal line for solidity
        for (x=0; x<GRIDXSIZE; x++)
        {
            if (grid->grid[x][y].position.x>=0)
                ;
            else
                break;
        }
        //if current horizontal line is completely empty
        if (x==GRIDXSIZE)
        {
            //score and clear current line
            numLines++;

            //Save current value of y in case of multiple line clears
            int oldY=y;

            //Shift all lines above current line down by one
            while (y>0)
            {
                for (x=0; x<GRIDXSIZE; x++)
                    grid->grid[x][y]=dropBlock(grid->grid[x][y-1]);
                y--;
            }

            //Go back 1 step to account for multiple line clears
            y=oldY+1;
        }
    }

    return numLines;
}

//Remove the bottom line from source and push it up into target
void pushLine(Grid *target, SDL_Surface *screen)
{
    int x,y;
    int blankSpot = roll(GRIDXSIZE);

    /*

    Push all lines in target upward by one leaving bottom line blank

    Replace bottom line with a white line containing one blank space

    */

    for (y=0; y<GRIDYSIZE-1; y++)
    {
        for (x=0; x<GRIDXSIZE; x++)
        {
            target->grid[x][y]=raiseBlock(target->grid[x][y+1]);
        }
    }

    for (x=0; x<GRIDXSIZE; x++)
    {
        Block whiteBlock = { x*BLOCKSIZE,BLOCKSIZE*(GRIDYSIZE-1),BLOCKSIZE,BLOCKSIZE,SDL_MapRGB(screen->format,255,255,255) };
        if (x!=blankSpot)
            setBlockFromBlock(&target->grid[x][y],&whiteBlock);
        else
            target->grid[x][y].position.x=-BLOCKSIZE;
    }
}

Block dropBlock(Block block)
{
    block.position.y+=BLOCKSIZE;
    return block;
}

Block raiseBlock(Block block)
{
    block.position.y-=BLOCKSIZE;
    return block;
}

void clearGrid(Grid* toClear)
{
    int x,y;
    for (x=0; x<GRIDXSIZE; x++)
        for (y=GRIDYSIZE-1; y>=0; y--)
            toClear->grid[x][y].position.x=-BLOCKSIZE;
}
