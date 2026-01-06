#pragma once

#include "EngineInfo.h"

class CObject
{
	friend class CEngine;

public:
	CObject();
	CObject(const CObject& ref);
	CObject(CObject&& ref)	noexcept;
	virtual ~CObject() = 0;

protected:
	size_t mClassType = 0;

protected:
	template <typename T>
	void SetClassType()
	{
		mClassType = typeid(T).hash_code();
	}

private:
	static std::unordered_map<size_t, std::shared_ptr<CObject>>	mCDOMap;

public:
	static void DestroyCDO();
	
protected:
	template <typename T>
	static std::weak_ptr<T> CreateCDO()
	{
		size_t	HashCode = typeid(T).hash_code();

		std::unordered_map<size_t, std::shared_ptr<CObject>>::iterator	iter = mCDOMap.find(HashCode);

		// 같은게 있을 경우 다시 생성하고 추가하면 안된다.
		if (iter != mCDOMap.end())
			return std::weak_ptr<T>();

		std::shared_ptr<T> Obj;

		Obj.reset(new T);

		Obj->SetSelf(Obj);

		/*if (!Obj->Init())
		{
			return std::weak_ptr<T>();
		}*/

		mCDOMap.insert(std::make_pair(HashCode, Obj));

		return std::dynamic_pointer_cast<T>(Obj);
	}

	template <typename T>
	static std::weak_ptr<T> FindCDO()
	{
		size_t	HashCode = typeid(T).hash_code();

		std::unordered_map<size_t, std::shared_ptr<CObject>>::iterator	iter = mCDOMap.find(HashCode);

		if (iter == mCDOMap.end())
			return std::weak_ptr<T>();

		return std::dynamic_pointer_cast<T>(iter->second);
	}

	static std::weak_ptr<CObject> FindCDO(size_t Key)
	{
		std::unordered_map<size_t, std::shared_ptr<CObject>>::iterator	iter = mCDOMap.find(Key);

		if (iter == mCDOMap.end())
			return std::weak_ptr<CObject>();

		return iter->second;
	}
};

