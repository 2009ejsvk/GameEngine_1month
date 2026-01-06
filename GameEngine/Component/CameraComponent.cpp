#include "CameraComponent.h"
#include "../Device.h"
#include "../World/World.h"
#include "../World/CameraManager.h"

CCameraComponent::CCameraComponent()
{
	SetClassType<CCameraComponent>();
}

CCameraComponent::CCameraComponent(const CCameraComponent& ref)	:
	CSceneComponent(ref)
{
	mProjType = ref.mProjType;
	mViewMatrix = ref.mViewMatrix;
	mProjMatrix = ref.mProjMatrix;
	mViewAngle = ref.mViewAngle;
	mWidth = ref.mWidth;
	mHeight = ref.mHeight;
	mViewDistance = ref.mViewDistance;
}

CCameraComponent::CCameraComponent(CCameraComponent&& ref) noexcept :
	CSceneComponent(std::move(ref))
{
	mProjType = ref.mProjType;
	mViewMatrix = ref.mViewMatrix;
	mProjMatrix = ref.mProjMatrix;
	mViewAngle = ref.mViewAngle;
	mWidth = ref.mWidth;
	mHeight = ref.mHeight;
	mViewDistance = ref.mViewDistance;
}

CCameraComponent::~CCameraComponent()
{
}

void CCameraComponent::SetProjection(ECameraProjectionType Type,
	float ViewAngle, float Width, float Height, float ViewDistance)
{
	mProjType = Type;
	mViewAngle = ViewAngle;
	mWidth = Width;
	mHeight = Height;
	mViewDistance = ViewDistance;

	switch (mProjType)
	{
	case ECameraProjectionType::Perspective:
		mProjMatrix = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(mViewAngle),
			mWidth / mHeight, 0.1f, mViewDistance);
		break;
	case ECameraProjectionType::Ortho:
		mProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
			mWidth / -2.f, mWidth / 2.f, mHeight / -2.f,
			mHeight / 2.f, 0.f, mViewDistance);
		break;
	}
}

bool CCameraComponent::Init()
{
	CSceneComponent::Init();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	SetProjection(ECameraProjectionType::Perspective, 90.f,
		(float)RS.Width, (float)RS.Height, 1000.f);

	// 카메라 매니저에 등록한다.
	auto	World = mWorld.lock();

	if (World)
	{
		std::weak_ptr<CCameraManager> Weak_CameraManager =
			World->GetCameraManager();

		auto	CameraManager = Weak_CameraManager.lock();

		if (CameraManager)
		{
			auto	Self = mSelf.lock();

			CameraManager->AddCamera(mName, 
				std::dynamic_pointer_cast<CCameraComponent>(Self));
		}
	}

	return true;
}

void CCameraComponent::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);
}

void CCameraComponent::PostUpdate(float DeltaTime)
{
	/*
	카메라를 적용하는 방법.
	월드의 모든 물체를 카메라가 0, 0, 0으로 이동하는 만큼
	이동시키고 카메라가 바라보는 방향(카메라의 Z축)이 0, 0, 1이
	되게 회전시키는 만큼 물체들을 공전시켜준다.
	뷰공간으로 카메라가 변환되면 카메라의 X, Y, Z 축은
	X축 : 1, 0, 0 Y축 : 0, 1, 0 Z축 : 0, 0, 1
	1, 0, 0
	0, 1, 0
	0, 0, 1

	카메라의 월드공간에서의 X, Y, Z축을 뷰공간으로 변환하면 나오는 결과값
	WXx WXy WXz * 뷰행렬 = 1, 0, 0
	WYx WYy WYz * 뷰행렬 = 0, 1, 0
	WZx WZy WZz * 뷰행렬 = 0, 0, 1

	카메라의 월드축 3개를 배치한 3x3 행렬을 뷰행렬의 회전부분 3x3에
	곱하면 항등행렬이 나오게 된다.
	뷰행렬 = WXx WXy WXz
			WYx WYy WYz
			WZx WZy WZz  의 역행렬이다.
	WXx WXy WXz
	WYx WYy WYz
	WZx WZy WZz
	위 행렬은 각 축이 서로에 대해 90도이다. 서로 90도인 축을 이용하여
	구성된 행렬을 직교행렬 이라고 한다. 직교행렬은 전치행렬과 역행렬이 같다.
	WXx WYx WZx 0
	WXy WYy WZy 0
	WXz WYz WZz 0
	0   0   0   1
	뷰행렬의 공전부분이다.
	1  0  0  0
	0  1  0  0
	0  0  1  0
	-x -y -z 1
	뷰행렬의 이동부분이다.
	1  0  0  0   WXx WYx WZx 0   WXx  WYx  WZx  0
	0  1  0  0	 WXy WYy WZy 0 = WXy  WYy  WZy  0
	0  0  1  0 * WXz WYz WZz 0   WXz  WYz  WZz  0
	-x -y -z 1	 0   0   0   1   -X.P -Y.P -Z.P 1
	*/
	mViewMatrix.Identity();

	for (int i = 0; i < EAxis::End; ++i)
	{
		memcpy(&mViewMatrix[i], &mWorldAxis[i], sizeof(FVector3));
	}

	/*
	위에서 memcpy 하면
	WXx WXy WXz 0
	WYx WYy WYz 0
	WZx WZy WZz 0
	0   0   0   1
	*/
	mViewMatrix.Transpose();

	/*
	WXx WYx WZx 0
	WXy WYy WZy 0
	WXz WYz WZz 0
	0   0   0   1
	*/
	for (int i = 0; i < EAxis::End; ++i)
	{
		mViewMatrix[3][i] = -mWorldPos.Dot(mWorldAxis[i]);
	}

	CSceneComponent::PostUpdate(DeltaTime);
}

CCameraComponent* CCameraComponent::Clone()	const
{
	return new CCameraComponent(*this);
}
