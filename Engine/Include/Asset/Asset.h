#pragma once

#include "../Object.h"

class CAsset :
    public CObject
{
	friend class CAssetManager;
	friend class CWorldAssetManager;

protected:
	CAsset();
	CAsset(const CAsset& ref);
	CAsset(CAsset&& ref)	noexcept;

public:
	virtual ~CAsset() = 0;

protected:
	std::string         mName;
	std::string         mSplitName;
	size_t		mID;
	EAssetType	mAssetType = EAssetType::None;
	bool		mKeep = false;

public:
	bool GetKeep()	const
	{
		return mKeep;
	}

	size_t GetID()	const
	{
		return mID;
	}

	const std::string& GetName()    const
	{
		return mName;
	}

	const std::string& GetSplitName()    const
	{
		return mSplitName;
	}

	EAssetType GetAssetType()	const
	{
		return mAssetType;
	}

public:
	void SetKeep(bool Keep)
	{
		mKeep = Keep;
	}

	void SetName(const std::string& Name)
	{
		mName = Name;
		mSplitName = Name;

		mID = std::hash<std::string>{}(Name);

		size_t	Pos = mSplitName.find('_');

		if (Pos != std::string::npos)
		{
			mSplitName.erase(0, Pos + 1);
		}
	}
};

