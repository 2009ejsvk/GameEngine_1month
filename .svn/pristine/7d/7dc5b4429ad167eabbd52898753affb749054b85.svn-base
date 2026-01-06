#pragma once

#include "Object/GameObject.h"

class CBulletEffect :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CBulletEffect();
	CBulletEffect(const CBulletEffect& ref);
	CBulletEffect(CBulletEffect&& ref)	noexcept;

public:
	virtual ~CBulletEffect();

private:
	std::weak_ptr<class CMeshComponent>	mMeshComponent;
	std::weak_ptr<class CAnimation2DComponent>	mAnimation2DComponent;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);

protected:
	virtual CBulletEffect* Clone();

private:
	void FinishAnimation();
};

