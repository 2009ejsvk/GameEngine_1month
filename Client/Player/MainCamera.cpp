#include "MainCamera.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../UI/CitizenInfoWidget.h"
#include "Component/CameraComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Device.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include <algorithm>
#include <cfloat>
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
    auto ActiveObject = mActivePlacementObject.lock();

    if (ActiveObject && ActiveObject->IsMovePreviewActive())
    {
        ActiveObject->CancelMovePreview();
        mActivePlacementObject.reset();
        return;
    }

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

    ActiveObject = mActivePlacementObject.lock();

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
    auto CitizenOrb = PickCitizenOrb(MouseWorldPos);

    if (CitizenOrb)
    {
        EnsureCitizenInfoWidget();
        auto CitizenInfoWidget = mCitizenInfoWidget.lock();

        if (CitizenInfoWidget)
        {
            CitizenInfoWidget->Open(
                CitizenOrb->GetName(),
                CitizenOrb->GetSatisfaction(),
                Input->GetMousePos());
        }
        return;
    }

    auto CitizenInfoWidget = mCitizenInfoWidget.lock();

    if (CitizenInfoWidget)
        CitizenInfoWidget->SetEnable(false);

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

    World->FindObjectListByType<CPlacementAreaObject>(
        mPlacementObjects);
}

std::shared_ptr<CPlacementAreaObject> CMainCamera::PickPlacementObject(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CPlacementAreaObject> BestObject;
    float BestDistSq = FLT_MAX;

    for (size_t i = 0; i < mPlacementObjects.size(); ++i)
    {
        auto PlacementObject = mPlacementObjects[i].lock();

        if (!PlacementObject)
            continue;

        if (!PlacementObject->ContainsPlacedTile(MouseWorldPos))
            continue;

        const float DistSq =
            PlacementObject->GetCenterDistanceSq(MouseWorldPos);

        if (!BestObject || DistSq < BestDistSq)
        {
            BestObject = PlacementObject;
            BestDistSq = DistSq;
        }
    }

    return BestObject;
}

std::shared_ptr<CBuildingMarkerOrb> CMainCamera::PickCitizenOrb(
    const FVector2& MouseWorldPos)
{
    auto World = mWorld.lock();

    if (!World)
        return std::shared_ptr<CBuildingMarkerOrb>();

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        return std::shared_ptr<CBuildingMarkerOrb>();
    }

    std::shared_ptr<CBuildingMarkerOrb> BestOrb;
    float BestDistSq = FLT_MAX;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        FVector3 OrbPos = Orb->GetWorldPos();
        const float dx = OrbPos.x - MouseWorldPos.x;
        const float dy = OrbPos.y - MouseWorldPos.y;
        const float DistSq = dx * dx + dy * dy;
        const float Radius = (std::max)(6.f, Orb->GetOrbDiameter() * 0.5f);
        const float RadiusSq = Radius * Radius;

        if (DistSq > RadiusSq)
            continue;

        if (!BestOrb || DistSq < BestDistSq)
        {
            BestOrb = Orb;
            BestDistSq = DistSq;
        }
    }

    return BestOrb;
}

void CMainCamera::EnsureCitizenInfoWidget()
{
    if (!mCitizenInfoWidget.expired())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    mCitizenInfoWidget =
        UIManager->FindWidget<CCitizenInfoWidget>("CitizenInfoWidget");

    if (!mCitizenInfoWidget.expired())
        return;

    mCitizenInfoWidget =
        UIManager->CreateWidget<CCitizenInfoWidget>(
            "CitizenInfoWidget", 200);
}
