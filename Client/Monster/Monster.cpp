#include "Monster.h"
#include "Component/MeshComponent.h"
#include "../Player/Bullet.h"
#include "World/World.h"
#include "../Component/StateComponent.h"
#include "Component/Animation2DComponent.h"
#include "Component/ColliderBox2D.h"
#include "Component/ColliderSphere2D.h"
#include "Component/ColliderLine2D.h"
#include "Component/WidgetComponent.h"
#include "../UI/WorldHUD.h"

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

	mMeshComponent = CreateComponent<CMeshComponent>("MonsterMesh");

	mStateComponent = CreateComponent<CStateComponent>("State");
	mAnimation2DComponent = CreateComponent<CAnimation2DComponent>("Animation2D");
	mHUDWidget = CreateComponent<CWidgetComponent>("Widget");

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

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("DefaultTexture2D");
		Mesh->SetMesh("CenterRectTex");
		Mesh->SetRelativeScale(100.f, 100.f);
		Mesh->SetBlendState(0, "AlphaBlend");

		//Mesh->SetSimulatePhysics(true);
		//Mesh->SetUseGravity(true);
	}

	//mBody = CreateComponent<CColliderBox2D>("Body");
	mBody = CreateComponent<CColliderSphere2D>("Body");
	auto	Body = mBody.lock();

	if (Body)
	{
		Body->SetCollisionProfile("Monster");
		//Body->SetBoxSize(100.f, 100.f);
		Body->SetRadius(sqrtf(100.f * 100.f + 100.f * 100.f) * 0.5f);
		Body->SetDebugDraw(true);
		Body->SetInheritScale(false);
	}

	//mLine2D = CreateComponent<CColliderLine2D>("Line2D");
	//auto	Line2D = mLine2D.lock();

	//if (Line2D)
	//{
	//	Line2D->SetCollisionProfile("Monster");
	//	//Line2D->SetRadius(sqrtf(20000.f) * 0.5f);
	//	Line2D->SetLineDistance(200.f);
	//	Line2D->SetDebugDraw(true);
	//	Line2D->SetInheritScale(false);
	//	//Line2D->SetRelativePos(0.f, 100.f, 0.f);
	//	//Line2D->SetCenterOffset(0.f, 100.f, 0.f);
	//}

	// Target을 구한다.
	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		mTargetObject = World->FindObject<CGameObject>("Player");
	}


	auto	Widget = mHUDWidget.lock();

	if (Widget)
	{
		Widget->SetInheritScale(false);
		Widget->SetInheritRot(false);
		Widget->SetRelativePos(0.f, 140.f, 0.f);
		Widget->SetRelativeScale(80.f, 40.f);

		auto InWidget = Widget->SetWidget<CWorldHUD>("MonsterHUD").lock();

		InWidget->SetSize(80.f, 40.f);
		InWidget->SetPlayerName(TEXT("Monster"));

		mHPWidgetFunc.push_back(std::bind(&CWorldHUD::SetPlayerHP, InWidget.get(),
			std::placeholders::_1, std::placeholders::_2));
	}

	return true;
}

void CMonster::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

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

float CMonster::TakeDamage(float Damage)
{
	mHP -= Damage;

	if (mHP < 0.f)
		mHP = 0.f;
	char	Test[256] = {};
	sprintf_s(Test, "HP : %d\n", mHP);
	OutputDebugStringA(Test);

	auto	iter = mHPWidgetFunc.begin();
	auto	iterEnd = mHPWidgetFunc.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)((float)mHP, (float)mHPMax);
	}

	return Damage;
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

			BulletObj->SetCollisionName("MonsterAttack");
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
