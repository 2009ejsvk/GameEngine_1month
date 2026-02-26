#pragma once

#include "Sound.h"

class CSoundManager
{
	friend class CAssetManager;

private:
	CSoundManager();

public:
	~CSoundManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CSound>>	mSoundMap;
	std::unordered_map<std::string, FMOD::ChannelGroup*>	mChannelGroupMap;
	FMOD::System* mSystem = nullptr;
	FMOD::ChannelGroup* mMasterGroup = nullptr;

public:
	bool Init();
	void Update();
	bool CreateChannelGroup(const std::string& Name);
	bool LoadSound(const std::string& Name,
		const std::string& GroupName, bool Loop, const char* FileName,
		const std::string& PathName = "Sound");
	std::weak_ptr<CSound> FindSound(const std::string& Name);

	void Play(const std::string& Name);
	void Stop(const std::string& Name);
	void Pause(const std::string& Name);
	void Resume(const std::string& Name);
	// Volume : 0 ~ 100
	void SetMasterVolume(int Volume);
	void SetGroupVolume(const std::string & GroupName, int Volume);

private:
	FMOD::ChannelGroup* FindChannelGroup(const std::string& Name);

public:
	void ReleaseAsset(const std::string& Name);
};

