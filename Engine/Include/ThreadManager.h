#pragma once

#include "ThreadBase.h"

class CThreadManager
{
private:
	CThreadManager();
	~CThreadManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CThreadBase>> mThreadMap;

public:
	bool Init();
	bool DeleteThread(const std::string& Name);

public:
	template <typename T>
	std::weak_ptr<T> FindThread(const std::string& Name)
	{
		auto	iter = mThreadMap.find(Name);

		if (iter == mThreadMap.end())
			return std::weak_ptr<T>();

		return std::dynamic_pointer_cast<T>(iter->second);
	}

	template <typename T>
	std::shared_ptr<T> Create(const std::string& Name,
		bool Pause = false)
	{
		auto	iter = mThreadMap.find(Name);

		if (iter != mThreadMap.end())
			return std::dynamic_pointer_cast<T>(iter->second);

		std::shared_ptr<CThreadBase>	Thread;

		Thread.reset(new T);

		Thread->SetName(Name);

		if (!Thread->Init(Pause))
		{
			return std::shared_ptr<T>();
		}

		mThreadMap.insert(std::make_pair(Name, Thread));

		return std::dynamic_pointer_cast<T>(Thread);
	}

private:
	static CThreadManager* mInst;

public:
	static CThreadManager* GetInst()
	{
		if (!mInst)
			mInst = new CThreadManager;
		return mInst;
	}

	static void DestroyInst()
	{
		SAFE_DELETE(mInst);
	}
};

