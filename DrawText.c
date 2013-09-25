#include <string.h>
#include "DrawText.h"

//a single character is drawn 8 pixels wide by 10 pixels high

void drawString(char *toDraw ,SDL_Surface* screen, int x, int y)
{
    int i;
    SDL_Rect background = {x,y,strlen(toDraw)*8,12};
    SDL_FillRect(screen,&background,SDL_MapRGB(screen->format,0,0,0));
    for (i=0;toDraw[i]!='\0';i++)
        drawChar(toDraw[i],screen,x+(i*8),y);
}

void drawChar(char toDraw ,SDL_Surface* screen, int x, int y)
{
    // ASCII characters arranged 16x16 in 12x12pixel blocks
    // 0 1 2 etc...
    static SDL_Surface *asciiText;

    if (asciiText==0)
        asciiText = SDL_LoadBMP("text.bmp");

    SDL_SetColorKey(asciiText,SDL_SRCCOLORKEY,SDL_MapRGB(asciiText->format,255,0,255));

    int drawX, drawY;

    drawX=(toDraw%16)*12;
    drawY=(toDraw/16)*12;

    SDL_Rect asciiPosition={drawX,drawY,12,12};

    SDL_Rect charPosition={x,y,12,12};

    SDL_BlitSurface(asciiText,&asciiPosition,screen,&charPosition);
}
