#include "MainCamera.h"
#include "Component/CameraComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Device.h"
#include "../ObjectNames.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/EdictWidget.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include <cmath>

CMainCamera::CMainCamera()
{
    SetClassType<CMainCamera>();
}

CMainCamera::CMainCamera(const CMainCamera& ref) :
    CGameObject(ref)
{
}

CMainCamera::CMainCamera(CMainCamera&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CMainCamera::~CMainCamera()
{
}

bool CMainCamera::Init()
{
    CGameObject::Init();

    mCameraComponent = CreateComponent<CCameraComponent>("MainCamera");
    mMovement = CreateComponent<CObjectMovementComponent>("MainCameraMovement");

    auto Movement = mMovement.lock();

    if (Movement)
    {
        Movement->SetUpdateComponent(mCameraComponent);
        Movement->SetSpeed(800.f);
    }

    auto Camera = mCameraComponent.lock();

    if (Camera)
    {
        const FResolution& RS = CDevice::GetInst()->GetResolution();
        mZoomWidth = (float)RS.Width;
        mZoomHeight = (float)RS.Height;
        mZoomAspect = mZoomWidth / mZoomHeight;
        mMaxZoomWidth = mZoomWidth * 8.f;

        Camera->SetProjection(ECameraProjectionType::Ortho,
            90.f, mZoomWidth, mZoomHeight, mViewDistance);
        Camera->SetInheritRot(false);
    }

    auto World = mWorld.lock();
    auto Input = World->GetInput().lock();

    Input->AddBindKey("MainCameraMoveUp", 'W');
    Input->SetBindFunction<CMainCamera>("MainCameraMoveUp",
        EInputType::Hold, this, &CMainCamera::MoveUp);

    Input->AddBindKey("MainCameraMoveDown", 'S');
    Input->SetBindFunction<CMainCamera>("MainCameraMoveDown",
        EInputType::Hold, this, &CMainCamera::MoveDown);

    Input->AddBindKey("MainCameraMoveLeft", 'A');
    Input->SetBindFunction<CMainCamera>("MainCameraMoveLeft",
        EInputType::Hold, this, &CMainCamera::MoveLeft);

    Input->AddBindKey("MainCameraMoveRight", 'D');
    Input->SetBindFunction<CMainCamera>("MainCameraMoveRight",
        EInputType::Hold, this, &CMainCamera::MoveRight);

    return true;
}

void CMainCamera::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    auto Camera = mCameraComponent.lock();

    if (!Camera)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    int WheelDelta = Input->GetMouseWheelDelta();

    auto UIManager = World->GetUIManager().lock();

    if (UIManager)
    {
        auto BuildMenu = UIManager->FindWidget<CBuildMenuWidget>(
            GBuildMenuWidgetName).lock();
        auto EdictWidget = UIManager->FindWidget<CEdictWidget>(
            GEdictWidgetName).lock();

        if (BuildMenu && BuildMenu->IsMouseOverOpenPanel(Input->GetMousePos()))
            WheelDelta = 0;
        if (EdictWidget && EdictWidget->IsMouseOverOpenPanel(Input->GetMousePos()))
            WheelDelta = 0;
    }

    if (WheelDelta != 0)
    {
        float WheelStep = WheelDelta / (float)WHEEL_DELTA;
        float ZoomFactor = std::pow(0.9f, WheelStep);

        mZoomWidth = Clamp<float>(mZoomWidth * ZoomFactor,
            mMinZoomWidth, mMaxZoomWidth);
        mZoomHeight = mZoomWidth / mZoomAspect;

        Camera->SetProjection(ECameraProjectionType::Ortho,
            90.f, mZoomWidth, mZoomHeight, mViewDistance);
    }
}

void CMainCamera::MoveUp()
{
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(GetAxis(EAxis::Y));
}

void CMainCamera::MoveDown()
{
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(-GetAxis(EAxis::Y));
}

void CMainCamera::MoveLeft()
{
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(-GetAxis(EAxis::X));
}

void CMainCamera::MoveRight()
{
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(GetAxis(EAxis::X));
}
