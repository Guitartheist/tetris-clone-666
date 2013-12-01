#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "Block.h"
#include "Piece.h"
#include "Grid.h"
#include "Player.h"
#include "DrawText.h"

void singlePlayerLoop(SDL_Surface* ,int );

//process a dropped piece, return 0 if game is over
int scoreDrop(Piece *piece, Grid *grid, SDL_Surface *surface, int *score, int *lines, int *dropped, int level, Uint8 *swappable);

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
    while (!done)
    {
        resetPieceLists();

        drawString("\1 Left SHIFT key swaps hold piece with active piece",screen,30,screen->h/2-CHARHEIGHT*4);
        drawString("\1 Left/Right arrow moves left/right P: pause",screen,30,screen->h/2-CHARHEIGHT*3);
        drawString("\1 Z: rotate left, Up arrow: rotate right",screen,30,screen->h/2-CHARHEIGHT*2);
        drawString("\1 Space: hard drop, Down arrow: soft drop",screen,30,screen->h/2-CHARHEIGHT);
        drawString("\1 Press 0 1 2 3 4 5 6 7 8 or 9 to start",screen,30,screen->h/2);

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

                default:
                    break;

                case SDLK_0:
                    singlePlayerLoop(screen,0);
                    break;

                case SDLK_1:
                    singlePlayerLoop(screen,1);
                    break;

                case SDLK_2:
                    singlePlayerLoop(screen,2);
                    break;

                case SDLK_3:
                    singlePlayerLoop(screen,3);
                    break;

                case SDLK_4:
                    singlePlayerLoop(screen,4);
                    break;

                case SDLK_5:
                    singlePlayerLoop(screen,5);
                    break;

                case SDLK_6:
                    singlePlayerLoop(screen,6);
                    break;

                case SDLK_7:
                    singlePlayerLoop(screen,7);
                    break;

                case SDLK_8:
                    singlePlayerLoop(screen,8);
                    break;

                case SDLK_9:
                    singlePlayerLoop(screen,9);
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
Mix_Chunk *singleSound = NULL;
Mix_Chunk *doubleSound = NULL;
Mix_Chunk *tripleSound = NULL;
Mix_Chunk *tetrisSound = NULL;

void singlePlayerLoop(SDL_Surface* screen, int startingLevel )
{
    //sound effects
    //initialize SDL_mixer
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_AllocateChannels(128);
    moveSound = Mix_LoadWAV("sounds/move.wav");
    lockSound = Mix_LoadWAV("sounds/lock.wav");
    singleSound = Mix_LoadWAV("sounds/single.wav");
    doubleSound = Mix_LoadWAV("sounds/double.wav");
    tripleSound = Mix_LoadWAV("sounds/triple.wav");
    tetrisSound = Mix_LoadWAV("sounds/tetris.wav");
    int lockChannel;
    int moveChannel;

    //screen rectangle
    SDL_Rect screenrect= {0,0,screen->w,screen->h};

    int MSDelay; //millisecond delay
    const int slideDelay=1000; //delay before automatically locking a piece to the grid

    // Surface for drawing tetris grid
    SDL_Surface *tetrisGrid = SDL_CreateRGBSurface(0,(GRIDXSIZE*BLOCKSIZE)+1,(GRIDYSIZE*BLOCKSIZE)+1,32,0,0,0,0);
    SDL_Rect tetrisRect = {BLOCKSIZE*3+BLOCKSIZE/2,0,tetrisGrid->w,tetrisGrid->h};

    // spawn a piece
    Piece testPiece;
    spawnPiece(&testPiece,getPiece(0));

    // number of pieces dropped so far
    int dropped=1;

    // Level of difficulty and lines completed
    int level=1;
    int lines=0;

    // SCORE
    int score=0;
    char *scoreString = (char*)malloc(sizeof(char)*10);

    // create a grid
    Grid testGrid;
    clearGrid(&testGrid);

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

    // hold piece
    Uint8 swapped = 0; //set to 1 when hold is used for the first time
    Uint8 swappable = 0; //set to 0 when hold is used, 1 when hold piece is locked into the grid
    Piece holdPiece;
    createPiece(&holdPiece,0,BLOCKSIZE,BLOCKSIZE*7.5);
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
                // exit if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    if (paused)
                        done = 1;
                    else
                        paused = 1;

                else if (event.key.keysym.sym == SDLK_p)
                {
                    if (paused)
                        paused=0;
                    else
                        paused=1;
                }

                if (!paused)
                {
                    //play move sound
                    moveChannel = Mix_PlayChannel(-1, moveSound, 0);
                    Mix_Volume(moveChannel,10);

                    switch (event.key.keysym.sym)
                    {
                    case SDLK_LSHIFT: //swap holdpiece
                        if (!swapped)
                        {
                            swapped=1;
                            holdPiece.type=testPiece.type;
                            holdPiece.direction=RIGHT;
                            spawnPiece(&testPiece,getPiece(dropped));
                            dropped++;
                        }
                        else if (swappable)
                        {
                            bufferPiece.type=testPiece.type;
                            testPiece.type=holdPiece.type;
                            holdPiece.type=bufferPiece.type;
                            spawnPiece(&testPiece,testPiece.type);
                            swappable=0;
                        }
                        break;
                    case SDLK_z:
                        rotatePieceLeft(&testPiece,&testGrid);
                        break;
                    case SDLK_UP:
                        rotatePieceRight(&testPiece,&testGrid);
                        break;
                    case SDLK_DOWN:
                        ticks=SDL_GetTicks();
                        //1 point per cell for a softdrop
                        score+=movePieceDown(&testPiece,&testGrid);
                        fastDrop=1;
                        break;
                    case SDLK_LEFT:
                        slideTime = SDL_GetTicks();
                        movePieceLeft(&testPiece,&testGrid);
                        if (fastSlide == 0)
                            fastSlide = -1;
                        else if (fastSlide == 1)
                            fastSlide = -2;
                        break;
                    case SDLK_RIGHT:
                        slideTime = SDL_GetTicks();
                        movePieceRight(&testPiece,&testGrid);
                        if (fastSlide == 0)
                            fastSlide = 1;
                        else if (fastSlide == -1)
                            fastSlide = 2;
                        break;
                    case SDLK_SPACE:

                        lockChannel = Mix_PlayChannel(-1,lockSound,0);
                        Mix_Volume(lockChannel,90);
                        if (!scoreDrop(&testPiece,&testGrid,tetrisGrid,&score,&lines,&dropped,level,&swappable))
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
            }
            }// end switch
        } // end of message processing

        level=(lines/10)+startingLevel;

        MSDelay=1000;
        int loop=0;
        while (loop<=level)
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
                movePieceLeft(&testPiece,&testGrid);
            if (fastSlide>0)
                movePieceRight(&testPiece,&testGrid);
            slideTime = SDL_GetTicks();
        }

        if (SDL_GetTicks()-ticks>MSDelay&&!paused)
        {
            if (!movePieceDown(&testPiece,&testGrid))
            {
                if (SDL_GetTicks()-ticks>slideDelay)
                {
                    if (!scoreDrop(&testPiece,&testGrid,tetrisGrid,&score,&lines,&dropped,level,&swappable))
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
                    score+=1;
                ticks=SDL_GetTicks();
            }
        }

        // DRAWING STARTS HERE

        // CLEAR DRAWING AREA

        SDL_FillRect(screen,&screenrect,SDL_MapRGB(screen->format,127,127,127));

        // DRAW GRID AND PIECE THAT PLAYER IS CURRENTLY CONTROLLING

        drawGame(&testGrid,&testPiece,NOTHING,tetrisGrid,&score,level);
        SDL_BlitSurface(tetrisGrid,0,screen,&tetrisRect);

        // DRAW NEXT 5 PIECES
        drawString("Next 5 Pieces:",screen,BLOCKSIZE*GRIDXSIZE*1.5-(BLOCKSIZE),0);
        Piece nextFive[5];
        createPiece(&nextFive[0],getPiece(dropped),BLOCKSIZE*GRIDXSIZE*1.5+BLOCKSIZE,BLOCKSIZE*2);
        createPiece(&nextFive[1],getPiece(dropped+1),BLOCKSIZE*GRIDXSIZE*1.5+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4));
        createPiece(&nextFive[2],getPiece(dropped+2),BLOCKSIZE*GRIDXSIZE*1.5+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*2);
        createPiece(&nextFive[3],getPiece(dropped+3),BLOCKSIZE*GRIDXSIZE*1.5+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*3);
        createPiece(&nextFive[4],getPiece(dropped+4),BLOCKSIZE*GRIDXSIZE*1.5+BLOCKSIZE,(BLOCKSIZE*2)+(BLOCKSIZE*4)*4);
        int i;
        for (i=0; i<5; i++)
            drawPiece(&nextFive[i],screen);

        //Draw Scoring information

        drawString("Level:",screen,0,0);
        sprintf(scoreString,"%i",level);
        drawString(scoreString,screen,0,CHARHEIGHT);
        drawString("Score:",screen,0,CHARHEIGHT*2);
        sprintf(scoreString,"%i",score);
        drawString(scoreString,screen,0,CHARHEIGHT*3);
        drawString("Lines:",screen,0,CHARHEIGHT*4);
        sprintf(scoreString,"%i",lines);
        drawString(scoreString,screen,0,CHARHEIGHT*5);

        //Draw hold piece information

        drawString("Holding:",screen,0,CHARHEIGHT*7);
        if (swapped)
            drawPiece(&holdPiece,screen);

        //Display pause state

        if (paused)
            drawString("(P)aused",screen,screen->w/2,screen->h/2);

        // DRAWING ENDS HERE

        // Update the screen
        SDL_Flip(screen);
    } // end main loop

    //cleanup sound effects
    Mix_FreeChunk(lockSound);
    Mix_FreeChunk(moveSound);
    Mix_FreeChunk(singleSound);
    Mix_FreeChunk(doubleSound);
    Mix_FreeChunk(tripleSound);
    Mix_FreeChunk(tetrisSound);
    Mix_CloseAudio();
}

//Return 0 if dropping the latest piece has ended the game
//Return 1 if game should continue
int scoreDrop(Piece *piece, Grid *grid, SDL_Surface *surface, int *score, int *lines, int *dropped, int level, Uint8 *swappable)
{
    int dropTest = dropPiece(piece,grid,surface,score,level);
    if (dropTest<0)
        return 0;
    *lines+=dropTest;

    int clearChannel;

    switch(dropTest)
    {
    case 4:
        clearChannel = Mix_PlayChannel(-1,tetrisSound,0);
        Mix_Volume(clearChannel,80);
    case 3:
        clearChannel = Mix_PlayChannel(-1,tripleSound,0);
        Mix_Volume(clearChannel,80);
    case 2:
        clearChannel = Mix_PlayChannel(-1,doubleSound,0);
        Mix_Volume(clearChannel,80);
    case 1:
        clearChannel = Mix_PlayChannel(-1,singleSound,0);
        Mix_Volume(clearChannel,80);
        break;
    }
    spawnPiece(piece,getPiece(*dropped));
    *dropped+=1;
    *swappable=1;
    return 1;
}

