#include "ResultWidget.h"
#include "TropicoUiAssetCatalog.h"
#include "../World/LoadingWorld.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/WorldManager.h"

namespace
{
    constexpr float GPanelWidth = 420.f;
    constexpr float GPanelHeight = 336.f;

    void ConfigureDetailText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(86, 72, 54, 255);
        Text->EnableShadow(true);
        Text->SetShadowTextColor(245, 240, 228, 160);
        Text->SetShadowOffset(1.f, 1.f);
    }

    void ConfigureButtonTexture(
        const std::shared_ptr<CButton>& Button,
        const std::string& TexturePrefix)
    {
        if (!Button)
            return;

        Button->SetTexture(
            EButtonState::Normal,
            TexturePrefix + "_Normal",
            TropicoUiAssets::GBigTextButtonTexture);
        Button->SetTexture(
            EButtonState::Hovered,
            TexturePrefix + "_Hovered",
            TropicoUiAssets::GBigTextButtonHoverTexture);
        Button->SetTexture(
            EButtonState::Click,
            TexturePrefix + "_Click",
            TropicoUiAssets::GBigTextButtonSelectedTexture);
        Button->SetTexture(
            EButtonState::Disable,
            TexturePrefix + "_Disable",
            TropicoUiAssets::GBigTextButtonDisabledTexture);
        Button->SetTint(EButtonState::Normal, 1.f, 1.f, 1.f, 0.96f);
        Button->SetTint(EButtonState::Hovered, 1.f, 1.f, 1.f, 1.f);
        Button->SetTint(EButtonState::Click, 0.95f, 0.92f, 0.86f, 1.f);
        Button->SetTint(EButtonState::Disable, 1.f, 1.f, 1.f, 0.55f);
    }
}

CResultWidget::CResultWidget()
{
}

CResultWidget::~CResultWidget()
{
}

bool CResultWidget::Init()
{
    CWidgetContainer::Init();

    FResolution Resolution = CDevice::GetInst()->GetResolution();
    SetPos(0.f, 0.f);
    SetSize(0.f, 0.f);

    auto DimBackground =
        CreateWidget<CImage>("ResultWidget_DimBackground", 0).lock();
    if (DimBackground)
    {
        DimBackground->SetTexture(
            "ResultWidget_DimTexture",
            TropicoUiAssets::GMainMenuPanelTexture);
        DimBackground->SetTint(0.03f, 0.03f, 0.03f, 0.76f);
        DimBackground->SetPos(0.f, 0.f);
        DimBackground->SetSize(
            static_cast<float>(Resolution.Width),
            static_cast<float>(Resolution.Height));
        mDimBackground = DimBackground;
    }

    const float PanelX =
        (static_cast<float>(Resolution.Width) - GPanelWidth) * 0.5f;
    const float PanelY =
        (static_cast<float>(Resolution.Height) - GPanelHeight) * 0.5f - 18.f;

    auto PanelBackground =
        CreateWidget<CImage>("ResultWidget_PanelBackground", 1).lock();
    if (PanelBackground)
    {
        PanelBackground->SetTexture(
            "ResultWidget_PanelTexture",
            TropicoUiAssets::GModernPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 0.98f);
        PanelBackground->SetPos(PanelX, PanelY);
        PanelBackground->SetSize(GPanelWidth, GPanelHeight);
        mPanelBackground = PanelBackground;
    }

    auto TitleRibbon =
        CreateWidget<CImage>("ResultWidget_TitleRibbon", 2).lock();
    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "ResultWidget_TitleRibbonTexture",
            TropicoUiAssets::GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 0.95f);
        TitleRibbon->SetPos(PanelX + 52.f, PanelY + 18.f);
        TitleRibbon->SetSize(316.f, 46.f);
        mTitleRibbon = TitleRibbon;
    }

    auto IconBadge =
        CreateWidget<CImage>("ResultWidget_IconBadge", 3).lock();
    if (IconBadge)
    {
        IconBadge->SetTexture(
            "ResultWidget_IconTexture",
            TropicoUiAssets::GRoundButtonTexture);
        IconBadge->SetTint(0.92f, 0.78f, 0.24f, 0.98f);
        IconBadge->SetPos(PanelX + 18.f, PanelY + 20.f);
        IconBadge->SetSize(42.f, 42.f);
        mIconBadge = IconBadge;
    }

    auto IconText =
        CreateWidget<CTextBlock>("ResultWidget_IconText", 4).lock();
    if (IconText)
    {
        IconText->SetPos(PanelX + 18.f, PanelY + 20.f);
        IconText->SetSize(42.f, 42.f);
        IconText->SetFontSize(24.f);
        IconText->SetAlignH(ETextAlignH::Center);
        IconText->SetAlignV(ETextAlignV::Middle);
        IconText->SetTextColor(110, 74, 14, 255);
        IconText->EnableShadow(true);
        IconText->SetShadowTextColor(255, 241, 204, 210);
        IconText->SetShadowOffset(1.f, 1.f);
        mIconText = IconText;
    }

    auto TitleText =
        CreateWidget<CTextBlock>("ResultWidget_TitleText", 4).lock();
    if (TitleText)
    {
        TitleText->SetPos(PanelX + 74.f, PanelY + 22.f);
        TitleText->SetSize(280.f, 40.f);
        TitleText->SetFontSize(24.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(96, 73, 32, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowTextColor(242, 235, 220, 220);
        TitleText->SetShadowOffset(1.f, 1.f);
        mTitleText = TitleText;
    }

    auto SummaryText =
        CreateWidget<CTextBlock>("ResultWidget_SummaryText", 4).lock();
    if (SummaryText)
    {
        SummaryText->SetPos(PanelX + 42.f, PanelY + 86.f);
        SummaryText->SetSize(GPanelWidth - 84.f, 56.f);
        SummaryText->SetFontSize(18.f);
        SummaryText->SetAlignH(ETextAlignH::Center);
        SummaryText->SetAlignV(ETextAlignV::Middle);
        SummaryText->SetTextColor(88, 76, 59, 255);
        SummaryText->EnableShadow(true);
        SummaryText->SetShadowTextColor(248, 243, 233, 170);
        SummaryText->SetShadowOffset(1.f, 1.f);
        mSummaryText = SummaryText;
    }

    auto PrimaryDetailText =
        CreateWidget<CTextBlock>("ResultWidget_PrimaryDetail", 4).lock();
    if (PrimaryDetailText)
    {
        PrimaryDetailText->SetPos(PanelX + 44.f, PanelY + 146.f);
        PrimaryDetailText->SetSize(GPanelWidth - 88.f, 24.f);
        ConfigureDetailText(PrimaryDetailText);
        mPrimaryDetailText = PrimaryDetailText;
    }

    auto SecondaryDetailText =
        CreateWidget<CTextBlock>("ResultWidget_SecondaryDetail", 4).lock();
    if (SecondaryDetailText)
    {
        SecondaryDetailText->SetPos(PanelX + 44.f, PanelY + 174.f);
        SecondaryDetailText->SetSize(GPanelWidth - 88.f, 24.f);
        ConfigureDetailText(SecondaryDetailText);
        mSecondaryDetailText = SecondaryDetailText;
    }

    auto TertiaryDetailText =
        CreateWidget<CTextBlock>("ResultWidget_TertiaryDetail", 4).lock();
    if (TertiaryDetailText)
    {
        TertiaryDetailText->SetPos(PanelX + 44.f, PanelY + 214.f);
        TertiaryDetailText->SetSize(GPanelWidth - 88.f, 24.f);
        ConfigureDetailText(TertiaryDetailText);
        mTertiaryDetailText = TertiaryDetailText;
    }

    auto QuaternaryDetailText =
        CreateWidget<CTextBlock>("ResultWidget_QuaternaryDetail", 4).lock();
    if (QuaternaryDetailText)
    {
        QuaternaryDetailText->SetPos(PanelX + 44.f, PanelY + 242.f);
        QuaternaryDetailText->SetSize(GPanelWidth - 88.f, 24.f);
        ConfigureDetailText(QuaternaryDetailText);
        mQuaternaryDetailText = QuaternaryDetailText;
    }

    auto RestartButton =
        CreateWidget<CButton>("ResultWidget_RestartButton", 3).lock();
    if (RestartButton)
    {
        RestartButton->SetPos(PanelX + 130.f, PanelY + 274.f);
        RestartButton->SetSize(160.f, 40.f);
        ConfigureButtonTexture(RestartButton, "ResultWidgetRestartButton");
        RestartButton->SetEventCallback<CResultWidget>(
            EButtonEventState::Click,
            this,
            &CResultWidget::OnRestartClick);
        mRestartButton = RestartButton;
    }

    auto RestartButtonText =
        CreateWidget<CTextBlock>("ResultWidget_RestartButtonText", 4).lock();
    if (RestartButtonText)
    {
        RestartButtonText->SetText(L"계속 이어서 하기");
        RestartButtonText->SetPos(PanelX + 130.f, PanelY + 274.f);
        RestartButtonText->SetSize(160.f, 40.f);
        RestartButtonText->SetFontSize(18.f);
        RestartButtonText->SetAlignH(ETextAlignH::Center);
        RestartButtonText->SetAlignV(ETextAlignV::Middle);
        RestartButtonText->SetTextColor(95, 68, 18, 255);
        RestartButtonText->EnableShadow(true);
        RestartButtonText->SetShadowTextColor(240, 228, 199, 220);
        RestartButtonText->SetShadowOffset(1.f, 1.f);
        mRestartButtonText = RestartButtonText;
    }

    RefreshFromState();
    return true;
}

void CResultWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshFromState();
}

void CResultWidget::RefreshFromState()
{
    SetPopupVisible(mState.Visible);

    if (!mState.Visible)
        return;

    FResolution Resolution = CDevice::GetInst()->GetResolution();
    SetPos(0.f, 0.f);
    SetSize(
        static_cast<float>(Resolution.Width),
        static_cast<float>(Resolution.Height));

    if (auto IconText = mIconText.lock())
        IconText->SetText(mState.Victory ? L"승" : L"!");

    if (auto TitleText = mTitleText.lock())
        TitleText->SetText(mState.Title.c_str());

    if (auto SummaryText = mSummaryText.lock())
        SummaryText->SetText(mState.Summary.c_str());

    if (auto Detail = mPrimaryDetailText.lock())
        Detail->SetText(mState.DetailPrimary.c_str());

    if (auto Detail = mSecondaryDetailText.lock())
        Detail->SetText(mState.DetailSecondary.c_str());

    if (auto Detail = mTertiaryDetailText.lock())
        Detail->SetText(mState.DetailTertiary.c_str());

    if (auto Detail = mQuaternaryDetailText.lock())
        Detail->SetText(mState.DetailQuaternary.c_str());

    if (auto RestartButton = mRestartButton.lock())
        RestartButton->ButtonEnable(true);
}

void CResultWidget::SetPopupVisible(bool Visible)
{
    if (Visible)
    {
        FResolution Resolution = CDevice::GetInst()->GetResolution();
        SetSize(
            static_cast<float>(Resolution.Width),
            static_cast<float>(Resolution.Height));
    }
    else
    {
        SetSize(0.f, 0.f);
    }

    if (auto Widget = mDimBackground.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mPanelBackground.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mTitleRibbon.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mIconBadge.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mIconText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mTitleText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mSummaryText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mPrimaryDetailText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mSecondaryDetailText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mTertiaryDetailText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mQuaternaryDetailText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mRestartButton.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mRestartButtonText.lock())
        Widget->SetEnable(Visible);
}

void CResultWidget::OnRestartClick()
{
    auto World = CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();

    if (World)
        World->Load(EWorldType::Main);
}
