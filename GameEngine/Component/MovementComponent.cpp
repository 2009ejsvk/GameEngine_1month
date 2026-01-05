#include "MovementComponent.h"
#include "SceneComponent.h"

CMovementComponent::CMovementComponent()
{
}

CMovementComponent::CMovementComponent(
	const CMovementComponent& ref)	:
	CObjectComponent(ref)
{
}

CMovementComponent::CMovementComponent(
	CMovementComponent&& ref) noexcept :
	CObjectComponent(std::move(ref))
{
}

CMovementComponent::~CMovementComponent()
{
}

void CMovementComponent::SetUpdateComponent(
	const std::weak_ptr<CSceneComponent>& Component)
{
	mUpdateComponent = Component;
}

bool CMovementComponent::Init()
{
	return true;
}

void CMovementComponent::Update(float DeltaTime)
{
}

void CMovementComponent::PostUpdate(float DeltaTime)
{
}

void CMovementComponent::Destroy()
{
	CObjectComponent::Destroy();
}
