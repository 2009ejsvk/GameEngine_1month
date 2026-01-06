#include "WorldCollision.h"
#include "../Component/Collider.h"

CWorldCollision::CWorldCollision()
{
}

CWorldCollision::~CWorldCollision()
{
}

void CWorldCollision::AddCollider(
	const std::weak_ptr<class CCollider>& Collider)
{
	mColliderList.push_back(Collider);
}

bool CWorldCollision::Init()
{
	return true;
}

void CWorldCollision::Update(float DeltaTime)
{
	if (mInterval > 0.f)
	{
		mIntervalTime += DeltaTime;

		if (mIntervalTime < mInterval)
			return;

		mIntervalTime -= mInterval;
	}

	auto	iter = mColliderList.begin();
	auto	iterEnd = mColliderList.end();
	--iterEnd;

	auto	iterEnd1 = mColliderList.end();

	for (; iter != iterEnd;)
	{
		if ((*iter).expired())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			iterEnd1 = iterEnd;
			--iterEnd;
			continue;
		}

		auto	SrcCollider = (*iter).lock();

		if (!SrcCollider->GetAlive())
		{
			iter = mColliderList.erase(iter);
			iterEnd = mColliderList.end();
			iterEnd1 = iterEnd;
			--iterEnd;
			continue;
		}

		else if (!SrcCollider->GetEnable())
		{
			++iter;
			continue;
		}

		FCollisionProfile* SrcProfile = SrcCollider->GetCollisionProfile();

		if (!SrcProfile->Enable)
		{
			++iter;
			continue;
		}

		auto	iter1 = iter;
		++iter1;

		for (; iter1 != iterEnd1;)
		{
			if ((*iter1).expired())
			{
				iter1 = mColliderList.erase(iter1);
				iterEnd = mColliderList.end();
				iterEnd1 = iterEnd;
				--iterEnd;
				continue;
			}

			auto	DestCollider = (*iter1).lock();

			if (!DestCollider->GetAlive())
			{
				iter1 = mColliderList.erase(iter1);
				iterEnd = mColliderList.end();
				iterEnd1 = iterEnd;
				--iterEnd;
				continue;
			}

			else if (!DestCollider->GetEnable())
			{
				++iter1;
				continue;
			}

			// 두 물체의 충돌처리를 진행한다.
			// 두 물체의 충돌 프로파일을 비교한다.
			FCollisionProfile* DestProfile = DestCollider->GetCollisionProfile();

			if (!DestProfile->Enable)
			{
				++iter1;
				continue;
			}

			// 상대방 채널에 대해 충돌이 무시일 경우 충돌처리를 건너뛴다.
			else if (SrcProfile->Interaction[DestProfile->Channel->Channel] == ECollisionInteraction::Ignore ||
				DestProfile->Interaction[SrcProfile->Channel->Channel] == ECollisionInteraction::Ignore)
			{
				++iter1;
				continue;
			}

			// 실제 충돌처리를 진행한다.
			FVector3	HitPoint;

			// Collision함수의 반환값이 true일 경우 충돌 되었다는
			// 의미이다.
			if (SrcCollider->Collision(HitPoint, DestCollider))
			{
				// 두 물체가 이전 프레임에 충돌이 되어 서로 충돌 오브젝트
				// 로 가지고 있는지 판단한다.
				if (!SrcCollider->CheckCollisionObject(
					DestCollider.get()))
				{
					SrcCollider->CallCollisionBegin(HitPoint,
						*iter1);

					DestCollider->CallCollisionBegin(HitPoint,
						*iter);
				}
			}

			// 두 물체가 현재 프레임에는 부딪히지 않았지만 이전프레임 까지는
			// 부딪히고 있었을 경우는 이제 막 떨어지는 경우이다.
			else if (SrcCollider->CheckCollisionObject(
				DestCollider.get()))
			{
				SrcCollider->CallCollisionEnd(DestCollider.get());

				DestCollider->CallCollisionEnd(SrcCollider.get());
			}

			++iter1;
		}

		++iter;
	}
}
