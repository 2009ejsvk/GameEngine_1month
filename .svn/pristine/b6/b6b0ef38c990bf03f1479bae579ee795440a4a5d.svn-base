#include "CameraManager.h"
#include "../Component/CameraComponent.h"

CCameraManager::CCameraManager()
{
}

CCameraManager::~CCameraManager()
{
}


const FVector3& CCameraManager::GetMainCameraWorldPos()	const
{
	auto	Camera = mMainCamera.lock();

	if (Camera)
		return Camera->GetWorldPos();

	return FVector3::Zero;
}

const FVector3& CCameraManager::GetMainCameraAxis(EAxis::Type Axis)	const
{
	auto	Camera = mMainCamera.lock();

	if (Camera)
		return Camera->GetAxis(Axis);

	return FVector3::Zero;
}

const FMatrix& CCameraManager::GetViewMatrix()	const
{
	auto	Camera = mMainCamera.lock();

	if (Camera)
		return Camera->GetViewMatrix();

	return FMatrix::IdentityMatrix;
}

const FMatrix& CCameraManager::GetProjMatrix()	const
{
	auto	Camera = mMainCamera.lock();

	if (Camera)
		return Camera->GetProjMatrix();

	return FMatrix::IdentityMatrix;
}

void CCameraManager::AddCamera(const std::string& Name, 
	const std::weak_ptr<class CCameraComponent>& Camera)
{
	auto iter = mCameraMap.find(Name);

	// 같은 이름이 없을 경우에만 추가
	if (iter == mCameraMap.end())
	{
		if (mCameraMap.empty())
			mMainCamera = Camera;

		mCameraMap.insert(std::make_pair(Name, Camera));
	}
}

void CCameraManager::ChangeMainCamera(const std::string& Name)
{
	auto iter = mCameraMap.find(Name);

	if (iter != mCameraMap.end())
	{
		mMainCamera = iter->second;
	}
}

void CCameraManager::ChangeMainCamera(
	const std::weak_ptr<class CCameraComponent>& Camera)
{
	mMainCamera = Camera;
}

bool CCameraManager::Init()
{
	return true;
}

void CCameraManager::Update(float DeltaTime)
{
	// 카메라중 유효하지 않은 카메라는 제거한다.
	auto	iter = mCameraMap.begin();
	auto	iterEnd = mCameraMap.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.expired())
		{
			iter = mCameraMap.erase(iter);
			continue;
		}

		++iter;
	}

	if (mMainCamera.expired())
	{
		if (!mCameraMap.empty())
			mMainCamera = mCameraMap.begin()->second;
	}
}
