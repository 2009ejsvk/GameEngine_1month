#pragma once

#include "ObjectComponent.h"
#include "../Timer.h"

class CMovementComponent :
    public CObjectComponent
{
	friend class CGameObject;

protected:
	CMovementComponent();
	CMovementComponent(const CMovementComponent& ref);
	CMovementComponent(CMovementComponent&& ref)	noexcept;

public:
	virtual ~CMovementComponent();

protected:
	std::weak_ptr<class CSceneComponent>	mUpdateComponent;

public:
	void SetUpdateComponent(
		const std::weak_ptr<class CSceneComponent>& Component);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void PostRender();
	virtual void Destroy();

protected:
	virtual CMovementComponent* Clone()	const = 0;
};

