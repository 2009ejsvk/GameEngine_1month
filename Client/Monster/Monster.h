#pragma once

#include "Object/GameObject.h"
#include "../Interface/StateInterface.h"

class CMonster :
    public CGameObject,
	public CStateInterface
{
	friend class CWorld;
	friend class CObject;

protected:
	CMonster();
	CMonster(const CMonster& ref);
	CMonster(CMonster&& ref)	noexcept;

public:
	virtual ~CMonster();

private:
	std::weak_ptr<class CMeshComponent>	mMeshComponent;
	std::weak_ptr<class CStateComponent>	mStateComponent;
	std::weak_ptr<class CAnimation2DComponent>	mAnimation2DComponent;
	std::weak_ptr<class CColliderSphere2D>	mBody;
	//std::weak_ptr<class CColliderBox2D>	mBody;
	std::weak_ptr<class CColliderLine2D>	mLine2D;
	std::weak_ptr<class CWidgetComponent>	mHUDWidget;
	float	mFireTime = 0.f;
	std::weak_ptr<CGameObject>	mTargetObject;
	float	mDetectRange = 600.f;
	int		mHP = 10;
	int		mHPMax = 10;
	std::vector<std::function<void(float, float)>>	mHPWidgetFunc;

public:
	void Damage(int Dmg)
	{
		mHP -= Dmg;

		char	Test[256] = {};
		sprintf_s(Test, "HP : %d\n", mHP);
		OutputDebugStringA(Test);
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual float TakeDamage(float Damage);

protected:
	virtual CMonster* Clone();

private:
	void AttackNotify();
	void AttackFinish();
};

