#include "GameObject.h"

CGameObject::CGameObject()
{
	SetClassType<CGameObject>();
}

CGameObject::CGameObject(const CGameObject& ref)
{
	// SceneComponent를 복제하여 가지고 있게 한다.
	size_t	Size = ref.mSceneComponentList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		CSceneComponent* Component = ref.mSceneComponentList[i]->Clone();
		
		std::shared_ptr<CSceneComponent>	Com(Component);

		Com->SetSelf(Com);

		mSceneComponentList.push_back(Com);

		// 인자로 들어온 오브젝트의 root를 shared_ptr로 변환한다.
		std::shared_ptr<CSceneComponent> Cmp = ref.mRoot.lock();

		// 인자로 들어온 오브젝트의 컴포넌트와 비교하여 같은 주소일 경우
		// root 라는 의미이므로 현재 객체의 root도 지정한다.
		if (Cmp.get() == ref.mSceneComponentList[i].get())
			mRoot = Com;
	}

	for (size_t i = 0; i < Size; ++i)
	{
		if (!ref.mSceneComponentList[i]->mParent.expired())
		{
			// Parent를 shared_ptr로 만든다.
			auto	Com = ref.mSceneComponentList[i]->mParent.lock();

			// 구해준 부모 컴포넌트가 몇번째 인덱스에 있는지 구한다.
			for (size_t j = 0; j < Size; ++j)
			{
				// 같은 컴포넌트를 찾아서 이 컴포넌트의 Child로 넣어준다.
				if (Com.get() == ref.mSceneComponentList[j].get())
				{
					// 이 객체의 컴포넌트를 이용해서 자식노드로 추가한다.
					mSceneComponentList[j]->AddChild(mSceneComponentList[i]);
					break;
				}
			}
		}
	}
}

CGameObject::CGameObject(CGameObject&& ref)	noexcept
{
	mSceneComponentList = ref.mSceneComponentList;
	ref.mSceneComponentList.clear();

	mRoot = ref.mRoot;
	ref.mRoot.reset();
}

CGameObject::~CGameObject()
{
}

void CGameObject::SetWorld(std::weak_ptr<CWorld> World)
{
	mWorld = World;

	size_t	Size = mSceneComponentList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mSceneComponentList[i]->SetWorld(World);
	}
}

void CGameObject::SetSelf(std::weak_ptr<CGameObject> Self)
{
	mSelf = Self;

	size_t	Size = mSceneComponentList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mSceneComponentList[i]->SetOwner(mSelf);
	}
}

void CGameObject::SetName(const std::string& Name)
{
	mName = Name;
}


void CGameObject::Begin()
{
	auto	iter1 = mSceneComponentList.begin();
	auto	iter1End = mSceneComponentList.end();

	for (; iter1 != iter1End; ++iter1)
	{
		(*iter1)->Begin();
	}

	auto	iter = mObjectComponentList.begin();
	auto	iterEnd = mObjectComponentList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->Begin();
	}
}

bool CGameObject::Init()
{
	return true;
}

void CGameObject::Update(float DeltaTime)
{
	/*auto Root = mRoot.lock();

	if (Root)
		Root->Update(DeltaTime);*/

	auto	iter1 = mSceneComponentList.begin();
	auto	iter1End = mSceneComponentList.end();

	for (; iter1 != iter1End;)
	{
		if ((*iter1).use_count() == 0)
		{
			iter1 = mSceneComponentList.erase(iter1);
			iter1End = mSceneComponentList.end();
			continue;
		}

		else if (!(*iter1)->GetAlive())
		{
			iter1 = mSceneComponentList.erase(iter1);
			iter1End = mSceneComponentList.end();
			continue;
		}

		else if (!(*iter1)->GetEnable())
		{
			++iter1;
			continue;
		}

		(*iter1)->Update(DeltaTime);
		++iter1;
	}

	auto	iter = mObjectComponentList.begin();
	auto	iterEnd = mObjectComponentList.end();

	for (; iter != iterEnd;)
	{
		if ((*iter).use_count() == 0)
		{
			iter = mObjectComponentList.erase(iter);
			iterEnd = mObjectComponentList.end();
			continue;
		}

		else if (!(*iter)->GetAlive())
		{
			iter = mObjectComponentList.erase(iter);
			iterEnd = mObjectComponentList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(DeltaTime);
		++iter;
	}
}

void CGameObject::PostUpdate(float DeltaTime)
{
	/*auto Root = mRoot.lock();

	if (Root)
		Root->PostUpdate(DeltaTime);*/

	auto	iter1 = mSceneComponentList.begin();
	auto	iter1End = mSceneComponentList.end();

	for (; iter1 != iter1End;)
	{
		if ((*iter1).use_count() == 0)
		{
			iter1 = mSceneComponentList.erase(iter1);
			iter1End = mSceneComponentList.end();
			continue;
		}

		else if (!(*iter1)->GetAlive())
		{
			iter1 = mSceneComponentList.erase(iter1);
			iter1End = mSceneComponentList.end();
			continue;
		}

		else if (!(*iter1)->GetEnable())
		{
			++iter1;
			continue;
		}

		(*iter1)->PostUpdate(DeltaTime);
		++iter1;
	}

	auto	iter = mObjectComponentList.begin();
	auto	iterEnd = mObjectComponentList.end();

	for (; iter != iterEnd;)
	{
		if ((*iter).use_count() == 0)
		{
			iter = mObjectComponentList.erase(iter);
			iterEnd = mObjectComponentList.end();
			continue;
		}

		else if (!(*iter)->GetAlive())
		{
			iter = mObjectComponentList.erase(iter);
			iterEnd = mObjectComponentList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->PostUpdate(DeltaTime);
		++iter;
	}
}

void CGameObject::Render()
{
	auto Root = mRoot.lock();

	if (Root)
		Root->Render();
}

CGameObject* CGameObject::Clone()
{
	return new CGameObject(*this);
}

void CGameObject::Destroy()
{
	mAlive = false;
}

const FVector3& CGameObject::GetAxis(EAxis::Type Axis)	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetAxis(Axis);

	return FVector3::Zero;
}

const FVector3& CGameObject::GetRelativeScale()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetRelativeScale();

	return FVector3::Zero;
}

const FVector3& CGameObject::GetRelativeRot()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetRelativeRot();

	return FVector3::Zero;
}

const FVector3& CGameObject::GetRelativePos()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetRelativePos();

	return FVector3::Zero;
}

const FVector3& CGameObject::GetWorldScale()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetWorldScale();

	return FVector3::Zero;
}

const FVector3& CGameObject::GetWorldRot()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetWorldRot();

	return FVector3::Zero;
}

const FVector3& CGameObject::GetWorldPos()	const
{
	auto	Root = mRoot.lock();

	if (Root)
		return Root->GetWorldPos();

	return FVector3::Zero;
}

void CGameObject::SetInheritScale(bool Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetInheritScale(Scale);
}

void CGameObject::SetInheritRot(bool Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetInheritRot(Rot);
}

void CGameObject::SetRelativeScale(const FVector3& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeScale(Scale);
}

void CGameObject::SetRelativeScale(const FVector2& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeScale(Scale);
}

void CGameObject::SetRelativeScale(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeScale(x, y, z);
}

void CGameObject::SetRelativeScale(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeScale(x, y);
}

void CGameObject::AddRelativeScale(const FVector3& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeScale(Scale);
}

void CGameObject::AddRelativeScale(const FVector2& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeScale(Scale);
}

void CGameObject::AddRelativeScale(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeScale(x, y, z);
}

void CGameObject::AddRelativeScale(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeScale(x, y);
}

void CGameObject::SetRelativeRotation(const FVector3& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotation(Rot);
}

void CGameObject::SetRelativeRotation(const FVector2& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotation(Rot);
}

void CGameObject::SetRelativeRotation(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotation(x, y, z);
}

void CGameObject::SetRelativeRotation(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotation(x, y);
}

void CGameObject::SetRelativeRotationX(float x)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotationX(x);
}

void CGameObject::SetRelativeRotationY(float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotationY(y);
}

void CGameObject::SetRelativeRotationZ(float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativeRotationZ(z);
}

void CGameObject::AddRelativeRotation(const FVector3& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotation(Rot);
}

void CGameObject::AddRelativeRotation(const FVector2& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotation(Rot);
}

void CGameObject::AddRelativeRotation(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotation(x, y, z);
}

void CGameObject::AddRelativeRotation(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotation(x, y);
}

void CGameObject::AddRelativeRotationX(float x)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotationX(x);
}

void CGameObject::AddRelativeRotationY(float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotationY(y);
}

void CGameObject::AddRelativeRotationZ(float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativeRotationZ(z);
}

void CGameObject::SetRelativePos(const FVector3& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativePos(Pos);
}

void CGameObject::SetRelativePos(const FVector2& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativePos(Pos);
}

void CGameObject::SetRelativePos(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativePos(x, y, z);
}

void CGameObject::SetRelativePos(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetRelativePos(x, y);
}

void CGameObject::AddRelativePos(const FVector3& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativePos(Pos);
}

void CGameObject::AddRelativePos(const FVector2& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativePos(Pos);
}

void CGameObject::AddRelativePos(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativePos(x, y, z);
}

void CGameObject::AddRelativePos(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddRelativePos(x, y);
}

void CGameObject::SetWorldScale(const FVector3& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldScale(Scale);
}

void CGameObject::SetWorldScale(const FVector2& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldScale(Scale);
}

void CGameObject::SetWorldScale(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldScale(x, y, z);
}

void CGameObject::SetWorldScale(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldScale(x, y);
}

void CGameObject::AddWorldScale(const FVector3& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldScale(Scale);
}

void CGameObject::AddWorldScale(const FVector2& Scale)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldScale(Scale);
}

void CGameObject::AddWorldScale(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldScale(x, y, z);
}

void CGameObject::AddWorldScale(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldScale(x, y);
}

void CGameObject::SetWorldRotation(const FVector3& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotation(Rot);
}

void CGameObject::SetWorldRotation(const FVector2& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotation(Rot);
}

void CGameObject::SetWorldRotation(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotation(x, y, z);
}

void CGameObject::SetWorldRotation(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotation(x, y);
}

void CGameObject::SetWorldRotationX(float x)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotationX(x);
}

void CGameObject::SetWorldRotationY(float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotationY(y);
}

void CGameObject::SetWorldRotationZ(float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldRotationZ(z);
}

void CGameObject::AddWorldRotation(const FVector3& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotation(Rot);
}

void CGameObject::AddWorldRotation(const FVector2& Rot)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotation(Rot);
}

void CGameObject::AddWorldRotation(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotation(x, y, z);
}

void CGameObject::AddWorldRotation(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotation(x, y);
}

void CGameObject::AddWorldRotationX(float x)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotationX(x);
}

void CGameObject::AddWorldRotationY(float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotationY(y);
}

void CGameObject::AddWorldRotationZ(float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldRotationZ(z);
}

void CGameObject::SetWorldPos(const FVector3& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldPos(Pos);
}

void CGameObject::SetWorldPos(const FVector2& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldPos(Pos);
}

void CGameObject::SetWorldPos(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldPos(x, y, z);
}

void CGameObject::SetWorldPos(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->SetWorldPos(x, y);
}

void CGameObject::AddWorldPos(const FVector3& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldPos(Pos);
}

void CGameObject::AddWorldPos(const FVector2& Pos)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldPos(Pos);
}

void CGameObject::AddWorldPos(float x, float y, float z)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldPos(x, y, z);
}

void CGameObject::AddWorldPos(float x, float y)
{
	auto	Root = mRoot.lock();

	if (Root)
		Root->AddWorldPos(x, y);
}
