#include "ObjectComponent.h"


CObjectComponent::CObjectComponent()
{
	mComponentType = EComponentType::Object;
}

CObjectComponent::CObjectComponent(const CObjectComponent& ref) :
	CComponent(ref)
{
	mComponentType = EComponentType::Object;
}

CObjectComponent::CObjectComponent(CObjectComponent&& ref)	noexcept :
	CComponent(std::move(ref))
{
	mComponentType = EComponentType::Object;
}

CObjectComponent::~CObjectComponent()
{
}

void CObjectComponent::Begin()
{
	CComponent::Begin();
}

bool CObjectComponent::Init()
{
	CComponent::Init();

	return true;
}

void CObjectComponent::Update(float DeltaTime)
{
}

void CObjectComponent::PostUpdate(float DeltaTime)
{
}

void CObjectComponent::Destroy()
{
	CComponent::Destroy();
}
