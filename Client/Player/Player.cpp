#include "Player.h"
#include "Component/MeshComponent.h"
#include "Component/CameraComponent.h"
#include "Bullet.h"
#include "World/World.h"
#include "Device.h"
#include "Missile.h"
#include "../Component/StateComponent.h"
#include "Component/Animation2DComponent.h"
#include "World/Input.h"
#include "Timer.h"
#include "Component/ObjectMovementComponent.h"
#include "Component/ColliderBox2D.h"
#include "Component/ColliderSphere2D.h"
#include "Component/ColliderLine2D.h"
#include "Component/WidgetComponent.h"
#include "World/WorldUIManager.h"
#include "../UI/MainWidget.h"
#include "../UI/PlayerStateWidget.h"
#include "../UI/WorldHUD.h"
#include "Render/RenderManager.h"

CPlayer::CPlayer()
{
	SetClassType<CPlayer>();
}

CPlayer::CPlayer(const CPlayer& ref)	:
	CGameObject(ref)
{
}

CPlayer::CPlayer(CPlayer&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CPlayer::~CPlayer()
{
}

bool CPlayer::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("PlayerMesh");
	mRot = CreateComponent<CSceneComponent>("Rot1");
	mSubMeshComponent = CreateComponent<CMeshComponent>("Mesh", "Rot1");
	mCameraComponent = CreateComponent<CCameraComponent>("PlayerCamera");

	mStateComponent = CreateComponent<CStateComponent>("State");
	mHUDWidget = CreateComponent<CWidgetComponent>("Widget");
	mAnimation2DComponent = CreateComponent<CAnimation2DComponent>("Animation2D");
	mMovement = CreateComponent<CObjectMovementComponent>("Movement");

	auto	Movement = mMovement.lock();

	if (Movement)
	{
		Movement->SetUpdateComponent(mMeshComponent);
	}

	// 애니메이션 지정
	auto	Anim = mAnimation2DComponent.lock();

	if (Anim)
	{
		Anim->SetUpdateComponent(mMeshComponent);

		Anim->AddAnimation("PlayerIdle");
		Anim->AddAnimation("PlayerWalk");
		Anim->AddAnimation("PlayerAttack");
		//Anim->ChangeAnimation("PlayerWalk");
		Anim->SetPlayRate("PlayerAttack", 2.f);

		//Anim->AddNotify<CPlayer>("PlayerIdle", "TestNotify",
		//	4, this, &CPlayer::TestNotify);
		Anim->AddNotify<CPlayer>("PlayerAttack",
			"AttackNotify", 2, this, &CPlayer::AttackNotify);
		Anim->SetFinishNotify<CPlayer>("PlayerAttack",
			this, &CPlayer::AttackFinish);

		//Anim->SetFinishNotify<CPlayer>("PlayerIdle", this,
		//	&CPlayer::TestNotify);

		//Anim->SetSymmetry("PlayerIdle", true);
		//Anim->SetSymmetry("PlayerWalk", true);

		Anim->SetLoop("PlayerIdle", true);
		Anim->SetLoop("PlayerWalk", true);
	}

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("DefaultTexture2D");
		Mesh->SetMesh("RectTex");
		Mesh->SetWorldScale(100.f, 100.f);
		Mesh->SetMaterialBaseColor(0, 255, 0, 0, 0);
		Mesh->SetBlendState(0, "AlphaBlend");
		Mesh->SetPivot(0.5f, 0.f);
	}

	mBody = CreateComponent<CColliderBox2D>("Body");
	auto	Body = mBody.lock();

	if (Body)
	{
		Body->SetCollisionProfile("Player");
		Body->SetBoxSize(100.f, 100.f);
		Body->SetDebugDraw(true);
		Body->SetInheritScale(false);
		Body->SetRelativePos(0.f, 50.f);
	}

	/*mSphere2D = CreateComponent<CColliderSphere2D>("Sphere2D");
	auto	Sphere2D = mSphere2D.lock();

	if (Sphere2D)
	{
		Sphere2D->SetCollisionProfile("Player");
		Sphere2D->SetRadius(sqrtf(20000.f) * 0.5f);
		Sphere2D->SetDebugDraw(true);
		Sphere2D->SetInheritScale(false);
		Sphere2D->SetRelativePos(0.f, 50.f);
	}*/

	//mLine2D = CreateComponent<CColliderLine2D>("Line2D");
	//auto	Line2D = mLine2D.lock();

	//if (Line2D)
	//{
	//	Line2D->SetCollisionProfile("Player");
	//	//Line2D->SetRadius(sqrtf(20000.f) * 0.5f);
	//	Line2D->SetLineDistance(200.f);
	//	Line2D->SetDebugDraw(true);
	//	Line2D->SetInheritScale(false);
	//	Line2D->SetRelativePos(0.f, 100.f);
	//}

	auto	RotCom = mRot.lock();

	if (RotCom)
	{
		RotCom->SetInheritRot(false);
		RotCom->SetInheritScale(false);
	}

	Mesh = mSubMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("MaterialColor2D");
		Mesh->SetMesh("CenterRectColor");
		Mesh->SetMaterialBaseColor(0, 255, 0, 0, 0);

		Mesh->SetInheritScale(false);
		Mesh->SetRelativePos(100.f, 0.f, 0.f);
		Mesh->SetRelativeScale(50.f, 50.f);
	}

	auto	Camera = mCameraComponent.lock();

	if (Camera)
	{
		const FResolution& RS = CDevice::GetInst()->GetResolution();
		//Camera->SetRelativePos(0.f, 0.f, -5.f);
		Camera->SetProjection(ECameraProjectionType::Ortho,
			90.f, (float)RS.Width, (float)RS.Height, 1000.f);

		Camera->SetInheritRot(false);
	}

	auto	Widget = mHUDWidget.lock();

	if (Widget)
	{
		Widget->SetInheritScale(false);
		Widget->SetInheritRot(false);
		Widget->SetRelativePos(0.f, 140.f, 0.f);
		Widget->SetRelativeScale(80.f, 40.f);

		auto InWidget = Widget->SetWidget<CWorldHUD>("PlayerHUD").lock();

		InWidget->SetSize(80.f, 40.f);
		InWidget->SetPlayerName(TEXT("Player"));

		mHPWidgetFunc.push_back(std::bind(&CWorldHUD::SetPlayerHP, InWidget.get(),
			std::placeholders::_1, std::placeholders::_2));
	}


	// 플레이어에서 사용할 키를 등록한다.
	auto	World = mWorld.lock();

	auto	Input = World->GetInput().lock();

	Input->AddBindKey("MoveUp", 'W');
	Input->SetBindFunction<CPlayer>("MoveUp",
		EInputType::Hold, this, &CPlayer::MoveUp);
	//Input->SetKeyCtrl("MoveUp", true);

	Input->AddBindKey("MoveDown", 'S');
	Input->SetBindFunction<CPlayer>("MoveDown",
		EInputType::Hold, this, &CPlayer::MoveDown);

	Input->AddBindKey("MoveLeft", 'A');
	Input->SetBindFunction<CPlayer>("MoveLeft",
		EInputType::Hold, this, &CPlayer::MoveLeft);

	Input->AddBindKey("MoveRight", 'D');
	Input->SetBindFunction<CPlayer>("MoveRight",
		EInputType::Hold, this, &CPlayer::MoveRight);

	Input->AddBindKey("Skill1", VK_LBUTTON);
	Input->SetKeyAlt("Skill1", true);
	Input->SetKeyCtrl("Skill1", true);

	Input->SetBindFunction<CPlayer>("Skill1",
		EInputType::Press, this, &CPlayer::Skill1Press);
	Input->SetBindFunction<CPlayer>("Skill1",
		EInputType::Hold, this, &CPlayer::Skill1Hold);
	Input->SetBindFunction<CPlayer>("Skill1",
		EInputType::Release, this, &CPlayer::Skill1Release);

	Input->AddBindKey("Attack", VK_SPACE);
	Input->SetBindFunction<CPlayer>("Attack",
		EInputType::Press, this, &CPlayer::AttackKey);

	Input->AddBindKey("Jump", VK_RETURN);
	Input->SetBindFunction<CPlayer>("Jump",
		EInputType::Press, this, &CPlayer::JumpKey);

	Input->AddBindKey("MoveTarget", VK_RBUTTON);
	Input->SetBindFunction<CPlayer>("MoveTarget",
		EInputType::Press, this, &CPlayer::MoveTarget);

	auto	UIManager = World->GetUIManager().lock();

	auto MainWidget = UIManager->FindWidget<CMainWidget>("MainWidget").lock();

	if (MainWidget)
	{
		auto PlayerState = MainWidget->FindWidget<CPlayerStateWidget>("PlayerState").lock();

		if (PlayerState)
		{
			PlayerState->SetPlayerName(TEXT("Player"));

			mHPWidgetFunc.push_back(std::bind(&CPlayerStateWidget::SetPlayerHP, PlayerState.get(),
				std::placeholders::_1, std::placeholders::_2));

			mMPWidgetFunc = std::bind(&CPlayerStateWidget::SetPlayerMP, PlayerState.get(),
				std::placeholders::_1, std::placeholders::_2);
		}
	}

	return true;
}

void CPlayer::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	auto	Mesh = mMeshComponent.lock();
	auto	Anim = mAnimation2DComponent.lock();


	if (Mesh->GetSpeed() == 0.f && mAutoIdle)
	{
		mAutoIdle = false;
		Anim->ChangeAnimation("PlayerIdle");
	}

	//auto	RotCom = mRot.lock();

	//if (RotCom)
	//{
	//	RotCom->AddRelativeRotationZ(100.f * DeltaTime);
	//}

	//auto	SubMesh = mSubMeshComponent.lock();

	//if (SubMesh)
	//{
	//	SubMesh->SetMaterialBaseColor(0, rand() % 256,
	//		rand() % 256, rand() % 256, 0);
	//	//SubMesh->AddRelativePos(FVector3(0.2f, 0.f, 0.f) * DeltaTime);
	//}

	//auto	Mesh = mMeshComponent.lock();

	//if (Mesh)
	//{
	//	auto	Anim = mAnimation2DComponent.lock();

	//	bool	Move = false;

	//	if (GetAsyncKeyState('W') & 0x8000)
	//	{
	//		Mesh->AddRelativePos(Mesh->GetAxis(EAxis::Y) * 100.f * DeltaTime);
	//		Anim->ChangeAnimation("PlayerWalk");
	//		Move = true;
	//	}

	//	if (GetAsyncKeyState('S') & 0x8000)
	//	{
	//		Mesh->AddRelativePos(Mesh->GetAxis(EAxis::Y) * -100.f * DeltaTime);
	//		Anim->ChangeAnimation("PlayerWalk");
	//		Move = true;
	//	}

	//	if (GetAsyncKeyState('A') & 0x8000)
	//	{
	//		Mesh->AddRelativeRotationZ(180.f * DeltaTime);
	//	}

	//	if (GetAsyncKeyState('D') & 0x8000)
	//	{
	//		Mesh->AddRelativeRotationZ(-180.f * DeltaTime);
	//	}

	//	// CBullet 클래스를 만들고 플레이어의 Y축 위쪽으로 위치를 잡아서
	//	// 생성하고 CBullet Update에서는 Y축 방향으로 계속 이동하게
	//	// 만들어보자.
	//	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	//	{
	//		Anim->ChangeAnimation("PlayerAttack");
	//		mAttack = true;
	//	}

	//	if (!Move && !mAttack)
	//		Anim->ChangeAnimation("PlayerIdle");

	//	if (GetAsyncKeyState('1') & 0x8000)
	//	{
	//		std::shared_ptr<CWorld>	World = mWorld.lock();

	//		if (World)
	//		{
	//			std::weak_ptr<CMissile>	Missile = World->CreateGameObject<CMissile>("Missile");

	//			std::shared_ptr<CMissile>	MissileObj = Missile.lock();

	//			if (MissileObj)
	//			{
	//				MissileObj->SetWorldPos(GetWorldPos() + GetAxis(EAxis::Y) * 100.f);
	//				MissileObj->SetWorldRotation(GetWorldRot());
	//				MissileObj->ChangeCamera();
	//			}
	//		}
	//	}

	//	if (GetAsyncKeyState('3') & 0x8000)
	//	{
	//		// 총알을 생성한다.
	//		for (int i = 0; i < 12; ++i)
	//		{
	//			std::shared_ptr<CWorld>	World = mWorld.lock();

	//			if (World)
	//			{
	//				std::weak_ptr<CBullet>	Bullet = World->CreateGameObject<CBullet>("Bullet");

	//				std::shared_ptr<CBullet>	BulletObj = Bullet.lock();

	//				if (BulletObj)
	//				{
	//					FVector3	BulletPos;
	//					FVector3	BulletDir;
	//					FMatrix		DirMatrix;

	//					// i값에 따라 30도만큼 증가된 각도를 이용하여
	//					// 총알을 생성할 방향이 회전되게 하기 위해
	//					// 회전행렬을 생성한다.
	//					DirMatrix.RotationZ(i * 30.f);

	//					// 플레이어의 Y축을 기준으로 회전된 방향을 구한다.
	//					BulletDir = GetAxis(EAxis::Y).TransformNormal(DirMatrix);
	//					BulletDir.Normalize();

	//					// 구해준 방향으로 90만큼 떨어진 위치를 구한다.
	//					BulletDir *= 90.f;

	//					BulletPos = GetWorldPos() + BulletDir;

	//					BulletObj->SetWorldPos(BulletPos);
	//					//BulletObj->SetWorldRotation(GetWorldRot());
	//					BulletObj->ComputeCollisionRange();

	//					// 타겟을 정한다.
	//					BulletObj->SetNearTarget("Monster");
	//					BulletObj->SetCollisionTargetName("Monster");
	//				}
	//			}
	//		}
	//	}
	//}
}

void CPlayer::Destroy()
{
}

float CPlayer::TakeDamage(float Damage)
{
	mHP -= Damage;

	CRenderManager::GetInst()->EnablePostProcess("Hit");

	if (mHP < 0.f)
		mHP = 0.f;

	auto	iter = mHPWidgetFunc.begin();
	auto	iterEnd = mHPWidgetFunc.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)((float)mHP, (float)mHPMax);
	}

	return Damage;
}

void CPlayer::TestNotify()
{
	OutputDebugString(TEXT("Test Notify\n"));
}

void CPlayer::AttackNotify()
{
	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		std::weak_ptr<CBullet>	Bullet = World->CreateGameObject<CBullet>("Bullet");

		std::shared_ptr<CBullet>	BulletObj = Bullet.lock();

		if (BulletObj)
		{
			BulletObj->SetCollisionName("PlayerAttack");
			BulletObj->SetWorldPos(GetWorldPos() + GetAxis(EAxis::Y) * 75.f);
			BulletObj->SetWorldRotation(GetWorldRot());
			BulletObj->SetCollisionTargetName("Monster");
			BulletObj->ComputeCollisionRange();
		}
	}
}

void CPlayer::AttackFinish()
{
	mAutoIdle = true;
	//mAutoIdle = false;
	
	//auto	Anim = mAnimation2DComponent.lock();

	//Anim->ChangeAnimation("PlayerIdle");
}

void CPlayer::MoveUp()
{
	mAutoIdle = true;

	auto	Movement = mMovement.lock();
	auto	Mesh = mMeshComponent.lock();
	auto	Anim = mAnimation2DComponent.lock();

	Movement->AddMove(Mesh->GetAxis(EAxis::Y));
	Anim->ChangeAnimation("PlayerWalk");
}

void CPlayer::MoveDown()
{
	mAutoIdle = true;

	auto	Movement = mMovement.lock();
	auto	Mesh = mMeshComponent.lock();
	auto	Anim = mAnimation2DComponent.lock();

	Movement->AddMove(-Mesh->GetAxis(EAxis::Y));
	Anim->ChangeAnimation("PlayerWalk");
}

void CPlayer::MoveLeft()
{
	mAutoIdle = true;

	auto	Movement = mMovement.lock();
	auto	Mesh = mMeshComponent.lock();
	auto	Anim = mAnimation2DComponent.lock();

	//Movement->AddMove(-Mesh->GetAxis(EAxis::X));
	Anim->ChangeAnimation("PlayerWalk");
	Mesh->AddRelativeRotationZ(-180.f * CTimer::GetDeltaTime());
}

void CPlayer::MoveRight()
{
	mAutoIdle = true;

	auto	Movement = mMovement.lock();
	auto	Mesh = mMeshComponent.lock();
	auto	Anim = mAnimation2DComponent.lock();

	//Movement->AddMove(Mesh->GetAxis(EAxis::X));
	Anim->ChangeAnimation("PlayerWalk");
	Mesh->AddRelativeRotationZ(180.f * CTimer::GetDeltaTime());
}

void CPlayer::AttackKey()
{
	mAutoIdle = false;
	auto	Anim = mAnimation2DComponent.lock();
	Anim->ChangeAnimation("PlayerAttack");
}

void CPlayer::JumpKey()
{
	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetUseGravity(true);
		//Mesh->AddForce
	}
}

void CPlayer::Skill1Press()
{
	//auto	Anim = mAnimation2DComponent.lock();
	//Anim->ChangeAnimation("PlayerAttack");

	mAutoIdle = false;

	std::shared_ptr<CWorld>	World = mWorld.lock();

	if (World)
	{
		mSkill1Bullet = World->CreateGameObject<CBullet>("Bullet");

		std::shared_ptr<CBullet>	BulletObj = mSkill1Bullet.lock();

		if (BulletObj)
		{
			BulletObj->SetWorldPos(GetWorldPos() + GetAxis(EAxis::Y) * 75.f);
			BulletObj->SetWorldRotation(GetWorldRot());
			BulletObj->SetCollisionTargetName("Monster");
			BulletObj->ComputeCollisionRange();
			BulletObj->SetMoveEnable(false);
		}
	}
}

void CPlayer::Skill1Hold()
{
	mAutoIdle = false;
	auto	Bullet = mSkill1Bullet.lock();

	Bullet->AddRelativeScale(50.f * CTimer::GetDeltaTime(),
		50.f * CTimer::GetDeltaTime());
}

void CPlayer::Skill1Release()
{
	mAutoIdle = true;
	auto	Bullet = mSkill1Bullet.lock();

	Bullet->SetMoveEnable(true);
}

void CPlayer::MoveTarget()
{
	auto	Movement = mMovement.lock();

	auto	World = mWorld.lock();

	auto	Input = World->GetInput().lock();

	Movement->MovePath(Input->GetMouseWorldPos());
}
