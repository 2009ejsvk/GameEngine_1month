#pragma once

#include "../Object.h"
#include "../Component/SceneComponent.h"
#include "../Component/ObjectComponent.h"

class CGameObject :
    public CObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CGameObject();
	CGameObject(const CGameObject& ref);
	CGameObject(CGameObject&& ref)	noexcept;

public:
	virtual ~CGameObject();

protected:
	std::weak_ptr<CGameObject>	mSelf;
	std::weak_ptr<class CWorld>	mWorld;
	std::string		mName;
	bool	mEnable = true;
	bool	mAlive = true;
	std::vector<std::shared_ptr<CSceneComponent>>	mSceneComponentList;
	std::weak_ptr<CSceneComponent>	mRoot;

	std::vector<std::shared_ptr<CObjectComponent>>	mObjectComponentList;

public:
	bool GetEnable()	const
	{
		return mEnable;
	}

	std::weak_ptr<CSceneComponent> GetRootComponent()	const
	{
		return mRoot;
	}

	bool GetAlive()	const
	{
		return mAlive;
	}

public:
	void SetEnable(bool Enable)
	{
		mEnable = Enable;
	}

	void SetWorld(std::weak_ptr<CWorld> World);
	void SetSelf(std::weak_ptr<CGameObject> Self);
	void SetName(const std::string& Name);

public:
	// Begin 함수는 오브젝트가 월드에 배치되고 처음 한번만 호출되는 함수이다.
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void Destroy();

protected:
	virtual CGameObject* Clone();

public:
	template <typename T>
	std::weak_ptr<T> FindComponent(const std::string& Name)
	{
		auto	iter = mSceneComponentList.begin();
		auto	iterEnd = mSceneComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->GetName() == Name)
			{
				return std::dynamic_pointer_cast<T>(*iter);
			}
		}

		auto	iter1 = mObjectComponentList.begin();
		auto	iter1End = mObjectComponentList.end();

		for (; iter1 != iter1End; ++iter1)
		{
			if ((*iter1)->GetName() == Name)
			{
				return std::dynamic_pointer_cast<T>(*iter1);
			}
		}

		return std::weak_ptr<T>();
	}

	template <typename T>
	std::weak_ptr<T> CreateComponent(
		const std::string& Name,
		const std::string& ParentName = "Root")
	{
		std::shared_ptr<CComponent> Component;

		Component.reset(new T);

		Component->SetWorld(mWorld);
		Component->SetOwner(mSelf);
		Component->SetName(Name);
		Component->SetSelf(Component);

		if (!Component->Init())
		{
			return std::weak_ptr<T>();
		}

		if (Component->mComponentType == EComponentType::Scene)
		{
			if (mSceneComponentList.empty())
				mRoot = std::dynamic_pointer_cast<CSceneComponent>(Component);

			else
			{
				if (ParentName == "Root")
				{
					auto Root = mRoot.lock();

					if (Root)
						Root->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
				}

				else
				{
					// 부모로 붙여줄 컴포넌트를 찾는다.
					size_t	Size = mSceneComponentList.size();

					std::shared_ptr<CSceneComponent>	Parent;

					for (size_t i = 0; i < Size; ++i)
					{
						if (mSceneComponentList[i]->mName == ParentName)
						{
							Parent = mSceneComponentList[i];
							break;
						}
					}

					if (!Parent)
					{
						auto Root = mRoot.lock();

						if (Root)
							Root->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
					}

					else
					{
						Parent->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
					}
				}
			}

			mSceneComponentList.push_back(std::dynamic_pointer_cast<CSceneComponent>(Component));
		}

		else
		{
			mObjectComponentList.push_back(std::dynamic_pointer_cast<CObjectComponent>(Component));
		}

		// dynamic_pointer_cast 를 통해 T 타입으로 변환한
		// shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
		return std::dynamic_pointer_cast<T>(Component);
	}

public:
	const FVector3& GetAxis(EAxis::Type Axis)	const;
	const FVector3& GetRelativeScale()	const;
	const FVector3& GetRelativeRot()	const;
	const FVector3& GetRelativePos()	const;
	const FVector3& GetWorldScale()	const;
	const FVector3& GetWorldRot()	const;
	const FVector3& GetWorldPos()	const;

public:
	void SetInheritScale(bool Scale);
	void SetInheritRot(bool Rot);

	void SetRelativeScale(const FVector3& Scale);
	void SetRelativeScale(const FVector2& Scale);
	void SetRelativeScale(float x, float y, float z);
	void SetRelativeScale(float x, float y);

	void AddRelativeScale(const FVector3& Scale);
	void AddRelativeScale(const FVector2& Scale);
	void AddRelativeScale(float x, float y, float z);
	void AddRelativeScale(float x, float y);

	void SetRelativeRotation(const FVector3& Rot);
	void SetRelativeRotation(const FVector2& Rot);
	void SetRelativeRotation(float x, float y, float z);
	void SetRelativeRotation(float x, float y);
	void SetRelativeRotationX(float x);
	void SetRelativeRotationY(float y);
	void SetRelativeRotationZ(float z);

	void AddRelativeRotation(const FVector3& Rot);
	void AddRelativeRotation(const FVector2& Rot);
	void AddRelativeRotation(float x, float y, float z);
	void AddRelativeRotation(float x, float y);
	void AddRelativeRotationX(float x);
	void AddRelativeRotationY(float y);
	void AddRelativeRotationZ(float z);

	void SetRelativePos(const FVector3& Pos);
	void SetRelativePos(const FVector2& Pos);
	void SetRelativePos(float x, float y, float z);
	void SetRelativePos(float x, float y);

	void AddRelativePos(const FVector3& Pos);
	void AddRelativePos(const FVector2& Pos);
	void AddRelativePos(float x, float y, float z);
	void AddRelativePos(float x, float y);

	void SetWorldScale(const FVector3& Scale);
	void SetWorldScale(const FVector2& Scale);
	void SetWorldScale(float x, float y, float z);
	void SetWorldScale(float x, float y);

	void AddWorldScale(const FVector3& Scale);
	void AddWorldScale(const FVector2& Scale);
	void AddWorldScale(float x, float y, float z);
	void AddWorldScale(float x, float y);

	void SetWorldRotation(const FVector3& Rot);
	void SetWorldRotation(const FVector2& Rot);
	void SetWorldRotation(float x, float y, float z);
	void SetWorldRotation(float x, float y);
	void SetWorldRotationX(float x);
	void SetWorldRotationY(float y);
	void SetWorldRotationZ(float z);

	void AddWorldRotation(const FVector3& Rot);
	void AddWorldRotation(const FVector2& Rot);
	void AddWorldRotation(float x, float y, float z);
	void AddWorldRotation(float x, float y);
	void AddWorldRotationX(float x);
	void AddWorldRotationY(float y);
	void AddWorldRotationZ(float z);

	void SetWorldPos(const FVector3& Pos);
	void SetWorldPos(const FVector2& Pos);
	void SetWorldPos(float x, float y, float z);
	void SetWorldPos(float x, float y);

	void AddWorldPos(const FVector3& Pos);
	void AddWorldPos(const FVector2& Pos);
	void AddWorldPos(float x, float y, float z);
	void AddWorldPos(float x, float y);
};

