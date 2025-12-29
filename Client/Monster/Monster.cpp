#include "Monster.h"
#include "Component/MeshComponent.h"
#include "../Player/Bullet.h"
#include "World/World.h"
#include "../Component/StateComponent.h"
#include "Component/Animation2DComponent.h"

#include "Component/Animation2DComponent.h"

CMonster::CMonster()
{
	SetClassType<CMonster>();
}

CMonster::CMonster(const CMonster& ref) :
	CGameObject(ref)
{
}

CMonster::CMonster(CMonster&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CMonster::~CMonster()
{
}

bool CMonster::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("Mesh");

	mStateComponent = CreateComponent<CStateComponent>("State");
	mAnimation2DComponent = CreateComponent<CAnimation2DComponent>("Animation2D");

<<<<<<< .mine
	mAnimation2DComponent_ = CreateComponent<CAnimation2DComponent>("Animation2D");

	// 애니메이션 지정
	auto	Anim_ = mAnimation2DComponent_.lock();

	if (Anim_)
	{
		Anim_->AddAnimation("PlayerIdle");
		Anim_->SetUpdateComponent(mMeshComponent);
		Anim_->SetLoop("PlayerIdle", true);
		
	}

||||||| .r44
=======
	// 애니메이션 지정
	auto	Anim = mAnimation2DComponent.lock();

	if (Anim)
	{
		Anim->SetUpdateComponent(mMeshComponent);

		Anim->AddAnimation("MonsterIdle");
		Anim->AddAnimation("MonsterAttack");

		Anim->AddNotify<CMonster>("MonsterAttack",
			"AttackNotify", 8, this, &CMonster::AttackNotify);
		Anim->SetFinishNotify<CMonster>("MonsterAttack",
			this, &CMonster::AttackFinish);

		Anim->SetLoop("MonsterIdle", true);
		Anim->SetLoop("MonsterAttack", true);
	}

>>>>>>> .r45
	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
<<<<<<< .mine
		//Mesh->SetShader("MaterialColor2D");
		//Mesh->SetMesh("CenterRectColor");

		Mesh->SetShader("DefaultTexture2D");
		Mesh->SetMesh("CenterRectTex");
||||||| .r44
		Mesh->SetShader("MaterialColor2D");
		Mesh->SetMesh("CenterRectColor");
=======
		Mesh->SetShader("DefaultTexture2D");
		Mesh->SetMesh("CenterRectTex");
>>>>>>> .r45
		Mesh->SetRelativeScale(100.f, 100.f);
<<<<<<< .mine

		Mesh->AddTexture(0, "PlayerSheet", TEXT("Player/Player.png"));

||||||| .r44
=======
		Mesh->SetBlendState(0, "AlphaBlend");
>>>>>>> .r45
	}

	// Target을 구한다.
	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		mTargetObject = World->FindObject<CGameObject>("Player");
	}

	return true;
}

void CMonster::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);


	auto	Mesh = mMeshComponent.lock();

	auto	Anim_ = mAnimation2DComponent_.lock();
	Anim_->ChangeAnimation("PlayerAttack");

	auto Target = mTargetObject.lock();

	// 감지 반경 안에 들어오는지 계산한다.
	FVector3	TargetPos = Target->GetWorldPos();
	FVector3	TargetDir = TargetPos - GetWorldPos();

	// 타겟과의 거리를 구해준다.
	float TargetDistance = TargetDir.Length();

	// 탐지반경 안에 들어왔을 경우
	if (TargetDistance <= mDetectRange)
	{
		// 플레이어 방향을 바라보게 회전시킨다.
		float Angle = GetWorldPos().GetViewTargetAngle2D(Target->GetWorldPos(), EAxis::Y);

		SetWorldRotationZ(Angle);

		/*char	Test[256] = {};
		sprintf_s(Test, "Target Angle : %.4f\n", Angle);
		OutputDebugStringA(Test);*/

		auto	Anim = mAnimation2DComponent.lock();

		if (Anim)
		{
			Anim->ChangeAnimation("MonsterAttack");
		}

		//mFireTime -= DeltaTime;

		//if (mFireTime <= 0.f)
		//{
		//	mFireTime += 1.f;

		//	std::shared_ptr<CWorld>	World = mWorld.lock();

		//	if (World)
		//	{
		//		std::weak_ptr<CBullet>	Bullet = World->CreateGameObject<CBullet>("Bullet");

		//		std::shared_ptr<CBullet>	BulletObj = Bullet.lock();

		//		if (BulletObj)
		//		{
		//			FVector3	BulletPos = GetWorldPos() + GetAxis(EAxis::Y) * 75.f;

<<<<<<< .mine
						BulletObj->SetMoveDir(Dir);

						
					}
				}
			}
		}
||||||| .r44
						BulletObj->SetMoveDir(Dir);
					}
				}
			}
		}
=======
		//			BulletObj->SetWorldPos(BulletPos);
		//			BulletObj->SetWorldRotation(GetWorldRot());
		//			BulletObj->SetCollisionTargetName("Player");
		//			BulletObj->ComputeCollisionRange();

		//			// 플레이어를 향하는 방향을 구해준다.
		//			if (Target)
		//			{
		//				// Bullet -> TargetPos 방향 구하기
		//				FVector3	Dir = TargetPos - BulletPos;
		//				Dir.Normalize();

		//				BulletObj->SetMoveDir(Dir);
		//			}
		//		}
		//	}
		//}
>>>>>>> .r45
		else {
			//Anim_->ChangeAnimation("PlayerIdle");
		}
	}

	else
	{
		//mFireTime = 0.f;
		/*auto	Anim = mAnimation2DComponent.lock();

		if (Anim)
		{
			Anim->ChangeAnimation("MonsterIdle");
		}*/
	}
}

CMonster* CMonster::Clone()
{
	return new CMonster(*this);
}

void CMonster::AttackNotify()
{
	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		std::weak_ptr<CBullet>	Bullet = World->CreateGameObject<CBullet>("Bullet");

		std::shared_ptr<CBullet>	BulletObj = Bullet.lock();

		if (BulletObj)
		{
			FVector3	BulletPos = GetWorldPos() + GetAxis(EAxis::Y) * 75.f;

			BulletObj->SetWorldPos(BulletPos);
			BulletObj->SetWorldRotation(GetWorldRot());
			BulletObj->SetCollisionTargetName("Player");
			BulletObj->ComputeCollisionRange();

			auto Target = mTargetObject.lock();

			// 감지 반경 안에 들어오는지 계산한다.
			FVector3	TargetPos = Target->GetWorldPos();

			// 플레이어를 향하는 방향을 구해준다.
			if (Target)
			{
				// Bullet -> TargetPos 방향 구하기
				FVector3	Dir = TargetPos - BulletPos;
				Dir.Normalize();

				BulletObj->SetMoveDir(Dir);
			}
		}
	}
}

void CMonster::AttackFinish()
{
	auto	Anim = mAnimation2DComponent.lock();

	if (Anim)
	{
		Anim->ChangeAnimation("MonsterIdle");
	}
}
