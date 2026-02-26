#include "SoundManager.h"

CSoundManager::CSoundManager()
{
}

CSoundManager::~CSoundManager()
{
	mSoundMap.clear();

	auto	iter = mChannelGroupMap.begin();
	auto	iterEnd = mChannelGroupMap.end();

	for (; iter != iterEnd; ++iter)
	{
		iter->second->release();
	}

	if (mSystem)
	{
		mSystem->release();
		mSystem->close();
	}
}

bool CSoundManager::Init()
{
	FMOD_RESULT result = FMOD::System_Create(&mSystem);

	if (result != FMOD_OK)
		return false;

	result = mSystem->init(256, FMOD_INIT_NORMAL, nullptr);

	if (result != FMOD_OK)
		return false;

	result = mSystem->getMasterChannelGroup(&mMasterGroup);

	if (result != FMOD_OK)
		return false;

	mChannelGroupMap.insert(std::make_pair("Master", mMasterGroup));

	CreateChannelGroup("BGM");
	CreateChannelGroup("Effect");
	CreateChannelGroup("UI");

	return true;
}

void CSoundManager::Update()
{
	mSystem->update();
}

bool CSoundManager::CreateChannelGroup(const std::string& Name)
{
	FMOD::ChannelGroup* Group = FindChannelGroup(Name);

	if (Group)
		return true;

	FMOD_RESULT	result = mSystem->createChannelGroup(Name.c_str(),
		&Group);

	if (result != FMOD_OK)
		return false;

	// 생성된 그룹을 마스터 그룹에 추가한다.
	mMasterGroup->addGroup(Group, false);

	mChannelGroupMap.insert(std::make_pair(Name, Group));

	return true;
}

bool CSoundManager::LoadSound(const std::string& Name,
	const std::string& GroupName, bool Loop, const char* FileName,
	const std::string& PathName)
{
	if (!FindSound(Name).expired())
		return true;

	FMOD::ChannelGroup* Group = FindChannelGroup(GroupName);

	if (!Group)
		Group = mMasterGroup;

	std::shared_ptr<CSound>	Sound;
	Sound.reset(new CSound);

	Sound->SetName(Name);

	if (!Sound->LoadSound(mSystem, Group, Loop, FileName, PathName))
		return false;

	mSoundMap.insert(std::make_pair(Name, Sound));

	return true;
}

std::weak_ptr<CSound> CSoundManager::FindSound(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter == mSoundMap.end())
		return std::weak_ptr<CSound>();

	return iter->second;
}

void CSoundManager::Play(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter == mSoundMap.end())
		return;

	iter->second->Play();
}

void CSoundManager::Stop(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter == mSoundMap.end())
		return;

	iter->second->Stop();
}

void CSoundManager::Pause(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter == mSoundMap.end())
		return;

	iter->second->Pause();
}

void CSoundManager::Resume(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter == mSoundMap.end())
		return;

	iter->second->Resume();
}

void CSoundManager::SetMasterVolume(int Volume)
{
	if (Volume < 0)
		Volume = 0;

	else if (Volume > 100)
		Volume = 100;

	mMasterGroup->setVolume(Volume / 100.f);
}

void CSoundManager::SetGroupVolume(const std::string& GroupName,
	int Volume)
{
	FMOD::ChannelGroup* Group = FindChannelGroup(GroupName);

	if (!Group)
		return;

	if (Volume < 0)
		Volume = 0;

	else if (Volume > 100)
		Volume = 100;

	Group->setVolume(Volume / 100.f);
}

FMOD::ChannelGroup* CSoundManager::FindChannelGroup(
	const std::string& Name)
{
	auto	iter = mChannelGroupMap.find(Name);

	if (iter == mChannelGroupMap.end())
		return nullptr;

	return iter->second;
}

void CSoundManager::ReleaseAsset(const std::string& Name)
{
	auto	iter = mSoundMap.find(Name);

	if (iter != mSoundMap.end())
	{
		// 다른 월드에서 더이상 들고 있지 않을 경우
		if (iter->second.use_count() == 1)
		{
			mSoundMap.erase(iter);
		}
	}
}
