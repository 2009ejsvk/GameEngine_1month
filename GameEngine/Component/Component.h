#pragma once

#include "../Object.h"

enum class EComponentType : unsigned char
{
	Scene,
	Object
};

class CComponent :
    public CObject
{
	friend class CGameObject;

protected:
	CComponent();
	CComponent(const CComponent& ref);
	CComponent(CComponent&& ref)	noexcept;

public:
	virtual ~CComponent();

protected:
	std::weak_ptr<class CWorld>	mWorld;
	std::weak_ptr<class CGameObject>	mOwner;
	std::weak_ptr<CComponent>	mSelf;
	std::string		mName;
	bool	mAlive = true;
	bool	mEnable = true;
	EComponentType	mComponentType;

public:
	bool GetEnable()	const
	{
		return mEnable;
	}

	EComponentType GetComponentType()	const
	{
		return mComponentType;
	}

	const std::string& GetName()	const
	{
		return mName;
	}

	bool GetAlive()	const
	{
		return mAlive;
	}

	std::weak_ptr<CWorld> GetWorld()	const
	{
		return mWorld;
	}

	std::weak_ptr<CGameObject> GetOwner()	const
	{
		return mOwner;
	}

public:
	void SetEnable(bool Enable)
	{
		mEnable = Enable;
	}

	void SetSelf(std::weak_ptr<CComponent> Self)
	{
		mSelf = Self;
	}
	void SetWorld(std::weak_ptr<CWorld> World);
	void SetOwner(std::weak_ptr<CGameObject> Owner);

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void Destroy();

protected:
	virtual CComponent* Clone()	const = 0;

public:
	template <typename T>
	std::weak_ptr<T> GetSelf()	const
	{
		return std::dynamic_pointer_cast<T>(mSelf.lock());
	}
};

