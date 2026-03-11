#pragma once

#include "AlmanacWidget.h"
#include "../Building/BuildingCategoryInfo.h"
#include "../Politics/PoliticalTypes.h"
#include "TropicoUiStyle.h"
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
    using namespace TropicoUiAssets;

    constexpr float GDataRefreshIntervalSeconds = 0.5f;
    constexpr int GOverviewCardCount = 11;
    constexpr int GOverviewSectionTitleCount = 6;
    constexpr int GSatisfactionRowCount = 9;
    constexpr int GSatisfactionDetailCount = 6;
    constexpr int GSatisfactionGraphGridLineCount = 4;
    constexpr int GSatisfactionGraphPointCount = 4;
    constexpr int GSatisfactionGraphSegmentCount =
        GSatisfactionGraphPointCount - 1;
    constexpr int GPopulationDetailCount = 14;
    constexpr int GPopulationMetricCount = 5;
    constexpr int GPopulationTrendPointCount = 12;
    constexpr int GPopulationTrendSegmentCount =
        GPopulationTrendPointCount - 1;
    constexpr int GPopulationTrendGridLineCount = 5;
    constexpr int GPopulationTrendXAxisLabelCount = 4;
    constexpr int GPopulationTrendYAxisLabelCount = 6;
    constexpr int GPopulationDistributionBarCount = 24;
    constexpr int GPopulationChangeBarCount = 12;
    constexpr int GPopulationChangeGridLineCount = 5;
    constexpr int GPopulationChangeXAxisLabelCount = 4;
    constexpr int GPopulationChangeYAxisLabelCount = 6;
    constexpr int GEconomyDetailCount = 13;
    constexpr int GEconomyMetricCount = 9;
    constexpr int GEconomyTrendBarCount = 24;
    constexpr int GEconomyTrendSegmentCount =
        GEconomyTrendBarCount - 1;
    constexpr int GEconomyTrendGridLineCount = 6;
    constexpr int GEconomyTrendXAxisLabelCount = 5;
    constexpr int GEconomyTrendYAxisLabelCount = 6;
    constexpr int GEconomyChangeBarCount = 24;
    constexpr int GEconomyChangeGridLineCount = 6;
    constexpr int GEconomyChangeYAxisLabelCount = 6;
    constexpr int GEconomyBreakdownRowCount = 10;
    constexpr int GResourceRowCount = 12;
    constexpr int GResourceProductionBarCount = 24;
    constexpr int GResourceProductionGridLineCount = 3;
    constexpr int GResourceProductionXAxisLabelCount = 4;
    constexpr int GResourceProductionYAxisLabelCount = 3;
    constexpr int GResourceDistributionRowCount = 4;
    constexpr int GResourceDetailCount = 4;
    constexpr int GPoliticsFactionTileCount = 8;
    constexpr int GPoliticsNeutralCount = 4;
    constexpr int GPoliticsSupportRowCount = 4;
    constexpr int GPoliticsDetailCount = 14;
    constexpr int GForeignPowerCount = 5;
    constexpr int GForeignDetailCount = 11;
    constexpr int GForeignMetricCount = 4;
    constexpr int GBuildingRowCount = 13;
    constexpr int GBuildingDetailCount = 8;
    constexpr int GConflictDetailCount = 8;
    constexpr int GConflictMetricCount = 3;

    constexpr const TCHAR* GPanelTexture = GMainMenuPanelTexture;
    constexpr const TCHAR* GContentFrameTexture = GMenuGridFrameTexture;
    constexpr const TCHAR* GTitleRibbonTexture = GMenuTitleRibbonTexture;
    constexpr const TCHAR* GTabTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\IconBackground\\T_icon_background.png");
    constexpr const TCHAR* GTabMarkerTexture = GDropdownArrowTexture;
    constexpr const TCHAR* GRowTexture = GBigTextButtonTexture;
    constexpr const TCHAR* GRowTextureSelected = GBigTextButtonSelectedTexture;
    constexpr const TCHAR* GCardTexture = GSlotCardTexture;
    constexpr const TCHAR* GCardTextureSelected = GSlotCardSelectedTexture;
    constexpr const TCHAR* GRailTrackTexture = GScrollTrackTexture;
    constexpr const TCHAR* GRailThumbTexture = GScrollThumbTexture;
    constexpr const TCHAR* GBarBackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar_bg.png");
    constexpr const TCHAR* GBarFillTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Gamespeed\\T_gamespeed_timeBar.png");
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

    constexpr const TCHAR* GSatisfactionIcons[GSatisfactionRowCount] =
    {
        GApprovalIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_restaurant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_hospital.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_funFairPier.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_church.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png")
    };

    constexpr const TCHAR* GPoliticsFactionIcons[GPoliticsFactionTileCount] =
    {
        GMoneyIcon,
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Constitution.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_church.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\HudIcons\\T_ICO_Raids.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_highschool.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_political_approval.png")
    };

    constexpr const TCHAR* GForeignPowerIcons[GForeignPowerCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_china.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_russia.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_flags_us.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_almanac_middleEast.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FlagIcons\\T_ICO_flags_eu.png")
    };

    constexpr const wchar_t* GSatisfactionTrendLabels[GSatisfactionGraphPointCount] =
    {
        L"3년전",
        L"2년전",
        L"1년전",
        L"현재 연도"
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

    std::wstring FormatCompactCurrency(long long Value)
    {
        const bool Negative = Value < 0;
        const double AbsValue =
            static_cast<double>(Negative ? -Value : Value);

        std::wstring Prefix = Negative ? L"-$" : L"$";

        if (AbsValue >= 1000000.0)
        {
            const double InMillions = AbsValue / 1000000.0;
            wchar_t Buffer[32] = {};
            swprintf_s(Buffer, L"%.2f", InMillions);
            return Prefix + Buffer + L"백만";
        }

        return FormatCurrency(Value);
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
    void ConfigureAuxiliaryText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardTitleText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardValueText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureCardDetailText(const std::shared_ptr<CTextBlock>& Text);
    void ConfigureFrameImage(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName,
        const TCHAR* TextureFile);
    void ConfigureRowBackground(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName);
    void ApplySelectableBackground(
        const std::shared_ptr<CImage>& Image,
        bool Highlight,
        bool CardStyle = false);
    void ConfigureMetricBar(const std::shared_ptr<CProgressBar>& Bar);
    void ConfigureSatisfactionRowButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected);
    void ConfigureSatisfactionRowBar(const std::shared_ptr<CProgressBar>& Bar);
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
    void SetSatisfactionRowData(
        const CAlmanacWidget::FSatisfactionRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Selected);
    void SetDetailRowData(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        bool Highlight = false,
        const FVector4& ValueColor = FVector4(0.31f, 0.27f, 0.21f, 1.f));
    void SetLineSegment(
        const std::shared_ptr<CImage>& Segment,
        float X0,
        float Y0,
        float X1,
        float Y1,
        float Thickness,
        const FVector4& Tint);

}

namespace
{
    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(30.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(96, 73, 32, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(246, 234, 192, 170);
    }

    void ConfigureSectionText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(121, 92, 42, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(247, 241, 220, 140);
    }

    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(76, 70, 60, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 248, 238, 96);
    }

    void ConfigureBodyValueText(
        const std::shared_ptr<CTextBlock>& Text,
        unsigned char r,
        unsigned char g,
        unsigned char b)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(r, g, b, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 248, 238, 92);
    }

    void ConfigureNoticeText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(13.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(120, 108, 91, 255);
    }

    void ConfigureAuxiliaryText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(13.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(136, 123, 98, 255);
    }

    void ConfigureCardTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(103, 79, 39, 255);
    }

    void ConfigureCardValueText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(24.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(63, 59, 51, 255);
    }

    void ConfigureCardDetailText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(125, 112, 92, 255);
    }

    void ConfigureFrameImage(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName,
        const TCHAR* TextureFile)
    {
        if (!Image || !TextureFile)
            return;

        Image->SetTexture(TextureName, TextureFile);
        Image->SetTint(1.f, 1.f, 1.f, 1.f);
    }

    void ConfigureRowBackground(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName)
    {
        ConfigureFrameImage(Image, TextureName, GRowTexture);
        ApplySelectableBackground(Image, false);
    }

    void ApplySelectableBackground(
        const std::shared_ptr<CImage>& Image,
        bool Highlight,
        bool CardStyle)
    {
        if (!Image)
            return;

        const TCHAR* TextureFile = nullptr;

        if (CardStyle)
            TextureFile = Highlight ? GCardTextureSelected : GCardTexture;
        else
            TextureFile = Highlight ? GRowTextureSelected : GRowTexture;

        Image->SetTexture(
            Image->GetName() + (Highlight ? "_sel" : "_base"),
            TextureFile);
        Image->SetTint(
            Highlight ? FVector4(1.f, 0.98f, 0.88f, 1.f) :
                FVector4(1.f, 1.f, 1.f, 0.94f));
    }

    void ConfigureMetricBar(const std::shared_ptr<CProgressBar>& Bar)
    {
        if (!Bar)
            return;

        Bar->SetTexture(
            EProgressBarImageType::Back,
            "Almanac_BarBack",
            GBarBackTexture);
        Bar->SetTexture(
            EProgressBarImageType::Fill,
            "Almanac_BarFill",
            GBarFillTexture);
        Bar->SetTint(EProgressBarImageType::Back,
            FVector4(0.92f, 0.86f, 0.66f, 0.45f));
        Bar->SetTint(EProgressBarImageType::Fill,
            FVector4(0.90f, 0.72f, 0.18f, 0.95f));
        Bar->SetBarDir(EProgressBarDir::RightToLeft);
        Bar->SetPercent(0.f);
    }

    void ConfigureSatisfactionRowButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            TropicoUiStyle::ApplyButtonTextureSet(
                Button,
                Button->GetName() + "_selected",
                GBigTextButtonSelectedTexture,
                GBigTextButtonSelectedTexture,
                GBigTextButtonSelectedTexture,
                GBigTextButtonDisabledTexture);
            Button->SetTint(EButtonState::Normal,
                FVector4(1.02f, 0.98f, 0.70f, 0.98f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.04f, 1.00f, 0.78f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.98f, 0.94f, 0.62f, 0.98f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.64f, 0.62f, 0.46f, 0.72f));
            return;
        }

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_normal",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonDisabledTexture);
        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.02f, 1.01f, 0.98f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.96f, 0.94f, 0.90f, 0.96f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.62f, 0.62f, 0.62f, 0.70f));
    }

    void ConfigurePoliticsFactionButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            TropicoUiStyle::ApplyButtonTextureSet(
                Button,
                Button->GetName() + "_selected",
                GCardTextureSelected,
                GCardTextureSelected,
                GCardTextureSelected,
                GCardTexture);
            Button->SetTint(EButtonState::Normal,
                FVector4(1.04f, 0.98f, 0.72f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.05f, 1.00f, 0.80f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.98f, 0.92f, 0.62f, 0.98f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.68f, 0.64f, 0.48f, 0.72f));
            return;
        }

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_normal",
            GCardTexture,
            GCardTextureSelected,
            GCardTextureSelected,
            GCardTexture);
        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.02f, 1.00f, 0.96f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.96f, 0.94f, 0.88f, 0.96f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.62f, 0.62f, 0.62f, 0.70f));
    }

    void ConfigureSatisfactionRowBar(const std::shared_ptr<CProgressBar>& Bar)
    {
        ConfigureMetricBar(Bar);

        if (!Bar)
            return;

        Bar->SetTint(EProgressBarImageType::Back,
            FVector4(0.82f, 0.76f, 0.58f, 0.14f));
        Bar->SetTint(EProgressBarImageType::Fill,
            FVector4(0.92f, 0.78f, 0.24f, 0.18f));
    }

    void ConfigureTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        ApplyTextureToAllButtonStates(
            Button,
            Selected ? Button->GetName() + "_selected" :
                Button->GetName() + "_normal",
            GTabTexture);

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(1.08f, 1.02f, 0.84f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.10f, 1.06f, 0.90f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.94f, 0.86f, 0.62f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.60f, 0.60f, 0.60f, 0.70f));
            return;
        }

        Button->SetTint(EButtonState::Normal,
            FVector4(0.84f, 0.90f, 0.97f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.94f, 0.98f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.76f, 0.84f, 0.92f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.50f, 0.50f, 0.50f, 0.70f));
    }

    void ConfigureCloseButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_close",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        TropicoUiStyle::ConfigureIconSlotButtonStyle(Button);
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

        ApplySelectableBackground(Background, Highlight, true);
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

        ApplySelectableBackground(Background, Highlight);
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

    void SetSatisfactionRowData(
        const CAlmanacWidget::FSatisfactionRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Selected)
    {
        auto Button = Row.Button.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();
        auto Icon = Row.Icon.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Selected);
            Button->ButtonEnable(true);
        }

        if (Icon)
        {
            Icon->SetTint(
                Selected ?
                    FVector4(1.06f, 1.02f, 0.96f, 1.f) :
                    FVector4(0.98f, 0.98f, 0.98f, 1.f));
        }

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Selected ? 84 : 78,
                Selected ? 64 : 68,
                Selected ? 22 : 54,
                255);
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Selected ? 96 : 78,
                Selected ? 62 : 68,
                Selected ? 8 : 54,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Clamp01(Percent));
            Bar->SetTint(
                EProgressBarImageType::Fill,
                Selected ?
                    FVector4(0.98f, 0.80f, 0.12f, 0.98f) :
                    FillTint);
            Bar->SetEnable(false);
        }
    }

    void SetPoliticsFactionTileData(
        const CAlmanacWidget::FPoliticsFactionTileWidgets& Tile,
        const TCHAR* IconTexture,
        const FVector4& IconTint,
        const std::wstring& Label,
        int Count,
        int Favor,
        bool Selected)
    {
        auto Button = Tile.Button.lock();
        auto Icon = Tile.Icon.lock();
        auto LabelText = Tile.Label.lock();
        auto CountIcon = Tile.CountIcon.lock();
        auto CountValue = Tile.CountValue.lock();
        auto FavorIcon = Tile.FavorIcon.lock();
        auto FavorValue = Tile.FavorValue.lock();

        if (Button)
        {
            ConfigurePoliticsFactionButtonStyle(Button, Selected);
            Button->ButtonEnable(true);
        }

        if (Icon && IconTexture)
        {
            Icon->SetTexture(Icon->GetName() + "_texture", IconTexture);
            Icon->SetTint(IconTint);
        }

        if (CountIcon)
            CountIcon->SetTint(0.88f, 0.70f, 0.18f, 0.96f);

        if (FavorIcon)
            FavorIcon->SetTint(0.92f, 0.68f, 0.16f, 0.96f);

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Selected ? 86 : 74,
                Selected ? 68 : 70,
                Selected ? 18 : 58,
                255);
        }

        if (CountValue)
        {
            CountValue->SetText(std::to_wstring(Count).c_str());
            CountValue->SetTextColor(
                Selected ? 96 : 104,
                Selected ? 70 : 95,
                Selected ? 18 : 64,
                255);
        }

        if (FavorValue)
        {
            FavorValue->SetText(std::to_wstring(Favor).c_str());
            FavorValue->SetTextColor(
                Selected ? 102 : 121,
                Selected ? 66 : 92,
                Selected ? 6 : 28,
                255);
        }
    }

    void SetForeignPowerRowData(
        const CAlmanacWidget::FSatisfactionRowWidgets& Row,
        const TCHAR* IconTexture,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        bool Selected)
    {
        auto Button = Row.Button.lock();
        auto Icon = Row.Icon.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Selected);
            Button->ButtonEnable(true);
        }

        if (Icon && IconTexture)
        {
            Icon->SetTexture(Icon->GetName() + "_texture", IconTexture);
            Icon->SetTint(1.f, 1.f, 1.f, 1.f);
        }

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Selected ? 84 : 78,
                Selected ? 64 : 68,
                Selected ? 22 : 54,
                255);
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Selected ? 96 : 74,
                Selected ? 62 : 96,
                Selected ? 8 : 128,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Clamp01(Percent));
            Bar->SetTint(
                EProgressBarImageType::Back,
                FVector4(0.78f, 0.80f, 0.74f, 0.24f));
            Bar->SetTint(
                EProgressBarImageType::Fill,
                FVector4(0.32f, 0.60f, 0.90f, 0.96f));
            Bar->SetEnable(false);
        }
    }

    void SetDetailRowData(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        bool Highlight,
        const FVector4& ValueColor)
    {
        auto Button = Row.Button.lock();
        auto Background = Row.Background.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Highlight);
            Button->ButtonEnable(true);
        }
        else if (Background)
            ApplySelectableBackground(Background, Highlight);

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            if (Button)
            {
                LabelText->SetTextColor(
                    Highlight ? 84 : (Value.empty() ? 92 : 76),
                    Highlight ? 64 : (Value.empty() ? 84 : 70),
                    Highlight ? 22 : (Value.empty() ? 66 : 60),
                    255);
            }
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Button && Highlight ? 96 :
                    static_cast<unsigned char>(ValueColor.x * 255.f),
                Button && Highlight ? 62 :
                    static_cast<unsigned char>(ValueColor.y * 255.f),
                Button && Highlight ? 8 :
                    static_cast<unsigned char>(ValueColor.z * 255.f),
                255);
        }
    }

    void SetLineSegment(
        const std::shared_ptr<CImage>& Segment,
        float X0,
        float Y0,
        float X1,
        float Y1,
        float Thickness,
        const FVector4& Tint)
    {
        if (!Segment)
            return;

        const float DX = X1 - X0;
        const float DY = Y1 - Y0;
        const float Length = std::sqrt(DX * DX + DY * DY);

        if (Length <= 0.5f)
        {
            Segment->SetEnable(false);
            return;
        }

        Segment->SetEnable(true);
        Segment->SetPivot(0.f, 0.5f);
        Segment->SetPos(X0, Y0);
        Segment->SetSize(Length, Thickness);
        Segment->SetAngle(static_cast<float>(
            std::atan2(DY, DX) * 180.0 / 3.14159265358979323846));
        Segment->SetTint(Tint);
    }
}
