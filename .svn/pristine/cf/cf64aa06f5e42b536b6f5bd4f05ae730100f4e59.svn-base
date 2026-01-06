#include "CollisionInfoManager.h"

CCollisionInfoManager* CCollisionInfoManager::mInst = nullptr;

CCollisionInfoManager::CCollisionInfoManager()
{
}

CCollisionInfoManager::~CCollisionInfoManager()
{
	auto	iter = mProfileMap.begin();
	auto	iterEnd = mProfileMap.end();

	for (; iter != iterEnd; ++iter)
	{
		SAFE_DELETE(iter->second);
	}

	auto	iter1 = mChannelMap.begin();
	auto	iter1End = mChannelMap.end();

	for (; iter1 != iter1End; ++iter1)
	{
		SAFE_DELETE(iter1->second);
	}
}

bool CCollisionInfoManager::Init()
{
	CreateChannel("Static");
	CreateChannel("Player");
	CreateChannel("Monster");

	CreateProfile("Static", "Static", true);
	CreateProfile("Player", "Player", true);
	CreateProfile("Monster", "Monster", true);

	return true;
}

void CCollisionInfoManager::CreateChannel(const std::string& Name)
{
	if (mCreateChannel == ECollisionChannel::End)
		return;

	FCollisionChannel* Channel = FindChannel(Name);

	if (Channel)
		return;

	Channel = new FCollisionChannel;

	Channel->Name = Name;
	Channel->Channel = mCreateChannel;

	mCreateChannel = (ECollisionChannel::Type)(mCreateChannel + 1);

	mChannelMap.insert(std::make_pair(Name, Channel));
}

bool CCollisionInfoManager::CreateProfile(const std::string& Name, 
	const std::string& ChannelName, bool Enable,
	ECollisionInteraction::Type DefaultInteraction)
{
	FCollisionProfile* Profile = FindProfile(Name);

	if (Profile)
		return true;

	FCollisionChannel* Channel = FindChannel(ChannelName);

	if (!Channel)
		return false;

	Profile = new FCollisionProfile;

	Profile->Name = Name;
	Profile->Channel = Channel;
	Profile->Enable = Enable;

	for (int i = 0; i < ECollisionChannel::End; ++i)
	{
		Profile->Interaction[i] = DefaultInteraction;
	}

	mProfileMap.insert(std::make_pair(Name, Profile));

	return true;
}

bool CCollisionInfoManager::CreateProfile(const std::string& Name,
	ECollisionChannel::Type Channel, bool Enable,
	ECollisionInteraction::Type DefaultInteraction)
{
	FCollisionProfile* Profile = FindProfile(Name);

	if (Profile)
		return true;

	FCollisionChannel* ChannelObject = nullptr;

	auto	iter = mChannelMap.begin();
	auto	iterEnd = mChannelMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (iter->second->Channel == Channel)
		{
			ChannelObject = iter->second;
			break;
		}
	}

	if (!ChannelObject)
		return false;

	Profile = new FCollisionProfile;

	Profile->Name = Name;
	Profile->Channel = ChannelObject;
	Profile->Enable = Enable;

	for (int i = 0; i < ECollisionChannel::End; ++i)
	{
		Profile->Interaction[i] = DefaultInteraction;
	}

	mProfileMap.insert(std::make_pair(Name, Profile));

	return true;
}

bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
	const std::string& ChannelName, ECollisionInteraction::Type Interaction)
{
	FCollisionProfile* Profile = FindProfile(Name);

	if (!Profile)
		return false;

	FCollisionChannel* Channel = FindChannel(ChannelName);

	if (!Channel)
		return false;

	Profile->Interaction[Channel->Channel] = Interaction;

	return true;
}

bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
	ECollisionChannel::Type Channel, ECollisionInteraction::Type Interaction)
{
	FCollisionProfile* Profile = FindProfile(Name);

	if (!Profile)
		return false;

	Profile->Interaction[Channel] = Interaction;

	return true;
}

bool CCollisionInfoManager::SetProfileEnable(const std::string& Name,
	bool Enable)
{
	FCollisionProfile* Profile = FindProfile(Name);

	if (!Profile)
		return false;

	Profile->Enable = Enable;

	return true;
}

FCollisionProfile* CCollisionInfoManager::FindProfile(const std::string& Name)
{
	auto	iter = mProfileMap.find(Name);

	if (iter == mProfileMap.end())
		return nullptr;

	return iter->second;
}

FCollisionChannel* CCollisionInfoManager::FindChannel(const std::string& Name)
{
	auto	iter = mChannelMap.find(Name);

	if (iter == mChannelMap.end())
		return nullptr;

	return iter->second;
}
