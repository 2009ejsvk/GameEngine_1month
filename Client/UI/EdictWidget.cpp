#include "EdictWidget.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorld.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr const TCHAR* GEdictPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\3_ColdWar\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GEdictDetailFrameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\3_ColdWar\\CenterPopUp\\T_center_popUp_frameDescription.png");
    constexpr const TCHAR* GEdictButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");

    void ConfigureDefaultButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.20f, 0.22f, 0.26f, 0.95f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.28f, 0.31f, 0.36f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.14f, 0.16f, 0.20f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.10f, 0.12f, 0.70f));
    }

    void ConfigureSelectedButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.16f, 0.37f, 0.60f, 0.98f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.22f, 0.47f, 0.74f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.12f, 0.30f, 0.48f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.18f, 0.28f, 0.72f));
    }

    void ApplyTextureToAllButtonStates(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKey,
        const TCHAR* TextureFile)
    {
        if (!Button || !TextureFile)
            return;

        if (!Button->SetTexture(EButtonState::Normal, TextureKey, TextureFile))
            return;

        Button->SetTexture(EButtonState::Hovered, TextureKey);
        Button->SetTexture(EButtonState::Click, TextureKey);
        Button->SetTexture(EButtonState::Disable, TextureKey);
    }

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = false;
        unsigned long long AbsValue = 0;

        if (Value < 0)
        {
            Negative = true;
            AbsValue = static_cast<unsigned long long>(-Value);
        }
        else
        {
            AbsValue = static_cast<unsigned long long>(Value);
        }

        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatDaysAsYearsMonths(int Days)
    {
        const int SafeDays = (std::max)(0, Days);
        const int Years = SafeDays / 365;
        const int Months = (SafeDays % 365) / 30;

        if (Years <= 0 && Months <= 0)
            return L"-";

        if (Years > 0 && Months > 0)
            return std::to_wstring(Years) + L"년 " +
                std::to_wstring(Months) + L"개월";

        if (Years > 0)
            return std::to_wstring(Years) + L"년";

        return std::to_wstring(Months) + L"개월";
    }

    std::wstring BuildStateText(
        const FGovernmentEdictState* State,
        const FGovernmentEdictDefinition& Definition)
    {
        if (!State)
            return L"준비";

        if (State->Active)
        {
            if (Definition.Mode == EGovernmentEdictMode::Passive)
                return L"활성";

            return L"활성 " + FormatDaysAsYearsMonths(State->RemainingDays);
        }

        if (State->CooldownDays > 0)
            return L"대기 " + FormatDaysAsYearsMonths(State->CooldownDays);

        return L"준비";
    }
}

CEdictWidget::CEdictWidget()
{
}

CEdictWidget::~CEdictWidget()
{
}

bool CEdictWidget::Init()
{
    CWidgetContainer::Init();

    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    mEdictTypes.reserve(Definitions.size());

    for (size_t i = 0; i < Definitions.size(); ++i)
        mEdictTypes.push_back(Definitions[i].Type);

    if (!mEdictTypes.empty())
        mSelectedEdict = mEdictTypes[0];

    auto Background = CreateWidget<CImage>("Edict_Background", 20).lock();

    if (Background)
    {
        Background->SetTexture("EdictPanelTexture", GEdictPanelTexture);
        Background->SetTint(1.f, 1.f, 1.f, 1.f);
        mBackground = Background;
    }

    auto DetailFrame = CreateWidget<CImage>("Edict_DetailFrame", 21).lock();

    if (DetailFrame)
    {
        DetailFrame->SetTexture(
            "EdictDetailFrameTexture",
            GEdictDetailFrameTexture);
        DetailFrame->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailFrame = DetailFrame;
    }

    auto TitleText = CreateWidget<CTextBlock>("Edict_TitleText", 22).lock();

    if (TitleText)
    {
        TitleText->SetText(TEXT("칙령"));
        TitleText->SetFontSize(24.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(245, 242, 232, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(20, 20, 20, 220);
        mTitleText = TitleText;
    }

    auto SubtitleText =
        CreateWidget<CTextBlock>("Edict_SubtitleText", 22).lock();

    if (SubtitleText)
    {
        SubtitleText->SetFontSize(14.f);
        SubtitleText->SetAlignH(ETextAlignH::Left);
        SubtitleText->SetAlignV(ETextAlignV::Middle);
        SubtitleText->SetTextColor(220, 220, 220, 255);
        SubtitleText->EnableShadow(true);
        SubtitleText->SetShadowOffset(1.f, 1.f);
        SubtitleText->SetShadowTextColor(20, 20, 20, 220);
        mSubtitleText = SubtitleText;
    }

    auto DetailIcon = CreateWidget<CImage>("Edict_DetailIcon", 23).lock();

    if (DetailIcon)
    {
        DetailIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailIcon = DetailIcon;
    }

    auto DetailTitle =
        CreateWidget<CTextBlock>("Edict_DetailTitle", 23).lock();

    if (DetailTitle)
    {
        DetailTitle->SetFontSize(22.f);
        DetailTitle->SetAlignH(ETextAlignH::Left);
        DetailTitle->SetAlignV(ETextAlignV::Middle);
        DetailTitle->SetTextColor(245, 245, 245, 255);
        DetailTitle->EnableShadow(true);
        DetailTitle->SetShadowOffset(1.f, 1.f);
        DetailTitle->SetShadowTextColor(16, 16, 16, 220);
        mDetailTitleText = DetailTitle;
    }

    auto DetailBody = CreateWidget<CTextBlock>("Edict_DetailBody", 23).lock();

    if (DetailBody)
    {
        DetailBody->SetFontSize(14.f);
        DetailBody->SetAlignH(ETextAlignH::Left);
        DetailBody->SetAlignV(ETextAlignV::Top);
        DetailBody->SetTextColor(226, 226, 226, 255);
        DetailBody->EnableShadow(true);
        DetailBody->SetShadowOffset(1.f, 1.f);
        DetailBody->SetShadowTextColor(16, 16, 16, 220);
        mDetailBodyText = DetailBody;
    }

    auto FeedbackText =
        CreateWidget<CTextBlock>("Edict_FeedbackText", 23).lock();

    if (FeedbackText)
    {
        FeedbackText->SetFontSize(14.f);
        FeedbackText->SetAlignH(ETextAlignH::Left);
        FeedbackText->SetAlignV(ETextAlignV::Middle);
        FeedbackText->SetTextColor(238, 220, 160, 255);
        FeedbackText->EnableShadow(true);
        FeedbackText->SetShadowOffset(1.f, 1.f);
        FeedbackText->SetShadowTextColor(16, 16, 16, 220);
        mFeedbackText = FeedbackText;
    }

    auto ApplyButton = CreateWidget<CButton>("Edict_ApplyButton", 24).lock();

    if (ApplyButton)
    {
        ConfigureSelectedButtonStyle(ApplyButton);
        ApplyButton->SetEventCallback<CEdictWidget>(
            EButtonEventState::Click, this,
            &CEdictWidget::OnApplyButtonClick);
        ApplyTextureToAllButtonStates(
            ApplyButton,
            "EdictApplyButtonTexture",
            GEdictButtonTexture);

        auto ApplyText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "Edict_ApplyButtonText", mWorld);

        if (ApplyText)
        {
            ApplyText->SetText(TEXT("시행"));
            ApplyText->SetFontSize(18.f);
            ApplyText->SetAlignH(ETextAlignH::Center);
            ApplyText->SetAlignV(ETextAlignV::Middle);
            ApplyText->SetTextColor(255, 255, 255, 255);
            ApplyText->EnableShadow(true);
            ApplyText->SetShadowOffset(1.f, 1.f);
            ApplyText->SetShadowTextColor(20, 20, 20, 220);
            ApplyButton->SetChild(ApplyText);
            mApplyButtonText = ApplyText;
        }

        mApplyButton = ApplyButton;
    }

    mEdictButtons.resize(mEdictTypes.size());
    mEdictIcons.resize(mEdictTypes.size());
    mEdictLabels.resize(mEdictTypes.size());
    mEdictStatuses.resize(mEdictTypes.size());

    for (size_t i = 0; i < mEdictTypes.size(); ++i)
    {
        auto Button = CreateWidget<CButton>(
            "Edict_ListButton_" + std::to_string(i), 24).lock();

        if (!Button)
            continue;

        ConfigureDefaultButtonStyle(Button);
        ApplyTextureToAllButtonStates(
            Button,
            "EdictListButtonTexture_" + std::to_string(i),
            GEdictButtonTexture);

        switch (i)
        {
        case 0:
            Button->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this,
                &CEdictWidget::OnEdictButton0Click);
            break;
        case 1:
            Button->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this,
                &CEdictWidget::OnEdictButton1Click);
            break;
        case 2:
            Button->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this,
                &CEdictWidget::OnEdictButton2Click);
            break;
        case 3:
            Button->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this,
                &CEdictWidget::OnEdictButton3Click);
            break;
        case 4:
            Button->SetEventCallback<CEdictWidget>(
                EButtonEventState::Click, this,
                &CEdictWidget::OnEdictButton4Click);
            break;
        default:
            break;
        }

        auto Content =
            CWidget::CreateStaticWidget<CWidgetContainer>(
                "Edict_ListButtonContent_" + std::to_string(i),
                mWorld);

        if (Content)
        {
            Button->SetChild(Content);

            auto Icon = CWidget::CreateStaticWidget<CImage>(
                "Edict_ListIcon_" + std::to_string(i),
                mWorld, 1);

            if (Icon)
            {
                Icon->SetTint(1.f, 1.f, 1.f, 1.f);
                Content->AddWidget(Icon);
                mEdictIcons[i] = Icon;
            }

            auto Label = CWidget::CreateStaticWidget<CTextBlock>(
                "Edict_ListLabel_" + std::to_string(i),
                mWorld, 2);

            if (Label)
            {
                Label->SetFontSize(16.f);
                Label->SetAlignH(ETextAlignH::Left);
                Label->SetAlignV(ETextAlignV::Middle);
                Label->SetTextColor(245, 245, 245, 255);
                Label->EnableShadow(true);
                Label->SetShadowOffset(1.f, 1.f);
                Label->SetShadowTextColor(16, 16, 16, 220);
                Content->AddWidget(Label);
                mEdictLabels[i] = Label;
            }

            auto Status = CWidget::CreateStaticWidget<CTextBlock>(
                "Edict_ListStatus_" + std::to_string(i),
                mWorld, 2);

            if (Status)
            {
                Status->SetFontSize(13.f);
                Status->SetAlignH(ETextAlignH::Left);
                Status->SetAlignV(ETextAlignV::Middle);
                Status->SetTextColor(220, 210, 160, 255);
                Status->EnableShadow(true);
                Status->SetShadowOffset(1.f, 1.f);
                Status->SetShadowTextColor(16, 16, 16, 220);
                Content->AddWidget(Status);
                mEdictStatuses[i] = Status;
            }
        }

        mEdictButtons[i] = Button;
    }

    ApplyOpenState();
    RefreshData();
    RefreshLayout();
    return true;
}

void CEdictWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    RefreshData();
    RefreshLayout();
}

void CEdictWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CEdictWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;
    ApplyOpenState();
}

void CEdictWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float Scale =
        (std::max)(0.72f, (std::min)(1.f, ScreenWidth / 1920.f));
    const float PanelWidth = 700.f * Scale;
    const float PanelHeight = 500.f * Scale;
    const float PanelX = ScreenWidth - PanelWidth - 22.f;
    const float PanelY = 78.f;
    const float Margin = 22.f * Scale;
    const float HeaderHeight = 34.f * Scale;
    const float ListWidth = 248.f * Scale;
    const float ButtonHeight = 72.f * Scale;
    const float ButtonGap = 10.f * Scale;
    const float DetailX = PanelX + Margin + ListWidth + 16.f * Scale;
    const float DetailY = PanelY + 78.f * Scale;
    const float DetailWidth = PanelWidth - (DetailX - PanelX) - Margin;
    const float DetailHeight = PanelHeight - 176.f * Scale;

    auto Background = mBackground.lock();
    auto DetailFrame = mDetailFrame.lock();
    auto TitleText = mTitleText.lock();
    auto SubtitleText = mSubtitleText.lock();
    auto DetailIcon = mDetailIcon.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto ApplyButton = mApplyButton.lock();

    if (Background)
    {
        Background->SetPos(PanelX, PanelY);
        Background->SetSize(PanelWidth, PanelHeight);
    }

    if (TitleText)
    {
        TitleText->SetPos(PanelX + Margin, PanelY + 18.f * Scale);
        TitleText->SetSize(260.f * Scale, HeaderHeight);
    }

    if (SubtitleText)
    {
        SubtitleText->SetPos(PanelX + Margin, PanelY + 48.f * Scale);
        SubtitleText->SetSize(PanelWidth - Margin * 2.f, 24.f * Scale);
    }

    for (size_t i = 0; i < mEdictButtons.size(); ++i)
    {
        auto Button = mEdictButtons[i].lock();

        if (!Button)
            continue;

        const float ButtonX = PanelX + Margin;
        const float ButtonY =
            PanelY + 82.f * Scale +
            static_cast<float>(i) * (ButtonHeight + ButtonGap);

        Button->SetPos(ButtonX, ButtonY);
        Button->SetSize(ListWidth, ButtonHeight);

        auto Icon = mEdictIcons[i].lock();
        auto Label = mEdictLabels[i].lock();
        auto Status = mEdictStatuses[i].lock();

        if (Icon)
        {
            Icon->SetPos(12.f * Scale, 10.f * Scale);
            Icon->SetSize(52.f * Scale, 52.f * Scale);
        }

        if (Label)
        {
            Label->SetPos(74.f * Scale, 10.f * Scale);
            Label->SetSize(ListWidth - 84.f * Scale, 28.f * Scale);
        }

        if (Status)
        {
            Status->SetPos(74.f * Scale, 38.f * Scale);
            Status->SetSize(ListWidth - 84.f * Scale, 22.f * Scale);
        }
    }

    if (DetailFrame)
    {
        DetailFrame->SetPos(DetailX, DetailY);
        DetailFrame->SetSize(DetailWidth, DetailHeight);
    }

    if (DetailIcon)
    {
        DetailIcon->SetPos(DetailX + 20.f * Scale, DetailY + 18.f * Scale);
        DetailIcon->SetSize(72.f * Scale, 72.f * Scale);
    }

    if (DetailTitleText)
    {
        DetailTitleText->SetPos(
            DetailX + 104.f * Scale,
            DetailY + 18.f * Scale);
        DetailTitleText->SetSize(
            DetailWidth - 124.f * Scale,
            32.f * Scale);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetPos(
            DetailX + 20.f * Scale,
            DetailY + 102.f * Scale);
        DetailBodyText->SetSize(
            DetailWidth - 40.f * Scale,
            DetailHeight - 168.f * Scale);
    }

    if (FeedbackText)
    {
        FeedbackText->SetPos(
            DetailX,
            PanelY + PanelHeight - 80.f * Scale);
        FeedbackText->SetSize(
            DetailWidth - 166.f * Scale,
            24.f * Scale);
    }

    if (ApplyButton)
    {
        ApplyButton->SetPos(
            DetailX + DetailWidth - 150.f * Scale,
            PanelY + PanelHeight - 90.f * Scale);
        ApplyButton->SetSize(150.f * Scale, 42.f * Scale);
    }
}

void CEdictWidget::RefreshData()
{
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
        return;

    const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
    int ActiveCount = 0;

    for (size_t i = 0; i < MainWorld->GetGovernmentEdictStates().size(); ++i)
    {
        if (MainWorld->GetGovernmentEdictStates()[i].Active)
            ++ActiveCount;
    }

    auto SubtitleText = mSubtitleText.lock();

    if (SubtitleText)
    {
        const std::wstring Subtitle =
            L"활성 칙령 " + std::to_wstring(ActiveCount) +
            L"개 | 일일 재정 영향 " +
            FormatCurrency(-MainWorld->GetLastDailyEdictCost());
        SubtitleText->SetText(Subtitle.c_str());
    }

    const FGovernmentEdictDefinition* SelectedDefinition = nullptr;

    for (size_t i = 0; i < Definitions.size(); ++i)
    {
        if (Definitions[i].Type == mSelectedEdict)
        {
            SelectedDefinition = &Definitions[i];
            break;
        }
    }

    if (!SelectedDefinition && !Definitions.empty())
        SelectedDefinition = &Definitions[0];

    for (size_t i = 0; i < mEdictTypes.size(); ++i)
    {
        const FGovernmentEdictDefinition* Definition =
            EdictSystem::FindGovernmentEdictDefinition(mEdictTypes[i]);
        const FGovernmentEdictState* State =
            MainWorld->GetGovernmentEdictState(mEdictTypes[i]);
        auto Button = mEdictButtons[i].lock();
        auto Icon = mEdictIcons[i].lock();
        auto Label = mEdictLabels[i].lock();
        auto Status = mEdictStatuses[i].lock();

        if (Button)
        {
            if (mEdictTypes[i] == mSelectedEdict)
                ConfigureSelectedButtonStyle(Button);
            else
                ConfigureDefaultButtonStyle(Button);
        }

        if (!Definition)
            continue;

        if (Icon && Definition->IconPath)
        {
            Icon->SetTexture(
                "EdictListIconTexture_" + std::to_string(i),
                Definition->IconPath);
        }

        if (Label)
            Label->SetText(Definition->DisplayName.c_str());

        if (Status)
            Status->SetText(
                BuildStateText(State, *Definition).c_str());
    }

    if (!SelectedDefinition)
        return;

    const FGovernmentEdictState* SelectedState =
        MainWorld->GetGovernmentEdictState(SelectedDefinition->Type);
    const int ActiveCitizenCount =
        MainWorld->GetPoliticalSnapshot().ActiveCitizenCount;
    const long long ActivationCost =
        EdictSystem::ResolveEdictActivationCost(
            *SelectedDefinition,
            ActiveCitizenCount);

    auto DetailIcon = mDetailIcon.lock();
    auto DetailTitle = mDetailTitleText.lock();
    auto DetailBody = mDetailBodyText.lock();
    auto ApplyButtonText = mApplyButtonText.lock();
    auto FeedbackText = mFeedbackText.lock();

    if (DetailIcon && SelectedDefinition->IconPath)
    {
        DetailIcon->SetTexture(
            "EdictDetailIconTexture_" +
                std::to_string(static_cast<int>(SelectedDefinition->Type)),
            SelectedDefinition->IconPath);
    }

    if (DetailTitle)
        DetailTitle->SetText(SelectedDefinition->DisplayName.c_str());

    if (DetailBody)
    {
        std::wstring Body;
        Body += L"유형: ";
        Body += SelectedDefinition->Mode == EGovernmentEdictMode::Passive ?
            L"패시브" : L"액티브";
        Body += L"\n";
        Body += L"상태: ";
        Body += BuildStateText(SelectedState, *SelectedDefinition);
        Body += L"\n";
        Body += L"시행 비용: ";
        Body += FormatCurrency(ActivationCost);
        Body += L"\n";
        Body += L"월간 유지비: ";
        Body += FormatCurrency(SelectedDefinition->MonthlyUpkeep);
        Body += L"\n";
        Body += L"지속 시간: ";
        Body += SelectedDefinition->Mode == EGovernmentEdictMode::Active ?
            FormatDaysAsYearsMonths(SelectedDefinition->DurationDays) :
            L"해제 전까지";
        Body += L"\n";
        Body += L"재사용 대기: ";
        Body += SelectedDefinition->CooldownDays > 0 ?
            FormatDaysAsYearsMonths(SelectedDefinition->CooldownDays) :
            L"-";
        Body += L"\n\n";
        Body += SelectedDefinition->Summary;
        Body += L"\n";
        Body += SelectedDefinition->EffectText;
        Body += L"\n\n정치 신호:";

        for (size_t SignalIndex = 0;
            SignalIndex < SelectedDefinition->Signals.size();
            ++SignalIndex)
        {
            const FPoliticalSignalDef& Signal =
                SelectedDefinition->Signals[SignalIndex];
            Body += L"\n- ";
            Body += GetPoliticalFactionDisplayName(
                Signal.Axis,
                Signal.FavoredStance);
            Body += L" ";
            Body += Signal.Strength >= 0.f ? L"+" : L"";
            Body += std::to_wstring(static_cast<int>(
                std::round(Signal.Strength)));
        }

        DetailBody->SetText(Body.c_str());
    }

    if (ApplyButtonText)
    {
        if (SelectedDefinition->Mode == EGovernmentEdictMode::Passive &&
            SelectedState &&
            SelectedState->Active)
        {
            ApplyButtonText->SetText(TEXT("해제"));
        }
        else if (SelectedState && SelectedState->CooldownDays > 0)
        {
            ApplyButtonText->SetText(TEXT("대기중"));
        }
        else
        {
            ApplyButtonText->SetText(TEXT("시행"));
        }
    }

    if (FeedbackText)
        FeedbackText->SetText(mFeedbackMessage.c_str());
}

void CEdictWidget::ApplyOpenState()
{
    auto Background = mBackground.lock();
    auto DetailFrame = mDetailFrame.lock();
    auto DetailIcon = mDetailIcon.lock();
    auto TitleText = mTitleText.lock();
    auto SubtitleText = mSubtitleText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto FeedbackText = mFeedbackText.lock();
    auto ApplyButton = mApplyButton.lock();

    if (Background)
        Background->SetEnable(mOpen);
    if (DetailFrame)
        DetailFrame->SetEnable(mOpen);
    if (DetailIcon)
        DetailIcon->SetEnable(mOpen);
    if (TitleText)
        TitleText->SetEnable(mOpen);
    if (SubtitleText)
        SubtitleText->SetEnable(mOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(mOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(mOpen);
    if (FeedbackText)
        FeedbackText->SetEnable(mOpen);
    if (ApplyButton)
        ApplyButton->SetEnable(mOpen);

    for (size_t i = 0; i < mEdictButtons.size(); ++i)
    {
        auto Button = mEdictButtons[i].lock();

        if (Button)
            Button->SetEnable(mOpen);
    }
}

void CEdictWidget::SelectEdictByIndex(int Index)
{
    if (Index < 0 ||
        Index >= static_cast<int>(mEdictTypes.size()))
    {
        return;
    }

    mSelectedEdict = mEdictTypes[Index];
    mFeedbackMessage.clear();
}

void CEdictWidget::OnApplyButtonClick()
{
    auto World = mWorld.lock();
    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
        return;

    std::wstring Message;
    MainWorld->TryApplyEdict(mSelectedEdict, Message);
    mFeedbackMessage = Message;
}

void CEdictWidget::OnEdictButton0Click()
{
    SelectEdictByIndex(0);
}

void CEdictWidget::OnEdictButton1Click()
{
    SelectEdictByIndex(1);
}

void CEdictWidget::OnEdictButton2Click()
{
    SelectEdictByIndex(2);
}

void CEdictWidget::OnEdictButton3Click()
{
    SelectEdictByIndex(3);
}

void CEdictWidget::OnEdictButton4Click()
{
    SelectEdictByIndex(4);
}
