#pragma once

#include "Object/GameObject.h"

class CMissile :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CMissile();
	CMissile(const CMissile& ref);
	CMissile(CMissile&& ref)	noexcept;

public:
	virtual ~CMissile();

private:
	std::weak_ptr<class CMeshComponent>		mMeshComponent;
	std::weak_ptr<class CCameraComponent>	mCameraComponent;
	float		mDistance = 1200.f;

public:
	void ChangeCamera();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

protected:
	virtual CMissile* Clone();
};

