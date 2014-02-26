#include <SDL/SDL_mixer.h>

#ifndef SOUND_H
#define SOUND_H

void initSound();

void playMoveSound();

void playLockSound();

void playAttackSound();

void startMusic();

void toggleSound();

void toggleMusic();

#endif // SOUND_H
