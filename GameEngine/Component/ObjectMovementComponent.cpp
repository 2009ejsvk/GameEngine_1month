#include "ObjectMovementComponent.h"
#include "SceneComponent.h"
#include "../World/World.h"
#include "../World/WorldNavigation.h"
#include <cstdarg>
#include <cstdio>

namespace
{
#ifdef _DEBUG
	void DebugMoveLog(const char* Format, ...)
	{
		char Text[512] = {};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Text, Format, Args);
		va_end(Args);

		OutputDebugStringA(Text);
	}
#endif
}

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

void CObjectMovementComponent::Move(const FVector3& Pos)
{
	AdvancePathRequestId();

	mPathList.clear();
	mPathTargetObjectName.clear();
	mRepathAccumulator = 0.f;

	mPathList.emplace_back(Pos);
}

void CObjectMovementComponent::Move(const FVector2& Pos)
{
	Move(FVector3(Pos.x, Pos.y, 0.f));
}

void CObjectMovementComponent::MovePath(const FVector3& Pos)
{
	auto	World = mWorld.lock();

	if (!World)
		return;

	auto	Nav = World->GetNavigation().lock();

	auto	UpdateComponent = mUpdateComponent.lock();

	if (!Nav || !UpdateComponent)
		return;

	const unsigned int RequestId = AdvancePathRequestId();

	mPathTargetObjectName.clear();
	mRepathAccumulator = 0.f;
	mPathList.clear();
	mPathIndex = 0;
	mMoveDist = 0.f;
	Nav->FindPath(UpdateComponent->GetWorldPos(), Pos, &mSelf, "",
		RequestId);
}

void CObjectMovementComponent::MovePath(const FVector2& Pos)
{
	MovePath(FVector3(Pos.x, Pos.y, 0.f));
}

void CObjectMovementComponent::MovePathToObject(
	const std::string& TargetObjectName)
{
	auto World = mWorld.lock();

	if (!World)
		return;

	auto Nav = World->GetNavigation().lock();
	auto UpdateComponent = mUpdateComponent.lock();

	if (!Nav || !UpdateComponent)
		return;

	const unsigned int RequestId = AdvancePathRequestId();

	mPathTargetObjectName = TargetObjectName;
	mRepathAccumulator = 0.f;
	mPathList.clear();
	mPathIndex = 0;
	mMoveDist = 0.f;

#ifdef _DEBUG
	const FVector3 StartPos = UpdateComponent->GetWorldPos();

	DebugMoveLog(
		"[Move] MovePathToObject owner=%s target=%s req=%u start=(%.1f, %.1f)\n",
		mOwner.expired() ? "<no-owner>" : mOwner.lock()->GetName().c_str(),
		TargetObjectName.c_str(),
		RequestId,
		StartPos.x, StartPos.y);
#endif

	Nav->FindPath(UpdateComponent->GetWorldPos(),
		UpdateComponent->GetWorldPos(), &mSelf,
		TargetObjectName, RequestId);
}

void CObjectMovementComponent::StartPath()
{
	if (mPathList.empty())
	{
		mPathIndex = 0;
		mMoveDist = 0.f;
		return;
	}

	auto	UpdateComponent = mUpdateComponent.lock();

	if (!UpdateComponent)
		return;

	FVector3	Target = mPathList[mPathIndex];

	mMoveDist = Target.Distance(UpdateComponent->GetWorldPos());
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

	if (mPathTargetObjectName.empty())
	{
		mRepathAccumulator = 0.f;
	}

	else
	{
		mRepathAccumulator += DeltaTime;

		if (mRepathAccumulator >= mRepathInterval)
		{
			mRepathAccumulator = 0.f;

			auto World = mWorld.lock();

			if (World)
			{
				auto Nav = World->GetNavigation().lock();
				auto UpdateComponent = mUpdateComponent.lock();

				if (Nav && UpdateComponent)
				{
					const unsigned int RequestId =
						AdvancePathRequestId();

#ifdef _DEBUG
					const FVector3 StartPos = UpdateComponent->GetWorldPos();

					DebugMoveLog(
						"[Move] Repath owner=%s target=%s req=%u pos=(%.1f, %.1f)\n",
						mOwner.expired() ? "<no-owner>" : mOwner.lock()->GetName().c_str(),
						mPathTargetObjectName.c_str(),
						RequestId,
						StartPos.x, StartPos.y);
#endif

					Nav->FindPath(UpdateComponent->GetWorldPos(),
						UpdateComponent->GetWorldPos(), &mSelf,
						mPathTargetObjectName, RequestId);
				}
			}
		}
	}

	if (mPathList.empty())
	{
		if (mMoveDir.IsZero())
		{
			mVelocity = FVector3::Zero;
			return;
		}

		mMoveDir.Normalize();

		mVelocity = mMoveDir * mSpeed * DeltaTime;

		auto	UpdateComponent = mUpdateComponent.lock();

		UpdateComponent->AddWorldPos(mVelocity);
	}

	// 길찾기
	else
	{
		auto	UpdateComponent = mUpdateComponent.lock();

		FVector3	Target = mPathList[mPathIndex];

		// 이동 방향을 구한다.
		FVector3	Dir = Target - UpdateComponent->GetWorldPos();
		Dir.z = 0.f;
		Dir.Normalize();

		mVelocity = Dir * mSpeed * DeltaTime;

		UpdateComponent->AddWorldPos(mVelocity);

		mMoveDist -= (mSpeed * DeltaTime);

		// 이동된 위치와 타겟까지의 거리를 구한다.
		float	Dist = Target.Distance(UpdateComponent->GetWorldPos());

		// 도착점에 도착했을 경우
		if (Dist <= mAcceptDistance || mMoveDist < 0.f)
		{
			++mPathIndex;

			UpdateComponent->SetWorldPos(Target);

			// 이동을 종료시킨다.
			if (mPathIndex >= mPathList.size())
			{
				mPathList.clear();
				mPathIndex = 0;
				mMoveDist = 0.f;
			}

			// 다음 포인트로 이동해야 할 경우
			else
			{
				FVector3	Target = mPathList[mPathIndex];

				mMoveDist = Target.Distance(
					UpdateComponent->GetWorldPos());
			}
		}
	}
}

void CObjectMovementComponent::PostRender()
{
	CMovementComponent::PostRender();

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
