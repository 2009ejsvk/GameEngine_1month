#include "EventWidget.h"
#include "TaskWidget.h"
#include "TopHudWidget.h"
#include "TropicoUiAssetCatalog.h"
#include "../RuntimeConfigRegistry.h"
#include "../World/GovernmentCommandService.h"
#include "../World/IWorldUIAccess.h"
#include "../ObjectNames.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Asset/AssetManager.h"
#include "Asset/Font/Font.h"
#include "Asset/Font/FontManager.h"
#include "Device.h"
#include <Windows.h>
#include "World/World.h"
#include "World/WorldUIManager.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    constexpr float GPanelWidth = 400.f;
    constexpr float GPanelHeight = 220.f;
    constexpr float GAcceptButtonLeftX = -130.f;
    constexpr float GRejectButtonLeftX = 20.f;
    constexpr float GSingleButtonLeftX = -55.f;
    constexpr float GButtonY = 104.f;
    constexpr float GCornerCloseButtonX = 156.f;
    constexpr float GCornerCloseButtonY = -102.f;
    constexpr float GCornerCloseButtonSize = 34.f;
    constexpr const wchar_t* GPopupConfigFileName = L"PopupConfig.ini";

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

    std::wstring BuildPopupConfigPath()
    {
        return RuntimeConfigRegistry::BuildExeRelativePath(GPopupConfigFileName);
    }

    void DisablePopupConfigSection(const std::wstring& SectionName)
    {
        if (SectionName.empty())
            return;

        WritePrivateProfileStringW(
            SectionName.c_str(),
            L"Enabled",
            L"0",
            BuildPopupConfigPath().c_str());
    }

    float MeasureEngineDefaultTextWidth(
        const std::wstring& Text,
        float FontSize)
    {
        if (Text.empty())
            return 0.f;

        auto FontManager = CAssetManager::GetInst()->GetFontManager().lock();

        if (!FontManager)
            return static_cast<float>(Text.length()) * FontSize * 0.6f;

        auto Font = FontManager->FindFont("EngineDefault").lock();

        if (!Font)
            return static_cast<float>(Text.length()) * FontSize * 0.6f;

        IDWriteTextLayout* Layout =
            Font->CreateLayout(
                Text.c_str(),
                static_cast<int>(Text.length()),
                4096.f,
                512.f);

        if (!Layout)
            return static_cast<float>(Text.length()) * FontSize * 0.6f;

        DWRITE_TEXT_RANGE Range = {};
        Range.startPosition = 0;
        Range.length = static_cast<UINT32>(Text.length());
        Layout->SetFontSize(FontSize, Range);

        DWRITE_TEXT_METRICS Metrics = {};
        float Width = static_cast<float>(Text.length()) * FontSize * 0.6f;

        if (SUCCEEDED(Layout->GetMetrics(&Metrics)))
            Width = Metrics.widthIncludingTrailingWhitespace;

        SAFE_RELEASE(Layout);
        return Width;
    }

    std::wstring BuildIndentSpaces(float IndentWidth, float FontSize)
    {
        if (IndentWidth <= 0.f)
            return std::wstring();

        const float SpaceWidth =
            MeasureEngineDefaultTextWidth(L" ", FontSize);

        if (SpaceWidth <= 0.01f)
        {
            const float FallbackSpaceCount = static_cast<float>(
                std::floor(IndentWidth / (FontSize * 0.35f)));
            const size_t SpaceCount = static_cast<size_t>(
                (std::max)(1.f, FallbackSpaceCount));
            return std::wstring(SpaceCount, L' ');
        }

        const float MeasuredSpaceCount = static_cast<float>(
            std::floor(IndentWidth / SpaceWidth));
        const size_t SpaceCount = static_cast<size_t>(
            (std::max)(0.f, MeasuredSpaceCount));

        return std::wstring(SpaceCount, L' ');
    }

    std::wstring BuildWrappedFormulaBody(
        const std::vector<std::wstring>& Terms,
        float MaxWidth,
        float FontSize)
    {
        if (Terms.empty())
            return std::wstring();

        if (Terms.size() == 1 || MaxWidth <= 0.f || FontSize <= 0.f)
        {
            std::wstring SingleLine;

            for (size_t Index = 0; Index < Terms.size(); ++Index)
            {
                if (Index > 0)
                    SingleLine += L" ";

                SingleLine += Terms[Index];
            }

            return SingleLine;
        }

        std::vector<std::wstring> Lines;
        Lines.reserve(Terms.size());

        std::wstring CurrentLine = Terms.front();
        const float ContinuationIndentWidth =
            MeasureEngineDefaultTextWidth(Terms.front() + L" ", FontSize);
        const std::wstring ContinuationIndent =
            BuildIndentSpaces(ContinuationIndentWidth, FontSize);

        for (size_t Index = 1; Index < Terms.size(); ++Index)
        {
            const std::wstring Candidate = CurrentLine + L" " + Terms[Index];

            if (MeasureEngineDefaultTextWidth(Candidate, FontSize) <= MaxWidth)
            {
                CurrentLine = Candidate;
                continue;
            }

            Lines.push_back(CurrentLine);

            std::wstring ContinuedLine =
                ContinuationIndent + Terms[Index];

            if (MeasureEngineDefaultTextWidth(
                    ContinuedLine,
                    FontSize) > MaxWidth)
            {
                ContinuedLine = Terms[Index];
            }

            CurrentLine = ContinuedLine;
        }

        Lines.push_back(CurrentLine);

        std::wstring Result;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            if (Index > 0)
                Result += L"\n";

            Result += Lines[Index];
        }

        return Result;
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

    auto CornerCloseButton =
        CreateWidget<CButton>("EventWidget_CloseButton", 4).lock();
    if (CornerCloseButton)
    {
        CornerCloseButton->SetPos(
            GCornerCloseButtonX,
            GCornerCloseButtonY);
        CornerCloseButton->SetSize(
            GCornerCloseButtonSize,
            GCornerCloseButtonSize);
        CornerCloseButton->SetTexture(
            EButtonState::Normal,
            "EventWidgetCloseButton_Normal",
            TropicoUiAssets::GRoundButtonTexture);
        CornerCloseButton->SetTexture(
            EButtonState::Hovered,
            "EventWidgetCloseButton_Hovered",
            TropicoUiAssets::GRoundButtonHoverTexture);
        CornerCloseButton->SetTexture(
            EButtonState::Click,
            "EventWidgetCloseButton_Click",
            TropicoUiAssets::GRoundButtonSelectedTexture);
        CornerCloseButton->SetTexture(
            EButtonState::Disable,
            "EventWidgetCloseButton_Disable",
            TropicoUiAssets::GRoundButtonTexture);
        CornerCloseButton->SetTint(EButtonState::Normal, 1.f, 1.f, 1.f, 0.96f);
        CornerCloseButton->SetTint(EButtonState::Hovered, 1.f, 1.f, 1.f, 1.f);
        CornerCloseButton->SetTint(EButtonState::Click, 0.94f, 0.91f, 0.84f, 1.f);
        CornerCloseButton->SetTint(EButtonState::Disable, 1.f, 1.f, 1.f, 0.55f);
        CornerCloseButton->SetEventCallback<CEventWidget>(
            EButtonEventState::Click,
            this,
            &CEventWidget::OnCornerCloseClick);
        CornerCloseButton->SetEnable(false);

        auto CornerCloseText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "EventWidget_CloseText",
                mWorld);

        if (CornerCloseText)
        {
            CornerCloseText->SetText(TEXT("X"));
            CornerCloseText->SetFontSize(20.f);
            CornerCloseText->SetAlignH(ETextAlignH::Center);
            CornerCloseText->SetAlignV(ETextAlignV::Middle);
            CornerCloseText->SetTextColor(92, 60, 12, 255);
            CornerCloseText->EnableShadow(true);
            CornerCloseText->SetShadowOffset(1.f, 1.f);
            CornerCloseText->SetShadowTextColor(255, 239, 196, 170);
            CornerCloseButton->SetChild(CornerCloseText);
        }

        mCornerCloseButton = CornerCloseButton;
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
    {
        std::wstring DisplayBody = mState.Body;

        if (mState.UseBodyFormulaTermWrap &&
            !mState.BodyFormulaTerms.empty())
        {
            DisplayBody = BuildWrappedFormulaBody(
                mState.BodyFormulaTerms,
                BodyText->GetSize().x,
                BodyText->GetFontSize());
        }

        BodyText->SetText(DisplayBody.c_str());
    }

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
        AcceptButtonText->SetText(L"수락");
        AcceptButtonText->SetEnable(ShowAcceptButton);
        AcceptButtonText->SetPos(AcceptButtonLeftX, GButtonY);
    }

    if (auto RejectButtonText = mRejectButtonText.lock())
    {
        RejectButtonText->SetText(L"거부");
        RejectButtonText->SetEnable(ShowRejectButton);
        RejectButtonText->SetPos(GRejectButtonLeftX, GButtonY);
    }

    if (auto CornerCloseButton = mCornerCloseButton.lock())
    {
        CornerCloseButton->SetEnable(mState.ShowCornerCloseButton);
        CornerCloseButton->ButtonEnable(mState.ShowCornerCloseButton);
    }
}

void CEventWidget::SetPopupVisible(bool Visible)
{
    if (Visible != mPopupWasVisible)
    {
        auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

        if (Visible)
        {
            if (Access && !Access->Read().IsSimulationPaused())
            {
                Access->Commands().ToggleSimulationPaused();
                mAutoPaused = true;
            }
        }
        else if (mAutoPaused)
        {
            if (Access && Access->Read().IsSimulationPaused())
                Access->Commands().ToggleSimulationPaused();
            mAutoPaused = false;
        }

        mPopupWasVisible = Visible;
    }

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
    if (auto Widget = mCornerCloseButton.lock())
        Widget->SetEnable(Visible && mState.ShowCornerCloseButton);
}

void CEventWidget::OnCornerCloseClick()
{
    mState.Visible = false;
    mState.AutoCloseSeconds = 0.f;
    RefreshFromState();
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
