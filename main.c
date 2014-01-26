#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include "Block.h"
#include "Piece.h"
#include "Grid.h"
#include "Player.h"
#include "DrawText.h"

//player structs
Player player1;
Player player2;
Player player3;
Player player4;
Player *players[4] = {&player1,&player2,&player3,&player4};

enum GameType {MARATHON,BLITZ,SPRINT};
void singlePlayerGame(Player player, SDL_Surface*, enum GameType type);

void multiPlayerMenu(SDL_Surface*);
void multiPlayerGame(SDL_Surface*,enum GameType type);

//process a dropped piece, return 0 if game is over
int scoreDrop(Player *player, SDL_Surface *surface);

int main ( int argc, char** argv )
{
    // seed random number generator
    srand(time(0));

    initControls(&player1);
    initControls(&player2);
    initControls(&player3);
    initControls(&player4);

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // Make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // Create a new window
    SDL_Surface* screen = SDL_SetVideoMode(2*BLOCKSIZE*GRIDXSIZE*2+1,2*BLOCKSIZE*GRIDYSIZE+1, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);

    if ( !screen )
    {
        printf("Unable to set video: %s\n", SDL_GetError());
        return 1;
    }

    SDL_WM_SetCaption("Djentris", 0);

    // Open joysticks

    SDL_JoystickEventState(SDL_ENABLE);
    int i;
    SDL_Joystick *joystick[SDL_NumJoysticks()];
    for( i=0; i < SDL_NumJoysticks(); i++ )
    {
        joystick[i] = SDL_JoystickOpen(i);
    }

    //Main menu

    int done = 0;
    int option = 4;
    //for tracking joystick motion
    int oldValue = 0;
    while (!done)
    {
        resetPieceLists();

        drawString(" Marathon - Surive   ",screen,30,screen->h/2-CHARHEIGHT*5);
        drawString(" Blitz    - 2 minutes",screen,30,screen->h/2-CHARHEIGHT*4);
        drawString(" Sprint   - 40 lines ",screen,30,screen->h/2-CHARHEIGHT*3);
        drawString(" Multi-player        ",screen,30,screen->h/2-CHARHEIGHT*2);
        drawString(" Configure Controls  ",screen,30,screen->h/2-CHARHEIGHT);
        drawString(" Quit                ",screen,30,screen->h/2);
        drawString("*",screen,30,screen->h/2-CHARHEIGHT*option);

        SDL_Flip(screen);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
            // exit if the window is closed
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_JOYAXISMOTION:
                //UP
                if (event.jaxis.axis % 2 == 1 && event.jaxis.value < -3200 )
                {
                    if (oldValue > -3200)
                    {

                        if (option<5)
                            option++;
                        else
                            option = 0;
                    }
                }
                //DOWN
                else if (event.jaxis.axis % 2 == 1 && event.jaxis.value > 3200 )
                {
                    if (oldValue < 3200)
                    {
                        if (option<1)
                            option=5;
                        else
                            option--;
                    }
                }
                oldValue = event.jaxis.value;
                break;

            case SDL_JOYBUTTONDOWN:
                switch (option)
                {
                case 0:
                    exit(0);
                    break;

                case 1:
                    configureSinglePlayerControls(&player1,screen);
                    break;

                case 2:
                    multiPlayerMenu(screen);
                    break;

                case 3:
                    singlePlayerGame(player1,screen,SPRINT);
                    break;

                case 4:
                    singlePlayerGame(player1,screen,BLITZ);
                    break;

                case 5:
                    singlePlayerGame(player1,screen,MARATHON);
                    break;
                }
                break;

            case SDL_JOYHATMOTION:
                if (oldValue != event.jhat.value)
                    if (event.jhat.value & SDL_HAT_UP)
                    {
                        if (option<5)
                            option++;
                        else
                            option = 0;
                    }

                if (event.jhat.value & SDL_HAT_DOWN)
                {
                    if (option<1)
                        option=5;
                    else
                        option--;
                    break;
                }
                oldValue = event.jhat.value;
                break;


            // check for keypresses
            case SDL_KEYDOWN:
                // exit if ESCAPE is pressed
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    done = 1;
                    break;
                case SDLK_UP:
                    if (option<5)
                        option++;
                    else
                        option = 0;
                    break;

                case SDLK_DOWN:
                    if (option<1)
                        option=5;
                    else
                        option--;
                    break;

                case SDLK_RETURN:
                    switch (option)
                    {
                    case 0:
                        exit(0);
                        break;

                    case 1:
                        configureSinglePlayerControls(&player1,screen);
                        break;

                    case 2:
                        multiPlayerMenu(screen);
                        break;

                    case 3:
                        singlePlayerGame(player1,screen,SPRINT);
                        break;

                    case 4:
                        singlePlayerGame(player1,screen,BLITZ);
                        break;

                    case 5:
                        singlePlayerGame(player1,screen,MARATHON);
                        break;
                    }
                    break;

                default:
                    break;

                }
            }
        }
    }

    //Close joysticks

    for( i=0; i < SDL_NumJoysticks(); i++ )
    {
        SDL_JoystickClose(joystick[i]);
    }

    return 0;
}

void multiPlayerMenu(SDL_Surface* screen)
{
    int option=3;
    int oldValue=0;
    int done=0;
    while (!done)
    {
        resetPieceLists();

        drawString(" Sprint Race         ",screen,30,screen->h/2-CHARHEIGHT*5);
        drawString(" Elimination Battle  ",screen,30,screen->h/2-CHARHEIGHT*4);
        drawString(" Configure Controls  ",screen,30,screen->h/2-CHARHEIGHT*3);
        drawString(" Back to Main Menu   ",screen,30,screen->h/2-CHARHEIGHT*2);
        drawString("                     ",screen,30,screen->h/2-CHARHEIGHT);
        drawString("                     ",screen,30,screen->h/2);
        drawString("*",screen,30,screen->h/2-CHARHEIGHT*option);

        SDL_Flip(screen);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
            // exit if the window is closed
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_JOYAXISMOTION:
                //UP
                if (event.jaxis.axis % 2 == 1 && event.jaxis.value < -3200 )
                {
                    if (oldValue > -3200)
                    {

                        if (option<5)
                            option++;
                        else
                            option = 2;
                    }
                }
                //DOWN
                else if (event.jaxis.axis % 2 == 1 && event.jaxis.value > 3200 )
                {
                    if (oldValue < 3200)
                    {
                        if (option<3)
                            option=5;
                        else
                            option--;
                    }
                }
                oldValue = event.jaxis.value;
                break;

            case SDL_JOYBUTTONDOWN:
                switch (option)
                {
                case 2:
                    return;
                    break;

                case 3:
                    configureMultiPlayerControls(players,screen);
                    break;

                case 4:
                    ;
                    break;

                case 5:
                    multiPlayerGame(screen,SPRINT);
                    break;
                }
                break;

            case SDL_JOYHATMOTION:
                if (oldValue != event.jhat.value)
                    if (event.jhat.value & SDL_HAT_UP)
                    {
                        if (option<5)
                            option++;
                        else
                            option = 2;
                    }

                if (event.jhat.value & SDL_HAT_DOWN)
                {
                    if (option<3)
                        option=5;
                    else
                        option--;
                    break;
                }
                oldValue = event.jhat.value;
                break;


            // check for keypresses
            case SDL_KEYDOWN:
                // exit if ESCAPE is pressed
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    done = 1;
                    break;
                case SDLK_UP:
                    if (option<5)
                        option++;
                    else
                        option = 2;
                    break;

                case SDLK_DOWN:
                    if (option<3)
                        option=5;
                    else
                        option--;
                    break;

                case SDLK_RETURN:
                    switch (option)
                    {
                    case 2:
                        return;
                        break;

                    case 3:
                        configureMultiPlayerControls(players,screen);
                        break;

                    case 4:
                        ;
                        break;

                    case 5:
                        multiPlayerGame(screen,SPRINT);
                        break;
                    }
                    break;

                default:
                    break;

                }
            }
        }
    }
}

void singlePlayerGame(Player player, SDL_Surface* window, enum GameType type)
{
    char scoreString[30];
    int currentTime = 0;

    initPlayer(&player);

    player.ticks=SDL_GetTicks();
    player.startTime = SDL_GetTicks();
    player.totalTime = 0;

    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_SWSURFACE, BLOCKSIZE*GRIDXSIZE*2, BLOCKSIZE*GRIDYSIZE*2, 32,
                          window->format->Rmask, window->format->Gmask, window->format->Bmask, window->format->Amask);
    if(screen == NULL)
    {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }

    //screen rectangles
    SDL_Rect screenRect= {0,0,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE*2};
    SDL_Rect windowRect= {0,0,window->w,window->h};
    SDL_Rect singlePlayerRect= {window->w/2-window->w/4,window->h/2-window->h/4,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE*2};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    //MS delay for fast drop or slide
    const int dropWait = 50;
    const int slideWait = 100;

    // program main loop
    Uint8 done = 0;
    Uint8 paused = 0;
    while (!done)
    {
        //update currentTime
        currentTime = (player.totalTime+(SDL_GetTicks()-player.startTime));

        // process user controllers
        if (singleControllerProcess(&player,window))
            done = 1;


        MSDelay=1000;
        int loop=0;
        while (loop<=(player.lines/10)+1)
        {
            int temp=MSDelay/5;
            MSDelay-=temp;
            loop++;
        }

        if (player.fastDrop&&MSDelay>dropWait)
        {
            MSDelay=dropWait;
        }

        if (player.fastSlide&&SDL_GetTicks()-player.slideTime>slideWait&&!paused)
        {
            if (player.fastSlide<0)
                movePieceLeft(&player.active,&player.grid);
            if (player.fastSlide>0)
                movePieceRight(&player.active,&player.grid);
            player.slideTime = SDL_GetTicks();
        }

        if (SDL_GetTicks()-player.ticks>MSDelay&&!paused)
        {
            if (!movePieceDown(&player.active,&player.grid))
            {
                if (SDL_GetTicks()-player.ticks>slideDelay)
                {
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    player.ticks=SDL_GetTicks();
                }
            }
            else
            {
                if (player.fastDrop)
                    player.score+=1;
                player.ticks=SDL_GetTicks();
            }
        }

        // DRAWING STARTS HERE

        // DRAW BACKGROUND

        SDL_FillRect(screen,&screenRect,SDL_MapRGB(screen->format,127,127,127));
        SDL_FillRect(window,&windowRect,SDL_MapRGB(window->format,127,127,127));

        // DRAW GAME

        drawGame(player,screen);

        // DRAW ELAPSED TIME

        switch (type)
        {
        case MARATHON:
            currentTime = (player.totalTime+(SDL_GetTicks()-player.startTime));
            sprintf(scoreString,"%02d:%02d MARATHON",(currentTime/1000)/60,(currentTime/1000)%60);
            drawString(scoreString,screen,0,0);
            break;

        case BLITZ:
            sprintf(scoreString,"%02d:%02d BLITZ",(120-(currentTime/1000))/60,(120-(currentTime/1000))%60);
            drawString(scoreString,screen,0,0);
            break;

        case SPRINT:
            sprintf(scoreString,"%02d:%02d SPRINT",(currentTime/1000)/60,(currentTime/1000)%60);
            drawString(scoreString,screen,0,0);
            break;
        }

        // Update the screen
        SDL_Flip(screen);
        SDL_BlitSurface(screen,&screenRect,window,&singlePlayerRect);
        SDL_Flip(window);

        switch (type)
        {
        case MARATHON:
            break;

        case BLITZ:
            if (currentTime/1000>=120)
                done=1;
            break;

        case SPRINT:
            if (player.lines>39)
                done=1;
            break;
        }

    } // end main loop

    SDL_Rect endRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,CHARHEIGHT*4};

    SDL_FillRect(screen,&endRect,SDL_MapRGB(screen->format,0,0,0));

    switch (type)
    {
    case MARATHON:
        sprintf(scoreString,"Marathon Score: %02d:%02d.%02d",(currentTime/1000)/60,(currentTime/1000)%60,currentTime%100);
        scoreString[29]='\0';
        drawString(scoreString,screen,0,0);
        sprintf(scoreString,"with %d points",player.score);
        scoreString[29]='\0';
        drawString(scoreString,screen,0,CHARHEIGHT);
        break;

    case BLITZ:
        if (currentTime/1000<120)
        {
            drawString("Blitz Score: Failure.",screen,0,0);
        }
        else
        {
            sprintf(scoreString,"Blitz Score: %d",player.score);
            scoreString[29]='\0';
            drawString(scoreString,screen,0,0);
        }
        break;

    case SPRINT:
        if (player.lines<40)
        {
            drawString("Sprint Score: Failure.",screen,0,0);
        }
        else
        {
            sprintf(scoreString,"Sprint Score: %02d:%02d.%02d",(currentTime/1000)/60,(currentTime/1000)%60,currentTime%100);
            scoreString[29]='\0';
            drawString(scoreString,screen,0,0);
        }
        break;
    }

    // Update the screen
    SDL_Flip(screen);
    SDL_BlitSurface(screen,&screenRect,window,&singlePlayerRect);
    SDL_Flip(window);

    SDL_FreeSurface(screen);
}

void multiPlayerGame(SDL_Surface* window, enum GameType type)
{
    initPlayer(&player1);
    initPlayer(&player2);
    initPlayer(&player3);
    initPlayer(&player4);

    char scoreString[30];
    int currentTime = 0;
    int i;

    for (i=0; i<4; i++)
    {
        players[i]->ticks=SDL_GetTicks();
        players[i]->startTime = SDL_GetTicks();
        players[i]->totalTime = 0;
    }

    SDL_Rect playerRect1 = {0,0,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect2 = {BLOCKSIZE*GRIDXSIZE*2,0,BLOCKSIZE*GRIDXSIZE,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect3 = {0,BLOCKSIZE*GRIDYSIZE,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};
    SDL_Rect playerRect4 = {BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE};

    SDL_Surface *quad[4];

    for (i=0; i<4; i++)
    {
        quad[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, BLOCKSIZE*GRIDXSIZE*2, BLOCKSIZE*GRIDYSIZE*2, 32,
                                       window->format->Rmask, window->format->Gmask, window->format->Bmask, window->format->Amask);
    }

    //screen rectangles
    SDL_Rect screenRect= {0,0,BLOCKSIZE*GRIDXSIZE*2,BLOCKSIZE*GRIDYSIZE*2};
    SDL_Rect windowRect= {0,0,window->w,window->h};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    //MS delay for fast drop or slide
    const int dropWait = 50;
    const int slideWait = 100;

    // program main loop
    Uint8 done = 0;
    Uint8 paused = 0;
    while (!done)
    {
        //update currentTime
        currentTime = (players[0]->totalTime+(SDL_GetTicks()-players[0]->startTime));

        // process user controllers
        if (multiControllerProcess(players,window))
            done = 1;

        for (i=0; i<4; i++)
        {
            if (players[i]->isActive)
            {

                MSDelay=1000;

                int loop=0;
                while (loop<=(players[i]->lines/10)+1)
                {
                    int temp=MSDelay/5;
                    MSDelay-=temp;
                    loop++;
                }

                if (players[i]->fastDrop&&MSDelay>dropWait)
                {
                    MSDelay=dropWait;
                }

                if (players[i]->fastSlide&&SDL_GetTicks()-players[i]->slideTime>slideWait&&!paused)
                {
                    if (players[i]->fastSlide<0)
                        movePieceLeft(&players[i]->active,&players[i]->grid);
                    if (players[i]->fastSlide>0)
                        movePieceRight(&players[i]->active,&players[i]->grid);
                    players[i]->slideTime = SDL_GetTicks();
                }

                if (SDL_GetTicks()-players[i]->ticks>MSDelay&&!paused)
                {
                    if (!movePieceDown(&players[i]->active,&players[i]->grid))
                    {
                        if (SDL_GetTicks()-players[i]->ticks>slideDelay)
                        {
                            if (!scoreDrop(players[i],quad[i]))
                            {
                                done=1;
                                break;
                            }
                            players[i]->ticks=SDL_GetTicks();
                        }
                    }
                    else
                    {
                        if (players[i]->fastDrop)
                            players[i]->score+=1;
                        players[i]->ticks=SDL_GetTicks();
                    }
                }
            }

            // DRAWING STARTS HERE

            // DRAW BACKGROUNDS
            SDL_FillRect(window,&windowRect,SDL_MapRGB(window->format,127,127,127));
            for (i=0; i<4; i++)
            {
                switch (i)
                {
                case 0:
                    SDL_FillRect(quad[i],&screenRect,SDL_MapRGB(quad[i]->format,200,200,200));
                    break;
                case 1:
                    SDL_FillRect(quad[i],&screenRect,SDL_MapRGB(quad[i]->format,127,127,127));
                    break;
                case 2:
                    SDL_FillRect(quad[i],&screenRect,SDL_MapRGB(quad[i]->format,127,127,127));
                    break;
                case 3:
                    SDL_FillRect(quad[i],&screenRect,SDL_MapRGB(quad[i]->format,200,200,200));
                    break;
                }

                // DRAW GAME

                if (players[i]->isActive)
                {
                    drawGame(*players[i],quad[i]);
                }

                // DRAW ELAPSED TIME

                switch (type)
                {
                case MARATHON:
                    currentTime = (players[i]->totalTime+(SDL_GetTicks()-players[i]->startTime));
                    sprintf(scoreString,"%02d:%02d MARATHON",(currentTime/1000)/60,(currentTime/1000)%60);
                    drawString(scoreString,quad[i],0,0);
                    break;

                case BLITZ:
                    sprintf(scoreString,"%02d:%02d BLITZ",(120-(currentTime/1000))/60,(120-(currentTime/1000))%60);
                    drawString(scoreString,quad[i],0,0);
                    break;

                case SPRINT:
                    sprintf(scoreString,"%02d:%02d SPRINT",(currentTime/1000)/60,(currentTime/1000)%60);
                    drawString(scoreString,quad[i],0,0);
                    break;
                }
                SDL_Flip(quad[i]);
            }
        }

        // Update the screen
        SDL_BlitSurface(quad[0],&screenRect,window,&playerRect1);
        SDL_BlitSurface(quad[1],&screenRect,window,&playerRect2);
        SDL_BlitSurface(quad[2],&screenRect,window,&playerRect3);
        SDL_BlitSurface(quad[3],&screenRect,window,&playerRect4);
        SDL_Flip(window);

        switch (type)
        {
        case MARATHON:
            break;

        case BLITZ:
            if (currentTime/1000>=120)
                done=1;
            break;

        case SPRINT:
            for (i=0; i<4; i++)
            {
                if (players[i]->lines>39)
                {
                    done=1;
                }
            }
            break;
        }

    } // end main loop

    SDL_Rect endRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,CHARHEIGHT*4};

    for (i=0; i<4; i++)
    {
        SDL_FillRect(quad[i],&endRect,SDL_MapRGB(quad[i]->format,0,0,0));

        switch (type)
        {
        case MARATHON:
            sprintf(scoreString,"Marathon Score: %02d:%02d.%02d",(currentTime/1000)/60,(currentTime/1000)%60,currentTime%100);
            scoreString[29]='\0';
            drawString(scoreString,quad[i],0,0);
            sprintf(scoreString,"with %d points",players[i]->score);
            scoreString[29]='\0';
            drawString(scoreString,quad[i],0,CHARHEIGHT);
            break;

        case BLITZ:
            if (currentTime/1000<120)
            {
                drawString("Blitz Score: Failure.",quad[i],0,0);
            }
            else
            {
                sprintf(scoreString,"Blitz Score: %d",players[i]->score);
                scoreString[29]='\0';
                drawString(scoreString,quad[i],0,0);
            }
            break;

        case SPRINT:
            if (players[i]->lines<40)
            {
                drawString("Sprint Score: Failure.",quad[i],0,0);
            }
            else
            {
                sprintf(scoreString,"Sprint Score: %02d:%02d.%02d",(currentTime/1000)/60,(currentTime/1000)%60,currentTime%100);
                scoreString[29]='\0';
                drawString(scoreString,quad[i],0,0);
            }
            break;
        }
    }

    // Update the screen
    for (i=0; i<4; i++)
    {
        SDL_Flip(quad[i]);
    }
    SDL_BlitSurface(quad[0],&screenRect,window,&playerRect1);
    SDL_BlitSurface(quad[1],&screenRect,window,&playerRect2);
    SDL_BlitSurface(quad[2],&screenRect,window,&playerRect3);
    SDL_BlitSurface(quad[3],&screenRect,window,&playerRect4);

    SDL_Flip(window);

    for (i=0; i<4; i++)
    {
        SDL_FreeSurface(quad[i]);
    }
}
