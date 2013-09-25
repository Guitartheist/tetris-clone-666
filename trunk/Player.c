#include "Player.h"

void drawGame(Grid* grid, Piece* piece, enum Action action, SDL_Surface* screen,int *score, int level)
{
    switch (action)
    {
    case NOTHING:
        break;
    case ROTATERIGHT:
        rotatePieceRight(piece,grid);
        break;
    case ROTATELEFT:
        rotatePieceLeft(piece,grid);
        break;
    case MOVERIGHT:
        movePieceRight(piece,grid);
        break;
    case MOVELEFT:
        movePieceLeft(piece,grid);
        break;
    case MOVEDOWN:
        break;
        movePieceDown(piece,grid);
    case DROP:
        dropPiece(piece,grid,screen,score,level);
        break;
    }

    // DRAWING STARTS HERE

    // draw background
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    // draw piece
    drawPiece(piece,screen);

    // draw grid
    drawGrid(grid,screen);

    // DRAWING ENDS HERE
}
