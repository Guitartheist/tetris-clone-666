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

void drawAttackGrid(Grid* toDraw,SDL_Surface* screen)
{
    int x,y;
    SDL_Rect gridLine;
    gridLine.y=0;
    gridLine.w=1;
    gridLine.h=GRIDYSIZE*BLOCKSIZE/4;

    SDL_Rect screenRect = {0,0,GRIDXSIZE*BLOCKSIZE/4,GRIDYSIZE*BLOCKSIZE/4};
    //draw background

    SDL_FillRect(screen,&screenRect,SDL_MapRGB(screen->format,0,0,0));

    //draw blocks
    for (x=0; x<GRIDXSIZE; x++)
    {
        gridLine.x=x*BLOCKSIZE/4;
        //draw a vertical line
        SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
        for (y=0; y<GRIDYSIZE; y++)
            if (toDraw->grid[x][y].position.x>=0)
                drawBlock( &toDraw->grid[x][y], screen);
    }

    //
    gridLine.x=GRIDXSIZE*BLOCKSIZE/4;
    SDL_FillRect(screen,&gridLine,SDL_MapRGB(screen->format,127,127,127));
    gridLine.x=0;
    gridLine.y=GRIDYSIZE*BLOCKSIZE/4;
    gridLine.w=GRIDXSIZE*BLOCKSIZE/4;
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

int battleLines(Grid *grid, Grid *attackBuffer, int coords[8])
{
    int x,y;
    int numLines=0;

    //drop coordinates down to floor of attackBuffer
    int lowestY = 20;
    int i=1;
    while (i<8)
    {
        if (coords[i]<lowestY)
            lowestY = coords[i];
        i+=2;
    }
    coords[1] += lowestY;
    coords[3] += lowestY;
    coords[5] += lowestY;
    coords[7] += lowestY;

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

            /*

            If numLines>1:

            Move attackBuffer lines up by one

            Copy cleared game grid line to bottom of attackBuffer

            Always:

            Shift all game grid lines above current line down by one

            */

            if (numLines > 1)
            {

            int attackY;

            for (attackY=0; attackY>GRIDYSIZE-1; attackY++)
            {
                for (x=0; x<GRIDXSIZE; x++)
                {
                    attackBuffer->grid[x][attackY]=raiseBlock(attackBuffer->grid[x][attackY+1]);
                }
            }

            for (x=0; x<GRIDXSIZE; x++)
            {
                attackBuffer->grid[x][GRIDYSIZE-1]=grid->grid[x][y];
                attackBuffer->grid[x][GRIDYSIZE-1].position.y=GRIDYSIZE*BLOCKSIZE-BLOCKSIZE;
            }

            }

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

    //carve out empty blocks in attackBuffer where the scoring piece settled
    for (i=0;i<numLines;i++)
    {
        if (coords[1]==i)
            attackBuffer->grid[coords[0]][i].position.x=-BLOCKSIZE;
        if (coords[3]==i)
            attackBuffer->grid[coords[2]][i].position.x=-BLOCKSIZE;
        if (coords[5]==i)
            attackBuffer->grid[coords[4]][i].position.x=-BLOCKSIZE;
        if (coords[7]==i)
            attackBuffer->grid[coords[6]][i].position.x=-BLOCKSIZE;
    }

    return numLines;
}

//Remove the bottom line from source and push it up into target
void pushLine(Grid *target, Grid *source)
{
    int x,y;
    Block buffer[GRIDXSIZE];

    /*

    Push all lines in target upward by one leaving bottom line blank

    Remove bottom line from source and drop the rest of the grid down

    Replace bottom line of target with bottom line copied from source

    */

    for (y=0; y>GRIDYSIZE-1; y++)
    {
        for (x=0; x<GRIDXSIZE; x++)
        {
            target->grid[x][y]=raiseBlock(target->grid[x][y+1]);
        }
    }
    for (x=0; x<GRIDXSIZE; x++)
    {
        buffer[x]=source->grid[x][GRIDYSIZE-1];
        source->grid[x][GRIDYSIZE-1].position.x=-BLOCKSIZE;
    }
    for (y=GRIDYSIZE-1; y>0; y--)
    {
        for (x=0; x<GRIDXSIZE; x++)
        {
            source->grid[x][y]=dropBlock(source->grid[x][y-1]);
        }
    }
    //drop the target grids lines back to the floor of the grid if source grid was empty
    scoreLines(target);
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
