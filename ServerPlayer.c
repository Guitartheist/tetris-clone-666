#include "DrawText.h"
#include "Sound.h"
#include "Player.h"

void initPlayer(Player* player)
{
    spawnPiece(&player->active,getPiece(0));
    player->pieces=1;
    player->lines=0;
    player->attackLines=0;
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
                else if (event.key.keysym.sym == player[i]->controller.attackDiagonal)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[2]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[1]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[0]->grid,screen);
                            break;
                        }
                    }
                }
                else if (event.key.keysym.sym == player[i]->controller.attackVertical)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[2]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[0]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[1]->grid,screen);
                            break;
                        }
                    }
                }
                else if (event.key.keysym.sym == player[i]->controller.attackHorizontal)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[1]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[0]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[2]->grid,screen);
                            break;
                        }
                    }
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
                else if (event.jbutton.button == player[i]->controller.attackDiagonal)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[2]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[1]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[0]->grid,screen);
                            break;
                        }
                    }
                }
                else if (event.jbutton.button == player[i]->controller.attackVertical)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[2]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[0]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[1]->grid,screen);
                            break;
                        }
                    }
                }
                else if (event.jbutton.button == player[i]->controller.attackHorizontal)
                {
                    if (player[i]->attackLines)
                    {
                        player[i]->attackLines-=1;
                        switch (i)
                        {
                        case 0:
                            pushLine(&player[1]->grid,screen);
                            break;
                        case 1:
                            pushLine(&player[0]->grid,screen);
                            break;
                        case 2:
                            pushLine(&player[3]->grid,screen);
                            break;
                        case 3:
                            pushLine(&player[2]->grid,screen);
                            break;
                        }
                    }
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
    int dropTest = dropPiece(&player->active,&player->grid,surface,&player->score,(player->lines/10)+1);
    if (dropTest<0)
    {
        player->isActive=2;
        return 0;
    }
    player->lines+=dropTest;
    if (dropTest>1)
    {
        player->attackLines+=dropTest-1;
    }
    spawnPiece(&player->active,getPiece(player->pieces));
    player->pieces+=1;
    player->swappable=1;
    return 1;
}
