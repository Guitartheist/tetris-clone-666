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

    //
    gridLine.x=GRIDXSIZE*BLOCKSIZE;
    SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
    gridLine.x=0;
    gridLine.y=GRIDYSIZE*BLOCKSIZE;
    gridLine.w=GRIDXSIZE*BLOCKSIZE;
    gridLine.h=1;
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

Block dropBlock(Block block)
{
    block.position.y+=BLOCKSIZE;
    return block;
}

void clearGrid(Grid* toClear)
{
    int x,y;
    for (x=0; x<GRIDXSIZE; x++)
        for (y=GRIDYSIZE-1; y>=0; y--)
            toClear->grid[x][y].position.x=-BLOCKSIZE;
}
