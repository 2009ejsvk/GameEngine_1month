#pragma once

#include "../EngineInfo.h"

class CCameraManager
{
	friend class CWorld;

private:
	CCameraManager();

public:
	~CCameraManager();

private:
	// unordered_multimap : 이름이 중복되어도 추가된다.
	std::unordered_multimap<std::string, std::weak_ptr<class CCameraComponent>>	mCameraMap;
	std::weak_ptr<class CCameraComponent>	mMainCamera;

public:
	const std::weak_ptr<CCameraComponent>& GetMainCamera()	const
	{
		return mMainCamera;
	}

	const FVector3& GetMainCameraWorldPos()	const;
	const FVector3& GetMainCameraAxis(EAxis::Type Axis)	const;
	const FMatrix& GetViewMatrix()	const;
	const FMatrix& GetProjMatrix()	const;

public:
	void AddCamera(const std::string& Name, 
		const std::weak_ptr<class CCameraComponent>& Camera);
	void ChangeMainCamera(const std::string& Name);
	void ChangeMainCamera(const std::weak_ptr<class CCameraComponent>& Camera);
	bool Init();
	void Update(float DeltaTime);
};

