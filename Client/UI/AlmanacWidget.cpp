#include "AlmanacWidget.h"
#include "AlmanacDataProvider.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorldAccess.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <map>
#include <string>
#include <vector>

namespace
{
    constexpr float GDataRefreshIntervalSeconds = 0.5f;
    constexpr int GBuildingCategoryCount = 8;
    constexpr int GOverviewCardCount = 6;
    constexpr int GSatisfactionRowCount = 9;
    constexpr int GSatisfactionDetailCount = 6;
    constexpr int GPopulationDetailCount = 8;
    constexpr int GPopulationMetricCount = 4;
    constexpr int GEconomyDetailCount = 9;
    constexpr int GEconomyMetricCount = 9;
    constexpr int GResourceRowCount = 8;
    constexpr int GResourceDetailCount = 6;
    constexpr int GPoliticsRowCount = 13;
    constexpr int GPoliticsDetailCount = 13;
    constexpr int GForeignDetailCount = 6;
    constexpr int GForeignMetricCount = 4;
    constexpr int GBuildingRowCount = GBuildingCategoryCount;
    constexpr int GBuildingDetailCount = 8;
    constexpr int GConflictDetailCount = 8;
    constexpr int GConflictMetricCount = 3;

    constexpr const TCHAR* GPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GTabTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GRowTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");
    constexpr const TCHAR* GMoneyIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GPopulationIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_population.png");
    constexpr const TCHAR* GApprovalIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png");

    constexpr const TCHAR* GPageTabIcons[static_cast<size_t>(EAlmanacPage::Count)] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Tasks.png"),
        GApprovalIcon,
        GPopulationIcon,
        GMoneyIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Trade.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Construction.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png")
    };

    constexpr const wchar_t* GPageTitles[static_cast<size_t>(EAlmanacPage::Count)] =
    {
        L"개요",
        L"만족도",
        L"국민",
        L"경제",
        L"자원 개요",
        L"정치",
        L"대외관계",
        L"건물 목록",
        L"분쟁"
    };

    constexpr const wchar_t* GBuildingCategoryLabels[GBuildingCategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"오락",
        L"미디어 및 교육",
        L"관광업",
        L"공익 서비스"
    };

    constexpr const wchar_t* GSatisfactionLabels[GSatisfactionRowCount] =
    {
        L"종합 만족도",
        L"음식",
        L"보건",
        L"유흥",
        L"신앙",
        L"주거",
        L"직업",
        L"자유",
        L"치안"
    };

    float Clamp01(float Value)
    {
        return (std::max)(0.f, (std::min)(1.f, Value));
    }

    double Clamp01(double Value)
    {
        return (std::max)(0.0, (std::min)(1.0, Value));
    }

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatPercent(double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.0f%%", Value);
        return Buffer;
    }

    std::wstring FormatFixed1(double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.1f", Value);
        return Buffer;
    }

    std::wstring FormatSignedFixed1(double Value)
    {
        if (Value > 0.0)
            return L"+" + FormatFixed1(Value);

        return FormatFixed1(Value);
    }

    std::wstring FormatSignedPercentUnit(double Value)
    {
        const double PercentValue = Value * 100.0;

        if (std::fabs(PercentValue) < 0.5)
            return L"0%";

        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%+.0f%%", PercentValue);
        return Buffer;
    }

    std::wstring FormatDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d.%02d.%02d", Year, Month, Day);
        return Buffer;
    }

    std::wstring FormatCountWithPercent(int Count, double Ratio)
    {
        return std::to_wstring(Count) +
            L"명 (" + FormatPercent(Ratio * 100.0) + L")";
    }

    std::wstring FormatTaxPolicySummary(const FTaxPolicy& TaxPolicy)
    {
        return
            L"소비 " + std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L"% / 소득 " + std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L"% / 재산 " + std::to_wstring(TaxPolicy.PropertyRatePercent) +
            L"%";
    }

    std::wstring FormatTaxPolicyCompact(const FTaxPolicy& TaxPolicy)
    {
        return
            std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L" / " +
            std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L" / " +
            std::to_wstring(TaxPolicy.PropertyRatePercent);
    }

    const wchar_t* GetElectionWarningTierLabel(double Score)
    {
        if (Score >= 0.78)
            return L"재선 위험 높음";
        if (Score >= 0.52)
            return L"재선 주의";
        if (Score >= 0.32)
            return L"선거 점검";
        return L"안정";
    }

    FVector4 ResolveElectionWarningTint(double Score)
    {
        if (Score >= 0.78)
            return FVector4(0.82f, 0.24f, 0.18f, 1.f);
        if (Score >= 0.52)
            return FVector4(0.84f, 0.48f, 0.12f, 1.f);
        if (Score >= 0.32)
            return FVector4(0.78f, 0.68f, 0.18f, 1.f);
        return FVector4(0.20f, 0.56f, 0.20f, 1.f);
    }

    std::wstring BuildElectionWarningSummary(
        bool GameLost,
        int DaysUntilNextElection,
        double ElectionWarningScore,
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        if (GameLost)
            return L"정권 상실";

        if (DaysUntilNextElection < 0)
            return L"선거 일정 없음";

        if (DaysUntilNextElection > 180 && ElectionWarningScore < 0.32)
            return L"안정";

        std::wstring Summary =
            std::wstring(GetElectionWarningTierLabel(ElectionWarningScore)) +
            L" / " +
            std::to_wstring(DaysUntilNextElection) +
            L"일 남음";

        if (TaxEventStatus.Active && !TaxEventStatus.Title.empty())
        {
            Summary += L" / " + TaxEventStatus.Title;
        }
        else if (ElectionWarningScore >= 0.78)
        {
            Summary += L" / 지지 기반 급락";
        }
        else if (ElectionWarningScore >= 0.52)
        {
            Summary += L" / 야권 결집";
        }
        else if (DaysUntilNextElection <= 90)
        {
            Summary += L" / 박빙 진입 가능";
        }

        return Summary;
    }

    const wchar_t* GetPoliticalFactionCompactName(
        EPoliticalAxis Axis,
        EPoliticalStance Stance)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"자본";
            case EPoliticalStance::Right: return L"공산";
            default: return L"중립";
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"종교";
            case EPoliticalStance::Right: return L"군국";
            default: return L"중립";
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"환경";
            case EPoliticalStance::Right: return L"산업";
            default: return L"중립";
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"지식";
            case EPoliticalStance::Right: return L"보수";
            default: return L"중립";
            }
        default:
            return L"중립";
        }
    }

    const wchar_t* GetPoliticalFactionVerboseName(
        EPoliticalAxis Axis,
        EPoliticalStance Stance)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"자본주의자";
            case EPoliticalStance::Right: return L"공산주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"종교인";
            case EPoliticalStance::Right: return L"군국주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"환경주의자";
            case EPoliticalStance::Right: return L"산업주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"지식인";
            case EPoliticalStance::Right: return L"보수주의자";
            default: return L"중립";
            }
        default:
            return L"중립";
        }
    }

    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureSectionText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureBodyValueText(
        const std::shared_ptr<CTextBlock>& Text,
        unsigned char r = 78,
        unsigned char g = 68,
        unsigned char b = 54);
    void ConfigureNoticeText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardTitleText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardValueText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardDetailText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureRowBackground(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName);
    void ConfigureMetricBar(const std::shared_ptr<CProgressBar>& Bar);
    void ConfigureTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected);
    void ConfigureCloseButtonStyle(const std::shared_ptr<CButton>& Button);
    void ApplyTextureToAllButtonStates(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKey,
        const TCHAR* TextureFile);
    void SetCardTint(const CAlmanacWidget::FCardWidgets& Card, bool Highlight);
    void SetCardData(
        const CAlmanacWidget::FCardWidgets& Card,
        const std::wstring& Title,
        const std::wstring& Value,
        const std::wstring& Detail,
        bool Highlight = false);
    void SetMetricRowBackground(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        bool Highlight);
    void SetMetricRowData(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Highlight = false);
    void SetDetailRowData(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        bool Highlight = false,
        const FVector4& ValueColor = FVector4(0.31f, 0.27f, 0.21f, 1.f));

}

CAlmanacWidget::CAlmanacWidget()
{
}

CAlmanacWidget::~CAlmanacWidget()
{
}

namespace
{
    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(28.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(92, 62, 28, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(245, 237, 215, 180);
    }

    void ConfigureSectionText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(20.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(88, 64, 34, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(245, 237, 215, 150);
    }

    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(58, 58, 58, 255);
    }

    void ConfigureBodyValueText(
        const std::shared_ptr<CTextBlock>& Text,
        unsigned char r,
        unsigned char g,
        unsigned char b)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(r, g, b, 255);
    }

    void ConfigureNoticeText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(14.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(102, 92, 76, 255);
    }

    void ConfigureCardTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(88, 66, 36, 255);
    }

    void ConfigureCardValueText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(28.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(48, 48, 48, 255);
    }

    void ConfigureCardDetailText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(14.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(112, 102, 84, 255);
    }

    void ConfigureRowBackground(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName)
    {
        if (!Image)
            return;

        Image->SetTexture(TextureName, GRowTexture);
        Image->SetTint(1.f, 1.f, 1.f, 0.95f);
    }

    void ConfigureMetricBar(const std::shared_ptr<CProgressBar>& Bar)
    {
        if (!Bar)
            return;

        Bar->SetTint(EProgressBarImageType::Back,
            FVector4(0.80f, 0.78f, 0.72f, 0.72f));
        Bar->SetTint(EProgressBarImageType::Fill,
            FVector4(0.90f, 0.72f, 0.18f, 0.95f));
        Bar->SetBarDir(EProgressBarDir::RightToLeft);
        Bar->SetPercent(0.f);
    }

    void ConfigureTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(1.00f, 0.95f, 0.52f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.00f, 0.98f, 0.66f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.94f, 0.82f, 0.28f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.60f, 0.60f, 0.60f, 0.70f));
            return;
        }

        Button->SetTint(EButtonState::Normal,
            FVector4(0.78f, 0.86f, 0.96f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.90f, 0.95f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.68f, 0.78f, 0.90f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.50f, 0.50f, 0.50f, 0.70f));
    }

    void ConfigureCloseButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.95f, 0.84f, 0.24f, 1.f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.f, 0.92f, 0.38f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.86f, 0.72f, 0.12f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.5f, 0.5f, 0.5f, 0.7f));
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

    void SetCardTint(const CAlmanacWidget::FCardWidgets& Card, bool Highlight)
    {
        auto Background = Card.Background.lock();

        if (!Background)
            return;

        if (Highlight)
            Background->SetTint(1.00f, 0.92f, 0.20f, 1.f);
        else
            Background->SetTint(1.f, 1.f, 1.f, 0.95f);
    }

    void SetCardData(
        const CAlmanacWidget::FCardWidgets& Card,
        const std::wstring& Title,
        const std::wstring& Value,
        const std::wstring& Detail,
        bool Highlight)
    {
        SetCardTint(Card, Highlight);

        auto TitleText = Card.Title.lock();
        auto ValueText = Card.Value.lock();
        auto DetailText = Card.Detail.lock();

        if (TitleText)
            TitleText->SetText(Title.c_str());
        if (ValueText)
            ValueText->SetText(Value.c_str());
        if (DetailText)
            DetailText->SetText(Detail.c_str());
    }

    void SetMetricRowBackground(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        bool Highlight)
    {
        auto Background = Row.Background.lock();

        if (!Background)
            return;

        if (Highlight)
            Background->SetTint(1.00f, 0.92f, 0.20f, 1.f);
        else
            Background->SetTint(1.f, 1.f, 1.f, 0.95f);
    }

    void SetMetricRowData(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Highlight)
    {
        SetMetricRowBackground(Row, Highlight);

        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (LabelText)
            LabelText->SetText(Label.c_str());

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Highlight ? 92 : 78,
                Highlight ? 54 : 68,
                Highlight ? 12 : 54,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Clamp01(Percent));
            Bar->SetTint(EProgressBarImageType::Fill, FillTint);
        }
    }

    void SetDetailRowData(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        bool Highlight,
        const FVector4& ValueColor)
    {
        auto Background = Row.Background.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();

        if (Background)
        {
            if (Highlight)
                Background->SetTint(1.00f, 0.92f, 0.20f, 1.f);
            else
                Background->SetTint(1.f, 1.f, 1.f, 0.95f);
        }

        if (LabelText)
            LabelText->SetText(Label.c_str());

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                static_cast<unsigned char>(ValueColor.x * 255.f),
                static_cast<unsigned char>(ValueColor.y * 255.f),
                static_cast<unsigned char>(ValueColor.z * 255.f),
                255);
        }
    }
}

bool CAlmanacWidget::Init()
{
    CWidgetContainer::Init();

    auto PanelBackground = CreateWidget<CImage>("Almanac_Background", 6).lock();

    if (PanelBackground)
    {
        PanelBackground->SetTexture("AlmanacPanelBackground", GPanelTexture);
        PanelBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mPanelBackground = PanelBackground;
    }

    auto TitleText = CreateWidget<CTextBlock>("Almanac_Title", 8).lock();

    if (TitleText)
    {
        TitleText->SetText(GPageTitles[static_cast<size_t>(mSelectedPage)]);
        ConfigureTitleText(TitleText);
        mTitleText = TitleText;
    }

    auto CloseButton = CreateWidget<CButton>("Almanac_Close", 9).lock();

    if (CloseButton)
    {
        ConfigureCloseButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CAlmanacWidget>(
            EButtonEventState::Click, this,
            &CAlmanacWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "Almanac_CloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(18.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(72, 48, 12, 255);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    mTabButtons.resize(static_cast<size_t>(EAlmanacPage::Count));

    void (CAlmanacWidget::*TabCallbacks[static_cast<size_t>(EAlmanacPage::Count)])() =
    {
        &CAlmanacWidget::OnOverviewTabClick,
        &CAlmanacWidget::OnSatisfactionTabClick,
        &CAlmanacWidget::OnPopulationTabClick,
        &CAlmanacWidget::OnEconomyTabClick,
        &CAlmanacWidget::OnResourcesTabClick,
        &CAlmanacWidget::OnPoliticsTabClick,
        &CAlmanacWidget::OnForeignTabClick,
        &CAlmanacWidget::OnBuildingsTabClick,
        &CAlmanacWidget::OnConflictTabClick
    };

    for (size_t Index = 0; Index < mTabButtons.size(); ++Index)
    {
        auto Button = CreateWidget<CButton>(
            "Almanac_Tab_" + std::to_string(Index + 1), 9).lock();

        if (!Button)
            continue;

        ConfigureTabButtonStyle(
            Button, Index == static_cast<size_t>(mSelectedPage));
        ApplyTextureToAllButtonStates(
            Button,
            "AlmanacTabTexture_" + std::to_string(Index),
            GTabTexture);
        Button->SetEventCallback<CAlmanacWidget>(
            EButtonEventState::Click, this, TabCallbacks[Index]);

        auto Icon = CWidget::CreateStaticWidget<CImage>(
            "Almanac_TabIcon_" + std::to_string(Index + 1), mWorld);

        if (Icon)
        {
            Icon->SetTexture(
                "AlmanacTabIconTex_" + std::to_string(Index),
                GPageTabIcons[Index]);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(Icon);
        }

        mTabButtons[Index] = Button;
    }

    for (size_t Index = 0; Index < mPages.size(); ++Index)
    {
        auto Page = CreateWidget<CWidgetContainer>(
            "Almanac_Page_" + std::to_string(Index), 7).lock();

        if (Page)
            mPages[Index] = Page;
    }

    auto CreateCard = [this](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix,
        const TCHAR* IconPath) -> FCardWidgets
    {
        FCardWidgets Card;

        if (!Page)
            return Card;

        Card.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);

        if (auto Background = Card.Background.lock())
            ConfigureRowBackground(Background, Prefix + "_Texture");

        Card.Icon = Page->CreateWidget<CImage>(Prefix + "_Icon", 2);

        if (auto Icon = Card.Icon.lock())
        {
            Icon->SetTexture(Prefix + "_IconTexture", IconPath);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        }

        Card.Title = Page->CreateWidget<CTextBlock>(Prefix + "_Title", 2);
        Card.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);
        Card.Detail = Page->CreateWidget<CTextBlock>(Prefix + "_Detail", 2);

        ConfigureCardTitleText(Card.Title.lock());
        ConfigureCardValueText(Card.Value.lock());
        ConfigureCardDetailText(Card.Detail.lock());

        return Card;
    };

    auto CreateMetricRow = [this](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix) -> FMetricRowWidgets
    {
        FMetricRowWidgets Row;

        if (!Page)
            return Row;

        Row.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);
        Row.Label = Page->CreateWidget<CTextBlock>(Prefix + "_Label", 2);
        Row.Bar = Page->CreateWidget<CProgressBar>(Prefix + "_Bar", 2);
        Row.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);

        ConfigureRowBackground(Row.Background.lock(), Prefix + "_Texture");
        ConfigureBodyLabelText(Row.Label.lock());
        ConfigureMetricBar(Row.Bar.lock());
        ConfigureBodyValueText(Row.Value.lock());

        return Row;
    };

    auto CreateDetailRow = [this](
        const std::shared_ptr<CWidgetContainer>& Page,
        const std::string& Prefix) -> FDetailRowWidgets
    {
        FDetailRowWidgets Row;

        if (!Page)
            return Row;

        Row.Background = Page->CreateWidget<CImage>(
            Prefix + "_Background", 1);
        Row.Label = Page->CreateWidget<CTextBlock>(Prefix + "_Label", 2);
        Row.Value = Page->CreateWidget<CTextBlock>(Prefix + "_Value", 2);

        ConfigureRowBackground(Row.Background.lock(), Prefix + "_Texture");
        ConfigureBodyLabelText(Row.Label.lock());
        ConfigureBodyValueText(Row.Value.lock());

        return Row;
    };

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Overview)].lock();
        const TCHAR* CardIcons[GOverviewCardCount] =
        {
            GPopulationIcon,
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
            GApprovalIcon,
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
            GMoneyIcon
        };

        mOverviewCards.reserve(GOverviewCardCount);

        for (int Index = 0; Index < GOverviewCardCount; ++Index)
        {
            mOverviewCards.push_back(CreateCard(
                Page,
                "Almanac_OverviewCard_" + std::to_string(Index + 1),
                CardIcons[Index]));
        }

        mOverviewSummaryLeft = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryLeft", 2);
        mOverviewSummaryRight = Page->CreateWidget<CTextBlock>(
            "Almanac_OverviewSummaryRight", 2);

        ConfigureNoticeText(mOverviewSummaryLeft.lock());
        ConfigureNoticeText(mOverviewSummaryRight.lock());
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Satisfaction)].lock();

        for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
        {
            mSatisfactionRows.push_back(CreateMetricRow(
                Page,
                "Almanac_SatisfactionRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GSatisfactionDetailCount; ++Index)
        {
            mSatisfactionDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_SatisfactionDetail_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Population)].lock();

        for (int Index = 0; Index < GPopulationDetailCount; ++Index)
        {
            mPopulationDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_PopulationDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GPopulationMetricCount; ++Index)
        {
            mPopulationMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_PopulationMetric_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Economy)].lock();

        for (int Index = 0; Index < GEconomyDetailCount; ++Index)
        {
            mEconomyDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_EconomyDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GEconomyMetricCount; ++Index)
        {
            mEconomyMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_EconomyMetric_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Resources)].lock();

        for (int Index = 0; Index < GResourceRowCount; ++Index)
        {
            mResourceRows.push_back(CreateMetricRow(
                Page,
                "Almanac_ResourceRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GResourceDetailCount; ++Index)
        {
            mResourceDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ResourceDetail_" + std::to_string(Index + 1)));
        }

        mResourceNotice = Page->CreateWidget<CTextBlock>(
            "Almanac_ResourceNotice", 2);
        ConfigureNoticeText(mResourceNotice.lock());
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Politics)].lock();

        for (int Index = 0; Index < GPoliticsRowCount; ++Index)
        {
            mPoliticsRows.push_back(CreateMetricRow(
                Page,
                "Almanac_PoliticsRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GPoliticsDetailCount; ++Index)
        {
            mPoliticsDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_PoliticsDetail_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Foreign)].lock();

        for (int Index = 0; Index < GForeignDetailCount; ++Index)
        {
            mForeignDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ForeignDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GForeignMetricCount; ++Index)
        {
            mForeignMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_ForeignMetric_" + std::to_string(Index + 1)));
        }

        mForeignNotice = Page->CreateWidget<CTextBlock>(
            "Almanac_ForeignNotice", 2);
        ConfigureNoticeText(mForeignNotice.lock());
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Buildings)].lock();

        for (int Index = 0; Index < GBuildingRowCount; ++Index)
        {
            mBuildingRows.push_back(CreateMetricRow(
                Page,
                "Almanac_BuildingRow_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GBuildingDetailCount; ++Index)
        {
            mBuildingDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_BuildingDetail_" + std::to_string(Index + 1)));
        }
    }

    {
        auto Page = mPages[static_cast<size_t>(EAlmanacPage::Conflict)].lock();

        mConflictHeadlineBackground = Page->CreateWidget<CImage>(
            "Almanac_ConflictHeadlineBackground", 1);
        mConflictHeadlineText = Page->CreateWidget<CTextBlock>(
            "Almanac_ConflictHeadlineText", 2);

        ConfigureRowBackground(
            mConflictHeadlineBackground.lock(),
            "Almanac_ConflictHeadlineTexture");
        ConfigureSectionText(mConflictHeadlineText.lock());
        if (auto HeadlineText = mConflictHeadlineText.lock())
            HeadlineText->SetAlignV(ETextAlignV::Top);

        for (int Index = 0; Index < GConflictDetailCount; ++Index)
        {
            mConflictDetails.push_back(CreateDetailRow(
                Page,
                "Almanac_ConflictDetail_" + std::to_string(Index + 1)));
        }

        for (int Index = 0; Index < GConflictMetricCount; ++Index)
        {
            mConflictMetrics.push_back(CreateMetricRow(
                Page,
                "Almanac_ConflictMetric_" + std::to_string(Index + 1)));
        }
    }

    ApplySelectedPage();
    ApplyOpenState();
    RefreshData();
    RefreshLayout();

    return true;
}

void CAlmanacWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    mDataRefreshAccum += DeltaTime;

    if (mDataRefreshAccum >= GDataRefreshIntervalSeconds)
    {
        RefreshData();
        mDataRefreshAccum =
            std::fmod(mDataRefreshAccum, GDataRefreshIntervalSeconds);
    }

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();

    if (mLastResolutionWidth != Resolution.Width ||
        mLastResolutionHeight != Resolution.Height)
    {
        mLayoutDirty = true;
    }

    if (mLayoutDirty)
        RefreshLayout();
}

void CAlmanacWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CAlmanacWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mDataRefreshAccum = 0.f;
        mLayoutDirty = true;
        RefreshData();
        RefreshLayout();
    }

    ApplyOpenState();
}

void CAlmanacWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mLastResolutionWidth = Resolution.Width;
    mLastResolutionHeight = Resolution.Height;
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(360.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(360.f, ScreenHeight - 120.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(
                AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f + 18.f * Scale;
    const float HeaderHeight = 68.f * Scale;
    const float HeaderPadding = 24.f * Scale;
    const float ContentLeft = PanelLeft + 38.f * Scale;
    const float ContentTop = PanelTop + HeaderHeight + 18.f * Scale;
    const float ContentWidth = PanelWidth - 76.f * Scale;
    const float ContentHeight = PanelHeight - HeaderHeight - 42.f * Scale;

    if (auto Background = mPanelBackground.lock())
    {
        Background->SetPos(PanelLeft, PanelTop);
        Background->SetSize(PanelWidth, PanelHeight);
    }

    if (auto TitleText = mTitleText.lock())
    {
        TitleText->SetPos(PanelLeft + HeaderPadding, PanelTop + 18.f * Scale);
        TitleText->SetSize(PanelWidth - 2.f * HeaderPadding, 38.f * Scale);
    }

    if (auto CloseButton = mCloseButton.lock())
    {
        CloseButton->SetPos(
            PanelLeft + PanelWidth - HeaderPadding - 42.f * Scale,
            PanelTop + 12.f * Scale);
        CloseButton->SetSize(42.f * Scale, 42.f * Scale);
    }

    const float TabSize = 66.f * Scale;
    const float TabGap = 8.f * Scale;
    const float TabsWidth =
        TabSize * static_cast<float>(mTabButtons.size()) +
        TabGap * static_cast<float>((std::max)(0, static_cast<int>(mTabButtons.size()) - 1));
    const float TabsStartX = PanelLeft + (PanelWidth - TabsWidth) * 0.5f;
    const float TabBaseY = PanelTop - 18.f * Scale;

    for (size_t Index = 0; Index < mTabButtons.size(); ++Index)
    {
        auto Button = mTabButtons[Index].lock();

        if (!Button)
            continue;

        const bool Selected = Index == static_cast<size_t>(mSelectedPage);
        const float OffsetY = Selected ? 12.f * Scale : 0.f;

        Button->SetPos(
            TabsStartX + (TabSize + TabGap) * static_cast<float>(Index),
            TabBaseY + OffsetY);
        Button->SetSize(TabSize, TabSize);
    }

    for (size_t Index = 0; Index < mPages.size(); ++Index)
    {
        auto Page = mPages[Index].lock();

        if (!Page)
            continue;

        Page->SetPos(ContentLeft, ContentTop);
        Page->SetSize(ContentWidth, ContentHeight);
    }

    auto LayoutMetricRows =
        [Scale](const std::vector<FMetricRowWidgets>& Rows,
            float X,
            float Y,
            float Width,
            float RowHeight,
            float Gap)
    {
        const float LabelWidth = Width * 0.34f;
        const float ValueWidth = 92.f * Scale;
        const float BarLeft = X + LabelWidth + 12.f * Scale;
        const float BarWidth =
            (std::max)(40.f, Width - LabelWidth - ValueWidth - 30.f * Scale);

        for (size_t Index = 0; Index < Rows.size(); ++Index)
        {
            const float RowY =
                Y + (RowHeight + Gap) * static_cast<float>(Index);

            if (auto Background = Rows[Index].Background.lock())
            {
                Background->SetPos(X, RowY);
                Background->SetSize(Width, RowHeight);
            }

            if (auto Label = Rows[Index].Label.lock())
            {
                Label->SetPos(X + 14.f * Scale, RowY);
                Label->SetSize(LabelWidth - 18.f * Scale, RowHeight);
            }

            if (auto Bar = Rows[Index].Bar.lock())
            {
                Bar->SetPos(BarLeft, RowY + RowHeight * 0.30f);
                Bar->SetSize(BarWidth, RowHeight * 0.34f);
            }

            if (auto Value = Rows[Index].Value.lock())
            {
                Value->SetPos(X + Width - ValueWidth - 12.f * Scale, RowY);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutDetailRows =
        [Scale](const std::vector<FDetailRowWidgets>& Rows,
            float X,
            float Y,
            float Width,
            float RowHeight,
            float Gap)
    {
        const float ValueWidth = 160.f * Scale;

        for (size_t Index = 0; Index < Rows.size(); ++Index)
        {
            const float RowY =
                Y + (RowHeight + Gap) * static_cast<float>(Index);

            if (auto Background = Rows[Index].Background.lock())
            {
                Background->SetPos(X, RowY);
                Background->SetSize(Width, RowHeight);
            }

            if (auto Label = Rows[Index].Label.lock())
            {
                Label->SetPos(X + 14.f * Scale, RowY);
                Label->SetSize(Width - ValueWidth - 24.f * Scale, RowHeight);
            }

            if (auto Value = Rows[Index].Value.lock())
            {
                Value->SetPos(X + Width - ValueWidth - 12.f * Scale, RowY);
                Value->SetSize(ValueWidth, RowHeight);
            }
        }
    };

    auto LayoutCards =
        [Scale](const std::vector<FCardWidgets>& Cards,
            float X,
            float Y,
            float Width,
            float Height,
            int Columns,
            float GapX,
            float GapY)
    {
        if (Columns <= 0)
            return;

        const float CardWidth =
            (Width - GapX * static_cast<float>(Columns - 1)) /
            static_cast<float>(Columns);
        const float CardHeight =
            (Height - GapY * static_cast<float>(1)) * 0.5f;

        for (size_t Index = 0; Index < Cards.size(); ++Index)
        {
            const int Row = static_cast<int>(Index) / Columns;
            const int Col = static_cast<int>(Index) % Columns;
            const float CardX =
                X + (CardWidth + GapX) * static_cast<float>(Col);
            const float CardY =
                Y + (CardHeight + GapY) * static_cast<float>(Row);

            if (auto Background = Cards[Index].Background.lock())
            {
                Background->SetPos(CardX, CardY);
                Background->SetSize(CardWidth, CardHeight);
            }

            if (auto Icon = Cards[Index].Icon.lock())
            {
                Icon->SetPos(CardX + 16.f * Scale, CardY + 18.f * Scale);
                Icon->SetSize(36.f * Scale, 36.f * Scale);
            }

            if (auto Title = Cards[Index].Title.lock())
            {
                Title->SetPos(CardX + 60.f * Scale, CardY + 14.f * Scale);
                Title->SetSize(CardWidth - 72.f * Scale, 28.f * Scale);
            }

            if (auto Value = Cards[Index].Value.lock())
            {
                Value->SetPos(CardX + 14.f * Scale, CardY + 42.f * Scale);
                Value->SetSize(CardWidth - 28.f * Scale, 40.f * Scale);
            }

            if (auto Detail = Cards[Index].Detail.lock())
            {
                Detail->SetPos(CardX + 14.f * Scale, CardY + 86.f * Scale);
                Detail->SetSize(CardWidth - 28.f * Scale, CardHeight - 96.f * Scale);
            }
        }
    };

    const float LeftWide = ContentWidth * 0.56f;
    const float RightWide = ContentWidth - LeftWide - 22.f * Scale;

    LayoutCards(
        mOverviewCards, 0.f, 0.f, ContentWidth, ContentHeight - 70.f * Scale,
        3, 14.f * Scale, 14.f * Scale);

    if (auto SummaryLeft = mOverviewSummaryLeft.lock())
    {
        SummaryLeft->SetPos(0.f, ContentHeight - 52.f * Scale);
        SummaryLeft->SetSize(ContentWidth * 0.48f, 48.f * Scale);
    }

    if (auto SummaryRight = mOverviewSummaryRight.lock())
    {
        SummaryRight->SetPos(ContentWidth * 0.52f, ContentHeight - 52.f * Scale);
        SummaryRight->SetSize(ContentWidth * 0.48f, 48.f * Scale);
    }

    LayoutMetricRows(mSatisfactionRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(mSatisfactionDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 10.f * Scale);
    LayoutDetailRows(mPopulationDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(mPopulationMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 52.f * Scale, 14.f * Scale);
    LayoutDetailRows(mEconomyDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(mEconomyMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 10.f * Scale);
    LayoutMetricRows(mResourceRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(mResourceDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 8.f * Scale);
    const float PoliticsMetricGap = 6.f * Scale;
    const float PoliticsMetricRowHeight =
        (std::max)(
            28.f * Scale,
            (ContentHeight -
                PoliticsMetricGap *
                static_cast<float>((std::max)(0, GPoliticsRowCount - 1))) /
            static_cast<float>((std::max)(1, GPoliticsRowCount)));
    LayoutMetricRows(
        mPoliticsRows,
        0.f,
        0.f,
        ContentWidth * 0.52f,
        PoliticsMetricRowHeight,
        PoliticsMetricGap);
    const float PoliticsDetailGap = 6.f * Scale;
    const float PoliticsDetailRowHeight =
        (std::max)(
            30.f * Scale,
            (ContentHeight -
                PoliticsDetailGap *
                static_cast<float>((std::max)(0, GPoliticsDetailCount - 1))) /
            static_cast<float>((std::max)(1, GPoliticsDetailCount)));
    LayoutDetailRows(
        mPoliticsDetails,
        ContentWidth * 0.52f + 22.f * Scale,
        0.f,
        ContentWidth - ContentWidth * 0.52f - 22.f * Scale,
        PoliticsDetailRowHeight,
        PoliticsDetailGap);
    LayoutDetailRows(mForeignDetails, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutMetricRows(mForeignMetrics, LeftWide + 22.f * Scale, 0.f, RightWide, 54.f * Scale, 12.f * Scale);
    LayoutMetricRows(mBuildingRows, 0.f, 0.f, LeftWide, 46.f * Scale, 8.f * Scale);
    LayoutDetailRows(mBuildingDetails, LeftWide + 22.f * Scale, 0.f, RightWide, 46.f * Scale, 8.f * Scale);

    if (auto Notice = mResourceNotice.lock())
    {
        Notice->SetPos(LeftWide + 22.f * Scale, 6.f * 54.f * Scale);
        Notice->SetSize(RightWide, 120.f * Scale);
    }

    if (auto Notice = mForeignNotice.lock())
    {
        Notice->SetPos(LeftWide + 22.f * Scale, 4.f * 66.f * Scale);
        Notice->SetSize(RightWide, 120.f * Scale);
    }

    const float ConflictLeft = ContentWidth * 0.54f;
    const float ConflictRight = ContentWidth - ConflictLeft - 22.f * Scale;
    const float ConflictDetailGap = 8.f * Scale;
    const float ConflictDetailRowHeight =
        (std::max)(
            30.f * Scale,
            (ContentHeight -
                140.f * Scale -
                ConflictDetailGap *
                static_cast<float>((std::max)(0, GConflictDetailCount - 1))) /
            static_cast<float>((std::max)(1, GConflictDetailCount)));

    if (auto HeadlineBackground = mConflictHeadlineBackground.lock())
    {
        HeadlineBackground->SetPos(0.f, 0.f);
        HeadlineBackground->SetSize(ConflictLeft, 122.f * Scale);
    }

    if (auto HeadlineText = mConflictHeadlineText.lock())
    {
        HeadlineText->SetPos(18.f * Scale, 14.f * Scale);
        HeadlineText->SetSize(ConflictLeft - 36.f * Scale, 96.f * Scale);
    }

    LayoutDetailRows(
        mConflictDetails,
        0.f,
        140.f * Scale,
        ConflictLeft,
        ConflictDetailRowHeight,
        ConflictDetailGap);
    LayoutMetricRows(mConflictMetrics, ConflictLeft + 22.f * Scale, 0.f, ConflictRight, 54.f * Scale, 14.f * Scale);
    mLayoutDirty = false;
}

void CAlmanacWidget::ApplyOpenState()
{
    if (auto Background = mPanelBackground.lock())
        Background->SetEnable(mOpen);

    if (auto TitleText = mTitleText.lock())
        TitleText->SetEnable(mOpen);

    if (auto CloseButton = mCloseButton.lock())
        CloseButton->SetEnable(mOpen);

    for (size_t Index = 0; Index < mTabButtons.size(); ++Index)
    {
        auto Button = mTabButtons[Index].lock();

        if (Button)
            Button->SetEnable(mOpen);
    }

    ApplySelectedPage();
}

void CAlmanacWidget::ApplySelectedPage()
{
    if (auto TitleText = mTitleText.lock())
        TitleText->SetText(GPageTitles[static_cast<size_t>(mSelectedPage)]);

    for (size_t Index = 0; Index < mTabButtons.size(); ++Index)
    {
        auto Button = mTabButtons[Index].lock();

        if (Button)
        {
            ConfigureTabButtonStyle(
                Button,
                Index == static_cast<size_t>(mSelectedPage));
        }
    }

    for (size_t Index = 0; Index < mPages.size(); ++Index)
    {
        auto Page = mPages[Index].lock();

        if (Page)
            Page->SetEnable(
                mOpen && Index == static_cast<size_t>(mSelectedPage));
    }
}

void CAlmanacWidget::SelectPage(EAlmanacPage Page)
{
    if (mSelectedPage == Page)
        return;

    mSelectedPage = Page;
    ApplySelectedPage();
    mLayoutDirty = true;
    RefreshLayout();
}

void CAlmanacWidget::RefreshData()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);
    const AlmanacDataProvider::FAlmanacSnapshot Snapshot =
        AlmanacDataProvider::BuildSnapshot(World, MainWorld);

    const int ActiveCitizenCount = (std::max)(1, Snapshot.ActiveCitizenCount);
    const int HousingVacancy =
        (std::max)(0, Snapshot.ResidentialCapacity - Snapshot.AssignedHomeCount);
    const int JobVacancy =
        (std::max)(0, Snapshot.JobCapacity - Snapshot.AssignedJobCount);
    const double HousingOccupancyRate =
        Snapshot.ResidentialCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedHomeCount) /
        static_cast<double>(Snapshot.ResidentialCapacity) : 0.0;
    const double EmploymentRate =
        Snapshot.JobCapacity > 0 ?
        static_cast<double>(Snapshot.AssignedJobCount) /
        static_cast<double>(Snapshot.JobCapacity) : 0.0;
    const double HomelessRate =
        static_cast<double>(Snapshot.HomelessCount) /
        static_cast<double>(ActiveCitizenCount);
    const double UnemploymentRate =
        static_cast<double>(Snapshot.UnemployedCount) /
        static_cast<double>(ActiveCitizenCount);
    const double MonthlyBuildingCost =
        static_cast<double>(Snapshot.MonthlyWageCost + Snapshot.MonthlyUpkeepCost);
    const double MonthlyPolicyCost =
        (std::max)(0.0, static_cast<double>(Snapshot.DailyEdictCost) * 30.0);
    const double MonthlyTotalCost = MonthlyBuildingCost + MonthlyPolicyCost;
    const double WagePressure =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.MonthlyWageCost) / MonthlyBuildingCost : 0.0;
    const double UpkeepPressure =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.MonthlyUpkeepCost) / MonthlyBuildingCost : 0.0;
    const double TradeCoverage =
        MonthlyBuildingCost > 0.0 ?
        static_cast<double>(Snapshot.DailyExportIncome) /
        (MonthlyBuildingCost / 30.0) : 0.0;
    const double ConsumptionTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyConsumptionTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double IncomeTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyIncomeTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double PropertyTaxShare =
        Snapshot.DailyTaxIncome > 0 ?
        static_cast<double>(Snapshot.DailyPropertyTaxIncome) /
        static_cast<double>(Snapshot.DailyTaxIncome) : 0.0;
    const double EdictPressure =
        MonthlyTotalCost > 0.0 ?
        MonthlyPolicyCost / MonthlyTotalCost : 0.0;
    const double BudgetRunwayMonths =
        MonthlyTotalCost > 0.0 ?
        (std::max)(
            0.0,
            static_cast<double>(Snapshot.NationalBudget) / MonthlyTotalCost) :
        0.0;
    const double BudgetReserve =
        MonthlyTotalCost > 0.0 ?
        Clamp01(BudgetRunwayMonths / 6.0) :
        (Snapshot.NationalBudget >= 0 ? 1.0 : 0.0);
    const std::wstring BudgetRunwayText =
        MonthlyTotalCost > 0.0 ?
        FormatFixed1(BudgetRunwayMonths) + L"개월" :
        (Snapshot.NationalBudget >= 0 ? std::wstring(L"운영비 0") :
            std::wstring(L"적자"));
    const std::wstring TaxPolicySummary =
        FormatTaxPolicySummary(Snapshot.GovernmentProfile.TaxPolicy);
    const double ConsumptionTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Consumption));
    const double IncomeTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income));
    const double PropertyTaxDeviation =
        static_cast<double>(GetTaxPolicyDeviationNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property));
    const double TaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            true,
            true));
    const double WorkerTaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            true,
            false));
    const double ResidentTaxBurden =
        static_cast<double>(GetCitizenTaxBurdenNormalized(
            Snapshot.GovernmentProfile.TaxPolicy,
            false,
            true));
    const auto ResolveTaxPressureTint =
        [](double Value)
    {
        if (Value > 0.08)
            return FVector4(0.82f, 0.22f, 0.18f, 0.95f);

        if (Value < -0.08)
            return FVector4(0.18f, 0.66f, 0.34f, 0.95f);

        return FVector4(0.28f, 0.56f, 0.82f, 0.95f);
    };
    const auto ResolveFactionReactionTint =
        [](double Value)
    {
        if (Value > 0.08)
            return FVector4(0.18f, 0.66f, 0.34f, 0.95f);

        if (Value < -0.08)
            return FVector4(0.82f, 0.22f, 0.18f, 0.95f);

        return FVector4(0.28f, 0.56f, 0.82f, 0.95f);
    };
    const auto ResolveTaxStanceText =
        [](double Value)
    {
        if (Value >= 0.55)
            return std::wstring(L"강경 증세");
        if (Value >= 0.22)
            return std::wstring(L"증세");
        if (Value <= -0.55)
            return std::wstring(L"강한 감세");
        if (Value <= -0.22)
            return std::wstring(L"감세");
        return std::wstring(L"중립");
    };
    const auto ResolveTaxPressureFocusText =
        [](double WorkerValue, double ResidentValue)
    {
        const double WorkerMagnitude = std::fabs(WorkerValue);
        const double ResidentMagnitude = std::fabs(ResidentValue);
        const double DominantMagnitude =
            (std::max)(WorkerMagnitude, ResidentMagnitude);

        if (DominantMagnitude < 0.12)
            return std::wstring(L"부담 낮음");

        if (WorkerMagnitude > ResidentMagnitude + 0.08)
        {
            return WorkerValue >= 0.0 ?
                std::wstring(L"근로층 압박") :
                std::wstring(L"근로층 완화");
        }

        if (ResidentMagnitude > WorkerMagnitude + 0.08)
        {
            return ResidentValue >= 0.0 ?
                std::wstring(L"거주층 압박") :
                std::wstring(L"거주층 완화");
        }

        return (WorkerValue + ResidentValue) >= 0.0 ?
            std::wstring(L"전반 압박") :
            std::wstring(L"전반 완화");
    };
    const auto ClampSignedUnit =
        [](double Value)
    {
        return (std::max)(-1.0, (std::min)(1.0, Value));
    };
    const double CapitalistReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.28 +
            IncomeTaxDeviation * 0.42 +
            PropertyTaxDeviation * 0.30));
    const double CommunistReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.25) +
        IncomeTaxDeviation * 0.35 +
        PropertyTaxDeviation * 0.40);
    const double IntellectualReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.48) +
        IncomeTaxDeviation * 0.34 +
        PropertyTaxDeviation * 0.18);
    const double ConservativeReaction = ClampSignedUnit(
        -(ConsumptionTaxDeviation * 0.18 +
            IncomeTaxDeviation * 0.27 +
            PropertyTaxDeviation * 0.55));
    const std::array<std::pair<const wchar_t*, double>, 4>
        TaxFactionReactions =
    {
        std::pair<const wchar_t*, double>(L"자본주의자", CapitalistReaction),
        std::pair<const wchar_t*, double>(L"공산주의자", CommunistReaction),
        std::pair<const wchar_t*, double>(L"지식인", IntellectualReaction),
        std::pair<const wchar_t*, double>(L"보수주의자", ConservativeReaction)
    };
    const auto StrongestPositiveReactionIter = std::max_element(
        TaxFactionReactions.begin(),
        TaxFactionReactions.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const auto StrongestNegativeReactionIter = std::min_element(
        TaxFactionReactions.begin(),
        TaxFactionReactions.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const std::wstring TaxStanceSummary =
        ResolveTaxStanceText(TaxBurden) +
        L" / " +
        ResolveTaxPressureFocusText(
            WorkerTaxBurden,
            ResidentTaxBurden);
    std::wstring EconomicBlocReaction = L"뚜렷한 파벌 반응 없음";

    if (StrongestPositiveReactionIter->second > 0.08 ||
        StrongestNegativeReactionIter->second < -0.08)
    {
        EconomicBlocReaction = L"호의: ";

        if (StrongestPositiveReactionIter->second > 0.08)
        {
            EconomicBlocReaction += StrongestPositiveReactionIter->first;
            EconomicBlocReaction += L" ";
            EconomicBlocReaction +=
                FormatSignedPercentUnit(
                    StrongestPositiveReactionIter->second);
        }
        else
        {
            EconomicBlocReaction += L"뚜렷한 지지 없음";
        }

        EconomicBlocReaction += L" / 반발: ";

        if (StrongestNegativeReactionIter->second < -0.08)
        {
            EconomicBlocReaction += StrongestNegativeReactionIter->first;
            EconomicBlocReaction += L" ";
            EconomicBlocReaction +=
                FormatSignedPercentUnit(
                    StrongestNegativeReactionIter->second);
        }
        else
        {
            EconomicBlocReaction += L"뚜렷한 반발 없음";
        }
    }
    std::wstring FactionDemandLabel = L"파벌 요구";
    std::wstring FactionDemandSummary = L"현재 세금 사건에 묶인 요구 없음";
    FVector4 FactionDemandTint(0.31f, 0.27f, 0.21f, 1.f);

    if (Snapshot.TaxEventStatus.Active)
    {
        FactionDemandLabel = L"활성 파벌 요구";
        const std::wstring DaySuffix =
            L" (" +
            std::to_wstring((std::max)(1, Snapshot.TaxEventStatus.DaysActive + 1)) +
            L"일차)";

        switch (Snapshot.TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Left)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left) +
                L" / 근로세 경감 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.82f, 0.48f, 0.12f, 1.f);
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Left) +
                L" / 재산세 유예 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.84f, 0.42f, 0.16f, 1.f);
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            FactionDemandSummary =
                std::wstring(GetPoliticalFactionVerboseName(
                    EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right)) +
                L"·" +
                GetPoliticalFactionVerboseName(
                    EPoliticalAxis::Economy,
                    EPoliticalStance::Right) +
                L" / 재정 안정 대책 요구" +
                DaySuffix;
            FactionDemandTint = FVector4(0.82f, 0.24f, 0.18f, 1.f);
            break;
        default:
            break;
        }
    }
    else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
        !Snapshot.TaxEventStatus.Summary.empty())
    {
        FactionDemandLabel = L"직전 파벌 요구";
        FactionDemandSummary = L"최근 요구 해소 / 경계 유지";
        FactionDemandTint = FVector4(0.42f, 0.52f, 0.72f, 1.f);
    }

    std::wstring TaxEventWorldEffectSummary = L"직접적인 월드 영향 없음";

    if (Snapshot.TaxEventStatus.Active)
    {
        switch (Snapshot.TaxEventStatus.Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            TaxEventWorldEffectSummary =
                L"생산 저하 · 선적 차질 · 근로세 누락";
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            TaxEventWorldEffectSummary =
                L"재산세 누락 · 주거 유지비 상승";
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            TaxEventWorldEffectSummary =
                L"수출 둔화 · 유지비 상승 · 징수 효율 저하";
            break;
        default:
            break;
        }
    }
    else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
        !Snapshot.TaxEventStatus.Summary.empty())
    {
        TaxEventWorldEffectSummary = L"최근 혼란 진정 · 정상화 진행 중";
    }
    const double DailyOperatingCost =
        MonthlyTotalCost > 0.0 ? MonthlyTotalCost / 30.0 : 0.0;
    const double FiscalStress =
        DailyOperatingCost > 0.0 && Snapshot.DailyNetChange < 0 ?
        Clamp01(
            static_cast<double>(-Snapshot.DailyNetChange) /
            DailyOperatingCost) :
        0.0;
    const double TourismShare =
        Snapshot.TotalBuildingCount > 0 ?
        static_cast<double>(Snapshot.TourismBuildingCount) /
        static_cast<double>(Snapshot.TotalBuildingCount) : 0.0;
    const double HarborShare =
        Snapshot.TotalBuildingCount > 0 ?
        static_cast<double>(Snapshot.HarborCount) /
        static_cast<double>(Snapshot.TotalBuildingCount) : 0.0;
    const double EmergencyPressure =
        Clamp01(
            Clamp01(Snapshot.RebelRiskScore / 100.0) * 0.80 +
            (Snapshot.MartialLawActive ? 0.20 : 0.0));
    const double ControlStrength =
        Clamp01(
            Clamp01(Snapshot.AverageSecurity / 100.0) * 0.55 +
            Clamp01(Snapshot.SupportPercent / 100.0) * 0.25 +
            (1.0 - FiscalStress) * 0.20);
    const double Stability =
        Clamp01(1.0 - Snapshot.RebelRiskScore / 100.0);

    const std::array<std::pair<const wchar_t*, double>, GSatisfactionRowCount - 1>
        NeedScores =
    {
        std::pair<const wchar_t*, double>(GSatisfactionLabels[1], Snapshot.AverageFood),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[2], Snapshot.AverageHealth),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[3], Snapshot.AverageFun),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[4], Snapshot.AverageFaith),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[5], Snapshot.AverageHousing),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[6], Snapshot.AverageJob),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[7], Snapshot.AverageFreedom),
        std::pair<const wchar_t*, double>(GSatisfactionLabels[8], Snapshot.AverageSecurity)
    };

    const auto WorstNeedIter = std::min_element(
        NeedScores.begin(),
        NeedScores.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });
    const auto BestNeedIter = std::max_element(
        NeedScores.begin(),
        NeedScores.end(),
        [](const std::pair<const wchar_t*, double>& A,
            const std::pair<const wchar_t*, double>& B)
        {
            return A.second < B.second;
        });

    const auto NormalizePoliticalScore =
        [](double Value)
    {
        return static_cast<float>(Clamp01((Value + 25.0) / 50.0));
    };
    const auto BuildAxisBreakdown =
        [&Snapshot](EPoliticalAxis Axis)
    {
        const int AxisIndex = static_cast<int>(Axis);
        return std::to_wstring(
            Snapshot.PoliticalCount[AxisIndex]
                                   [static_cast<int>(EPoliticalStance::Left)]) +
            L" / " +
            std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex]
                                       [static_cast<int>(EPoliticalStance::Neutral)]) +
            L" / " +
            std::to_wstring(
                Snapshot.PoliticalCount[AxisIndex]
                                       [static_cast<int>(EPoliticalStance::Right)]);
    };
    const std::wstring NextElectionLabel =
        Snapshot.ElectionStatus.GameLost ?
        std::wstring(L"정권 상실") :
        (Snapshot.ElectionStatus.NextElectionYear > 0 ?
            FormatDate(
                Snapshot.ElectionStatus.NextElectionYear,
                Snapshot.ElectionStatus.NextElectionMonth,
                Snapshot.ElectionStatus.NextElectionDay) :
            std::wstring(L"-"));
    const std::wstring LastElectionLabel =
        Snapshot.ElectionStatus.HasRecordedElection ?
        (FormatDate(
            Snapshot.ElectionStatus.LastElectionYear,
            Snapshot.ElectionStatus.LastElectionMonth,
            Snapshot.ElectionStatus.LastElectionDay) +
            L" " +
            (Snapshot.ElectionStatus.IncumbentWonLastElection ?
                L"재집권" :
                L"정권교체") +
            L" (" +
            FormatFixed1(Snapshot.ElectionStatus.LastVoteShare) +
            L"% / 투표율 " +
            FormatFixed1(Snapshot.ElectionStatus.LastTurnoutPercent) +
            L"%)") :
        std::wstring(L"선거 기록 없음");
    const std::wstring LastElectionCompactLabel =
        Snapshot.ElectionStatus.HasRecordedElection ?
        ((Snapshot.ElectionStatus.IncumbentWonLastElection ?
            std::wstring(L"재집권 ") :
            std::wstring(L"정권교체 ")) +
            FormatFixed1(Snapshot.ElectionStatus.LastVoteShare) +
            L"%") :
        std::wstring(L"선거 기록 없음");
    const std::wstring ElectionWarningSummary =
        BuildElectionWarningSummary(
            Snapshot.ElectionStatus.GameLost,
            Snapshot.DaysUntilNextElection,
            Snapshot.ElectionWarningScore,
            Snapshot.TaxEventStatus);
    const bool ElectionWarningActive =
        Snapshot.DaysUntilNextElection >= 0 &&
        Snapshot.DaysUntilNextElection <= 180 &&
        Snapshot.ElectionWarningScore >= 0.32;
    const FVector4 ElectionWarningTint =
        ResolveElectionWarningTint(Snapshot.ElectionWarningScore);

    if (mOverviewCards.size() >= GOverviewCardCount)
    {
        SetCardData(
            mOverviewCards[0],
            L"인구",
            std::to_wstring(Snapshot.ActiveCitizenCount),
            L"총 활성 시민 수",
            true);
        SetCardData(
            mOverviewCards[1],
            L"무주택 시민",
            std::to_wstring(Snapshot.HomelessCount),
            L"거주 배정 " + FormatPercent(HousingOccupancyRate * 100.0));
        SetCardData(
            mOverviewCards[2],
            L"실업 시민",
            std::to_wstring(Snapshot.UnemployedCount),
            L"고용률 " + FormatPercent(EmploymentRate * 100.0));
        SetCardData(
            mOverviewCards[3],
            L"종합 만족도",
            FormatFixed1(Snapshot.AverageOverall),
            L"자유 " + FormatFixed1(Snapshot.AverageFreedom) +
                L" / 치안 " + FormatFixed1(Snapshot.AverageSecurity));
        SetCardData(
            mOverviewCards[4],
            L"지지율",
            FormatPercent(Snapshot.SupportPercent),
            L"야권 " + FormatPercent(Snapshot.OppositionPercent));
        SetCardData(
            mOverviewCards[5],
            L"국고",
            FormatCurrency(Snapshot.NationalBudget),
            L"일일 순증감 " + FormatCurrency(Snapshot.DailyNetChange) +
                L" / 세수 " + FormatCurrency(Snapshot.DailyTaxIncome) +
                L"\n" + TaxPolicySummary);
    }

    if (auto SummaryLeft = mOverviewSummaryLeft.lock())
    {
        std::wstring Summary =
            L"활성 칙령: " + std::to_wstring(Snapshot.ActiveEdictCount) +
            L"개  |  차기 선거: " + NextElectionLabel;

        if (ElectionWarningActive)
            Summary += L"  |  선거 경고: " + ElectionWarningSummary;

        SummaryLeft->SetText(Summary.c_str());
    }

    if (auto SummaryRight = mOverviewSummaryRight.lock())
    {
        const std::wstring Summary =
            L"반란 위험: " + Snapshot.RebelRiskLabel +
            L"  |  직전 선거: " + LastElectionCompactLabel;
        SummaryRight->SetText(Summary.c_str());
    }

    const double SatisfactionValues[GSatisfactionRowCount] =
    {
        Snapshot.AverageOverall,
        Snapshot.AverageFood,
        Snapshot.AverageHealth,
        Snapshot.AverageFun,
        Snapshot.AverageFaith,
        Snapshot.AverageHousing,
        Snapshot.AverageJob,
        Snapshot.AverageFreedom,
        Snapshot.AverageSecurity
    };

    for (int Index = 0; Index < GSatisfactionRowCount; ++Index)
    {
        SetMetricRowData(
            mSatisfactionRows[Index],
            GSatisfactionLabels[Index],
            FormatFixed1(SatisfactionValues[Index]),
            static_cast<float>(Clamp01(SatisfactionValues[Index] / 100.0)),
            Index == 0 ?
                FVector4(0.96f, 0.82f, 0.12f, 0.98f) :
                FVector4(0.90f, 0.72f, 0.18f, 0.95f),
            Index == 0);
    }

    SetDetailRowData(
        mSatisfactionDetails[0],
        L"최저 만족 항목",
        std::wstring(WorstNeedIter->first) +
            L" " + FormatFixed1(WorstNeedIter->second),
        true,
        WorstNeedIter->second < 50.0 ?
            FVector4(0.78f, 0.18f, 0.18f, 1.f) :
            FVector4(0.84f, 0.48f, 0.12f, 1.f));
    SetDetailRowData(
        mSatisfactionDetails[1],
        L"최고 만족 항목",
        std::wstring(BestNeedIter->first) +
            L" " + FormatFixed1(BestNeedIter->second),
        false,
        FVector4(0.20f, 0.56f, 0.20f, 1.f));
    SetDetailRowData(
        mSatisfactionDetails[2],
        L"무주택 시민",
        FormatCountWithPercent(Snapshot.HomelessCount, HomelessRate));
    SetDetailRowData(
        mSatisfactionDetails[3],
        L"실업 시민",
        FormatCountWithPercent(Snapshot.UnemployedCount, UnemploymentRate));
    SetDetailRowData(
        mSatisfactionDetails[4],
        L"주거 점유율",
        FormatPercent(HousingOccupancyRate * 100.0));
    SetDetailRowData(
        mSatisfactionDetails[5],
        L"고용률",
        FormatPercent(EmploymentRate * 100.0));

    SetDetailRowData(
        mPopulationDetails[0],
        L"총 시민",
        std::to_wstring(Snapshot.ActiveCitizenCount),
        true);
    SetDetailRowData(
        mPopulationDetails[1],
        L"주거 배정",
        std::to_wstring(Snapshot.AssignedHomeCount));
    SetDetailRowData(
        mPopulationDetails[2],
        L"주거 수용력",
        std::to_wstring(Snapshot.ResidentialCapacity));
    SetDetailRowData(
        mPopulationDetails[3],
        L"빈 집",
        std::to_wstring(HousingVacancy));
    SetDetailRowData(
        mPopulationDetails[4],
        L"직업 배정",
        std::to_wstring(Snapshot.AssignedJobCount));
    SetDetailRowData(
        mPopulationDetails[5],
        L"일자리 수용력",
        std::to_wstring(Snapshot.JobCapacity));
    SetDetailRowData(
        mPopulationDetails[6],
        L"빈 일자리",
        std::to_wstring(JobVacancy));
    SetDetailRowData(
        mPopulationDetails[7],
        L"총 건물 수",
        std::to_wstring(Snapshot.TotalBuildingCount));

    SetMetricRowData(
        mPopulationMetrics[0],
        L"주거 점유율",
        FormatPercent(HousingOccupancyRate * 100.0),
        static_cast<float>(HousingOccupancyRate),
        FVector4(0.22f, 0.58f, 0.90f, 0.95f),
        true);
    SetMetricRowData(
        mPopulationMetrics[1],
        L"고용률",
        FormatPercent(EmploymentRate * 100.0),
        static_cast<float>(EmploymentRate),
        FVector4(0.28f, 0.74f, 0.36f, 0.95f));
    SetMetricRowData(
        mPopulationMetrics[2],
        L"무주택률",
        FormatPercent(HomelessRate * 100.0),
        static_cast<float>(HomelessRate),
        FVector4(0.88f, 0.58f, 0.18f, 0.95f));
    SetMetricRowData(
        mPopulationMetrics[3],
        L"실업률",
        FormatPercent(UnemploymentRate * 100.0),
        static_cast<float>(UnemploymentRate),
        FVector4(0.80f, 0.22f, 0.18f, 0.95f));

    SetDetailRowData(
        mEconomyDetails[0],
        L"국고",
        FormatCurrency(Snapshot.NationalBudget),
        true);
    SetDetailRowData(
        mEconomyDetails[1],
        L"월 임금 총액",
        FormatCurrency(Snapshot.MonthlyWageCost));
    SetDetailRowData(
        mEconomyDetails[2],
        L"월 유지비 총액",
        FormatCurrency(Snapshot.MonthlyUpkeepCost));
    SetDetailRowData(
        mEconomyDetails[3],
        L"일일 수출 수익",
        FormatCurrency(Snapshot.DailyExportIncome));
    SetDetailRowData(
        mEconomyDetails[4],
        L"일일 칙령 비용",
        FormatCurrency(Snapshot.DailyEdictCost));
    SetDetailRowData(
        mEconomyDetails[5],
        L"일일 순증감",
        FormatCurrency(Snapshot.DailyNetChange));
    SetDetailRowData(
        mEconomyDetails[6],
        L"일일 세수",
        FormatCurrency(Snapshot.DailyTaxIncome));
    SetDetailRowData(
        mEconomyDetails[7],
        L"세율 정책",
        TaxPolicySummary);
    SetDetailRowData(
        mEconomyDetails[8],
        L"예산 런웨이",
        BudgetRunwayText);

    SetMetricRowData(
        mEconomyMetrics[0],
        L"임금 부담",
        FormatPercent(WagePressure * 100.0),
        static_cast<float>(WagePressure),
        FVector4(0.84f, 0.54f, 0.14f, 0.95f),
        true);
    SetMetricRowData(
        mEconomyMetrics[1],
        L"유지비 부담",
        FormatPercent(UpkeepPressure * 100.0),
        static_cast<float>(UpkeepPressure),
        FVector4(0.72f, 0.34f, 0.18f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[2],
        L"수출 커버리지",
        FormatPercent(Clamp01(TradeCoverage) * 100.0),
        static_cast<float>(Clamp01(TradeCoverage)),
        FVector4(0.22f, 0.58f, 0.90f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[3],
        L"칙령 부담",
        FormatPercent(Clamp01(EdictPressure) * 100.0),
        static_cast<float>(Clamp01(EdictPressure)),
        FVector4(0.84f, 0.22f, 0.18f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[4],
        L"예산 여유",
        FormatPercent(BudgetReserve * 100.0),
        static_cast<float>(BudgetReserve),
        FVector4(0.18f, 0.70f, 0.30f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[5],
        L"징수 효율",
        FormatPercent(Snapshot.TaxCollectionEfficiency * 100.0),
        static_cast<float>(Clamp01(Snapshot.TaxCollectionEfficiency)),
        FVector4(0.28f, 0.62f, 0.82f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[6],
        L"소비세 비중",
        FormatPercent(ConsumptionTaxShare * 100.0),
        static_cast<float>(Clamp01(ConsumptionTaxShare)),
        FVector4(0.78f, 0.60f, 0.16f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[7],
        L"소득세 비중",
        FormatPercent(IncomeTaxShare * 100.0),
        static_cast<float>(Clamp01(IncomeTaxShare)),
        FVector4(0.32f, 0.72f, 0.34f, 0.95f));
    SetMetricRowData(
        mEconomyMetrics[8],
        L"재산세 비중",
        FormatPercent(PropertyTaxShare * 100.0),
        static_cast<float>(Clamp01(PropertyTaxShare)),
        FVector4(0.74f, 0.38f, 0.20f, 0.95f));

    const int ResourceRowCount =
        (std::min)(GResourceRowCount,
            static_cast<int>(Snapshot.TopResourceBuildings.size()));
    int MaxResourceStock = 1;

    for (int Index = 0; Index < ResourceRowCount; ++Index)
    {
        MaxResourceStock = (std::max)(
            MaxResourceStock,
            Snapshot.TopResourceBuildings[Index].second);
    }

    for (int Index = 0; Index < GResourceRowCount; ++Index)
    {
        if (Index < ResourceRowCount)
        {
            const auto& Entry = Snapshot.TopResourceBuildings[Index];
            const float Percent =
                static_cast<float>(Entry.second) /
                static_cast<float>(MaxResourceStock);

            SetMetricRowData(
                mResourceRows[Index],
                Entry.first,
                std::to_wstring(Entry.second),
                Percent,
                FVector4(0.26f, 0.64f, 0.32f, 0.95f),
                Index == 0);
        }
        else
        {
            SetMetricRowData(
                mResourceRows[Index],
                L"-",
                L"-",
                0.f,
                FVector4(0.78f, 0.78f, 0.78f, 0.95f));
        }
    }

    const int ProductionBuildingCount =
        Snapshot.BuildingCategoryCount[static_cast<int>(EBuildingCategory::FoodResource)] +
        Snapshot.BuildingCategoryCount[static_cast<int>(EBuildingCategory::Industry)];
    const std::wstring TopResourceName =
        !Snapshot.TopResourceBuildings.empty() ?
        Snapshot.TopResourceBuildings.front().first :
        std::wstring(L"-");
    const std::wstring TopResourceValue =
        !Snapshot.TopResourceBuildings.empty() ?
        std::to_wstring(Snapshot.TopResourceBuildings.front().second) :
        std::wstring(L"-");

    SetDetailRowData(
        mResourceDetails[0],
        L"총 저장 재고",
        std::to_wstring(Snapshot.TotalResourceStock),
        true);
    SetDetailRowData(
        mResourceDetails[1],
        L"생산 건물 수",
        std::to_wstring(ProductionBuildingCount));
    SetDetailRowData(
        mResourceDetails[2],
        L"식량 공급 건물",
        std::to_wstring(Snapshot.FoodProviderCount));
    SetDetailRowData(
        mResourceDetails[3],
        L"유흥 건물",
        std::to_wstring(Snapshot.EntertainmentBuildingCount));
    SetDetailRowData(
        mResourceDetails[4],
        L"항구/물류 거점",
        std::to_wstring(Snapshot.HarborCount));
    SetDetailRowData(
        mResourceDetails[5],
        TopResourceName,
        TopResourceValue);

    if (auto Notice = mResourceNotice.lock())
    {
        Notice->SetText(
            L"현재 자원 시스템은 건물별 단일 재고값만 집계합니다.\n"
            L"품목 구분과 생산/소비 추이는 아직 미연동입니다.");
    }

    struct FPoliticsMetric
    {
        std::wstring Label;
        std::wstring Value;
        float Percent = 0.f;
        FVector4 Tint = FVector4(0.90f, 0.72f, 0.18f, 0.95f);
        bool Highlight = false;
    };

    const std::array<FPoliticsMetric, GPoliticsRowCount> PoliticsMetrics =
    {
        FPoliticsMetric
        {
            L"지지율",
            FormatPercent(Snapshot.SupportPercent),
            static_cast<float>(Clamp01(Snapshot.SupportPercent / 100.0)),
            FVector4(0.18f, 0.62f, 0.40f, 0.95f),
            true
        },
        FPoliticsMetric
        {
            L"반대율",
            FormatPercent(Snapshot.OppositionPercent),
            static_cast<float>(Clamp01(Snapshot.OppositionPercent / 100.0)),
            FVector4(0.82f, 0.22f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"부동층",
            FormatPercent(Snapshot.AbstainPercent),
            static_cast<float>(Clamp01(Snapshot.AbstainPercent / 100.0)),
            FVector4(0.72f, 0.58f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"평균 생활 점수",
            FormatSignedFixed1(Snapshot.PoliticalSnapshot.AverageLifeScore),
            NormalizePoliticalScore(Snapshot.PoliticalSnapshot.AverageLifeScore),
            FVector4(0.18f, 0.62f, 0.44f, 0.95f)
        },
        FPoliticsMetric
        {
            L"정부 이념 일치",
            FormatSignedFixed1(
                Snapshot.PoliticalSnapshot.AverageGovernmentIdeologyScore),
            NormalizePoliticalScore(
                Snapshot.PoliticalSnapshot.AverageGovernmentIdeologyScore),
            FVector4(0.76f, 0.48f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"정책 행동 효과",
            FormatSignedFixed1(Snapshot.PoliticalSnapshot.AverageActionScore),
            NormalizePoliticalScore(Snapshot.PoliticalSnapshot.AverageActionScore),
            FVector4(0.46f, 0.36f, 0.18f, 0.95f)
        },
        FPoliticsMetric
        {
            L"세금 부담",
            FormatSignedPercentUnit(TaxBurden),
            static_cast<float>(Clamp01(std::fabs(TaxBurden))),
            ResolveTaxPressureTint(TaxBurden)
        },
        FPoliticsMetric
        {
            L"근로층 압박",
            FormatSignedPercentUnit(WorkerTaxBurden),
            static_cast<float>(Clamp01(std::fabs(WorkerTaxBurden))),
            ResolveTaxPressureTint(WorkerTaxBurden)
        },
        FPoliticsMetric
        {
            L"거주층 압박",
            FormatSignedPercentUnit(ResidentTaxBurden),
            static_cast<float>(Clamp01(std::fabs(ResidentTaxBurden))),
            ResolveTaxPressureTint(ResidentTaxBurden)
        },
        FPoliticsMetric
        {
            L"자본주의자 반응",
            FormatSignedPercentUnit(CapitalistReaction),
            static_cast<float>(Clamp01(std::fabs(CapitalistReaction))),
            ResolveFactionReactionTint(CapitalistReaction)
        },
        FPoliticsMetric
        {
            L"공산주의자 반응",
            FormatSignedPercentUnit(CommunistReaction),
            static_cast<float>(Clamp01(std::fabs(CommunistReaction))),
            ResolveFactionReactionTint(CommunistReaction)
        },
        FPoliticsMetric
        {
            L"지식인 반응",
            FormatSignedPercentUnit(IntellectualReaction),
            static_cast<float>(Clamp01(std::fabs(IntellectualReaction))),
            ResolveFactionReactionTint(IntellectualReaction)
        },
        FPoliticsMetric
        {
            L"보수주의자 반응",
            FormatSignedPercentUnit(ConservativeReaction),
            static_cast<float>(Clamp01(std::fabs(ConservativeReaction))),
            ResolveFactionReactionTint(ConservativeReaction)
        }
    };

    for (int Index = 0; Index < GPoliticsRowCount; ++Index)
    {
        SetMetricRowData(
            mPoliticsRows[Index],
            PoliticsMetrics[Index].Label,
            PoliticsMetrics[Index].Value,
            PoliticsMetrics[Index].Percent,
            PoliticsMetrics[Index].Tint,
            PoliticsMetrics[Index].Highlight);
    }

    SetDetailRowData(
        mPoliticsDetails[0],
        L"유권자",
        std::to_wstring(Snapshot.PoliticalSnapshot.ActiveCitizenCount),
        true);
    SetDetailRowData(
        mPoliticsDetails[1],
        std::wstring(GetPoliticalAxisDisplayName(EPoliticalAxis::Economy)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::Economy, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::Economy, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::Economy));
    SetDetailRowData(
        mPoliticsDetails[2],
        std::wstring(
            GetPoliticalAxisDisplayName(EPoliticalAxis::ReligionMilitarism)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::ReligionMilitarism, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::ReligionMilitarism, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::ReligionMilitarism));
    SetDetailRowData(
        mPoliticsDetails[3],
        std::wstring(
            GetPoliticalAxisDisplayName(EPoliticalAxis::EnvironmentIndustry)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::EnvironmentIndustry, EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::EnvironmentIndustry, EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::EnvironmentIndustry));
    SetDetailRowData(
        mPoliticsDetails[4],
        std::wstring(
            GetPoliticalAxisDisplayName(
                EPoliticalAxis::IntellectualConservative)) +
            L"축 " +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Left) +
            L"/중립/" +
            GetPoliticalFactionCompactName(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right),
        BuildAxisBreakdown(EPoliticalAxis::IntellectualConservative));
    SetDetailRowData(
        mPoliticsDetails[5],
        L"중립 축 보유 시민",
        FormatCountWithPercent(
            Snapshot.AnyNeutralAxisCitizenCount,
            static_cast<double>(Snapshot.AnyNeutralAxisCitizenCount) /
                static_cast<double>(ActiveCitizenCount)));
    SetDetailRowData(
        mPoliticsDetails[6],
        L"완전 중립 시민",
        FormatCountWithPercent(
            Snapshot.FullyNeutralCitizenCount,
            static_cast<double>(Snapshot.FullyNeutralCitizenCount) /
                static_cast<double>(ActiveCitizenCount)));
    SetDetailRowData(
        mPoliticsDetails[7],
        L"현재 세율",
        FormatTaxPolicyCompact(Snapshot.GovernmentProfile.TaxPolicy));
    SetDetailRowData(
        mPoliticsDetails[8],
        L"세금 기조",
        TaxStanceSummary,
        false,
        ResolveTaxPressureTint(TaxBurden));
    SetDetailRowData(
        mPoliticsDetails[9],
        L"파벌별 반응 요약",
        EconomicBlocReaction,
        false,
        ResolveFactionReactionTint(
            std::fabs(StrongestPositiveReactionIter->second) >=
            std::fabs(StrongestNegativeReactionIter->second) ?
            StrongestPositiveReactionIter->second :
            StrongestNegativeReactionIter->second));
    SetDetailRowData(
        mPoliticsDetails[10],
        FactionDemandLabel,
        FactionDemandSummary,
        Snapshot.TaxEventStatus.Active,
        FactionDemandTint);
    SetDetailRowData(
        mPoliticsDetails[11],
        L"선거 경고",
        ElectionWarningSummary,
        ElectionWarningActive,
        ElectionWarningTint);
    SetDetailRowData(
        mPoliticsDetails[12],
        Snapshot.ElectionStatus.HasRecordedElection ?
            L"직전 선거" :
            L"차기 선거",
        Snapshot.ElectionStatus.HasRecordedElection ?
            LastElectionLabel :
            NextElectionLabel);

    SetDetailRowData(
        mForeignDetails[0],
        L"대외 시스템",
        L"미연동",
        true);
    SetDetailRowData(
        mForeignDetails[1],
        L"일일 수출 수익",
        FormatCurrency(Snapshot.DailyExportIncome));
    SetDetailRowData(
        mForeignDetails[2],
        L"관광 건물",
        std::to_wstring(Snapshot.TourismBuildingCount));
    SetDetailRowData(
        mForeignDetails[3],
        L"항구 수",
        std::to_wstring(Snapshot.HarborCount));
    SetDetailRowData(
        mForeignDetails[4],
        L"지지율",
        FormatPercent(Snapshot.SupportPercent));
    SetDetailRowData(
        mForeignDetails[5],
        L"계엄령",
        Snapshot.MartialLawActive ? L"활성" : L"비활성");

    SetMetricRowData(
        mForeignMetrics[0],
        L"수출 커버리지",
        FormatPercent(Clamp01(TradeCoverage) * 100.0),
        static_cast<float>(Clamp01(TradeCoverage)),
        FVector4(0.22f, 0.54f, 0.88f, 0.95f),
        true);
    SetMetricRowData(
        mForeignMetrics[1],
        L"관광 기반 비중",
        FormatPercent(TourismShare * 100.0),
        static_cast<float>(TourismShare),
        FVector4(0.18f, 0.66f, 0.40f, 0.95f));
    SetMetricRowData(
        mForeignMetrics[2],
        L"항만 네트워크 비중",
        FormatPercent(HarborShare * 100.0),
        static_cast<float>(HarborShare),
        FVector4(0.78f, 0.68f, 0.18f, 0.95f));
    SetMetricRowData(
        mForeignMetrics[3],
        L"비상 통치 압박",
        FormatPercent(EmergencyPressure * 100.0),
        static_cast<float>(EmergencyPressure),
        FVector4(0.82f, 0.24f, 0.18f, 0.95f));

    if (auto Notice = mForeignNotice.lock())
    {
        Notice->SetText(
            L"국가별 관계, 원조, 무역 계약은 아직 연결되지 않았습니다.\n"
            L"현재는 대외 활동에 영향을 주는 교역·관광·항만 기반만 표시합니다.");
    }

    int HighestCategoryCount = 0;

    for (int Index = 0; Index < GBuildingRowCount; ++Index)
    {
        HighestCategoryCount = (std::max)(
            HighestCategoryCount,
            Snapshot.BuildingCategoryCount[Index]);
    }

    HighestCategoryCount = (std::max)(1, HighestCategoryCount);
    const int SafeBuildingCount = (std::max)(1, Snapshot.TotalBuildingCount);

    for (int Index = 0; Index < GBuildingRowCount; ++Index)
    {
        const int Count = Snapshot.BuildingCategoryCount[Index];
        const float Percent =
            static_cast<float>(Count) /
            static_cast<float>(SafeBuildingCount);

        SetMetricRowData(
            mBuildingRows[Index],
            GBuildingCategoryLabels[Index],
            std::to_wstring(Count),
            Percent,
            FVector4(0.84f, 0.66f, 0.18f, 0.95f),
            Count == HighestCategoryCount && HighestCategoryCount > 0);
    }

    for (int Index = 0; Index < GBuildingDetailCount; ++Index)
    {
        if (Index < static_cast<int>(Snapshot.TopBuildings.size()))
        {
            const auto& Entry = Snapshot.TopBuildings[Index];

            SetDetailRowData(
                mBuildingDetails[Index],
                Entry.first,
                std::to_wstring(Entry.second),
                Index == 0);
        }
        else
        {
            SetDetailRowData(
                mBuildingDetails[Index],
                L"-",
                L"-");
        }
    }

    auto ConflictHeadlineBackground = mConflictHeadlineBackground.lock();
    auto ConflictHeadlineText = mConflictHeadlineText.lock();
    FVector4 ConflictTint(0.82f, 0.92f, 0.76f, 0.98f);
    const bool HasRecentTaxEvent =
        Snapshot.TaxEventStatus.Active ||
        Snapshot.TaxEventStatus.NotificationDays > 0;

    if (Snapshot.RebelRiskScore >= 66.0)
        ConflictTint = FVector4(0.96f, 0.48f, 0.38f, 0.98f);
    else if (Snapshot.RebelRiskScore >= 33.0)
        ConflictTint = FVector4(0.96f, 0.78f, 0.28f, 0.98f);
    else if (Snapshot.TaxEventStatus.Active)
        ConflictTint =
            Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
            FVector4(0.94f, 0.54f, 0.40f, 0.98f) :
            FVector4(0.94f, 0.76f, 0.32f, 0.98f);
    else if (ElectionWarningActive)
        ConflictTint = ElectionWarningTint;

    if (ConflictHeadlineBackground)
        ConflictHeadlineBackground->SetTint(ConflictTint);

    if (ConflictHeadlineText)
    {
        std::wstring Headline =
            L"반란 위험: " + Snapshot.RebelRiskLabel;

        if (Snapshot.TaxEventStatus.Active)
        {
            Headline +=
                L" / 파벌 경고: " +
                Snapshot.TaxEventStatus.Summary;
            Headline +=
                L"\n월드 효과: " +
                TaxEventWorldEffectSummary;
        }
        else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
            !Snapshot.TaxEventStatus.Summary.empty())
        {
            Headline +=
                L" / 최근 경고: " +
                Snapshot.TaxEventStatus.Summary;
            Headline +=
                L"\n정상화 상태: " +
                TaxEventWorldEffectSummary;
        }

        if (ElectionWarningActive)
        {
            Headline +=
                L"\n선거 압박: " +
                ElectionWarningSummary;
        }

        Headline +=
            L"\n평균 자유 만족도 " + FormatFixed1(Snapshot.AverageFreedom) +
            L" / 평균 치안 만족도 " + FormatFixed1(Snapshot.AverageSecurity) +
            L" / 평균 음식 만족도 " + FormatFixed1(Snapshot.AverageFood) +
            L"\n계엄령: " +
            std::wstring(
                Snapshot.MartialLawActive ? L"활성" : L"비활성") +
            L" / 평균 보건 만족도 " + FormatFixed1(Snapshot.AverageHealth);
        ConflictHeadlineText->SetText(Headline.c_str());
    }

    SetDetailRowData(
        mConflictDetails[0],
        L"정치 사건",
        Snapshot.TaxEventStatus.Active ?
            Snapshot.TaxEventStatus.Title :
            (HasRecentTaxEvent ?
                Snapshot.TaxEventStatus.Title :
                std::wstring(L"없음")),
        Snapshot.TaxEventStatus.Active,
        Snapshot.TaxEventStatus.Active ?
            (Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                FVector4(0.82f, 0.48f, 0.12f, 1.f)) :
            FVector4(0.20f, 0.56f, 0.20f, 1.f));
    SetDetailRowData(
        mConflictDetails[1],
        Snapshot.TaxEventStatus.Active ? L"파벌 경고 / 효과" :
            (ElectionWarningActive ? L"선거 경고" : L"사건 메모"),
        Snapshot.TaxEventStatus.Active ?
            (Snapshot.TaxEventStatus.Summary +
                L" / " +
                TaxEventWorldEffectSummary) :
            (ElectionWarningActive ?
                ElectionWarningSummary :
                (HasRecentTaxEvent ?
                (Snapshot.TaxEventStatus.Summary +
                    L" / " +
                    TaxEventWorldEffectSummary) :
                std::wstring(L"안정"))),
        Snapshot.TaxEventStatus.Active || ElectionWarningActive,
        Snapshot.TaxEventStatus.Active ?
            FVector4(0.82f, 0.48f, 0.12f, 1.f) :
            (ElectionWarningActive ?
                ElectionWarningTint :
                FVector4(0.31f, 0.27f, 0.21f, 1.f)));
    SetDetailRowData(
        mConflictDetails[2],
        L"반란 위험 점수",
        FormatFixed1(Snapshot.RebelRiskScore),
        true,
        Snapshot.RebelRiskScore >= 66.0 ?
            FVector4(0.78f, 0.18f, 0.18f, 1.f) :
            (Snapshot.RebelRiskScore >= 33.0 ?
                FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                FVector4(0.20f, 0.56f, 0.20f, 1.f)));
    SetDetailRowData(
        mConflictDetails[3],
        L"평균 음식 만족도",
        FormatFixed1(Snapshot.AverageFood));
    SetDetailRowData(
        mConflictDetails[4],
        L"평균 보건 만족도",
        FormatFixed1(Snapshot.AverageHealth));
    SetDetailRowData(
        mConflictDetails[5],
        L"실업률",
        FormatPercent(UnemploymentRate * 100.0));
    SetDetailRowData(
        mConflictDetails[6],
        L"야권 지지율",
        FormatPercent(Snapshot.OppositionPercent));
    SetDetailRowData(
        mConflictDetails[7],
        L"재정 압박",
        FormatPercent(FiscalStress * 100.0));

    SetMetricRowData(
        mConflictMetrics[0],
        L"반란 위험",
        FormatPercent(Snapshot.RebelRiskScore),
        static_cast<float>(Clamp01(Snapshot.RebelRiskScore / 100.0)),
        FVector4(0.82f, 0.24f, 0.18f, 0.95f),
        true);
    SetMetricRowData(
        mConflictMetrics[1],
        L"체제 안정도",
        FormatPercent(Stability * 100.0),
        static_cast<float>(Stability),
        FVector4(0.18f, 0.66f, 0.32f, 0.95f));
    SetMetricRowData(
        mConflictMetrics[2],
        L"통제 강도",
        FormatPercent(ControlStrength * 100.0),
        static_cast<float>(ControlStrength),
        FVector4(0.24f, 0.52f, 0.88f, 0.95f));
}

void CAlmanacWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CAlmanacWidget::OnOverviewTabClick()
{
    SelectPage(EAlmanacPage::Overview);
}

void CAlmanacWidget::OnSatisfactionTabClick()
{
    SelectPage(EAlmanacPage::Satisfaction);
}

void CAlmanacWidget::OnPopulationTabClick()
{
    SelectPage(EAlmanacPage::Population);
}

void CAlmanacWidget::OnEconomyTabClick()
{
    SelectPage(EAlmanacPage::Economy);
}

void CAlmanacWidget::OnResourcesTabClick()
{
    SelectPage(EAlmanacPage::Resources);
}

void CAlmanacWidget::OnPoliticsTabClick()
{
    SelectPage(EAlmanacPage::Politics);
}

void CAlmanacWidget::OnForeignTabClick()
{
    SelectPage(EAlmanacPage::Foreign);
}

void CAlmanacWidget::OnBuildingsTabClick()
{
    SelectPage(EAlmanacPage::Buildings);
}

void CAlmanacWidget::OnConflictTabClick()
{
    SelectPage(EAlmanacPage::Conflict);
}
