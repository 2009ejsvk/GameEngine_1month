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
	EAssetType	mAssetType = EAssetType::None;

public:
	const std::string& GetName()    const
	{
		return mName;
	}

	EAssetType GetAssetType()	const
	{
		return mAssetType;
	}

public:
	void SetName(const std::string& Name)
	{
		mName = Name;
	}
};

