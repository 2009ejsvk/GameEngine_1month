#pragma once

#include "../Asset.h"
#include "fmod.hpp"


#pragma comment(lib, "fmod_vc.lib")

class CSound :
    public CAsset
{
    friend class CSoundManager;

protected:
    CSound();

public:
    virtual ~CSound();

protected:
    FMOD::System* mSystem = nullptr;
    FMOD::ChannelGroup* mGroup = nullptr;
    FMOD::Channel* mChannel = nullptr;
    FMOD::Sound* mSound = nullptr;

    int         mPlayCount = 0;
    bool        mLoop = false;

public:
    bool LoadSound(FMOD::System* System, FMOD::ChannelGroup* Group,
        bool Loop, const char* FileName,
        const std::string& PathName = "Sound");
    void Play();
    void Stop();
    void Pause();
    void Resume();

public:
    static FMOD_RESULT SoundPlayEnd(
        FMOD_CHANNELCONTROL* channelcontrol, 
        FMOD_CHANNELCONTROL_TYPE controltype, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
        void* commanddata1, void* commanddata2);
};

