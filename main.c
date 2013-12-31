#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "Block.h"
#include "Piece.h"
#include "Grid.h"
#include "Player.h"
#include "DrawText.h"

void singlePlayerMarathon(SDL_Surface*);
void singlePlayerBlitz(SDL_Surface*);
void singlePlayerSprint(SDL_Surface*);
int singlePlayerPauseMenu(SDL_Surface*);

//process a dropped piece, return 0 if game is over
int scoreDrop(Player *player, SDL_Surface *surface);

int main ( int argc, char** argv )
{
    // seed random number generator
    srand(time(0));

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(BLOCKSIZE*GRIDXSIZE*2+1,BLOCKSIZE*GRIDYSIZE+1, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);

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

        drawString(" Marathon - Surive   ",screen,30,screen->h/2-CHARHEIGHT*4);
        drawString(" Blitz    - 2 minutes",screen,30,screen->h/2-CHARHEIGHT*3);
        drawString(" Sprint   - 40 lines ",screen,30,screen->h/2-CHARHEIGHT*2);
        drawString(" Multi-player        ",screen,30,screen->h/2-CHARHEIGHT);
        drawString(" Configuration       ",screen,30,screen->h/2);
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
                    if (option<4)
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
                        break;

                    case 1:
                        break;

                    case 2:
                        singlePlayerSprint(screen);
                        break;

                    case 3:
                        singlePlayerBlitz(screen);
                        break;

                    case 4:
                        singlePlayerMarathon(screen);
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

void freeSound()
{
    Mix_FreeChunk(lockSound);
    Mix_FreeChunk(moveSound);
    Mix_FreeChunk(note1);
    Mix_FreeChunk(note2);
    Mix_FreeChunk(note3);
    Mix_FreeChunk(note4);
    Mix_FreeChunk(note5);
    Mix_FreeChunk(note6);
    Mix_FreeChunk(note7);
    Mix_CloseAudio();
}

void singlePlayerMarathon(SDL_Surface* screen)
{
    //sound effects
    //initialize SDL_mixer
    initSound();
    int lockChannel;
    int moveChannel;

    //screen rectangle
    SDL_Rect screenrect= {0,0,screen->w,screen->h};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    Player player;

    initPlayer(&player);

    // millisecond tracking variable
    int ticks=SDL_GetTicks();

    // fast drop (is user holding moveDown key?)
    int fastDrop = 0;
    // fast slide (is user holding down a lateral motion key?)
    int fastSlide = 0; //-1 moves left, 1 moves right
    //-2 moves right when left is released
    //2 moves left when right is released

    //MS delay for fast drop or slide
    const int dropWait = 50;
    const int slideWait = 100;

    int slideTime = 0; //slide time is measured against SDL_GetTicks() to calibrate movement speed

    // hold piece buffer
    Piece bufferPiece;

    // program main loop
    Uint8 done = 0;
    Uint8 paused = 0;
    while (!done)
    {
        // message processing loop
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

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    fastDrop = 0;
                    break;

                case SDLK_LEFT:
                    if (fastSlide == -2)
                        fastSlide = 1;
                    else
                        fastSlide = 0;
                    break;

                case SDLK_RIGHT:
                    if (fastSlide == 2)
                        fastSlide = -1;
                    else
                        fastSlide = 0;
                    break;

                default:
                    break;
                }

                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // pause if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (singlePlayerPauseMenu(screen))
                        done=1;
                }

                else if (event.key.keysym.sym == SDLK_p)
                {
                    if (singlePlayerPauseMenu(screen))
                        done=1;
                }

                //play move sound
                moveChannel = Mix_PlayChannel(-1, moveSound, 0);
                Mix_Volume(moveChannel,12);

                switch (event.key.keysym.sym)
                {
                case SDLK_LSHIFT: //swap holdpiece
                    if (!player.swapped)
                    {
                        player.swapped=1;
                        player.held.type=player.active.type;
                        spawnPiece(&player.active,getPiece(player.pieces));
                        player.pieces++;
                    }
                    else if (player.swappable)
                    {
                        bufferPiece.type=player.active.type;
                        player.active.type=player.held.type;
                        player.held.type=bufferPiece.type;
                        spawnPiece(&player.active,player.active.type);
                        player.swappable=0;
                    }
                    break;
                case SDLK_z:
                    rotatePieceLeft(&player.active,&player.grid);
                    break;
                case SDLK_UP:
                    rotatePieceRight(&player.active,&player.grid);
                    break;
                case SDLK_DOWN:
                    ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    player.score+=movePieceDown(&player.active,&player.grid);
                    fastDrop=1;
                    break;
                case SDLK_LEFT:
                    slideTime = SDL_GetTicks();
                    movePieceLeft(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = -1;
                    else if (fastSlide == 1)
                        fastSlide = -2;
                    break;
                case SDLK_RIGHT:
                    slideTime = SDL_GetTicks();
                    movePieceRight(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = 1;
                    else if (fastSlide == -1)
                        fastSlide = 2;
                    break;
                case SDLK_SPACE:

                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    ticks=SDL_GetTicks();
                    break;
                default:
                    break;
                }
                break;
            }
            }// end switch
        } // end of message processing

        MSDelay=1000;
        int loop=0;
        while (loop<=(player.lines/10)+1)
        {
            int temp=MSDelay/5;
            MSDelay-=temp;
            loop++;
        }

        if (fastDrop&&MSDelay>dropWait)
        {
            MSDelay=dropWait;
        }

        if (fastSlide&&SDL_GetTicks()-slideTime>slideWait&&!paused)
        {
            if (fastSlide<0)
                movePieceLeft(&player.active,&player.grid);
            if (fastSlide>0)
                movePieceRight(&player.active,&player.grid);
            slideTime = SDL_GetTicks();
        }

        if (SDL_GetTicks()-ticks>MSDelay&&!paused)
        {
            if (!movePieceDown(&player.active,&player.grid))
            {
                if (SDL_GetTicks()-ticks>slideDelay)
                {
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    ticks=SDL_GetTicks();
                }
            }
            else
            {
                if (fastDrop)
                    player.score+=1;
                ticks=SDL_GetTicks();
            }
        }

        // DRAWING STARTS HERE

        // DRAW BACKGROUND

        SDL_FillRect(screen,&screenrect,SDL_MapRGB(screen->format,127,127,127));

        // DRAW GAME

        drawGame(player,screen);

        // Update the screen
        SDL_Flip(screen);
    } // end main loop

    //cleanup
    freeSound();
}

void singlePlayerBlitz(SDL_Surface* screen)
{
    //used to calculate time passed since start of game
    int startTime = SDL_GetTicks();
    int totalTime = 0;

    //sound effects
    //initialize SDL_mixer
    initSound();
    int lockChannel;
    int moveChannel;

    //screen rectangle
    SDL_Rect screenrect= {0,0,screen->w,screen->h};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    Player player;

    initPlayer(&player);

    // millisecond tracking variable
    int ticks=SDL_GetTicks();

    // fast drop (is user holding moveDown key?)
    int fastDrop = 0;
    // fast slide (is user holding down a lateral motion key?)
    int fastSlide = 0; //-1 moves left, 1 moves right
    //-2 moves right when left is released
    //2 moves left when right is released

    //MS delay for fast drop or slide
    const int dropWait = 50;
    const int slideWait = 100;

    int slideTime = 0; //slide time is measured against SDL_GetTicks() to calibrate movement speed

    // hold piece buffer
    Piece bufferPiece;

    // program main loop
    Uint8 done = 0;
    Uint8 paused = 0;
    while (!done)
    {
        // message processing loop
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

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    fastDrop = 0;
                    break;

                case SDLK_LEFT:
                    if (fastSlide == -2)
                        fastSlide = 1;
                    else
                        fastSlide = 0;
                    break;

                case SDLK_RIGHT:
                    if (fastSlide == 2)
                        fastSlide = -1;
                    else
                        fastSlide = 0;
                    break;

                default:
                    break;
                }

                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // pause if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    totalTime += SDL_GetTicks() - startTime;
                    if (singlePlayerPauseMenu(screen))
                        return;
                    startTime = SDL_GetTicks();
                }

                else if (event.key.keysym.sym == SDLK_p)
                {
                    totalTime += SDL_GetTicks() - startTime;
                    if (singlePlayerPauseMenu(screen))
                        return;
                    startTime = SDL_GetTicks();
                }

                //play move sound
                moveChannel = Mix_PlayChannel(-1, moveSound, 0);
                Mix_Volume(moveChannel,12);

                switch (event.key.keysym.sym)
                {
                case SDLK_LSHIFT: //swap holdpiece
                    if (!player.swapped)
                    {
                        player.swapped=1;
                        player.held.type=player.active.type;
                        spawnPiece(&player.active,getPiece(player.pieces));
                        player.pieces++;
                    }
                    else if (player.swappable)
                    {
                        bufferPiece.type=player.active.type;
                        player.active.type=player.held.type;
                        player.held.type=bufferPiece.type;
                        spawnPiece(&player.active,player.active.type);
                        player.swappable=0;
                    }
                    break;
                case SDLK_z:
                    rotatePieceLeft(&player.active,&player.grid);
                    break;
                case SDLK_UP:
                    rotatePieceRight(&player.active,&player.grid);
                    break;
                case SDLK_DOWN:
                    ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    player.score+=movePieceDown(&player.active,&player.grid);
                    fastDrop=1;
                    break;
                case SDLK_LEFT:
                    slideTime = SDL_GetTicks();
                    movePieceLeft(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = -1;
                    else if (fastSlide == 1)
                        fastSlide = -2;
                    break;
                case SDLK_RIGHT:
                    slideTime = SDL_GetTicks();
                    movePieceRight(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = 1;
                    else if (fastSlide == -1)
                        fastSlide = 2;
                    break;
                case SDLK_SPACE:

                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    ticks=SDL_GetTicks();
                    break;
                default:
                    break;
                }
                break;
            }
            }// end switch
        } // end of message processing

        MSDelay=1000;
        int loop=0;
        while (loop<=(player.lines/10)+1)
        {
            int temp=MSDelay/5;
            MSDelay-=temp;
            loop++;
        }

        if (fastDrop&&MSDelay>dropWait)
        {
            MSDelay=dropWait;
        }

        if (fastSlide&&SDL_GetTicks()-slideTime>slideWait&&!paused)
        {
            if (fastSlide<0)
                movePieceLeft(&player.active,&player.grid);
            if (fastSlide>0)
                movePieceRight(&player.active,&player.grid);
            slideTime = SDL_GetTicks();
        }

        if (SDL_GetTicks()-ticks>MSDelay&&!paused)
        {
            if (!movePieceDown(&player.active,&player.grid))
            {
                if (SDL_GetTicks()-ticks>slideDelay)
                {
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    ticks=SDL_GetTicks();
                }
            }
            else
            {
                if (fastDrop)
                    player.score+=1;
                ticks=SDL_GetTicks();
            }
        }

        // DRAWING STARTS HERE

        // DRAW BACKGROUND

        SDL_FillRect(screen,&screenrect,SDL_MapRGB(screen->format,127,127,127));

        // DRAW GAME

        drawGame(player,screen);

        // DRAW ELAPSED TIME

        char scoreString[5];
        int currentTime = (totalTime+(SDL_GetTicks()-startTime));
        sprintf(scoreString,"%d:%02d",(120-(currentTime/1000))/60,(120-(currentTime/1000))%60);
        drawString(scoreString,screen,0,0);

        // Update the screen
        SDL_Flip(screen);

        if (currentTime/1000>120)
            done=1;

    } // end main loop

    totalTime += SDL_GetTicks() - startTime;

    //cleanup
    freeSound();
}

void singlePlayerSprint(SDL_Surface* screen)
{
    //used to calculate time passed since start of game
    int startTime = SDL_GetTicks();
    int totalTime = 0;

    //sound effects
    //initialize SDL_mixer
    initSound();
    int lockChannel;
    int moveChannel;

    //screen rectangle
    SDL_Rect screenrect= {0,0,screen->w,screen->h};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    Player player;

    initPlayer(&player);

    // millisecond tracking variable
    int ticks=SDL_GetTicks();

    // fast drop (is user holding moveDown key?)
    int fastDrop = 0;
    // fast slide (is user holding down a lateral motion key?)
    int fastSlide = 0; //-1 moves left, 1 moves right
    //-2 moves right when left is released
    //2 moves left when right is released

    //MS delay for fast drop or slide
    const int dropWait = 50;
    const int slideWait = 100;

    int slideTime = 0; //slide time is measured against SDL_GetTicks() to calibrate movement speed

    // hold piece buffer
    Piece bufferPiece;

    // program main loop
    Uint8 done = 0;
    Uint8 paused = 0;
    while (!done)
    {
        // message processing loop
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

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_DOWN:
                    fastDrop = 0;
                    break;

                case SDLK_LEFT:
                    if (fastSlide == -2)
                        fastSlide = 1;
                    else
                        fastSlide = 0;
                    break;

                case SDLK_RIGHT:
                    if (fastSlide == 2)
                        fastSlide = -1;
                    else
                        fastSlide = 0;
                    break;

                default:
                    break;
                }

                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // pause if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    totalTime += SDL_GetTicks() - startTime;
                    if (singlePlayerPauseMenu(screen))
                        return;
                    startTime = SDL_GetTicks();
                }

                else if (event.key.keysym.sym == SDLK_p)
                {
                    totalTime += SDL_GetTicks() - startTime;
                    if (singlePlayerPauseMenu(screen))
                        return;
                    startTime = SDL_GetTicks();
                }

                //play move sound
                moveChannel = Mix_PlayChannel(-1, moveSound, 0);
                Mix_Volume(moveChannel,12);

                switch (event.key.keysym.sym)
                {
                case SDLK_LSHIFT: //swap holdpiece
                    if (!player.swapped)
                    {
                        player.swapped=1;
                        player.held.type=player.active.type;
                        spawnPiece(&player.active,getPiece(player.pieces));
                        player.pieces++;
                    }
                    else if (player.swappable)
                    {
                        bufferPiece.type=player.active.type;
                        player.active.type=player.held.type;
                        player.held.type=bufferPiece.type;
                        spawnPiece(&player.active,player.active.type);
                        player.swappable=0;
                    }
                    break;
                case SDLK_z:
                    rotatePieceLeft(&player.active,&player.grid);
                    break;
                case SDLK_UP:
                    rotatePieceRight(&player.active,&player.grid);
                    break;
                case SDLK_DOWN:
                    ticks=SDL_GetTicks();
                    //1 point per cell for a softdrop
                    player.score+=movePieceDown(&player.active,&player.grid);
                    fastDrop=1;
                    break;
                case SDLK_LEFT:
                    slideTime = SDL_GetTicks();
                    movePieceLeft(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = -1;
                    else if (fastSlide == 1)
                        fastSlide = -2;
                    break;
                case SDLK_RIGHT:
                    slideTime = SDL_GetTicks();
                    movePieceRight(&player.active,&player.grid);
                    if (fastSlide == 0)
                        fastSlide = 1;
                    else if (fastSlide == -1)
                        fastSlide = 2;
                    break;
                case SDLK_SPACE:

                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    ticks=SDL_GetTicks();
                    break;
                default:
                    break;
                }
                break;
            }
            }// end switch
        } // end of message processing

        MSDelay=1000;
        int loop=0;
        while (loop<=(player.lines/10)+1)
        {
            int temp=MSDelay/5;
            MSDelay-=temp;
            loop++;
        }

        if (fastDrop&&MSDelay>dropWait)
        {
            MSDelay=dropWait;
        }

        if (fastSlide&&SDL_GetTicks()-slideTime>slideWait&&!paused)
        {
            if (fastSlide<0)
                movePieceLeft(&player.active,&player.grid);
            if (fastSlide>0)
                movePieceRight(&player.active,&player.grid);
            slideTime = SDL_GetTicks();
        }

        if (SDL_GetTicks()-ticks>MSDelay&&!paused)
        {
            if (!movePieceDown(&player.active,&player.grid))
            {
                if (SDL_GetTicks()-ticks>slideDelay)
                {
                    if (!scoreDrop(&player,screen))
                    {
                        done=1;
                        break;
                    }
                    lockChannel = Mix_PlayChannel(-1,lockSound,0);
                    Mix_Volume(lockChannel,90);
                    ticks=SDL_GetTicks();
                }
            }
            else
            {
                if (fastDrop)
                    player.score+=1;
                ticks=SDL_GetTicks();
            }
        }

        // DRAWING STARTS HERE

        // DRAW BACKGROUND

        SDL_FillRect(screen,&screenrect,SDL_MapRGB(screen->format,127,127,127));

        // DRAW GAME

        drawGame(player,screen);

        // DRAW ELAPSED TIME

        char scoreString[5];
        int currentTime = (totalTime+(SDL_GetTicks()-startTime));
        sprintf(scoreString,"%d:%02d",(currentTime/1000)/60,(currentTime/1000)%60);
        drawString(scoreString,screen,0,0);

        // Update the screen
        SDL_Flip(screen);

        if (player.lines>39)
            done=1;

    } // end main loop

    totalTime += SDL_GetTicks() - startTime;

    //cleanup
    freeSound();
}

int singlePlayerPauseMenu(SDL_Surface *screen)
{
    drawString("ESC - Quit to main menu",screen,BLOCKSIZE*GRIDXSIZE/2,BLOCKSIZE*GRIDYSIZE/2-CHARHEIGHT);
    drawString("Any other key will resume",screen,BLOCKSIZE*GRIDXSIZE/2,BLOCKSIZE*GRIDYSIZE/2);
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
                if (event.key.keysym.sym == SDLK_ESCAPE)
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

    static int clearChannel = 0;

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
    spawnPiece(&player->active,getPiece(player->pieces));
    player->pieces+=1;
    player->swappable=1;
    return 1;
}

