#include "Missile.h"
#include "Component/MeshComponent.h"
#include "Component/CameraComponent.h"
#include "Device.h"
#include "World/World.h"
#include "World/CameraManager.h"

CMissile::CMissile()
{
	SetClassType<CMissile>();
}

CMissile::CMissile(const CMissile& ref) :
	CGameObject(ref)
{
}

CMissile::CMissile(CMissile&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CMissile::~CMissile()
{
}

void CMissile::ChangeCamera()
{
	auto	World = mWorld.lock();

	if (World)
	{
		auto	CameraManager = World->GetCameraManager().lock();

		if (CameraManager)
			CameraManager->ChangeMainCamera(mCameraComponent);
	}
}

bool CMissile::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("Mesh");
	mCameraComponent = CreateComponent<CCameraComponent>("MissileCamera");

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("MaterialColor2D");
		Mesh->SetMesh("CenterRectColor");
		Mesh->SetRelativeScale(50.f, 100.f);
	}

	auto	Camera = mCameraComponent.lock();

	if (Camera)
	{
		const FResolution& RS = CDevice::GetInst()->GetResolution();
		//Camera->SetRelativePos(0.f, 0.f, -5.f);
		Camera->SetProjection(ECameraProjectionType::Ortho, 90.f,
			(float)RS.Width, (float)RS.Height, 1000.f);

		Camera->SetInheritRot(false);
	}

	return true;
}

void CMissile::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	FVector3	Move = GetAxis(EAxis::Y) * 300.f * DeltaTime;

	mDistance -= Move.Length();

	AddWorldPos(Move);

	if (mDistance <= 0.f)
		Destroy();
}

CMissile* CMissile::Clone()
{
	return new CMissile(*this);
}
