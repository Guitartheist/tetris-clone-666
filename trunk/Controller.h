#ifndef CONTROLLER_H
#define CONTROLLER_H

typedef struct Controller
{
    Uint8 keyboard; //0 if contoller, 1 if keyboard
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
