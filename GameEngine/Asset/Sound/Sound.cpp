#include "Sound.h"
#include "../PathManager.h"

CSound::CSound()
{
    mAssetType = EAssetType::Sound;
}

CSound::~CSound()
{
    if (mSound)
        mSound->release();
}

bool CSound::LoadSound(FMOD::System* System, 
    FMOD::ChannelGroup* Group, bool Loop, const char* FileName,
    const std::string& PathName)
{
    mSystem = System;
    mGroup = Group;
    mLoop = Loop;

    char    FullPath[MAX_PATH] = {};

    const char* Path = CPathManager::FindPathMultibyte(PathName);

    if (Path)
        strcpy_s(FullPath, Path);

    strcat_s(FullPath, FileName);

    FMOD_MODE   Mode = FMOD_DEFAULT;

    if (Loop)
        Mode = FMOD_LOOP_NORMAL;

    if (mSystem->createSound(FullPath, Mode, nullptr, &mSound) !=
        FMOD_OK)
        return false;

    return true;
}

void CSound::Play()
{
    // 마지막 인자로 재생되는 채널을 얻어온다.
    mSystem->playSound(mSound, mGroup, false, &mChannel);

    ++mPlayCount;

    // 사운드 재생이 끝날때 호출될 함수를 지정한다.
    mChannel->setCallback(CSound::SoundPlayEnd);
}

void CSound::Stop()
{
    if (mChannel)
    {
        --mPlayCount;

        bool    IsPlaying = false;

        // 현재 채널이 재생중인지 얻어온다.
        mChannel->isPlaying(&IsPlaying);

        if (IsPlaying)
        {
            mChannel->stop();

            if (mPlayCount == 0)
                mChannel = nullptr;
        }

        else
            mChannel = nullptr;
    }
}

void CSound::Pause()
{
    if (mChannel)
    {
        bool    IsPlaying = false;

        // 현재 채널이 재생중인지 얻어온다.
        mChannel->isPlaying(&IsPlaying);

        if (IsPlaying)
            mChannel->setPaused(true);
    }
}

void CSound::Resume()
{
    if (mChannel)
    {
        bool    IsPaused = false;

        mChannel->getPaused(&IsPaused);

        if (IsPaused)
            mChannel->setPaused(false);
    }
}

FMOD_RESULT CSound::SoundPlayEnd(FMOD_CHANNELCONTROL* channelcontrol,
    FMOD_CHANNELCONTROL_TYPE controltype,
    FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
    void* commanddata1, void* commanddata2)
{
    return FMOD_OK;
}
