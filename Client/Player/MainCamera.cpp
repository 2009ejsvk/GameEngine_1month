#include "MainCamera.h"
#include "../Map/PlacementAreaObject.h"
#include "Component/CameraComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Device.h"
#include "World/World.h"
#include "World/Input.h"
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
        Movement->SetSpeed(400.f);
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

    Input->AddBindKey("MainCameraMoveArea", VK_RBUTTON);
    Input->SetBindFunction<CMainCamera>("MainCameraMoveArea",
        EInputType::Press, this, &CMainCamera::MoveCurrentArea);

    Input->AddBindKey("MainCameraPlace", VK_LBUTTON);
    Input->SetBindFunction<CMainCamera>("MainCameraPlace",
        EInputType::Press, this, &CMainCamera::PlaceCurrentArea);

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

    if (WheelDelta == 0)
        return;

    float WheelStep = WheelDelta / (float)WHEEL_DELTA;
    float ZoomFactor = std::pow(0.9f, WheelStep);

    mZoomWidth = Clamp<float>(mZoomWidth * ZoomFactor,
        mMinZoomWidth, mMaxZoomWidth);
    mZoomHeight = mZoomWidth / mZoomAspect;

    Camera->SetProjection(ECameraProjectionType::Ortho,
        90.f, mZoomWidth, mZoomHeight, mViewDistance);
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

void CMainCamera::MoveCurrentArea()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    const FVector2 MouseWorldPos = Input->GetMouseWorldPos();

    RefreshPlacementObjects();

    auto PlacementObject = PickPlacementObject(MouseWorldPos);

    if (!PlacementObject)
        return;

    auto ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject && ActiveObject != PlacementObject)
    {
        ActiveObject->CancelMovePreview();
    }

    mActivePlacementObject = PlacementObject;
    PlacementObject->StartMovePreview(MouseWorldPos);
}

void CMainCamera::PlaceCurrentArea()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    const FVector2 MouseWorldPos = Input->GetMouseWorldPos();

    RefreshPlacementObjects();

    auto PlacementObject = mActivePlacementObject.lock();

    if (!PlacementObject)
    {
        PlacementObject = PickPlacementObject(MouseWorldPos);
        mActivePlacementObject = PlacementObject;
    }

    if (PlacementObject)
        PlacementObject->ConfirmPlacement();
}

void CMainCamera::RefreshPlacementObjects()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (mBuildingAObject.expired())
    {
        mBuildingAObject = World->FindObject<CPlacementAreaObject>(
            "BuildingA");
    }

    if (mBuildingBObject.expired())
    {
        mBuildingBObject = World->FindObject<CPlacementAreaObject>(
            "BuildingB");
    }
}

std::shared_ptr<CPlacementAreaObject> CMainCamera::PickPlacementObject(
    const FVector2& MouseWorldPos)
{
    auto BuildingA = mBuildingAObject.lock();
    auto BuildingB = mBuildingBObject.lock();

    if (BuildingA && BuildingA->ContainsPlacedTile(MouseWorldPos))
        return BuildingA;

    if (BuildingB && BuildingB->ContainsPlacedTile(MouseWorldPos))
        return BuildingB;

    if (BuildingA && BuildingB)
    {
        const float DistA = BuildingA->GetCenterDistanceSq(MouseWorldPos);
        const float DistB = BuildingB->GetCenterDistanceSq(MouseWorldPos);

        return DistA <= DistB ? BuildingA : BuildingB;
    }

    if (BuildingA)
        return BuildingA;

    return BuildingB;
}
