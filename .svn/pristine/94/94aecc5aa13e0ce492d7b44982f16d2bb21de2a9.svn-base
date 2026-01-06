#include "ObjectMovementComponent.h"
#include "SceneComponent.h"

CObjectMovementComponent::CObjectMovementComponent()
{
	SetClassType<CObjectMovementComponent>();
}

CObjectMovementComponent::CObjectMovementComponent(
	const CObjectMovementComponent& ref) :
	CMovementComponent(ref)
{
}

CObjectMovementComponent::CObjectMovementComponent(
	CObjectMovementComponent&& ref) noexcept :
	CMovementComponent(std::move(ref))
{
}

CObjectMovementComponent::~CObjectMovementComponent()
{
}

bool CObjectMovementComponent::Init()
{
	return true;
}

void CObjectMovementComponent::Update(float DeltaTime)
{
}

void CObjectMovementComponent::PostUpdate(float DeltaTime)
{
	if (mUpdateComponent.expired())
	{
		mVelocity = FVector3::Zero;
		return;
	}

	if (mMoveDir.IsZero())
	{
		mVelocity = FVector3::Zero;
		return;
	}

	mMoveDir.Normalize();

	mVelocity = mMoveDir * mSpeed * DeltaTime;

	auto	UpdateComponent = mUpdateComponent.lock();

	UpdateComponent->AddWorldPos(mVelocity);

	mMoveDir = FVector3::Zero;
}

void CObjectMovementComponent::Destroy()
{
	CMovementComponent::Destroy();
}

CObjectMovementComponent* CObjectMovementComponent::Clone()	const
{
	return new CObjectMovementComponent(*this);
}
