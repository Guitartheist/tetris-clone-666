#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include "Block.h"
#include "Piece.h"
#include "Grid.h"
#include "Player.h"
#include "DrawText.h"

void singlePlayerLoop(SDL_Surface* ,int );

int main ( int argc, char** argv )
{
    // seed random number generator
    srand(time(0));

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
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

        drawString("Enter LEVEL 0-9 or ESC to QUIT \1",screen,100,screen->h/2);

        SDL_Flip(screen);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = 1;
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

void singlePlayerLoop(SDL_Surface* screen, int startingLevel )
{
    int MSDelay; //millisecond delay

    // Surface for drawing tetris grid
    SDL_Surface *tetrisGrid = SDL_CreateRGBSurface(0,(GRIDXSIZE*BLOCKSIZE)+1,(GRIDYSIZE*BLOCKSIZE)+1,32,0,0,0,0);
    SDL_Rect tetrisRect = {BLOCKSIZE*3,0,tetrisGrid->w,tetrisGrid->h};

    // spawn a piece
    Piece testPiece;
    spawnPiece(&testPiece,getPiece(0));

    // buffer for processing dropPiece
    int dropTest=0;

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

    // slide lock variables
    int slideDelay = 1000;
    Uint8 sliding = 0;

    // fast drop (is user holding moveDown key?)
    int fastDrop = 0;

    // program main loop
    int done = 0;
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
                done = 1;
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_DOWN)
                    fastDrop = 0;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // exit if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = 1;

                switch (event.key.keysym.sym)
                {
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
                    movePieceLeft(&testPiece,&testGrid);
                    break;
                case SDLK_RIGHT:
                    movePieceRight(&testPiece,&testGrid);
                    break;
                case SDLK_SPACE:
                    ticks=0;
                    dropTest=dropPiece(&testPiece,&testGrid,tetrisGrid,&score,level);
                    if (dropTest<0)
                    {
                        done=1;
                        break;
                    }
                    lines+=dropTest;
                    spawnPiece(&testPiece,getPiece(dropped));
                    dropped++;
                    break;
                default:
                    break;
                }
                break;
            }
            } // end switch
        } // end of message processing



        switch (level)
        {
        default:
        case 0:
            MSDelay=1000;
            break;
        case 1:
            MSDelay=800;
            break;
        case 2:
            MSDelay=600;
            break;
        case 3:
            MSDelay=525;
            break;
        case 4:
            MSDelay=450;
            break;
        case 5:
            MSDelay=375;
            break;
        case 6:
            MSDelay=300;
            break;
        case 7:
            MSDelay=225;
            break;
        case 8:
            MSDelay=175;
            break;
        case 9:
            MSDelay=125;
            break;
        }

        if (fastDrop)
            MSDelay=50;

        if (SDL_GetTicks()-ticks>MSDelay)
        {
            if (!movePieceDown(&testPiece,&testGrid))
            {
                dropTest=dropPiece(&testPiece,&testGrid,tetrisGrid,&score,level);
                if (dropTest<0)
                {
                    done=1;
                    break;
                }
                spawnPiece(&testPiece,getPiece(dropped));
                dropped++;
            }
            ticks=SDL_GetTicks();
        }

        // DRAWING STARTS HERE

        // CLEAR DRAWING AREA

        SDL_Rect screenrect= {0,0,screen->w,screen->h};
        SDL_FillRect(screen,&screenrect,SDL_MapRGB(screen->format, 0, 0, 0));

        // DRAW GRID AND PIECES

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

        if (level<9)
            level=(lines/10)+startingLevel;

        drawString("Level:",screen,0,0);
        sprintf(scoreString,"%i",level);
        drawString(scoreString,screen,0,12);
        drawString("Score:",screen,0,24);
        sprintf(scoreString,"%i",score);
        drawString(scoreString,screen,0,36);
        drawString("Lines:",screen,0,48);
        sprintf(scoreString,"%i",lines);
        drawString(scoreString,screen,0,60);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop
}

