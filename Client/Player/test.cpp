#include "test.h"

#include "Component/MeshComponent.h"
#include "World/World.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ColliderBox2D.h"

Ctest::Ctest()
{
	SetClassType<Ctest>();
}

Ctest::Ctest(const Ctest& ref) :
	CGameObject(ref)
{
}

Ctest::Ctest(Ctest&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

Ctest::~Ctest()
{
}

bool Ctest::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("Mesh");

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("MaterialColor2D");
		Mesh->SetMesh("CenterRectColor");
		Mesh->SetRelativeScale(1920.f, 5.f, 100.f);
	}

	return true;
}

void Ctest::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);
}

void Ctest::PostUpdate(float DeltaTime)
{
	CGameObject::PostUpdate(DeltaTime);
}

Ctest* Ctest::Clone()
{
	return new Ctest(*this);
}