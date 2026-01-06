#pragma once

#include "Component/ObjectComponent.h"

class CStateComponent :
    public CObjectComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CStateComponent();
	CStateComponent(const CStateComponent& ref);
	CStateComponent(CStateComponent&& ref)	noexcept;

public:
	virtual ~CStateComponent();

protected:
	int		mHP = 10;
	int		mHPMax = 10;

public:
	bool AddHP(int Damage);

protected:
	virtual CStateComponent* Clone()	const;
};

