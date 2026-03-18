#include "EventWidget.h"
#include "TaskWidget.h"
#include "TopHudWidget.h"
#include "TropicoUiAssetCatalog.h"
#include "../World/GovernmentCommandService.h"
#include "../ObjectNames.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include "World/WorldUIManager.h"

namespace
{
    constexpr float GPanelWidth = 400.f;
    constexpr float GPanelHeight = 220.f;
    constexpr float GAcceptButtonLeftX = -130.f;
    constexpr float GRejectButtonLeftX = 20.f;
    constexpr float GSingleButtonLeftX = -55.f;
    constexpr float GButtonY = 104.f;

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
        Button->SetTint(EButtonState::Normal, 1.f, 1.f, 1.f, 0.94f);
        Button->SetTint(EButtonState::Hovered, 1.f, 1.f, 1.f, 1.f);
        Button->SetTint(EButtonState::Click, 0.94f, 0.91f, 0.84f, 1.f);
        Button->SetTint(EButtonState::Disable, 1.f, 1.f, 1.f, 0.55f);
    }

    void ConfigureButtonLabel(
        const std::shared_ptr<CTextBlock>& Label,
        const wchar_t* Text)
    {
        if (!Label)
            return;

        Label->SetText(Text);
        Label->SetFontSize(18.f);
        Label->SetAlignH(ETextAlignH::Center);
        Label->SetAlignV(ETextAlignV::Middle);
        Label->SetTextColor(95, 68, 18, 255);
        Label->EnableShadow(true);
        Label->SetShadowTextColor(240, 228, 199, 220);
        Label->SetShadowOffset(1.f, 1.f);
    }
}

CEventWidget::CEventWidget()
{
}

CEventWidget::~CEventWidget()
{
}

bool CEventWidget::Init()
{
    CWidgetContainer::Init();

    FResolution Resolution = CDevice::GetInst()->GetResolution();
    SetPivot(0.5f, 0.5f);
    SetPos(
        static_cast<float>(Resolution.Width) * 0.5f,
        static_cast<float>(Resolution.Height) * 0.5f - 18.f);
    SetSize(GPanelWidth, GPanelHeight);

    auto PanelBackground =
        CreateWidget<CImage>("EventWidget_Background", 0).lock();
    if (PanelBackground)
    {
        PanelBackground->SetTexture(
            "EventWidget_BackgroundTexture",
            TropicoUiAssets::GModernPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 0.97f);
        PanelBackground->SetPos(-GPanelWidth * 0.5f, -GPanelHeight * 0.5f);
        PanelBackground->SetSize(GPanelWidth, GPanelHeight);
        mPanelBackground = PanelBackground;
    }

    auto TitleRibbon =
        CreateWidget<CImage>("EventWidget_TitleRibbon", 1).lock();
    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "EventWidget_TitleRibbonTexture",
            TropicoUiAssets::GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 0.95f);
        TitleRibbon->SetPos(-156.f, -103.f);
        TitleRibbon->SetSize(284.f, 42.f);
        mTitleRibbon = TitleRibbon;
    }

    auto IconBadge =
        CreateWidget<CImage>("EventWidget_IconBadge", 2).lock();
    if (IconBadge)
    {
        IconBadge->SetTexture(
            "EventWidget_IconBadgeTexture",
            TropicoUiAssets::GRoundButtonTexture);
        IconBadge->SetTint(0.92f, 0.78f, 0.24f, 0.98f);
        IconBadge->SetPos(-184.f, -97.f);
        IconBadge->SetSize(40.f, 40.f);
        mIconBadge = IconBadge;
    }

    auto IconText =
        CreateWidget<CTextBlock>("EventWidget_IconText", 3).lock();
    if (IconText)
    {
        IconText->SetText(L"!");
        IconText->SetPos(-184.f, -97.f);
        IconText->SetSize(40.f, 40.f);
        IconText->SetFontSize(24.f);
        IconText->SetAlignH(ETextAlignH::Center);
        IconText->SetAlignV(ETextAlignV::Middle);
        IconText->SetTextColor(106, 72, 10, 255);
        IconText->EnableShadow(true);
        IconText->SetShadowTextColor(255, 241, 204, 200);
        IconText->SetShadowOffset(1.f, 1.f);
        mIconText = IconText;
    }

    auto TitleText =
        CreateWidget<CTextBlock>("EventWidget_TitleText", 3).lock();
    if (TitleText)
    {
        TitleText->SetPos(-132.f, -100.f);
        TitleText->SetSize(272.f, 40.f);
        TitleText->SetFontSize(22.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(96, 73, 32, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowTextColor(242, 235, 220, 220);
        TitleText->SetShadowOffset(1.f, 1.f);
        mTitleText = TitleText;
    }

    auto BodyPanel =
        CreateWidget<CImage>("EventWidget_BodyPanel", 1).lock();
    if (BodyPanel)
    {
        BodyPanel->SetTexture(
            "EventWidget_BodyPanelTexture",
            TropicoUiAssets::GDetailInfoPanelTexture);
        BodyPanel->SetTint(1.f, 1.f, 1.f, 0.92f);
        BodyPanel->SetPos(-180.f, -40.f);
        BodyPanel->SetSize(360.f, 118.f);
        mBodyPanel = BodyPanel;
    }

    auto BodyText =
        CreateWidget<CTextBlock>("EventWidget_BodyText", 3).lock();
    if (BodyText)
    {
        BodyText->SetPos(-168.f, -28.f);
        BodyText->SetSize(336.f, 78.f);
        BodyText->SetFontSize(16.f);
        BodyText->SetAlignH(ETextAlignH::Left);
        BodyText->SetAlignV(ETextAlignV::Top);
        BodyText->SetTextColor(74, 68, 57, 255);
        BodyText->EnableShadow(true);
        BodyText->SetShadowTextColor(255, 255, 255, 130);
        BodyText->SetShadowOffset(1.f, 1.f);
        mBodyText = BodyText;
    }

    auto AcceptConsequenceText =
        CreateWidget<CTextBlock>("EventWidget_AcceptConsequence", 3).lock();
    if (AcceptConsequenceText)
    {
        AcceptConsequenceText->SetPos(-168.f, 46.f);
        AcceptConsequenceText->SetSize(336.f, 18.f);
        AcceptConsequenceText->SetFontSize(13.f);
        AcceptConsequenceText->SetAlignH(ETextAlignH::Left);
        AcceptConsequenceText->SetAlignV(ETextAlignV::Top);
        AcceptConsequenceText->SetTextColor(44, 112, 60, 255);
        mAcceptConsequenceText = AcceptConsequenceText;
    }

    auto RejectConsequenceText =
        CreateWidget<CTextBlock>("EventWidget_RejectConsequence", 3).lock();
    if (RejectConsequenceText)
    {
        RejectConsequenceText->SetPos(-168.f, 62.f);
        RejectConsequenceText->SetSize(336.f, 18.f);
        RejectConsequenceText->SetFontSize(13.f);
        RejectConsequenceText->SetAlignH(ETextAlignH::Left);
        RejectConsequenceText->SetAlignV(ETextAlignV::Top);
        RejectConsequenceText->SetTextColor(148, 62, 48, 255);
        mRejectConsequenceText = RejectConsequenceText;
    }

    auto AcceptButton =
        CreateWidget<CButton>("EventWidget_AcceptButton", 2).lock();
    if (AcceptButton)
    {
        AcceptButton->SetPos(-130.f, 104.f);
        AcceptButton->SetSize(110.f, 40.f);
        ConfigureButtonTexture(AcceptButton, "EventWidgetAcceptButton");
        AcceptButton->SetEventCallback<CEventWidget>(
            EButtonEventState::Click,
            this,
            &CEventWidget::OnAcceptClick);
        mAcceptButton = AcceptButton;
    }

    auto RejectButton =
        CreateWidget<CButton>("EventWidget_RejectButton", 2).lock();
    if (RejectButton)
    {
        RejectButton->SetPos(20.f, 104.f);
        RejectButton->SetSize(110.f, 40.f);
        ConfigureButtonTexture(RejectButton, "EventWidgetRejectButton");
        RejectButton->SetEventCallback<CEventWidget>(
            EButtonEventState::Click,
            this,
            &CEventWidget::OnRejectClick);
        mRejectButton = RejectButton;
    }

    auto AcceptButtonText =
        CreateWidget<CTextBlock>("EventWidget_AcceptText", 3).lock();
    if (AcceptButtonText)
    {
        AcceptButtonText->SetPos(-130.f, 104.f);
        AcceptButtonText->SetSize(110.f, 40.f);
        ConfigureButtonLabel(AcceptButtonText, L"수락");
        mAcceptButtonText = AcceptButtonText;
    }

    auto RejectButtonText =
        CreateWidget<CTextBlock>("EventWidget_RejectText", 3).lock();
    if (RejectButtonText)
    {
        RejectButtonText->SetPos(20.f, 104.f);
        RejectButtonText->SetSize(110.f, 40.f);
        ConfigureButtonLabel(RejectButtonText, L"거부");
        mRejectButtonText = RejectButtonText;
    }

    RefreshFromState();
    return true;
}

void CEventWidget::Update(float DeltaTime)
{
    if (mState.Visible &&
        mState.AutoCloseSeconds > 0.f)
    {
        mState.AutoCloseSeconds =
            (std::max)(0.f, mState.AutoCloseSeconds - DeltaTime);

        if (mState.AutoCloseSeconds <= 0.f)
            mState.Visible = false;
    }

    CWidgetContainer::Update(DeltaTime);
    RefreshFromState();
}

bool CEventWidget::TryRedirectToTaskWidget()
{
    if (!mState.Visible ||
        mState.IssuerType == EPoliticalDemandIssuerType::None ||
        mState.IssuerIndex < 0)
    {
        return false;
    }

    auto World = mWorld.lock();

    if (!World)
        return false;

    auto UiManager = World->GetUIManager().lock();

    if (!UiManager)
        return false;

    if (auto TopHud = UiManager->FindWidget<CTopHudWidget>(GTopHudWidgetName).lock())
    {
        TopHud->OpenTaskWidgetForDemand(
            mState.IssuerType,
            mState.IssuerIndex);
        mState.Visible = false;
        SetPopupVisible(false);
        return true;
    }

    if (auto TaskWidget = UiManager->FindWidget<CTaskWidget>(GTaskWidgetName).lock())
    {
        TaskWidget->OpenForDemand(
            mState.IssuerType,
            mState.IssuerIndex);
        mState.Visible = false;
        SetPopupVisible(false);
        return true;
    }

    return false;
}

void CEventWidget::RefreshFromState()
{
    if (TryRedirectToTaskWidget())
        return;

    SetPopupVisible(mState.Visible);

    if (!mState.Visible)
        return;

    FResolution Resolution = CDevice::GetInst()->GetResolution();
    SetPos(
        static_cast<float>(Resolution.Width) * 0.5f,
        static_cast<float>(Resolution.Height) * 0.5f - 18.f);

    if (auto TitleText = mTitleText.lock())
        TitleText->SetText(mState.Title.c_str());

    if (auto BodyText = mBodyText.lock())
        BodyText->SetText(mState.Body.c_str());

    if (auto AcceptText = mAcceptConsequenceText.lock())
    {
        const bool Visible = !mState.AcceptConsequence.empty();
        AcceptText->SetText(mState.AcceptConsequence.c_str());
        AcceptText->SetEnable(Visible);
    }

    if (auto RejectText = mRejectConsequenceText.lock())
    {
        const bool Visible = !mState.RejectConsequence.empty();
        RejectText->SetText(mState.RejectConsequence.c_str());
        RejectText->SetEnable(Visible);
    }

    const bool ShowAcceptButton = mState.ShowAcceptButton;
    const bool ShowRejectButton = mState.ShowRejectButton;
    const float AcceptButtonLeftX =
        ShowAcceptButton && !ShowRejectButton ?
            GSingleButtonLeftX :
            GAcceptButtonLeftX;

    if (auto AcceptButton = mAcceptButton.lock())
    {
        AcceptButton->SetEnable(ShowAcceptButton);
        AcceptButton->SetPos(AcceptButtonLeftX, GButtonY);
        AcceptButton->ButtonEnable(ShowAcceptButton);
    }

    if (auto RejectButton = mRejectButton.lock())
    {
        RejectButton->SetEnable(ShowRejectButton);
        RejectButton->SetPos(GRejectButtonLeftX, GButtonY);
        RejectButton->ButtonEnable(ShowRejectButton);
    }

    if (auto AcceptButtonText = mAcceptButtonText.lock())
    {
        AcceptButtonText->SetEnable(ShowAcceptButton);
        AcceptButtonText->SetPos(AcceptButtonLeftX, GButtonY);
    }

    if (auto RejectButtonText = mRejectButtonText.lock())
    {
        RejectButtonText->SetEnable(ShowRejectButton);
        RejectButtonText->SetPos(GRejectButtonLeftX, GButtonY);
    }
}

void CEventWidget::SetPopupVisible(bool Visible)
{
    if (Visible)
        SetSize(GPanelWidth, GPanelHeight);
    else
        SetSize(0.f, 0.f);

    if (auto Widget = mPanelBackground.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mTitleRibbon.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mIconBadge.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mBodyPanel.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mIconText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mTitleText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mBodyText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mAcceptConsequenceText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mRejectConsequenceText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mAcceptButton.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mRejectButton.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mAcceptButtonText.lock())
        Widget->SetEnable(Visible);
    if (auto Widget = mRejectButtonText.lock())
        Widget->SetEnable(Visible);
}

void CEventWidget::OnAcceptClick()
{
    if (mState.IssuerType == EPoliticalDemandIssuerType::None ||
        mState.IssuerIndex < 0)
    {
        mState.Visible = false;
        mState.AutoCloseSeconds = 0.f;
        RefreshFromState();
        return;
    }

    auto CommandService = ResolveGovernmentCommandService(mWorld.lock());
    std::wstring Message;

    if (CommandService)
    {
        CommandService->RespondPoliticalDemand(
            mState.IssuerType,
            mState.IssuerIndex,
            true,
            Message);
    }

    mState.Visible = false;
    RefreshFromState();
}

void CEventWidget::OnRejectClick()
{
    if (mState.IssuerType == EPoliticalDemandIssuerType::None ||
        mState.IssuerIndex < 0)
    {
        mState.Visible = false;
        mState.AutoCloseSeconds = 0.f;
        RefreshFromState();
        return;
    }

    auto CommandService = ResolveGovernmentCommandService(mWorld.lock());
    std::wstring Message;

    if (CommandService)
    {
        CommandService->RespondPoliticalDemand(
            mState.IssuerType,
            mState.IssuerIndex,
            false,
            Message);
    }

    mState.Visible = false;
    RefreshFromState();
}
