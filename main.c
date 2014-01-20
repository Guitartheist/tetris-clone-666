#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "Block.h"
#include "Piece.h"
#include "Grid.h"
#include "Player.h"
#include "DrawText.h"

//if muted is non-zero do not play any sounds
int muted = 0;

//player structs
Player player1;
Player player2;
Player player3;
Player player4;

enum GameType {MARATHON,BLITZ,SPRINT};
void singlePlayerGame(Player player, SDL_Surface*, enum GameType type);
void multiPlayerGame(SDL_Surface*);

int singlePlayerPauseMenu(Player player, SDL_Surface*);

//ask player to enter their preferred controls
void configureSinglePlayerControls(Player *player, SDL_Surface *screen);
void configureMultiPlayerControls(SDL_Surface *screen);

void initSound();
void playMoveSound();
void playLockSound();

//process controller actions
int singleControllerProcess(Player *player,SDL_Surface *screen);
int multiPlayerControllerProcess(Player *player[],SDL_Surface *screen);

//process a dropped piece, return 0 if game is over
int scoreDrop(Player *player, SDL_Surface *surface);

int main ( int argc, char** argv )
{
    // seed random number generator
    srand(time(0));

    initPlayer(&player1);
    initPlayer(&player2);
    initPlayer(&player3);
    initPlayer(&player4);

    initSound();

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(2*BLOCKSIZE*GRIDXSIZE*2+1,2*BLOCKSIZE*GRIDYSIZE+1, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);

    SDL_WM_SetCaption("Djentris", 0);

    if ( !screen )
    {
        printf("Unable to set video: %s\n", SDL_GetError());
        return 1;
    }

    //Main menu

    int done = 0;
    int option = 4;
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

            // check for keypresses
            case SDL_KEYDOWN:
            {
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
                        option=4;
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
    }

    return 0;
}

Mix_Chunk *moveSound = NULL;
Mix_Chunk *lockSound = NULL;
Mix_Chunk *note1 = NULL;
Mix_Chunk *note2 = NULL;
Mix_Chunk *note3 = NULL;
Mix_Chunk *note4 = NULL;
Mix_Chunk *note5 = NULL;
Mix_Chunk *note6 = NULL;
Mix_Chunk *note7 = NULL;

void initSound()
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_AllocateChannels(128);
    moveSound = Mix_LoadWAV("sounds/move.wav");
    lockSound = Mix_LoadWAV("sounds/lock.wav");
    note1 = Mix_LoadWAV("sounds/1.wav");
    note2 = Mix_LoadWAV("sounds/2.wav");
    note3 = Mix_LoadWAV("sounds/3.wav");
    note4 = Mix_LoadWAV("sounds/4.wav");
    note5 = Mix_LoadWAV("sounds/5.wav");
    note6 = Mix_LoadWAV("sounds/6.wav");
    note7 = Mix_LoadWAV("sounds/7.wav");
}

void singlePlayerGame(Player player, SDL_Surface* window, enum GameType type)
{
    char scoreString[30];
    int currentTime = 0;

    player.ticks=SDL_GetTicks();
    player.startTime = SDL_GetTicks();
    player.totalTime = 0;

    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_SWSURFACE, BLOCKSIZE*GRIDXSIZE*2, BLOCKSIZE*GRIDYSIZE*2, 32,
                                               window->format->Rmask, window->format->Gmask, window->format->Bmask, window->format->Amask);
    if(screen == NULL) {
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
                    playLockSound();
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
            }
        }
}

void playMoveSound()
{
    //play move sound
    if (muted)
        return;
    int moveChannel = Mix_PlayChannel(-1, moveSound, 0);
    Mix_Volume(moveChannel,12);
}

void playLockSound()
{
    //play lock sound
    if (muted)
        return;
    int lockChannel = Mix_PlayChannel(-1,lockSound,0);
    Mix_Volume(lockChannel,90);
}

int singleControllerProcess(Player *player,SDL_Surface *screen)
{
    SDL_Event event;
    Piece bufferPiece;
    while (SDL_PollEvent(&event))
    {
        // check for messages
        switch (event.type)
        {
        // exit if the window is closed
        case SDL_QUIT:
            exit(0);
            break;

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
            // pause if ESCAPE is pressed
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

            playMoveSound();

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
                playLockSound();
                if (!scoreDrop(player,screen))
                {
                    return 1;
                }
                player->ticks=SDL_GetTicks();
            }
            //mute key is hard-coded to M
            else if (event.key.keysym.sym == SDLK_m)
            {
                if (muted)
                    muted=0;
                else
                    muted =1;
            }
            break;
        }
        }// end switch
    }
    return 0;
}

void configureSinglePlayerControls(Player *player, SDL_Surface *screen)
{
    SDL_Event event;
    int i=0;
    int keys[12];

    SDL_Rect endRect = {0,0,BLOCKSIZE*GRIDXSIZE*2,CHARHEIGHT*4};

    SDL_FillRect(screen,&endRect,SDL_MapRGB(screen->format,0,0,0));

    while (i<12)
    {
        switch (i)
        {
        case 0:
            drawString("Press move left",screen,0,0);
            break;

        case 1:
            drawString("Press move right",screen,0,0);
            break;

        case 2:
            drawString("Press rotate left",screen,0,0);
            break;

        case 3:
            drawString("Press rotate right",screen,0,0);
            break;

        case 4:
            drawString("Press hard drop   ",screen,0,0);
            break;

        case 5:
            drawString("Press move down   ",screen,0,0);
            break;

        case 6:
            drawString("Press hold/swap   ",screen,0,0);
            break;

        case 7:
            drawString("Press pause       ",screen,0,0);
            break;

        case 8:
            drawString("Press quit        ",screen,0,0);
            break;

        case 9:
            drawString("Press vertical ATK",screen,0,0);
            break;

        case 10:
            drawString("Press horizon ATK",screen,0,0);
            break;

        case 11:
            drawString("Press diagonal ATK",screen,0,0);
            break;
        }

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
                keys[i]=event.key.keysym.sym;
                i++;
                break;
            }// end switch
        }
    }
    player->controller.moveLeft = keys[0];
    player->controller.moveRight = keys[1];
    player->controller.rotateLeft = keys[2];
    player->controller.rotateRight = keys[3];
    player->controller.hardDrop = keys[4];
    player->controller.moveDown = keys[5];
    player->controller.hold = keys[6];
    player->controller.pause = keys[7];
    player->controller.quit = keys[8];
    player->controller.attackVertical = keys[9];
    player->controller.attackHorizontal = keys[10];
    player->controller.attackDiagonal = keys[11];

    drawString("Configuration Complete",screen,0,0);
    SDL_Flip(screen);
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

    static int clearChannel = 0;

    if (!muted)
    {
        switch(dropTest)
        {
        case 4:
            clearChannel = Mix_PlayChannel(-1,note1,0);
            Mix_Volume(clearChannel,80);
        case 3:
            switch (player->active.type)
            {
            case Z:
                clearChannel = Mix_PlayChannel(-1,note5,0);
                break;

            case L:
                clearChannel = Mix_PlayChannel(-1,note6,0);
                break;

            case I:
                clearChannel = Mix_PlayChannel(-1,note7,0);
                break;

            case T:
                clearChannel = Mix_PlayChannel(-1,note1,0);
                break;

            case O:
                clearChannel = Mix_PlayChannel(-1,note2,0);
                break;

            case J:
                clearChannel = Mix_PlayChannel(-1,note3,0);
                break;

            case S:
                clearChannel = Mix_PlayChannel(-1,note4,0);
                break;
            }
        case 2:
            switch (player->active.type)
            {
            case Z:
                clearChannel = Mix_PlayChannel(-1,note3,0);
                break;

            case L:
                clearChannel = Mix_PlayChannel(-1,note4,0);
                break;

            case I:
                clearChannel = Mix_PlayChannel(-1,note5,0);
                break;

            case T:
                clearChannel = Mix_PlayChannel(-1,note6,0);
                break;

            case O:
                clearChannel = Mix_PlayChannel(-1,note7,0);
                break;

            case J:
                clearChannel = Mix_PlayChannel(-1,note1,0);
                break;

            case S:
                clearChannel = Mix_PlayChannel(-1,note2,0);
                break;
            }
            Mix_Volume(clearChannel,80);
        case 1:
            switch (player->active.type)
            {
            case Z:
                clearChannel = Mix_PlayChannel(-1,note1,0);
                break;

            case L:
                clearChannel = Mix_PlayChannel(-1,note2,0);
                break;

            case I:
                clearChannel = Mix_PlayChannel(-1,note3,0);
                break;

            case T:
                clearChannel = Mix_PlayChannel(-1,note4,0);
                break;

            case O:
                clearChannel = Mix_PlayChannel(-1,note5,0);
                break;

            case J:
                clearChannel = Mix_PlayChannel(-1,note6,0);
                break;

            case S:
                clearChannel = Mix_PlayChannel(-1,note7,0);
                break;
            }
            Mix_Volume(clearChannel,80);
        }
    }
    spawnPiece(&player->active,getPiece(player->pieces));
    player->pieces+=1;
    player->swappable=1;
    return 1;
}
