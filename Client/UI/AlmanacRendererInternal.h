#pragma once

#include "AlmanacWidget.h"
#include "../Building/BuildingCategoryInfo.h"
#include "../Politics/PoliticalTypes.h"
#include "TropicoUiTheme.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace
{
    constexpr float GDataRefreshIntervalSeconds = 0.5f;
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
    constexpr int GBuildingRowCount =
        BuildingCategoryInfo::GBuildingCategoryCount;
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
            return TropicoUiTheme::GStatusDangerTint;
        if (Score >= 0.52)
            return TropicoUiTheme::GStatusWarningTint;
        if (Score >= 0.32)
            return TropicoUiTheme::GStatusCautionTint;
        return TropicoUiTheme::GStatusSuccessTint;
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
