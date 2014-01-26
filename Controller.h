#include <SDL/SDL.h>

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define KEYBOARD 255

typedef struct Controller
{
    Uint8 keyboard; //joystick ID if contoller, KEYBOARD if keyboard
    int rotateLeft;
    int rotateRight;
    int moveDown;
    int moveLeft;
    int moveRight;
    int hardDrop;
    int pause;
    int hold;
    int quit;
    int attackHorizontal;
    int attackVertical;
    int attackDiagonal;
} Controller;

#endif // CONTROLLER_H
