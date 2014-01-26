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
    drawString("Next Five:",screen,BLOCKSIZE*GRIDXSIZE*1.5-(BLOCKSIZE),0);
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

    //Draw where active piece will lock if player hard drops

    Piece potentialLock;
    createPiece(&potentialLock,player.active.type,player.active.x,player.active.y);
    potentialLock.direction = player.active.direction;
    while (movePieceDown(&potentialLock,&player.grid));
    drawPieceOutline(&potentialLock,screen);

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
    player->fastDrop = 0;
    player->fastSlide = 0;
    // millisecond tracking variable
    player->ticks=SDL_GetTicks();
    player->startTime = SDL_GetTicks();
    player->totalTime = 0;

    player->controller.keyboard = KEYBOARD;
    player->controller.hardDrop = SDLK_SPACE;
    player->controller.moveDown = SDLK_DOWN;
    player->controller.moveLeft = SDLK_LEFT;
    player->controller.moveRight = SDLK_RIGHT;
    player->controller.rotateLeft = SDLK_z;
    player->controller.rotateRight = SDLK_UP;
    player->controller.pause = SDLK_p;
    player->controller.hold = SDLK_LSHIFT;
    player->controller.quit = SDLK_ESCAPE;
    player->controller.attackHorizontal = SDLK_1;
    player->controller.attackVertical = SDLK_2;
    player->controller.attackDiagonal = SDLK_3;
}

///////////////////////////////////////////////////////////////////////
//
// Function: configureSinglePlayerControls                                    
//                                                                   
// Description:
//    Take the player through a controller configuration
//                     
// Parameters:  
//    player - the player to whom the controllers will be defined
//    screen - the surface upon which to draw prompts and progress bars
//                                                                     
///////////////////////////////////////////////////////////////////////

void configureSinglePlayerControls(Player *player, SDL_Surface *screen)
{
    SDL_Event event;
    int i=0;
    int keys[9];

    SDL_Rect endRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,CHARHEIGHT*4};

    SDL_FillRect(screen,&endRect,SDL_MapRGB(screen->format,0,0,0));

    while (i<9)
    {
        switch (i)
        {
        case 0:
            drawString("Hold rotate left",screen,0,0);
            break;

        case 1:
            drawString("Hold rotate right",screen,0,0);
            break;

        case 2:
            drawString("Hold hard drop   ",screen,0,0);
            break;

        case 3:
            drawString("Hold hold/swap   ",screen,0,0);
            break;

        case 4:
            drawString("Hold pause       ",screen,0,0);
            break;

        case 5:
            drawString("Hold quit        ",screen,0,0);
            break;

        case 6:
            drawString("Hold vertical ATK",screen,0,0);
            break;

        case 7:
            drawString("Hold horizon ATK  ",screen,0,0);
            break;

        case 8:
            drawString("Hold diagonal ATK",screen,0,0);
            break;
        }

        SDL_Flip(screen);

        while (SDL_PollEvent(&event))
        {
            // ignore events from the a different controller than what the player started with
            if (i>0&&player->controller.keyboard==KEYBOARD)
            {
                if (event.type==SDL_QUIT||event.type==SDL_KEYDOWN)
                {
                    //move on to the switch statement below and skip move left/right/down
                }
                else
                    //ignore current event
                    break;
            }

            // check for messages
            switch (event.type)
            {
            // exit if the window is closed
            case SDL_QUIT:
                exit(0);
                break;

            // check for keypresses
            case SDL_KEYDOWN:
                keys[i]=event.key.keysym.sym;
                i++;
                break;

            case SDL_JOYBUTTONDOWN:
                if (i>0)
                {
                    if (event.jbutton.which != player->controller.keyboard)
                    {
                        break;
                    }
                }
                else
                {
                    player->controller.keyboard = event.jbutton.which;
                }
                keys[i]=event.jbutton.button;
                printf("%d | %d\n",i,keys[i]);
                i++;
                break;

            }// end switch
        }
    }
    player->controller.rotateLeft = keys[0];
    player->controller.rotateRight = keys[1];
    player->controller.hardDrop = keys[2];
    player->controller.hold = keys[3];
    player->controller.pause = keys[4];
    player->controller.quit = keys[5];
    player->controller.attackVertical = keys[6];
    player->controller.attackHorizontal = keys[7];
    player->controller.attackDiagonal = keys[8];

    drawString("Configuration Complete",screen,0,0);
    SDL_Flip(screen);
}

int singleControllerProcess(Player *player,SDL_Surface *screen)
{
    SDL_Event event;
    Piece bufferPiece;
    while (SDL_PollEvent(&event))
    {

        // exit if the window is closed
        if (event.type == SDL_QUIT)
        {
            exit(0);
        }

        if (player->controller.keyboard==KEYBOARD)
        {

            // check for messages
            switch (event.type)
            {

            case SDL_KEYUP:
                if (event.key.keysym.sym == player->controller.moveDown)
                    player->fastDrop = 0;

                else if (event.key.keysym.sym == player->controller.moveLeft)
                {
                    if (player->fastSlide == -2)
                        player->fastSlide = 1;
                    else
                        player->fastSlide = 0;
                }

                else if (event.key.keysym.sym == player->controller.moveRight)
                {
                    if (player->fastSlide == 2)
                        player->fastSlide = -1;
                    else
                        player->fastSlide = 0;
                }

                break;

            // check for keypresses
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == player->controller.quit)
                {
                    player->totalTime += SDL_GetTicks() - player->startTime;
                    if (singlePlayerPauseMenu(*player,screen))
                        return 1;
                    player->startTime = SDL_GetTicks();
                }

                else if (event.key.keysym.sym == player->controller.pause)
                {
                    player->totalTime += SDL_GetTicks() - player->startTime;
                    if (singlePlayerPauseMenu(*player,screen))
                        return 1;
                    player->startTime = SDL_GetTicks();
                }

                if (event.key.keysym.sym == player->controller.hold) //swap holdpiece
                {
                    if (!player->swapped)
                    {
                        player->swapped=1;
                        player->held.type=player->active.type;
                        spawnPiece(&player->active,getPiece(player->pieces));
                        player->pieces++;
                    }
                    else if (player->swappable)
                    {
                        bufferPiece.type=player->active.type;
                        player->active.type=player->held.type;
                        player->held.type=bufferPiece.type;
                        spawnPiece(&player->active,player->active.type);
                        player->swappable=0;
                    }
                }
                else if (event.key.keysym.sym == player->controller.rotateLeft)
                    rotatePieceLeft(&player->active,&player->grid);

                else if (event.key.keysym.sym == player->controller.rotateRight)
                    rotatePieceRight(&player->active,&player->grid);

                else if (event.key.keysym.sym == player->controller.moveDown)
                {
                    player->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    player->score+=movePieceDown(&player->active,&player->grid);
                    player->fastDrop=1;
                }
                else if (event.key.keysym.sym == player->controller.moveLeft)
                {
                    player->slideTime = SDL_GetTicks();
                    movePieceLeft(&player->active,&player->grid);
                    if (player->fastSlide == 0)
                        player->fastSlide = -1;
                    else if (player->fastSlide == 1)
                        player->fastSlide = -2;
                }
                else if (event.key.keysym.sym == player->controller.moveRight)
                {
                    player->slideTime = SDL_GetTicks();
                    movePieceRight(&player->active,&player->grid);
                    if (player->fastSlide == 0)
                        player->fastSlide = 1;
                    else if (player->fastSlide == -1)
                        player->fastSlide = 2;
                }
                else if (event.key.keysym.sym == player->controller.hardDrop)
                {
                    if (!scoreDrop(player,screen))
                    {
                        return 1;
                    }
                    player->ticks=SDL_GetTicks();
                }
                break;
            }
            }// end switch
        }//end keyboard handling



        //joystick handling
        else
        {
            switch (event.type)
            {

            case SDL_JOYBUTTONDOWN:
                //ignore input from the controllers not assigned to this player
                if (player->controller.keyboard!=event.jbutton.which) break;

                if (event.jbutton.button == player->controller.quit)
                {
                    player->totalTime += SDL_GetTicks() - player->startTime;
                    if (singlePlayerPauseMenu(*player,screen))
                        return 1;
                    player->startTime = SDL_GetTicks();
                }

                else if (event.jbutton.button == player->controller.pause)
                {
                    player->totalTime += SDL_GetTicks() - player->startTime;
                    if (singlePlayerPauseMenu(*player,screen))
                        return 1;
                    player->startTime = SDL_GetTicks();
                }

                if (event.jbutton.button == player->controller.hold) //swap holdpiece
                {
                    if (!player->swapped)
                    {
                        player->swapped=1;
                        player->held.type=player->active.type;
                        spawnPiece(&player->active,getPiece(player->pieces));
                        player->pieces++;
                    }
                    else if (player->swappable)
                    {
                        bufferPiece.type=player->active.type;
                        player->active.type=player->held.type;
                        player->held.type=bufferPiece.type;
                        spawnPiece(&player->active,player->active.type);
                        player->swappable=0;
                    }
                }
                else if (event.jbutton.button == player->controller.rotateLeft)
                    rotatePieceLeft(&player->active,&player->grid);

                else if (event.jbutton.button == player->controller.rotateRight)
                    rotatePieceRight(&player->active,&player->grid);

                else if (event.jbutton.button == player->controller.hardDrop)
                {
                    if (!scoreDrop(player,screen))
                    {
                        return 1;
                    }
                    player->ticks=SDL_GetTicks();
                }

                break;

            case SDL_JOYHATMOTION:

                //ignore input from the controllers not assigned to this player
                if (player->controller.keyboard!=event.jhat.which) break;

                if ( ! (event.jhat.value & SDL_HAT_DOWN))
                {
                    player->fastDrop = 0;
                }
                if ( ! (event.jhat.value & SDL_HAT_LEFT) && ! (event.jhat.value & SDL_HAT_RIGHT))
                {
                    player->fastSlide = 0;
                }

                if (event.jhat.value & SDL_HAT_DOWN)
                {
                    player->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    if (!player->fastDrop)
                    {
                        player->score+=movePieceDown(&player->active,&player->grid);
                        player->fastDrop=1;
                    }
                }

                if (event.jhat.value & SDL_HAT_LEFT)
                {
                    player->slideTime = SDL_GetTicks();
                    if (player->fastSlide == 0)
                    {
                        player->fastSlide = -1;
                        movePieceLeft(&player->active,&player->grid);
                    }
                }

                if (event.jhat.value & SDL_HAT_RIGHT)
                {
                    player->slideTime = SDL_GetTicks();
                    if (player->fastSlide == 0)
                    {
                        player->fastSlide = 1;
                        movePieceRight(&player->active,&player->grid);
                    }
                }

                break;

            case SDL_JOYAXISMOTION:

                //ignore input from the controllers not assigned to this player
                if (player->controller.keyboard!=event.jaxis.which) break;

                //DOWN
                if (event.jaxis.axis % 2 == 1 && event.jaxis.value > 3200 )
                {
                    player->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    if (!player->fastDrop)
                    {
                        player->score+=movePieceDown(&player->active,&player->grid);
                        player->fastDrop=1;
                    }
                }

                //LEFT
                else if (event.jaxis.axis % 2 == 0 && event.jaxis.value < -3200)
                {
                    player->slideTime = SDL_GetTicks();
                    if (player->fastSlide == 0)
                    {
                        player->fastSlide = -1;
                        movePieceLeft(&player->active,&player->grid);
                    }
                }

                //RIGHT
                else if (event.jaxis.axis % 2 == 0 && event.jaxis.value > 3200)
                {
                    player->slideTime = SDL_GetTicks();
                    if (player->fastSlide == 0)
                    {
                        player->fastSlide = 1;
                        movePieceRight(&player->active,&player->grid);
                    }
                }

                //if axis is centered

                else if (event.jaxis.axis%2 == 1 && event.jaxis.value < 3200 && event.jaxis.value > -3200)
                {
                    player->fastDrop = 0;
                }
                else if (event.jaxis.axis%2 == 0 && event.jaxis.value < 3200 && event.jaxis.value > -3200)
                {
                    player->fastSlide = 0;
                }
                break;
            }
        }

        //end joystick handling
    }
    return 0;
}

int singlePlayerPauseMenu(Player player, SDL_Surface *screen)
{
    drawString("Quit - Quit to main menu",screen,screen->w/2-CHARWIDTH*10,screen->h/2-CHARHEIGHT);
    drawString("Any other key will resume",screen,screen->w/2-CHARWIDTH*10,screen->h/2);
    SDL_Flip(screen);

    SDL_Event event;
    while (1)
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == player.controller.quit)
                    return 1;
                else
                    return 0;
                break;

            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.button == player.controller.quit)
                    return 1;
                else
                    return 0;
                break;
            }
        }
}

//Return 0 if dropping the latest piece has ended the game
//Return 1 if game should continue
//Play different notes depending on how many lines have been cleared and what type of piece was used
//Spawn the next piece and allow the player to swap hold piece with active piece
int scoreDrop(Player *player, SDL_Surface *surface)
{
    int dropTest = dropPiece(&player->active,&player->grid,surface,&player->score,(player->lines/10)+1);
    if (dropTest<0)
        return 0;
    player->lines+=dropTest;
    spawnPiece(&player->active,getPiece(player->pieces));
    player->pieces+=1;
    player->swappable=1;
    return 1;
}
