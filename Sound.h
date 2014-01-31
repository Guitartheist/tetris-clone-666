#include <SDL/SDL_mixer.h>

#ifndef SOUND_H
#define SOUND_H

void initSound();

void playMoveSound();

void playLockSound();

void playAttackSound();

void startMusic();

void stopMusic();

#endif // SOUND_H
