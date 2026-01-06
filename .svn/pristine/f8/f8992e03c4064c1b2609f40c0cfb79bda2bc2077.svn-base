#include "Component.h"

CComponent::CComponent()
{
}

CComponent::CComponent(const CComponent& ref)
{
	//mWorld = ref.mWorld;
	//mOwner = ref.mOwner;
	mName = ref.mName;
}

CComponent::CComponent(CComponent&& ref)	noexcept
{
	//mWorld = ref.mWorld;
	//mOwner = ref.mOwner;
	mName = ref.mName;

	//ref.mWorld.reset();
	//ref.mOwner.reset();
}

CComponent::~CComponent()
{
}

void CComponent::SetWorld(std::weak_ptr<CWorld> World)
{
	mWorld = World;
}

void CComponent::SetOwner(std::weak_ptr<CGameObject> Owner)
{
	mOwner = Owner;
}

void CComponent::Begin()
{
}

bool CComponent::Init()
{
	return true;
}

void CComponent::Update(float DeltaTime)
{
}

void CComponent::PostUpdate(float DeltaTime)
{
}

void CComponent::Render()
{
}

void CComponent::Destroy()
{
	mAlive = false;
}
