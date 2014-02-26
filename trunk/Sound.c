#include "Sound.h"

Mix_Chunk *lockSound = NULL;

static int muteMusic = 0;

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
        Mix_Volume(lockChannel,127);
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
    music=Mix_LoadMUS("sounds/djentris.wav");
    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(99);
}

void toggleSound()
{
    if (muteSound)
    {
        muteSound=0;
        playLockSound();
    }
    else
    {
        muteSound=1;
    }
}

void toggleMusic()
{
    if (!muteMusic)
    {
        Mix_PauseMusic();
        muteMusic=1;
    }
    else
    {
        Mix_ResumeMusic();
        muteMusic=0;
    }
}
