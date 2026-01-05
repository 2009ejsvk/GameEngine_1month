#include "ProjectileMovementComponent.h"
#include "SceneComponent.h"

CProjectileMovementComponent::CProjectileMovementComponent()
{
	SetClassType<CProjectileMovementComponent>();
}

CProjectileMovementComponent::CProjectileMovementComponent(
	const CProjectileMovementComponent& ref) :
	CMovementComponent(ref)
{
}

CProjectileMovementComponent::CProjectileMovementComponent(
	CProjectileMovementComponent&& ref) noexcept :
	CMovementComponent(std::move(ref))
{
}

CProjectileMovementComponent::~CProjectileMovementComponent()
{
}

bool CProjectileMovementComponent::Init()
{
	return true;
}

void CProjectileMovementComponent::Update(float DeltaTime)
{
}

void CProjectileMovementComponent::PostUpdate(float DeltaTime)
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

	mVelocity = mMoveDir * mSpeed * DeltaTime;

	auto	UpdateComponent = mUpdateComponent.lock();

	UpdateComponent->AddWorldPos(mVelocity);

	if (mRange > 0.f)
	{
		mRange -= mSpeed * DeltaTime;

		if (mRange <= 0.f)
		{
			if (mRangeFunction)
				mRangeFunction();

			mMoveDir = FVector3::Zero;
		}
	}
}

void CProjectileMovementComponent::Destroy()
{
	CMovementComponent::Destroy();
}

CProjectileMovementComponent* CProjectileMovementComponent::Clone()	const
{
	return new CProjectileMovementComponent(*this);
}
