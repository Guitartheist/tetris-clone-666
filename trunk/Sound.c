#include "Sound.h"

Mix_Chunk *lockSound = NULL;

static int muteMusic = 1;

static int muteSound = 0;

static Mix_Music *music = 0;

void initSound()
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_AllocateChannels(128);
    lockSound = Mix_LoadWAV("sounds/lock.wav");
}

void playLockSound()
{
    //play lock sound
    if (!muteSound)
    {
        int lockChannel = Mix_PlayChannel(-1,lockSound,0);
        Mix_Volume(lockChannel,100);
    }
}

void playAttackSound()
{
    //play attack sound
    if (!muteSound)
    {
        int attackChannel = Mix_PlayChannel(-1,note3,0);
        Mix_Volume(attackChannel,80);
        attackChannel = Mix_PlayChannel(-1,note4,0);
        Mix_Volume(attackChannel,80);
    }
}

int getMuteSound()
{
    return muteSound;
}

int getMuteMusic()
{
    return muteMusic;
}

void startMusic()
{
    if (muteMusic)
    {
        music=Mix_LoadMUS("sounds/djentris.mp3");
        Mix_PlayMusic(music, -1);
        Mix_VolumeMusic(99);
    }
    else
    {
        Mix_ResumeMusic();
        muteMusic=0;
    }
}

void stopMusic()
{
    if (!muteMusic)
    {
        Mix_PauseMusic();
        muteMusic=1;
    }
}
