#ifndef PIECE_H
#define PIECE_H

#include <SDL/SDL.h>
#include "Block.h"
#include "Grid.h"

enum PieceShape{Z,L,I,T,O,J,S};
enum Direction{UP,RIGHT,DOWN,LEFT};

typedef struct Piece
{
    int x;
    int y;
    enum Direction direction;
    enum PieceShape type;
} Piece;

typedef struct PieceList PieceList;

struct PieceList
{
    enum PieceShape val;
    PieceList *next;
};

int roll(int maximum);
void resetPieceLists();

int getPiece(int piece);
void rollPieceSet(); //roll a set of 7 pieces

void spawnPiece(Piece*,enum PieceShape);
void createPiece(Piece*,enum PieceShape,int,int);
void rotatePieceRight(Piece*,Grid*);
void rotatePieceLeft(Piece*,Grid*);
void movePieceRight(Piece*,Grid*);
void movePieceLeft(Piece*,Grid*);
int movePieceDown(Piece*,Grid*); //return 1 if piece did not collide with highest non-null grid element
int dropPiece(Piece*,Grid*,SDL_Surface*,int*,int); //return -1 if player has topped out, add appropriate points to score
void drawPiece(Piece* const,SDL_Surface*);
void getPieceCollision(Piece* const,int[8]);

#endif // PIECE_H
