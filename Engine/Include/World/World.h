#pragma once

#include "../EngineInfo.h"
#include "../Object/GameObject.h"
#include "CameraManager.h"
#include "WorldAssetManager.h"
#include "Input.h"
#include "WorldCollision.h"
#include "WorldUIManager.h"
#include "WorldNavigation.h"

// enable_shared_from_this

class CWorld
{
	friend class CWorldManager;

public:
	CWorld();
	virtual ~CWorld();

protected:
	std::unordered_multimap<std::string, std::shared_ptr<CGameObject>>	mObjList;
	std::vector<std::weak_ptr<CGameObject>>	mStartObjList;
	std::weak_ptr<CWorld>	mSelf;
	std::shared_ptr<CCameraManager>	mCameraManager;
	std::shared_ptr<CWorldAssetManager>	mWorldAssetManager;
	std::shared_ptr<CInput>	mInput;
	std::shared_ptr<CWorldCollision>	mCollision;
	std::shared_ptr<CWorldUIManager>	mUIManager;
	std::shared_ptr<CWorldNavigation>	mNavigation;

public:
	std::weak_ptr<CWorldNavigation> GetNavigation()	const
	{
		return mNavigation;
	}

	std::weak_ptr<CCameraManager> GetCameraManager()	const
	{
		return mCameraManager;
	}

	std::weak_ptr<CWorldAssetManager> GetWorldAssetManager()	const
	{
		return mWorldAssetManager;
	}

	std::weak_ptr<CInput> GetInput()	const
	{
		return mInput;
	}

	std::weak_ptr<CWorldCollision> GetCollision()	const
	{
		return mCollision;
	}

	std::weak_ptr<CWorldUIManager> GetUIManager()	const
	{
		return mUIManager;
	}

public:
	void SetSelf(std::weak_ptr<CWorld> Self)
	{
		mSelf = Self;
	}

public:
	virtual bool IsSimulationPaused() const { return false; }
	virtual int GetSimulationSpeedMultiplier() const { return 1; }

public:
	void InputActive();
	void InputDeactive();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void RenderUI();
	void ClearWorld();
	virtual void OnUiManagerUpdated();
	bool BuildNavigationSnapshot(
		const std::shared_ptr<class CTileMapComponent>& TileMap,
		int EndTileIndex,
		const std::string& PreferredTargetObjectName,
		std::vector<unsigned char>& OutBlockedMask,
		std::vector<int>& OutGoalIndices,
		std::string& OutResolvedTargetObjectName);

private:
	void Begin();
	void BeginManager();

public:
	template <typename T>
	std::weak_ptr<T> CreateGameObject(
		const std::string& Name)
	{
		std::shared_ptr<T> Obj;

		Obj.reset(new T);

		Obj->SetWorld(mSelf);
		Obj->SetSelf(Obj);
		Obj->SetName(Name);

		if (!Obj->Init())
		{
			return std::weak_ptr<T>();
		}

		mObjList.insert(std::make_pair(Name, Obj));

		mStartObjList.push_back(Obj);

		return Obj;
	}


	template <typename T>
	std::weak_ptr<T> CreateCloneGameObject(
		const std::string& Name, const std::weak_ptr<T>& Origin)
	{
		if (Origin.expired())
			return std::weak_ptr<T>();

		auto	CloneShared = Origin.lock();

		T* CloneObj = nullptr;

		if (CloneShared)
			CloneObj = CloneShared->Clone();

		std::shared_ptr<T> Obj;

		Obj.reset(CloneObj);

		Obj->SetWorld(mSelf);
		Obj->SetSelf(Obj);
		Obj->SetName(Name);

		if (!Obj->Init())
		{
			return std::weak_ptr<T>();
		}

		mObjList.insert(std::make_pair(Name, Obj));

		mStartObjList.push_back(Obj);

		return Obj;
	}

	template <typename T>
	std::weak_ptr<T> FindObject(const std::string& Name)
	{
		std::unordered_multimap<std::string, std::shared_ptr<CGameObject>>::iterator	   iter = mObjList.find(Name);

		if (iter == mObjList.end())
			return std::weak_ptr<T>();

		return std::dynamic_pointer_cast<T>(iter->second);
	}

	template <typename T>
	bool FindObjectList(const std::string& Name,
		std::list<std::weak_ptr<T>>& ObjList)
	{
		// pair에는 시작 iterator와 끝 iterator를 받아오게 한다.
		std::pair<std::unordered_multimap<std::string, std::shared_ptr<CGameObject>>::iterator,
			std::unordered_multimap<std::string, std::shared_ptr<CGameObject>>::iterator>   Range =
			mObjList.equal_range(Name);

		// first와 second가 같다면 키를 못찾았다는 의미이다.
		if (Range.first == Range.second)
			return false;

		std::unordered_multimap<std::string, std::shared_ptr<CGameObject>>::iterator	   iter = Range.first;

		for (; iter != Range.second; ++iter)
		{
			ObjList.push_back(std::dynamic_pointer_cast<T>(iter->second));
		}

		return true;
	}

	template <typename T>
	bool FindObjectListByType(
		std::vector<std::weak_ptr<T>>& ObjList)
	{
		ObjList.clear();

		auto iter = mObjList.begin();
		auto iterEnd = mObjList.end();

		for (; iter != iterEnd; ++iter)
		{
			auto Obj = std::dynamic_pointer_cast<T>(iter->second);

			if (!Obj || !Obj->GetAlive() || !Obj->GetEnable())
				continue;

			ObjList.push_back(Obj);
		}

		return !ObjList.empty();
	}
};

