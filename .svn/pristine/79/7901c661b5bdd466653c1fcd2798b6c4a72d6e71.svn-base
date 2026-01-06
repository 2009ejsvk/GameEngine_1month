#include "Bullet.h"
#include "Component/MeshComponent.h"
#include "World/World.h"
#include "../Component/StateComponent.h"
#include "Player.h"
#include "../Monster/Monster.h"
#include "../Interface/StateInterface.h"
#include "BulletEffect.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ColliderBox2D.h"

CBullet::CBullet()
{
	SetClassType<CBullet>();
}

CBullet::CBullet(const CBullet& ref)	:
	CGameObject(ref)
{
}

CBullet::CBullet(CBullet&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CBullet::~CBullet()
{
}

void CBullet::SetCollisionName(const std::string& Name)
{
	auto	Body = mBody.lock();

	if (Body)
	{
		Body->SetCollisionProfile(Name);
	}
}

void CBullet::SetMoveEnable(bool Enable)
{
	mMoveEnable = Enable;

	auto	Movement = mMovement.lock();

	Movement->SetEnable(Enable);
}

void CBullet::SetDistance(float Distance)
{
	mDistance = Distance;

	auto	Movement = mMovement.lock();

	Movement->SetRange(Distance);
}

void CBullet::SetSpeed(float Speed)
{
	mSpeed = Speed;

	auto	Movement = mMovement.lock();

	Movement->SetSpeed(Speed);
}

void CBullet::SetMoveDir(const FVector3& MoveDir)
{
	mMoveDirEnable = true;
	mMoveDir = MoveDir;

	auto	Movement = mMovement.lock();

	Movement->SetMoveDir(mMoveDir);
}

void CBullet::SetNearTarget(const std::string& Name)
{
	std::list<std::weak_ptr<CGameObject>>	TargetList;

	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		// 타겟 후보가 있을 경우에만 동작한다.
		if (World->FindObjectList<CGameObject>(Name, TargetList))
		{
			mMoveDirEnable = true;

			// FLT_MAX는 float 타입으로 표현할 수 있는 최대치이다.
			float	Dist = FLT_MAX;

			auto	iter = TargetList.begin();
			auto	iterEnd = TargetList.end();

			for (; iter != iterEnd; ++iter)
			{
				auto	Obj = (*iter).lock();

				FVector3	TargetDir = Obj->GetWorldPos() - GetWorldPos();
				float	TargetDist = TargetDir.Length();

				if (TargetDist < Dist)
				{
					Dist = TargetDist;
					mMoveDir = TargetDir;
					mMoveDir.Normalize();
				}
			}
		}
	}
}

bool CBullet::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("Mesh");
	mMovement = CreateComponent<CProjectileMovementComponent>("Movement");

	auto	Movement = mMovement.lock();

	if (Movement)
	{
		Movement->SetMoveDir(GetAxis(EAxis::Y));
		Movement->SetRangeFunction<CBullet>(this,
			&CBullet::MoveEndFunction);
		Movement->SetRange(mDistance);
		Movement->SetSpeed(mSpeed);
		Movement->SetUpdateComponent(mMeshComponent);
	}

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("MaterialColor2D");
		Mesh->SetMesh("CenterRectColor");
		Mesh->SetRelativeScale(50.f, 50.f);
	}

	mBody = CreateComponent<CColliderBox2D>("Body");
	auto	Body = mBody.lock();

	if (Body)
	{
		Body->SetCollisionBeginFunction<CBullet>(this,
			&CBullet::CollisionBegin);

		Body->SetCollisionEndFunction<CBullet>(this,
			&CBullet::CollisionEnd);

		Body->SetCollisionProfile("PlayerAttack");
		Body->SetBoxSize(50.f, 50.f);
		Body->SetDebugDraw(true);
		Body->SetInheritScale(false);
		//Body->SetInheritRot(false);
	}

	return true;
}

void CBullet::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	/*auto	Movement = mMovement.lock();

	if (mMoveDirEnable)
		Movement->AddMove(mMoveDir);

	else
		Movement->AddMove(GetAxis(EAxis::Y));

	mDistance -= Movement->GetVelocityFrame();*/

	//if (mMoveEnable)
	//{
	//	FVector3	Move;

	//	if (mMoveDirEnable)
	//		Move = mMoveDir * 300.f * DeltaTime;

	//	else
	//		Move = GetAxis(EAxis::Y) * 300.f * DeltaTime;

	//	mDistance -= Move.Length();

	//	AddWorldPos(Move);

	//	// 충돌 대상을 찾아온다.
	//	std::list<std::weak_ptr<CGameObject>>	CollisionList;

	//	std::shared_ptr<CWorld>	World = mWorld.lock();

	//	if (World)
	//	{
	//		World->FindObjectList<CGameObject>(mCollisionTargetName,
	//			CollisionList);
	//	}

	//	auto	iter = CollisionList.begin();
	//	auto	iterEnd = CollisionList.end();

	//	for (; iter != iterEnd; ++iter)
	//	{
	//		auto	Obj = (*iter).lock();

	//		FVector3	Scale = Obj->GetWorldScale();

	//		Scale /= 2.f;

	//		float Range = sqrtf(Scale.x * Scale.x + Scale.y * Scale.y);

	//		// 물체 사이의 거리를 구한다.
	//		FVector3	SelfPos = GetWorldPos();
	//		FVector3	DestPos = Obj->GetWorldPos();
	//		FVector3	Dir = DestPos - SelfPos;
	//		float Dist = Dir.Length();

	//		if (Dist <= mCollisionRange + Range)
	//		{
	//			std::shared_ptr<CWorld>	World = mWorld.lock();

	//			if (World)
	//			{
	//				std::weak_ptr<CBulletEffect> Effect = World->CreateGameObject<CBulletEffect>("BulletEffect");

	//				auto	_Effect = Effect.lock();

	//				_Effect->SetWorldPos(GetWorldPos());
	//			}

	//			// 1. Component를 이용하는 방법
	//			std::weak_ptr<CStateComponent>	StatePtr = Obj->FindComponent<CStateComponent>("State");

	//			if (!StatePtr.expired())
	//			{
	//				auto	State = StatePtr.lock();

	//				State->AddHP(-1);
	//			}

	//			// 2. GameObject를 이용하는 방법
	//			// 충돌 대상이 플레이어일 경우 플레이어의 체력을 감소시킨다.
	//			// 플레이어가 아니라 몬스터일 경우 몬스터의 체력을 감소시킨다.
	//			/*if (std::dynamic_pointer_cast<CPlayer>(Obj))
	//			{
	//				std::dynamic_pointer_cast<CPlayer>(Obj)->Damage(1);
	//			}

	//			else
	//			{
	//				std::dynamic_pointer_cast<CMonster>(Obj)->Damage(1);
	//			}*/

	//			// 3. 다중상속을 활용하는 방법
	//			/*std::shared_ptr<CStateInterface>	StateInterface =
	//				std::dynamic_pointer_cast<CStateInterface>(Obj);

	//			StateInterface->AddHP(-1);*/


	//			Destroy();
	//		}
	//	}
	//}

	/*if (mDistance <= 0.f)
	{
		Destroy();
	}*/
}

void CBullet::PostUpdate(float DeltaTime)
{
	CGameObject::PostUpdate(DeltaTime);

	// 충돌 대상을 찾아온다.
	//std::list<std::weak_ptr<CGameObject>>	CollisionList;

	//std::shared_ptr<CWorld>	World = mWorld.lock();

	//if (World)
	//{
	//	World->FindObjectList<CGameObject>(mCollisionTargetName,
	//		CollisionList);
	//}

	//auto	iter = CollisionList.begin();
	//auto	iterEnd = CollisionList.end();

	//for (; iter != iterEnd; ++iter)
	//{
	//	auto	Obj = (*iter).lock();

	//	FVector3	Scale = Obj->GetWorldScale();

	//	Scale /= 2.f;

	//	float Range = sqrtf(Scale.x * Scale.x + Scale.y * Scale.y);

	//	// 물체 사이의 거리를 구한다.
	//	FVector3	SelfPos = GetWorldPos();
	//	FVector3	DestPos = Obj->GetWorldPos();
	//	FVector3	Dir = DestPos - SelfPos;
	//	float Dist = Dir.Length();

	//	if (Dist <= mCollisionRange + Range)
	//	{
	//		std::shared_ptr<CWorld>	World = mWorld.lock();

	//		if (World)
	//		{
	//			std::weak_ptr<CBulletEffect> Effect = World->CreateGameObject<CBulletEffect>("BulletEffect");

	//			auto	_Effect = Effect.lock();

	//			_Effect->SetWorldPos(GetWorldPos());
	//		}

	//		// 1. Component를 이용하는 방법
	//		std::weak_ptr<CStateComponent>	StatePtr = Obj->FindComponent<CStateComponent>("State");

	//		if (!StatePtr.expired())
	//		{
	//			auto	State = StatePtr.lock();

	//			State->AddHP(-1);
	//		}

	//		// 2. GameObject를 이용하는 방법
	//		// 충돌 대상이 플레이어일 경우 플레이어의 체력을 감소시킨다.
	//		// 플레이어가 아니라 몬스터일 경우 몬스터의 체력을 감소시킨다.
	//		/*if (std::dynamic_pointer_cast<CPlayer>(Obj))
	//		{
	//			std::dynamic_pointer_cast<CPlayer>(Obj)->Damage(1);
	//		}

	//		else
	//		{
	//			std::dynamic_pointer_cast<CMonster>(Obj)->Damage(1);
	//		}*/

	//		// 3. 다중상속을 활용하는 방법
	//		/*std::shared_ptr<CStateInterface>	StateInterface =
	//			std::dynamic_pointer_cast<CStateInterface>(Obj);

	//		StateInterface->AddHP(-1);*/


	//		Destroy();
	//	}
	//}
}

CBullet* CBullet::Clone()
{
	return new CBullet(*this);
}

void CBullet::MoveEndFunction()
{
	Destroy();
}

void CBullet::CollisionBegin(const FVector3& HitPoint, CCollider* Dest)
{
	Destroy();

	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		std::weak_ptr<CBulletEffect> Effect = World->CreateGameObject<CBulletEffect>("BulletEffect");

		auto	_Effect = Effect.lock();

		_Effect->SetWorldPos(HitPoint);
	}
}

void CBullet::CollisionEnd(CCollider* Dest)
{
}
