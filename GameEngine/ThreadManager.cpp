#include "ThreadManager.h"

CThreadManager* CThreadManager::mInst = nullptr;

CThreadManager::CThreadManager()
{
}

CThreadManager::~CThreadManager()
{
	auto	iter = mThreadMap.begin();
	auto	iterEnd = mThreadMap.end();

	for (; iter != iterEnd; ++iter)
	{
		iter->second->Exit();
	}
}

bool CThreadManager::Init()
{
	return true;
}

bool CThreadManager::DeleteThread(const std::string& Name)
{
	auto	iter = mThreadMap.find(Name);

	if (iter == mThreadMap.end())
		return false;

	iter->second->Exit();

	mThreadMap.erase(iter);

	return true;
}
