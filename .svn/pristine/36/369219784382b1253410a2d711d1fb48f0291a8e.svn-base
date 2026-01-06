#include "SceneComponent.h"
#include "../Render/RenderManager.h"

CSceneComponent::CSceneComponent()
{
	SetClassType<CSceneComponent>();
	mComponentType = EComponentType::Scene;
}

CSceneComponent::CSceneComponent(const CSceneComponent& ref)	:
	CComponent(ref)
{
	mComponentType = EComponentType::Scene;
	mRenderType = ref.mRenderType;
	mInheritScale = ref.mInheritScale;
	mInheritRot = ref.mInheritRot;
	mRelativeScale = ref.mRelativeScale;
	mRelativeRot = ref.mRelativeRot;
	mRelativePos = ref.mRelativePos;
	mWorldScale = ref.mWorldScale;
	mWorldRot = ref.mWorldRot;
	mWorldPos = ref.mWorldPos;

	memcpy(mWorldAxis, ref.mWorldAxis, sizeof(FVector3) * EAxis::End);

	mScaleMatrix = ref.mScaleMatrix;
	mRotMatrix = ref.mRotMatrix;
	mTranslateMatrix = ref.mTranslateMatrix;
	mWorldMatrix = ref.mWorldMatrix;
}

CSceneComponent::CSceneComponent(CSceneComponent&& ref)	noexcept :
	CComponent(std::move(ref))
{
	mComponentType = EComponentType::Scene;
	mRenderType = ref.mRenderType;
	mInheritScale = ref.mInheritScale;
	mInheritRot = ref.mInheritRot;
	mRelativeScale = ref.mRelativeScale;
	mRelativeRot = ref.mRelativeRot;
	mRelativePos = ref.mRelativePos;
	mWorldScale = ref.mWorldScale;
	mWorldRot = ref.mWorldRot;
	mWorldPos = ref.mWorldPos;

	memcpy(mWorldAxis, ref.mWorldAxis, sizeof(FVector3) * EAxis::End);

	mScaleMatrix = ref.mScaleMatrix;
	mRotMatrix = ref.mRotMatrix;
	mTranslateMatrix = ref.mTranslateMatrix;
	mWorldMatrix = ref.mWorldMatrix;
}

CSceneComponent::~CSceneComponent()
{
}

void CSceneComponent::Begin()
{
	CComponent::Begin();

	InheritScale();

	InheritRotation();

	InheritPos();
}

bool CSceneComponent::Init()
{
	CComponent::Init();

	if (mRenderType == EComponentRender::Render)
	{
		auto	self = std::dynamic_pointer_cast<CSceneComponent>(mSelf.lock());

		CRenderManager::GetInst()->AddRenderLayer(self);
	}

	return true;
}

void CSceneComponent::Update(float DeltaTime)
{
	/*size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
			Child->Update(DeltaTime);
	}*/
}

void CSceneComponent::PostUpdate(float DeltaTime)
{
	UpdateTransform();

	/*size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
			Child->PostUpdate(DeltaTime);
	}*/
}

void CSceneComponent::Render()
{
	/*size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
			Child->Render();
	}*/
}

void CSceneComponent::PostRender()
{
	mVelocity = FVector3::Zero;
}

CSceneComponent* CSceneComponent::Clone()	const
{
	return new CSceneComponent(*this);
}

void CSceneComponent::Destroy()
{
	CComponent::Destroy();
}

void CSceneComponent::AddChild(std::weak_ptr<CSceneComponent> Child)
{
	auto _Child = Child.lock();

	if (_Child)
	{
		_Child->mParent = std::dynamic_pointer_cast<CSceneComponent>(mSelf.lock());
		mChildList.push_back(Child);

		/*_Child->SetRelativeScale(FVector3(1.f, 1.f, 1.f));
		_Child->SetRelativeRotation(FVector3::Zero);
		_Child->SetRelativePos(FVector3::Zero);*/
	}
}

void CSceneComponent::UpdateTransform()
{
	mScaleMatrix.Scaling(mWorldScale);
	mRotMatrix.Rotation(mWorldRot);
	mTranslateMatrix.Translation(mWorldPos);

	mWorldMatrix = mScaleMatrix * mRotMatrix * mTranslateMatrix;
}

void CSceneComponent::SetInheritScale(bool Scale)
{
	mInheritScale = Scale;
}

void CSceneComponent::SetInheritRot(bool Rot)
{
	mInheritRot = Rot;
}

void CSceneComponent::SetRelativeScale(const FVector3& Scale)
{
	mRelativeScale = Scale;

	InheritScale();
}

void CSceneComponent::SetRelativeScale(const FVector2& Scale)
{
	SetRelativeScale(FVector3(Scale.x, Scale.y, mRelativeScale.z));
}

void CSceneComponent::SetRelativeScale(float x, float y, float z)
{
	SetRelativeScale(FVector3(x, y, z));
}

void CSceneComponent::SetRelativeScale(float x, float y)
{
	SetRelativeScale(FVector3(x, y, mRelativeScale.z));
}

void CSceneComponent::AddRelativeScale(const FVector3& Scale)
{
	SetRelativeScale(Scale + mRelativeScale);
}

void CSceneComponent::AddRelativeScale(const FVector2& Scale)
{
	AddRelativeScale(FVector3(Scale.x, Scale.y, 0.f));
}

void CSceneComponent::AddRelativeScale(float x, float y, float z)
{
	AddRelativeScale(FVector3(x, y, z));
}

void CSceneComponent::AddRelativeScale(float x, float y)
{
	AddRelativeScale(FVector3(x, y, 0.f));
}

void CSceneComponent::SetRelativeRotation(const FVector3& Rot)
{
	mRelativeRot = Rot;

	InheritRotation();
}

void CSceneComponent::SetRelativeRotation(const FVector2& Rot)
{
	SetRelativeRotation(FVector3(Rot.x, Rot.y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotation(float x, float y, float z)
{
	SetRelativeRotation(FVector3(x, y, z));
}

void CSceneComponent::SetRelativeRotation(float x, float y)
{
	SetRelativeRotation(FVector3(x, y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotationX(float x)
{
	SetRelativeRotation(FVector3(x, mRelativeRot.y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotationY(float y)
{
	SetRelativeRotation(FVector3(mRelativeRot.x, y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotationZ(float z)
{
	SetRelativeRotation(FVector3(mRelativeRot.x, mRelativeRot.y, z));
}

void CSceneComponent::AddRelativeRotation(const FVector3& Rot)
{
	SetRelativeRotation(Rot + mRelativeRot);
}

void CSceneComponent::AddRelativeRotation(const FVector2& Rot)
{
	AddRelativeRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::AddRelativeRotation(float x, float y, float z)
{
	AddRelativeRotation(FVector3(x, y, z));
}

void CSceneComponent::AddRelativeRotation(float x, float y)
{
	AddRelativeRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::AddRelativeRotationX(float x)
{
	AddRelativeRotation(FVector3(x, 0.f, 0.f));
}

void CSceneComponent::AddRelativeRotationY(float y)
{
	AddRelativeRotation(FVector3(0.f, y, 0.f));
}

void CSceneComponent::AddRelativeRotationZ(float z)
{
	AddRelativeRotation(FVector3(0.f, 0.f, z));
}

void CSceneComponent::SetRelativePos(const FVector3& Pos)
{
	mVelocity += (Pos - mRelativePos);

	mRelativePos = Pos;

	InheritPos();
}

void CSceneComponent::SetRelativePos(const FVector2& Pos)
{
	SetRelativePos(FVector3(Pos.x, Pos.y, mRelativePos.z));
}

void CSceneComponent::SetRelativePos(float x, float y, float z)
{
	SetRelativePos(FVector3(x, y, z));
}

void CSceneComponent::SetRelativePos(float x, float y)
{
	SetRelativePos(FVector3(x, y, mRelativePos.z));
}

void CSceneComponent::AddRelativePos(const FVector3& Pos)
{
	SetRelativePos(Pos + mRelativePos);
}

void CSceneComponent::AddRelativePos(const FVector2& Pos)
{
	AddRelativePos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::AddRelativePos(float x, float y, float z)
{
	AddRelativePos(FVector3(x, y, z));
}

void CSceneComponent::AddRelativePos(float x, float y)
{
	AddRelativePos(FVector3(x, y, 0.f));
}

void CSceneComponent::InheritScale()
{
	if (mInheritScale)
	{
		auto	Parent = mParent.lock();

		if (Parent)
		{
			mWorldScale = mRelativeScale * Parent->mWorldScale;
		}

		else
		{
			mWorldScale = mRelativeScale;
		}
	}

	else
	{
		mWorldScale = mRelativeScale;
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritScale();
	}
}

void CSceneComponent::InheritRotation()
{
	if (mInheritRot)
	{
		auto	Parent = mParent.lock();

		if (Parent)
		{
			mWorldRot = mRelativeRot + Parent->mWorldRot;
		}

		else
		{
			mWorldRot = mRelativeRot;
		}
	}

	else
	{
		mWorldRot = mRelativeRot;
	}

	// 3개의 축을 구해진 회전값으로 회전시켜준다.
	FMatrix	RotationMatrix;

	RotationMatrix.Rotation(mWorldRot);

	for (int i = 0; i < EAxis::End; ++i)
	{
		mWorldAxis[i] = FVector3::Axis[i].TransformNormal(RotationMatrix);

		mWorldAxis[i].Normalize();
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritRotation();
	}

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritPos();
	}
}

void CSceneComponent::InheritPos()
{
	auto	Parent = mParent.lock();

	if (Parent)
	{
		FMatrix	ParentMatrix;

		if (mInheritRot)
		{
			FVector3	ParentRot = Parent->GetWorldRot();
			ParentMatrix.Rotation(ParentRot);
		}

		FVector3	ParentPos = Parent->GetWorldPos();
		memcpy(&ParentMatrix._41, &ParentPos, sizeof(FVector3));

		// 3, 0, 0 떨어져 있을 경우
		// 부모 위치가 10, 10, 10 일경우
		/*
		3, 0, 0, 1 * 1  0  0  0 = 13, 10, 10
		             0  1  0  0
					 0  0  1  0
					 10 10 10 1
		*/
		mWorldPos = mRelativePos.TransformCoord(ParentMatrix);
	}

	else
	{
		mWorldPos = mRelativePos;
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritPos();
	}
}

void CSceneComponent::SetWorldScale(const FVector3& Scale)
{
	mWorldScale = Scale;

	InheritWorldScale();
}

void CSceneComponent::SetWorldScale(const FVector2& Scale)
{
	SetWorldScale(FVector3(Scale.x, Scale.y, mWorldScale.z));
}

void CSceneComponent::SetWorldScale(float x, float y, float z)
{
	SetWorldScale(FVector3(x, y, z));
}

void CSceneComponent::SetWorldScale(float x, float y)
{
	SetWorldScale(FVector3(x, y, mWorldScale.z));
}

void CSceneComponent::AddWorldScale(const FVector3& Scale)
{
	SetWorldScale(Scale + mWorldScale);
}

void CSceneComponent::AddWorldScale(const FVector2& Scale)
{
	AddWorldScale(FVector3(Scale.x, Scale.y, 0.f));
}

void CSceneComponent::AddWorldScale(float x, float y, float z)
{
	AddWorldScale(FVector3(x, y, z));
}

void CSceneComponent::AddWorldScale(float x, float y)
{
	AddWorldScale(FVector3(x, y, 0.f));
}

void CSceneComponent::SetWorldRotation(const FVector3& Rot)
{
	mWorldRot = Rot;

	InheritWorldRotation();
}

void CSceneComponent::SetWorldRotation(const FVector2& Rot)
{
	SetWorldRotation(FVector3(Rot.x, Rot.y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotation(float x, float y, float z)
{
	SetWorldRotation(FVector3(x, y, z));
}

void CSceneComponent::SetWorldRotation(float x, float y)
{
	SetWorldRotation(FVector3(x, y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotationX(float x)
{
	SetWorldRotation(FVector3(x, mWorldRot.y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotationY(float y)
{
	SetWorldRotation(FVector3(mWorldRot.x, y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotationZ(float z)
{
	SetWorldRotation(FVector3(mWorldRot.x, mWorldRot.y, z));
}

void CSceneComponent::AddWorldRotation(const FVector3& Rot)
{
	SetWorldRotation(Rot + mWorldRot);
}

void CSceneComponent::AddWorldRotation(const FVector2& Rot)
{
	AddWorldRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::AddWorldRotation(float x, float y, float z)
{
	AddWorldRotation(FVector3(x, y, z));
}

void CSceneComponent::AddWorldRotation(float x, float y)
{
	AddWorldRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::AddWorldRotationX(float x)
{
	AddWorldRotation(FVector3(x, 0.f, 0.f));
}

void CSceneComponent::AddWorldRotationY(float y)
{
	AddWorldRotation(FVector3(0.f, y, 0.f));
}

void CSceneComponent::AddWorldRotationZ(float z)
{
	AddWorldRotation(FVector3(0.f, 0.f, z));
}

void CSceneComponent::SetWorldPos(const FVector3& Pos)
{
	mVelocity += (Pos - mWorldPos);

	FVector3	Move = Pos - mWorldPos;

	//mWorldPos = Pos;

	UpdateChildWorldPos(Move);

	InheritWorldPos();
}

void CSceneComponent::SetWorldPos(const FVector2& Pos)
{
	SetWorldPos(FVector3(Pos.x, Pos.y, mWorldPos.z));
}

void CSceneComponent::SetWorldPos(float x, float y, float z)
{
	SetWorldPos(FVector3(x, y, z));
}

void CSceneComponent::SetWorldPos(float x, float y)
{
	SetWorldPos(FVector3(x, y, mWorldPos.z));
}

void CSceneComponent::AddWorldPos(const FVector3& Pos)
{
	SetWorldPos(Pos + mWorldPos);
}

void CSceneComponent::AddWorldPos(const FVector2& Pos)
{
	AddWorldPos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::AddWorldPos(float x, float y, float z)
{
	AddWorldPos(FVector3(x, y, z));
}

void CSceneComponent::AddWorldPos(float x, float y)
{
	AddWorldPos(FVector3(x, y, 0.f));
}

void CSceneComponent::UpdateChildWorldPos(const FVector3& Dist)
{
	mWorldPos += Dist;

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->UpdateChildWorldPos(Dist);
	}
}

void CSceneComponent::InheritWorldScale()
{
	if (mInheritScale)
	{
		auto	Parent = mParent.lock();

		if (Parent)
		{
			mRelativeScale = mWorldScale / Parent->mWorldScale;
		}

		else
		{
			mRelativeScale = mWorldScale;
		}
	}

	else
	{
		mRelativeScale = mWorldScale;
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritWorldScale();
	}
}

void CSceneComponent::InheritWorldRotation()
{
	if (mInheritRot)
	{
		auto	Parent = mParent.lock();

		if (Parent)
		{
			mRelativeRot = mWorldRot - Parent->mWorldRot;
		}

		else
		{
			mRelativeRot = mWorldRot;
		}
	}

	else
	{
		mRelativeRot = mWorldRot;
	}

	// 3개의 축을 구해진 회전값으로 회전시켜준다.
	FMatrix	RotationMatrix;

	RotationMatrix.Rotation(mWorldRot);

	for (int i = 0; i < EAxis::End; ++i)
	{
		mWorldAxis[i] = FVector3::Axis[i].TransformNormal(RotationMatrix);

		mWorldAxis[i].Normalize();
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritRotation();
	}

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritWorldPos();
	}
}

void CSceneComponent::InheritWorldPos()
{
	auto	Parent = mParent.lock();

	if (Parent)
	{
		FMatrix	ParentMatrix;

		if (mInheritRot)
		{
			FVector3	ParentRot = Parent->GetWorldRot();
			ParentMatrix.Rotation(ParentRot);
		}

		FVector3	ParentPos = Parent->GetWorldPos();
		memcpy(&ParentMatrix._41, &ParentPos, sizeof(FVector3));

		ParentMatrix.Inverse();

		mRelativePos = mWorldPos.TransformCoord(ParentMatrix);
	}

	else
	{
		mRelativePos = mWorldPos;
	}

	size_t	Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto	Child = mChildList[i].lock();

		if (Child)
			Child->InheritWorldPos();
	}
}
