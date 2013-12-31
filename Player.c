#include "DrawText.h"
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
//    Player - a struct containing the necessary pieces and grid
//                                                                     
///////////////////////////////////////////////////////////////////////
void drawGame(Player player, SDL_Surface *screen)
{
    char scoreString[10];

    // DRAWING STARTS HERE

    // DRAW GRID AND ACTIVE PIECE

    drawGrid(&player.grid,screen);
    drawPiece(&player.active,screen);

    // DRAW NEXT 5 PIECES
    SDL_Rect backgroundRect = {BLOCKSIZE*GRIDXSIZE*1.5,BLOCKSIZE,BLOCKSIZE*5,BLOCKSIZE*20};
    SDL_FillRect(screen,&backgroundRect,SDL_MapRGB(screen->format,0,0,0));
    drawString("Next 5 Pieces:",screen,BLOCKSIZE*GRIDXSIZE*1.5-(BLOCKSIZE),0);
    Piece nextFive[5];
    createPiece(&nextFive[0],getPiece(player.pieces),BLOCKSIZE*GRIDXSIZE*1.6+BLOCKSIZE,BLOCKSIZE*2);
    createPiece(&nextFive[1],getPiece(player.pieces+1),BLOCKSIZE*GRIDXSIZE*1.6+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4));
    createPiece(&nextFive[2],getPiece(player.pieces+2),BLOCKSIZE*GRIDXSIZE*1.6+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*2);
    createPiece(&nextFive[3],getPiece(player.pieces+3),BLOCKSIZE*GRIDXSIZE*1.6+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*3);
    createPiece(&nextFive[4],getPiece(player.pieces+4),BLOCKSIZE*GRIDXSIZE*1.6+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*4);
    int i;
    for (i=0; i<5; i++)
        drawPiece(&nextFive[i],screen);

    //Draw Scoring information

    int xOffset = BLOCKSIZE*GRIDXSIZE+1;

    drawString("Level:",screen,xOffset,0);
    sprintf(scoreString,"%i",player.lines/10);
    drawString(scoreString,screen,xOffset,CHARHEIGHT);
    drawString("Score:",screen,xOffset,CHARHEIGHT*2);
    sprintf(scoreString,"%i",player.score);
    drawString(scoreString,screen,xOffset,CHARHEIGHT*3);
    drawString("Lines:",screen,xOffset,CHARHEIGHT*4);
    sprintf(scoreString,"%i",player.lines);
    drawString(scoreString,screen,xOffset,CHARHEIGHT*5);

    //Draw hold piece information

    if (player.swapped>0)
    {
        drawString("Holding: ",screen,xOffset,CHARHEIGHT*10);
        createPiece(&player.held,player.held.type,xOffset+BLOCKSIZE*2,CHARHEIGHT*14);
        drawPiece(&player.held,screen);
    }

    // DRAWING ENDS HERE
}

void initPlayer(Player* player)
{
    spawnPiece(&player->active,getPiece(0));
    player->pieces=1;
    player->lines=0;
    player->score=0;
    clearGrid(&player->grid);
    createPiece(&player->held,0,BLOCKSIZE,BLOCKSIZE*7.5);
    player->swappable = 1;
    player->swapped = 0;
    player->controller.hardDrop = SDLK_SPACE;
    player->controller.moveDown = SDLK_DOWN;
    player->controller.moveLeft = SDLK_LEFT;
    player->controller.moveRight = SDLK_RIGHT;
    player->controller.rotateLeft = SDLK_z;
    player->controller.rotateRight = SDLK_UP;
    player->controller.pause = SDLK_p;
    player->controller.hold = SDLK_LSHIFT;
}
