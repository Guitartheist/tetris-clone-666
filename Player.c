#include "Player.h"

///////////////////////////////////////////////////////////////////////
//
// Function: drawGame                                         
//                                                                   
// Description:
//    Draw a game to the provided surface after performing the specified
//    action and updating the provided score.           
//                                                                
// Parameters:  
//    grid: a tetris grid
//    piece: the active tetromino
//    action: the action to be performed before drawing
//    surface: the SDL_Surface to draw upon
//    score: the current score of the game, modified if a piece is
//        locked into place on the grid.
//    level: the current level of difficulty, used as a score multiplier       
//                                                       
// Returns:  
//    returnVal : description of what is returned to caller
//                                                                     
///////////////////////////////////////////////////////////////////////
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
