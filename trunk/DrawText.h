#include <SDL/SDL.h>
#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#define CHARHEIGHT 12
#define CHARWIDTH 8

void drawString(char *toDraw ,SDL_Surface* screen, int x, int y);
void drawChar(char toDraw, SDL_Surface* screen, int x, int y);
#endif
