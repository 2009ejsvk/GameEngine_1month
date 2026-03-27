#include "MainCamera.h"
#include "Component/CameraComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Device.h"
#include "../ObjectNames.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementController.h"
#include "../UI/TopHudWidget.h"
#include "../UI/BuildMenuWidget.h"
#include "../UI/EdictWidget.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include <cmath>
#include "../World/MainWorld.h"
#include "../World/ScenarioSubsystem.h"

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

    Input->AddBindKey("HandleEscape", VK_ESCAPE);
    Input->SetBindFunction<CMainCamera>("HandleEscape",
        EInputType::Press, this, &CMainCamera::HandleEscape);

    Input->AddBindKey("DebugSkipScenarioPhase", VK_F6);
    Input->SetBindFunction<CMainCamera>("DebugSkipScenarioPhase",
        EInputType::Press, this, &CMainCamera::DebugSkipScenarioPhase);

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

    if (!mFollowCitizenName.empty())
    {
        auto Citizen = World->FindObject<CBuildingMarkerOrb>(
            mFollowCitizenName).lock();

        if (Citizen && Citizen->GetAlive())
        {
            const FVector3 CitizenPos = Citizen->GetWorldPos();
            const FVector3 CameraPos = GetWorldPos();
            SetWorldPos(CitizenPos.x, CitizenPos.y, CameraPos.z);
        }
        else
            mFollowCitizenName.clear();
    }
}

void CMainCamera::SetFollowCitizen(const std::string& CitizenName)
{
    mFollowCitizenName = CitizenName;
}

void CMainCamera::ClearFollowCitizen()
{
    mFollowCitizenName.clear();
}

void CMainCamera::MoveUp()
{
    mFollowCitizenName.clear();
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(GetAxis(EAxis::Y));
}

void CMainCamera::MoveDown()
{
    mFollowCitizenName.clear();
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(-GetAxis(EAxis::Y));
}

void CMainCamera::MoveLeft()
{
    mFollowCitizenName.clear();
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(-GetAxis(EAxis::X));
}

void CMainCamera::MoveRight()
{
    mFollowCitizenName.clear();
    auto Movement = mMovement.lock();

    if (Movement)
        Movement->AddMove(GetAxis(EAxis::X));
}

void CMainCamera::DebugSkipScenarioPhase()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto* MainWorld = dynamic_cast<CMainWorld*>(World.get());

    if (!MainWorld)
        return;

    CScenarioSubsystem* Scenario = MainWorld->GetScenario();

    if (Scenario)
        Scenario->DebugSkipPhase();
}

void CMainCamera::HandleEscape()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Placement = World->FindObject<CPlacementController>(
        GPlacementControllerName).lock();

    if (Placement && Placement->IsAnyModeActive())
    {
        Placement->CancelPlacementMode();
        return;
    }

    auto UIManager = World->GetUIManager().lock();

    if (!UIManager)
        return;

    auto TopHud = UIManager->FindWidget<CTopHudWidget>(
        GTopHudWidgetName).lock();

    if (!TopHud)
        return;

    if (TopHud->GetState().ExitConfirmPopupOpen)
    {
        TopHud->CloseExitConfirmPopup();
        return;
    }

    if (TopHud->IsAnyMenuOpen())
    {
        TopHud->CloseConstitutionPanel();
        TopHud->CloseMenus(true, true, true, true, true);
        return;
    }

    TopHud->OpenExitConfirmPopup();
}
