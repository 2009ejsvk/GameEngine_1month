#pragma once

#include "Component.h"

class CObjectComponent :
    public CComponent
{
	friend class CGameObject;

protected:
	CObjectComponent();
	CObjectComponent(const CObjectComponent& ref);
	CObjectComponent(CObjectComponent&& ref)	noexcept;

public:
	virtual ~CObjectComponent();

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

protected:
	virtual CObjectComponent* Clone()	const = 0;
};

