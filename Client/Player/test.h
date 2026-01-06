#pragma once

#include "Object/GameObject.h"

class Ctest : public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	Ctest();
	Ctest(const Ctest& ref);
	Ctest(Ctest&& ref)	noexcept;

public:
	virtual ~Ctest();

private:
	std::weak_ptr<class CMeshComponent>		mMeshComponent;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

protected:
	virtual Ctest* Clone();
};

