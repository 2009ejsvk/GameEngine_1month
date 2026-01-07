#pragma once

#include "Object/GameObject.h"
#include "../Interface/StateInterface.h"

class CPlayer :
    public CGameObject,
	public CStateInterface
{
	friend class CWorld;
	friend class CObject;

protected:
	CPlayer();
	CPlayer(const CPlayer& ref);
	CPlayer(CPlayer&& ref)	noexcept;

public:
	virtual ~CPlayer();

private:
	std::weak_ptr<class CMeshComponent>		mMeshComponent;
	std::weak_ptr<class CSceneComponent>	mRot;
	std::weak_ptr<class CMeshComponent>		mSubMeshComponent;
	std::weak_ptr<class CCameraComponent>	mCameraComponent;
	std::weak_ptr<class CStateComponent>	mStateComponent;
	std::weak_ptr<class CAnimation2DComponent>	mAnimation2DComponent;
	std::weak_ptr<class CObjectMovementComponent>	mMovement;
	std::weak_ptr<class CColliderBox2D>	mBody;
	std::weak_ptr<class CColliderSphere2D>	mSphere2D;
	std::weak_ptr<class CColliderLine2D>	mLine2D;
	int		mHP = 10;
	bool	mAutoIdle = false;

	std::weak_ptr<class CBullet>	mSkill1Bullet;

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
	virtual void Destroy();

private:
	void TestNotify();
	void AttackNotify();
	void AttackFinish();

private:
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void AttackKey();
	void Skill1Press();
	void Skill1Hold();
	void Skill1Release();
};

