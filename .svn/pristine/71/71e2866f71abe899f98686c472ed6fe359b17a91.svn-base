#pragma once

#include "EngineInfo.h"

class CCollisionInfoManager
{
private:
	CCollisionInfoManager();
	~CCollisionInfoManager();

private:
	std::unordered_map<std::string, FCollisionChannel*>	mChannelMap;
	ECollisionChannel::Type		mCreateChannel = ECollisionChannel::Static;

	std::unordered_map<std::string, FCollisionProfile*>	mProfileMap;

public:
	bool Init();
	void CreateChannel(const std::string& Name);
	bool CreateProfile(const std::string& Name,
		const std::string& ChannelName, bool Enable,
		ECollisionInteraction::Type DefaultInteraction =
		ECollisionInteraction::Collision);
	bool CreateProfile(const std::string& Name,
		ECollisionChannel::Type Channel, bool Enable,
		ECollisionInteraction::Type DefaultInteraction =
		ECollisionInteraction::Collision);
	bool SetProfileInteraction(const std::string& Name,
		const std::string& ChannelName,
		ECollisionInteraction::Type Interaction);
	bool SetProfileInteraction(const std::string& Name,
		ECollisionChannel::Type Channel,
		ECollisionInteraction::Type Interaction);
	bool SetProfileEnable(const std::string& Name, bool Enable);

	FCollisionProfile* FindProfile(const std::string& Name);

private:
	FCollisionChannel* FindChannel(const std::string& Name);

private:
	static CCollisionInfoManager* mInst;

public:
	static CCollisionInfoManager* GetInst()
	{
		if (!mInst)
			mInst = new CCollisionInfoManager;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

