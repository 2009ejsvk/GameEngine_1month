#include "StateComponent.h"
#include "Object/GameObject.h"

CStateComponent::CStateComponent()
{
	SetClassType<CStateComponent>();
}

CStateComponent::CStateComponent(const CStateComponent& ref) :
	CObjectComponent(ref)
{
}

CStateComponent::CStateComponent(CStateComponent&& ref)	noexcept :
	CObjectComponent(std::move(ref))
{
}

CStateComponent::~CStateComponent()
{
}

bool CStateComponent::AddHP(int Damage)
{
	mHP += Damage;

	char	Test[256] = {};
	sprintf_s(Test, "HP : %d\n", mHP);
	OutputDebugStringA(Test);

	if (mHP >= mHPMax)
		mHP = mHPMax;

	else if (mHP <= 0)
	{
		mHP = 0;

		auto Owner = mOwner.lock();

		Owner->Destroy();

		return false;
	}

	return true;
}

CStateComponent* CStateComponent::Clone()	const
{
	return new CStateComponent(*this);
}
