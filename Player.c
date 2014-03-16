#include "DrawText.h"
#include "Sound.h"
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
    clearGrid(&player->attackBuffer);
    createPiece(&player->held,0,BLOCKSIZE,BLOCKSIZE*7.5);
    player->swappable = 1;
    player->swapped = 0;
    player->fastDrop = 0;
    player->fastSlide = 0;
    // millisecond tracking variable
    player->ticks=SDL_GetTicks();
    player->startTime = SDL_GetTicks();
    player->totalTime = 0;

    if (player->isActive==2)
        player->isActive=1;
}

void initControls(Player *player)
{
    player->isActive = 0;
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

    player->isActive = 1;
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

///////////////////////////////////////////////////////////////////////
//
// Function: configureMultiPlayerControls                                    
//                                                                   
// Description:
//    Take the players through a controller configuration
//                     
// Parameters:  
//    players[] - the players to whom the controllers will be defined
//    screen - the surface upon which to draw prompts and progress bars
//                                                                     
///////////////////////////////////////////////////////////////////////

void configureMultiPlayerControls(Player *player[4], SDL_Surface *screen)
{
    int playerController[4] = {-1,-1,-1,-1};

    SDL_Event event;
    int done = 0;
    int playerProgress[4]= {0,0,0,0};
    int i;
    int keys[4][7];

    for (i=0;i<4;i++)
    {
        initControls(player[i]);
    }

    SDL_Rect screenRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect1 = {0,0,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect2 = {BLOCKSIZE*GRIDXSIZE*2,0,BLOCKSIZE*GRIDXSIZE,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect3 = {0,BLOCKSIZE*GRIDYSIZE,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect4 = {BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};

    SDL_Rect endRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,CHARHEIGHT*4};

    SDL_FillRect(screen,&endRect,SDL_MapRGB(screen->format,0,0,0));

    SDL_Surface *quad[4];

    for (i=0; i<4; i++)
    {
        quad[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, BLOCKSIZE*GRIDXSIZE*2, BLOCKSIZE*GRIDYSIZE*2, 32,
                                       screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    }

    while (!done)
    {
        done=1;
        for (i=0; i<4; i++)
        {
            if (playerProgress[i]<7&&playerController[i]>=0)
                done=0;
            if (playerController[0]<0)
                done=0;
        }

        for (i=0; i<4; i++)
            switch (playerProgress[i])
            {
            case 0:
                drawString("Press rotate left",quad[i],0,0);
                break;

            case 1:
                drawString("Press rotate right",quad[i],0,0);
                break;

            case 2:
                drawString("Press hard drop   ",quad[i],0,0);
                break;

            case 3:
                drawString("Press hold/swap   ",quad[i],0,0);
                break;

            case 4:
                drawString("Press vertical ATK",quad[i],0,0);
                break;

            case 5:
                drawString("Press horizon ATK  ",quad[i],0,0);
                break;

            case 6:
                drawString("Press diagonal ATK",quad[i],0,0);
                break;

            case 7:
                drawString("Waiting on other players...",quad[i],0,0);
                break;
            }

        for (i=0; i<4; i++)
        {
            SDL_Flip(quad[i]);
        }

        SDL_BlitSurface(quad[0],&screenRect,screen,&playerRect1);
        SDL_BlitSurface(quad[1],&screenRect,screen,&playerRect2);
        SDL_BlitSurface(quad[2],&screenRect,screen,&playerRect3);
        SDL_BlitSurface(quad[3],&screenRect,screen,&playerRect4);

        SDL_Flip(screen);

        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
            // exit if the window is closed
            case SDL_QUIT:
                exit(0);
                break;

            // check for keypresses
            case SDL_KEYDOWN:

                //determine which player this button press is assigned to
                for (i=0; i<4; i++)
                {
                    if (player[i]->controller.keyboard==KEYBOARD&&playerController[i]>=0)
                        break;
                }
                //attach the keyboard to a particular player if not yet attached
                if (i>=4)
                {
                    for (i=0; i<4; i++)
                    {
                        if (playerController[i]<0)
                        {
                            playerController[i]=KEYBOARD;
                            player[i]->controller.keyboard=KEYBOARD;
                            player[i]->isActive = 1;
                            break;
                        }
                    }
                }
                if (playerProgress[i]<7&&i<4)
                {
                    keys[i][playerProgress[i]]=event.key.keysym.sym;
                    playerProgress[i]++;
                }
                break;

            case SDL_JOYBUTTONDOWN:
                //determine which player this button press is assigned to
                for (i=0; i<4; i++)
                {
                    if (player[i]->controller.keyboard==event.jbutton.which)
                        break;
                }
                //attach the joystick of the current button press to a particular player
                //if joystick not yet assigned to a player
                if (i>=4)
                {
                    for (i=0; i<4; i++)
                    {
                        if (playerController[i]<0)
                        {
                            playerController[i]=event.jbutton.which;
                            player[i]->controller.keyboard=event.jbutton.which;
                            player[i]->isActive = 1;
                            break;
                        }
                    }
                }
                if (playerProgress[i]<7&&i<4)
                {
                    keys[i][playerProgress[i]]=event.jbutton.button;
                    playerProgress[i]++;
                }
                break;

            }// end switch
        }
    }

    for (i=0; i<4; i++)
    {
        player[i]->controller.rotateLeft = keys[i][0];
        player[i]->controller.rotateRight = keys[i][1];
        player[i]->controller.hardDrop = keys[i][2];
        player[i]->controller.hold = keys[i][3];
        player[i]->controller.attackVertical = keys[i][4];
        player[i]->controller.attackHorizontal = keys[i][5];
        player[i]->controller.attackDiagonal = keys[i][6];
    }

    for (i=0; i<4; i++)
    {
        SDL_FreeSurface(quad[i]);
    }

    endRect.h = screen->h;
    endRect.w = screen->w;

    SDL_FillRect(screen,&endRect,SDL_MapRGB(screen->format,0,0,0));
    drawString("Configuration Complete!",screen,0,0);
    SDL_Flip(screen);
}

int multiControllerProcess(Player *player[4],SDL_Surface *screen)
{
    SDL_Event event;
    Piece bufferPiece;
    int i;

    while (SDL_PollEvent(&event))
    {

        // exit if the window is closed
        if (event.type == SDL_QUIT)
        {
            exit(0);
        }

        // check for messages
        switch (event.type)
        {

        case SDL_KEYUP:
            //determine which player this event belongs to
            for (i=0; i<4; i++)
            {
                if (player[i]->controller.keyboard==KEYBOARD)
                    break;
            }

            if (i<4)
            {
                if (event.key.keysym.sym == player[i]->controller.moveDown)
                    player[i]->fastDrop = 0;

                else if (event.key.keysym.sym == player[i]->controller.moveLeft)
                {
                    if (player[i]->fastSlide == -2)
                        player[i]->fastSlide = 1;
                    else
                        player[i]->fastSlide = 0;
                }

                else if (event.key.keysym.sym == player[i]->controller.moveRight)
                {
                    if (player[i]->fastSlide == 2)
                        player[i]->fastSlide = -1;
                    else
                        player[i]->fastSlide = 0;
                }
            }
            break;

        // check for keypresses
        case SDL_KEYDOWN:
            //determine which player this event belongs to
            for (i=0; i<4; i++)
            {
                if (player[i]->controller.keyboard==KEYBOARD)
                    break;
            }

            if (i<4)
            {
                if (event.key.keysym.sym == player[i]->controller.hold) //swap holdpiece
                {
                    if (!player[i]->swapped)
                    {
                        player[i]->swapped=1;
                        player[i]->held.type=player[i]->active.type;
                        spawnPiece(&player[i]->active,getPiece(player[i]->pieces));
                        player[i]->pieces++;
                    }
                    else if (player[i]->swappable)
                    {
                        bufferPiece.type=player[i]->active.type;
                        player[i]->active.type=player[i]->held.type;
                        player[i]->held.type=bufferPiece.type;
                        spawnPiece(&player[i]->active,player[i]->active.type);
                        player[i]->swappable=0;
                    }
                }
                else if (event.key.keysym.sym == player[i]->controller.rotateLeft)
                    rotatePieceLeft(&player[i]->active,&player[i]->grid);

                else if (event.key.keysym.sym == player[i]->controller.rotateRight)
                    rotatePieceRight(&player[i]->active,&player[i]->grid);

                else if (event.key.keysym.sym == player[i]->controller.moveDown)
                {
                    player[i]->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    player[i]->score+=movePieceDown(&player[i]->active,&player[i]->grid);
                    player[i]->fastDrop=1;
                }
                else if (event.key.keysym.sym == player[i]->controller.moveLeft)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    movePieceLeft(&player[i]->active,&player[i]->grid);
                    if (player[i]->fastSlide == 0)
                        player[i]->fastSlide = -1;
                    else if (player[i]->fastSlide == 1)
                        player[i]->fastSlide = -2;
                }
                else if (event.key.keysym.sym == player[i]->controller.moveRight)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    movePieceRight(&player[i]->active,&player[i]->grid);
                    if (player[i]->fastSlide == 0)
                        player[i]->fastSlide = 1;
                    else if (player[i]->fastSlide == -1)
                        player[i]->fastSlide = 2;
                }
                else if (event.key.keysym.sym == player[i]->controller.hardDrop)
                {
                    if (!scoreDrop(player[i],screen))
                    {
                        return 1;
                    }
                    player[i]->ticks=SDL_GetTicks();
                }
            }
            break;

        case SDL_JOYBUTTONDOWN:
            //determine which player this button press is assigned to
            for (i=0; i<4; i++)
            {
                if (player[i]->controller.keyboard==event.jbutton.which)
                    break;
            }
            if (i<4)
            {
                if (event.jbutton.button == player[i]->controller.hold) //swap holdpiece
                {
                    if (!player[i]->swapped)
                    {
                        player[i]->swapped=1;
                        player[i]->held.type=player[i]->active.type;
                        spawnPiece(&player[i]->active,getPiece(player[i]->pieces));
                        player[i]->pieces++;
                    }
                    else if (player[i]->swappable)
                    {
                        bufferPiece.type=player[i]->active.type;
                        player[i]->active.type=player[i]->held.type;
                        player[i]->held.type=bufferPiece.type;
                        spawnPiece(&player[i]->active,player[i]->active.type);
                        player[i]->swappable=0;
                    }
                }
                else if (event.jbutton.button == player[i]->controller.rotateLeft)
                    rotatePieceLeft(&player[i]->active,&player[i]->grid);

                else if (event.jbutton.button == player[i]->controller.rotateRight)
                    rotatePieceRight(&player[i]->active,&player[i]->grid);

                else if (event.jbutton.button == player[i]->controller.hardDrop)
                {
                    if (!scoreDrop(player[i],screen))
                    {
                        return 1;
                    }
                    player[i]->ticks=SDL_GetTicks();
                }
            }
            break;

        case SDL_JOYHATMOTION:
//determine which player this button press is assigned to
            for (i=0; i<4; i++)
            {
                if (player[i]->controller.keyboard==event.jbutton.which)
                    break;
            }
            if (i<4)
            {

                if ( ! (event.jhat.value & SDL_HAT_DOWN))
                {
                    player[i]->fastDrop = 0;
                }
                if ( ! (event.jhat.value & SDL_HAT_LEFT) && ! (event.jhat.value & SDL_HAT_RIGHT))
                {
                    player[i]->fastSlide = 0;
                }

                if (event.jhat.value & SDL_HAT_DOWN)
                {
                    player[i]->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    if (!player[i]->fastDrop)
                    {
                        player[i]->score+=movePieceDown(&player[i]->active,&player[i]->grid);
                        player[i]->fastDrop=1;
                    }
                }

                if (event.jhat.value & SDL_HAT_LEFT)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    if (player[i]->fastSlide == 0)
                    {
                        player[i]->fastSlide = -1;
                        movePieceLeft(&player[i]->active,&player[i]->grid);
                    }
                }

                if (event.jhat.value & SDL_HAT_RIGHT)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    if (player[i]->fastSlide == 0)
                    {
                        player[i]->fastSlide = 1;
                        movePieceRight(&player[i]->active,&player[i]->grid);
                    }
                }
            }
            break;

        case SDL_JOYAXISMOTION:
//determine which player this button press is assigned to
            for (i=0; i<4; i++)
            {
                if (player[i]->controller.keyboard==event.jbutton.which)
                    break;
            }
            if (i<4)
            {

                //DOWN
                if (event.jaxis.axis % 2 == 1 && event.jaxis.value > 3200 )
                {
                    player[i]->ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    if (!player[i]->fastDrop)
                    {
                        player[i]->score+=movePieceDown(&player[i]->active,&player[i]->grid);
                        player[i]->fastDrop=1;
                    }
                }

                //LEFT
                else if (event.jaxis.axis % 2 == 0 && event.jaxis.value < -3200)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    if (player[i]->fastSlide == 0)
                    {
                        player[i]->fastSlide = -1;
                        movePieceLeft(&player[i]->active,&player[i]->grid);
                    }
                }

                //RIGHT
                else if (event.jaxis.axis % 2 == 0 && event.jaxis.value > 3200)
                {
                    player[i]->slideTime = SDL_GetTicks();
                    if (player[i]->fastSlide == 0)
                    {
                        player[i]->fastSlide = 1;
                        movePieceRight(&player[i]->active,&player[i]->grid);
                    }
                }

                //if axis is centered

                else if (event.jaxis.axis%2 == 1 && event.jaxis.value < 3200 && event.jaxis.value > -3200)
                {
                    player[i]->fastDrop = 0;
                }
                else if (event.jaxis.axis%2 == 0 && event.jaxis.value < 3200 && event.jaxis.value > -3200)
                {
                    player[i]->fastSlide = 0;
                }
            }
            break;
        }
    }
    return 0;
}

//Return 0 if dropping the latest piece has ended the game
//Return 1 if game should continue
//Play different notes depending on how many lines have been cleared and what type of piece was used
//Spawn the next piece and allow the player to swap hold piece with active piece
int scoreDrop(Player *player, SDL_Surface *surface)
{
    int dropTest = dropPiece(&player->active,&player->grid,&player->attackBuffer,surface,&player->score,(player->lines/10)+1);
    if (dropTest<0)
    {
        player->isActive=2;
        return 0;
    }
    playLockSound();
    player->lines+=dropTest;
    spawnPiece(&player->active,getPiece(player->pieces));
    player->pieces+=1;
    player->swappable=1;
    return 1;
}
