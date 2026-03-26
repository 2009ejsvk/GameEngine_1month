#include "StartWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "Engine.h"
#include "World/WorldManager.h"
#include "UIStrings.h"
#include "../GlobalSetting.h"
#include "../World/LoadingWorld.h"

namespace
{
    std::wstring GetUiStringOrDefault(
        const wchar_t* Key,
        const wchar_t* DefaultValue)
    {
        const std::wstring& Value = UIStrings::Get(Key);

        if (Value == Key)
            return std::wstring(DefaultValue);

        return Value;
    }

    // 버튼 이미지 + 위에 텍스트 레이블을 조합해 메뉴 버튼 한 개를 만든다.
    void CreateMenuButton(
        CWidgetContainer* Owner,
        const char* ButtonName,
        const char* LabelName,
        const std::wstring& /*ButtonTexture*/,
        const wchar_t* LabelText,
        float CenterX,
        float CenterY,
        float BtnW,
        float BtnH)
    {
        auto Btn = Owner->CreateWidget<CButton>(ButtonName, 1).lock();
        Btn->SetPivot(0.5f, 0.5f);
        Btn->SetPos(CenterX, CenterY);
        Btn->SetSize(BtnW, BtnH);
        Btn->SetTexture(EButtonState::Normal,  ButtonName, TEXT("TropicoMenuBtn.png"));
        Btn->SetTexture(EButtonState::Hovered, ButtonName, TEXT("TropicoMenuBtnHover.png"));
        Btn->SetTexture(EButtonState::Click,   ButtonName, TEXT("TropicoMenuBtnClick.png"));
        Btn->SetTexture(EButtonState::Disable, ButtonName, TEXT("TropicoMenuBtn.png"));
        Btn->SetTint(EButtonState::Normal,  FVector4(1.f, 1.f, 1.f, 1.f));
        Btn->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));
        Btn->SetTint(EButtonState::Click,   FVector4(0.8f, 0.8f, 0.8f, 1.f));

        auto Label = Owner->CreateWidget<CTextBlock>(LabelName, 2).lock();
        Label->SetText(LabelText);
        Label->SetTextColor(255, 240, 200, 255);
        Label->SetPos(CenterX - BtnW * 0.5f, CenterY - 18.f);
        Label->SetSize(BtnW, 36.f);
        Label->SetFontSize(22.f);
        Label->EnableShadow(true);
        Label->SetShadowOffset(2.f, 2.f);
        Label->SetShadowTextColor(60, 40, 10, 200);
    }
}

CStartWidget::CStartWidget()
{
}

CStartWidget::~CStartWidget()
{
}

bool CStartWidget::Init()
{
    CWidgetContainer::Init();

    const std::wstring BackgroundTexture =
        GetUiStringOrDefault(L"start.background.image", L"TropicoBack.png");
    const std::wstring ButtonTexture =
        GetUiStringOrDefault(L"start.button.start.image", L"Start.png");

    FResolution RS = CDevice::GetInst()->GetResolution();
    const float CX = RS.Width  * 0.5f;
    const float CY = RS.Height * 0.5f;

    // 배경
    auto Back = CreateWidget<CImage>("Back").lock();
    Back->SetSize((float)RS.Width, (float)RS.Height);
    Back->SetTexture("StartBack", BackgroundTexture.c_str());

    // 반투명 어두운 오버레이 — 버튼 가독성 확보
    auto Overlay = CreateWidget<CImage>("Overlay", 1).lock();
    Overlay->SetSize(280.f, 290.f);
    Overlay->SetPos(CX - 140.f, CY - 80.f);
    Overlay->SetTint(FVector4(0.f, 0.f, 0.f, 0.55f));

    // Tropico 6 로고 이미지
    auto Logo = CreateWidget<CImage>("Logo", 2).lock();
    Logo->SetSize(280.f, 80.f);
    Logo->SetPos(CX - 140.f, CY - 155.f);
    Logo->SetTexture("TropicoLogo", TEXT("TropicoLogo.png"));

    const float BtnW = 220.f;
    const float BtnH =  56.f;
    const float Gap  =  74.f;

    // 시나리오 모드
    CreateMenuButton(this,
        "ScenarioButton", "ScenarioLabel",
        ButtonTexture, L"시나리오 모드",
        CX, CY - 30.f,
        BtnW, BtnH);

    // 샌드박스 모드
    CreateMenuButton(this,
        "SandboxButton", "SandboxLabel",
        ButtonTexture, L"샌드박스 모드",
        CX, CY - 30.f + Gap,
        BtnW, BtnH);

    // 종료
    CreateMenuButton(this,
        "ExitButton", "ExitLabel",
        GetUiStringOrDefault(L"start.button.exit.image", L"End.png"),
        L"종료",
        CX, CY - 30.f + Gap * 2.f,
        BtnW, BtnH);

    // 콜백 연결
    if (auto Btn = FindWidget<CButton>("ScenarioButton").lock())
        Btn->SetEventCallback<CStartWidget>(
            EButtonEventState::Click, this, &CStartWidget::ScenarioClick);

    if (auto Btn = FindWidget<CButton>("SandboxButton").lock())
        Btn->SetEventCallback<CStartWidget>(
            EButtonEventState::Click, this, &CStartWidget::SandboxClick);

    if (auto Btn = FindWidget<CButton>("ExitButton").lock())
        Btn->SetEventCallback<CStartWidget>(
            EButtonEventState::Click, this, &CStartWidget::ExitClick);

    return true;
}

void CStartWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
}

void CStartWidget::Render()
{
    CWidgetContainer::Render();
}

void CStartWidget::ScenarioClick()
{
    GameSession::CurrentMode() = EGameMode::Scenario;

    auto World =
        CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();
    World->Load(EWorldType::Main);
}

void CStartWidget::SandboxClick()
{
    GameSession::CurrentMode() = EGameMode::Sandbox;

    auto World =
        CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();
    World->Load(EWorldType::Main);
}

void CStartWidget::ExitClick()
{
    CEngine::GetInst()->Destroy();
}
