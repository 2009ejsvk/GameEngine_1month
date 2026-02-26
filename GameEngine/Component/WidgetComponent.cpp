#include "WidgetComponent.h"
#include "../Device.h"
#include "../World/World.h"
#include "../World/CameraManager.h"

CWidgetComponent::CWidgetComponent()
{
	SetClassType<CWidgetComponent>();
	mRenderType = EComponentRender::Render;
}

CWidgetComponent::CWidgetComponent(const CWidgetComponent& ref)	:
	CSceneComponent(ref)
{
}

CWidgetComponent::CWidgetComponent(CWidgetComponent&& ref) noexcept :
	CSceneComponent(ref)
{
}

CWidgetComponent::~CWidgetComponent()
{
}

void CWidgetComponent::Begin()
{
	CSceneComponent::Begin();
}

bool CWidgetComponent::Init()
{
	CSceneComponent::Init();
	return true;
}

void CWidgetComponent::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);

	if (mWidget)
		mWidget->Update(DeltaTime);
}

void CWidgetComponent::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);
}

void CWidgetComponent::Render()
{
	CSceneComponent::Render();

	if (mWidget)
	{
		FVector3 Size = mWidget->GetSize();

		FResolution	RS = CDevice::GetInst()->GetResolution();

		FVector3	CameraPos = mWorld.lock()->GetCameraManager().lock()->GetMainCameraWorldPos();

		FVector3	LT;

		LT.x = CameraPos.x - RS.Width * 0.5f;
		LT.y = CameraPos.y + RS.Height * 0.5f;

		// 화면 기준의 위치를 구한다.
		FVector3	Pos;

		Pos.x = mWorldPos.x - LT.x;
		Pos.y = LT.y - mWorldPos.y;

		Pos.x -= Size.x * 0.5f;
		Pos.y -= Size.y * 0.5f;

		mWidget->SetPos(Pos);
		mWidget->Render();
	}
}

void CWidgetComponent::PostRender()
{
	CSceneComponent::PostRender();
}

void CWidgetComponent::Destroy()
{
	CSceneComponent::Destroy();
}

CWidgetComponent* CWidgetComponent::Clone()	const
{
	return new CWidgetComponent(*this);
}