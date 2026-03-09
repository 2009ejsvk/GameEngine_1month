#include "BuildMenuWidget.h"
#include "AlmanacWidget.h"
#include "AlmanacDataProvider.h"
#include "TropicoUiStyle.h"
#include "../Map/PlacementAreaObject.h"
#include "EdictWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementController.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorldAccess.h"
#include "../ObjectNames.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <string>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

    constexpr int CategoryCount = 8;
    constexpr int SlotsPerPage = 15;
    constexpr int SlotColumnCount = 5;
    constexpr int SlotRowCount = 3;
    constexpr const TCHAR* GBuildMenuPanelTexture = GMainMenuPanelTexture;
    constexpr const TCHAR* GYearbookPanelTexture = GModernPanelTexture;
    constexpr const TCHAR* GBlueprintCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_blueprint_cost.png");
    constexpr const TCHAR* GConstructionCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");

    struct FParsedDetailInfo
    {
        std::wstring BlueprintCost;
        std::wstring ConstructionCost;
        std::vector<std::wstring> Highlights;
        std::wstring Description;
    };

    EPlacementTemplateType ResolveTemplateTypeByBuildingId(
        const std::string& BuildingId)
    {
        // 건물별 배치 크기/형태를 한 곳에서 관리하기 위한 룰 테이블.
        // 필요한 건물 ID를 여기에 추가하면 된다.
        struct FTemplateRule
        {
            const char* BuildingId;
            EPlacementTemplateType TemplateType;
        };

        static const std::vector<FTemplateRule> GRules =
        {
            // 예시:
            // { "build_1_3", EPlacementTemplateType::Diamond5x5TwoMarker },
            // { "build_4_3", EPlacementTemplateType::Diamond5x5FourMarker },
            // { "build_7_5", EPlacementTemplateType::Diamond7x7ThreeMarker },
        };

        for (const FTemplateRule& Rule : GRules)
        {
            if (BuildingId == Rule.BuildingId)
                return Rule.TemplateType;
        }

        // 룰이 없으면 기존과 동일하게 3x3 템플릿 사용.
        return EPlacementTemplateType::Diamond3x3SingleMarker;
    }

    const wchar_t* CategoryLabels[CategoryCount] =
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

    const TCHAR* const GCategoryTabIcons[CategoryCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_infrastructure.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_raw_resources.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_industry.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_housing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_entertainment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_mediaEducation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_tourism.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingCategories\\T_ICO_publicServices.png")
    };
    bool StartsWith(
        const std::wstring& Text,
        const wchar_t* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = wcslen(Prefix);
        return Text.size() >= PrefixLength &&
            Text.compare(0, PrefixLength, Prefix) == 0;
    }

    std::wstring TrimCopy(const std::wstring& Text)
    {
        size_t Begin = 0;
        size_t End = Text.size();

        while (Begin < End && iswspace(Text[Begin]))
            ++Begin;

        while (End > Begin && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Begin, End - Begin);
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        size_t Start = 0;

        while (Start <= Text.size())
        {
            const size_t End = Text.find(L'\n', Start);

            if (End == std::wstring::npos)
            {
                Lines.push_back(Text.substr(Start));
                break;
            }

            Lines.push_back(Text.substr(Start, End - Start));
            Start = End + 1;
        }

        return Lines;
    }

    std::wstring JoinLines(const std::vector<std::wstring>& Lines)
    {
        std::wstring Result;

        for (size_t i = 0; i < Lines.size(); ++i)
        {
            if (Lines[i].empty())
                continue;

            if (!Result.empty())
                Result += L"\n";

            Result += Lines[i];
        }

        return Result;
    }

    std::wstring BuildHighlightsBlockText(
        const std::vector<std::wstring>& Highlights)
    {
        std::wstring Result = L"핵심 정보";

        if (Highlights.empty())
        {
            Result += L"\n- 준비 중";
            return Result;
        }

        for (size_t i = 0; i < Highlights.size(); ++i)
        {
            Result += L"\n- ";
            Result += Highlights[i];
        }

        return Result;
    }

    void AddHighlightFallbacks(
        const FBuildingCatalogEntry& Entry,
        std::vector<std::wstring>& Highlights)
    {
        if (Highlights.size() < 3)
        {
            Highlights.push_back(
                L"해금 시대: " +
                std::wstring(GetBuildingEraDisplayName(Entry.UnlockEra)));
        }

        if (Highlights.size() < 3 &&
            Entry.RequiredEducationLevel !=
                ECitizenEducationLevel::Uneducated)
        {
            Highlights.push_back(
                L"필요 학력: " +
                std::wstring(GetCitizenEducationDisplayName(
                    Entry.RequiredEducationLevel)));
        }

        if (Highlights.size() < 3 &&
            Entry.ProducedResourceType != EResourceType::None)
        {
            Highlights.push_back(
                L"생산 자원: " +
                std::wstring(GetResourceTypeDisplayName(
                    Entry.ProducedResourceType)));
        }

        if (Entry.Residential && Highlights.size() < 3 &&
            Entry.HousingSatisfactionCap > 0)
        {
            Highlights.push_back(
                L"주거 품질: " +
                std::to_wstring(Entry.HousingSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.JobSatisfactionCap > 0 &&
            Entry.JobSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"직업 품질: " +
                std::to_wstring(Entry.JobSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.FoodSatisfactionCap > 0 &&
            Entry.FoodSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"음식 품질: " +
                std::to_wstring(Entry.FoodSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.FunSatisfactionCap > 0 &&
            Entry.FunSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"서비스 품질: " +
                std::to_wstring(Entry.FunSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.Capacity > 0)
        {
            Highlights.push_back(
                L"수용 인원: " +
                std::to_wstring(Entry.Capacity));
        }
    }

    FParsedDetailInfo ParseDetailInfo(
        const FBuildingCatalogEntry& Entry)
    {
        FParsedDetailInfo Result;
        std::vector<std::wstring> DescriptionLines;
        const std::vector<std::wstring> Lines = SplitLines(Entry.DetailText);
        static const wchar_t* HighlightPrefixes[] =
        {
            L"주거 품질:",
            L"직업 품질:",
            L"음식 품질:",
            L"오락 품질:",
            L"서비스 품질:",
            L"수용 인원:",
            L"수용 가구:",
            L"생산 전력:",
            L"발전량:",
            L"필요 전력:",
            L"방문객:",
            L"미관:",
            L"효과:"
        };

        for (size_t i = 0; i < Lines.size(); ++i)
        {
            const std::wstring Line = TrimCopy(Lines[i]);

            if (Line.empty())
                continue;

            if (StartsWith(Line, L"설계도 비용:"))
            {
                Result.BlueprintCost =
                    TrimCopy(Line.substr(wcslen(L"설계도 비용:")));
                continue;
            }

            if (StartsWith(Line, L"건설 비용:"))
            {
                Result.ConstructionCost =
                    TrimCopy(Line.substr(wcslen(L"건설 비용:")));
                continue;
            }

            bool HighlightLine = false;

            for (size_t PrefixIndex = 0;
                PrefixIndex < sizeof(HighlightPrefixes) / sizeof(HighlightPrefixes[0]);
                ++PrefixIndex)
            {
                if (!StartsWith(Line, HighlightPrefixes[PrefixIndex]))
                    continue;

                if (Result.Highlights.size() < 3)
                    Result.Highlights.push_back(Line);

                HighlightLine = true;
                break;
            }

            if (!HighlightLine)
                DescriptionLines.push_back(Line);
        }

        AddHighlightFallbacks(Entry, Result.Highlights);
        Result.Description = JoinLines(DescriptionLines);

        if (Result.Description.empty())
            Result.Description = L"세부 데이터 준비 중";

        return Result;
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
}

CBuildMenuWidget::CBuildMenuWidget()
{
}

CBuildMenuWidget::~CBuildMenuWidget()
{
}

bool CBuildMenuWidget::Init()
{
    CWidgetContainer::Init();

    mVisibleEntryIndices.assign(SlotsPerPage, -1);

    auto BuildButton = CreateWidget<CButton>("BuildMenu_BuildButton", 10).lock();

    if (BuildButton)
    {
        ApplyButtonTextureSet(
            BuildButton,
            "BuildMenuOpenButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureIconSlotButtonStyle(BuildButton);
        BuildButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnBuildButtonClick);

        auto BuildButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_BuildButtonText", mWorld);

        if (BuildButtonText)
        {
            BuildButtonText->SetText(TEXT("건축"));
            BuildButtonText->SetFontSize(24.f);
            BuildButtonText->SetAlignH(ETextAlignH::Center);
            BuildButtonText->SetAlignV(ETextAlignV::Middle);
            BuildButtonText->SetTextColor(95, 68, 18, 255);
            BuildButtonText->EnableShadow(true);
            BuildButtonText->SetShadowOffset(1.f, 1.f);
            BuildButtonText->SetShadowTextColor(246, 225, 170, 170);
            BuildButton->SetChild(BuildButtonText);
        }

        mBuildButton = BuildButton;
    }

    auto YearbookButton =
        CreateWidget<CButton>("BuildMenu_YearbookButton", 10).lock();

    if (YearbookButton)
    {
        ApplyButtonTextureSet(
            YearbookButton,
            "BuildMenuYearbookButton",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureIconSlotButtonStyle(YearbookButton);
        YearbookButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnYearbookButtonClick);

        auto YearbookButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_YearbookButtonText", mWorld);

        if (YearbookButtonText)
        {
            YearbookButtonText->SetText(TEXT("연감"));
            YearbookButtonText->SetFontSize(24.f);
            YearbookButtonText->SetAlignH(ETextAlignH::Center);
            YearbookButtonText->SetAlignV(ETextAlignV::Middle);
            YearbookButtonText->SetTextColor(95, 68, 18, 255);
            YearbookButtonText->EnableShadow(true);
            YearbookButtonText->SetShadowOffset(1.f, 1.f);
            YearbookButtonText->SetShadowTextColor(246, 225, 170, 170);
            YearbookButton->SetChild(YearbookButtonText);
            mYearbookButtonText = YearbookButtonText;
        }

        mYearbookButton = YearbookButton;
    }

    auto NpcCountText = CreateWidget<CTextBlock>("BuildMenu_NpcCount", 11).lock();

    if (NpcCountText)
    {
        NpcCountText->SetText(TEXT("NPC: 0"));
        NpcCountText->SetFontSize(20.f);
        NpcCountText->SetAlignH(ETextAlignH::Left);
        NpcCountText->SetAlignV(ETextAlignV::Middle);
        NpcCountText->SetTextColor(245, 245, 245, 255);
        NpcCountText->EnableShadow(true);
        NpcCountText->SetShadowOffset(1.f, 1.f);
        NpcCountText->SetShadowTextColor(20, 20, 20, 255);
        mNpcCountText = NpcCountText;
    }

    auto BudgetText = CreateWidget<CTextBlock>(
        "BuildMenu_NationalBudget", 11).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("국가 예산: $0"));
        BudgetText->SetFontSize(20.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 245, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(20, 20, 20, 255);
        mBudgetText = BudgetText;
    }

    auto DateText = CreateWidget<CTextBlock>(
        "BuildMenu_SimulationDate", 11).lock();

    if (DateText)
    {
        DateText->SetText(TEXT("날짜: 2000-01-01"));
        DateText->SetFontSize(20.f);
        DateText->SetAlignH(ETextAlignH::Left);
        DateText->SetAlignV(ETextAlignV::Middle);
        DateText->SetTextColor(220, 235, 255, 255);
        DateText->EnableShadow(true);
        DateText->SetShadowOffset(1.f, 1.f);
        DateText->SetShadowTextColor(20, 20, 20, 255);
        mDateText = DateText;
    }

    auto DayProgressText = CreateWidget<CTextBlock>(
        "BuildMenu_DayProgressText", 11).lock();

    if (DayProgressText)
    {
        DayProgressText->SetText(TEXT("월 진행: 0%"));
        DayProgressText->SetFontSize(16.f);
        DayProgressText->SetAlignH(ETextAlignH::Left);
        DayProgressText->SetAlignV(ETextAlignV::Middle);
        DayProgressText->SetTextColor(220, 220, 220, 255);
        DayProgressText->EnableShadow(true);
        DayProgressText->SetShadowOffset(1.f, 1.f);
        DayProgressText->SetShadowTextColor(20, 20, 20, 255);
        mDayProgressText = DayProgressText;
    }

    auto DayProgressBar = CreateWidget<CProgressBar>(
        "BuildMenu_DayProgressBar", 10).lock();

    if (DayProgressBar)
    {
        DayProgressBar->SetTint(
            EProgressBarImageType::Back,
            FVector4(0.08f, 0.08f, 0.10f, 0.85f));
        DayProgressBar->SetTint(
            EProgressBarImageType::Fill,
            FVector4(0.18f, 0.62f, 0.34f, 0.95f));
        DayProgressBar->SetPercent(0.f);
        DayProgressBar->SetBarDir(EProgressBarDir::RightToLeft);
        mDayProgressBar = DayProgressBar;
    }

    auto YearbookPanel = CreateWidget<CImage>("BuildMenu_YearbookPanel", 12).lock();

    if (YearbookPanel)
    {
        YearbookPanel->SetTexture("BuildMenuYearbookBackground",
            GYearbookPanelTexture);
        YearbookPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mYearbookPanel = YearbookPanel;
    }

    auto YearbookTitleText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookTitle", 13).lock();

    if (YearbookTitleText)
    {
        YearbookTitleText->SetText(TEXT("연감"));
        YearbookTitleText->SetFontSize(24.f);
        YearbookTitleText->SetAlignH(ETextAlignH::Left);
        YearbookTitleText->SetAlignV(ETextAlignV::Middle);
        YearbookTitleText->SetTextColor(245, 245, 245, 255);
        YearbookTitleText->EnableShadow(true);
        YearbookTitleText->SetShadowOffset(1.f, 1.f);
        YearbookTitleText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookTitleText = YearbookTitleText;
    }

    auto YearbookCloseButton =
        CreateWidget<CButton>("BuildMenu_YearbookClose", 13).lock();

    if (YearbookCloseButton)
    {
        ApplyButtonTextureSet(
            YearbookCloseButton,
            "BuildMenuYearbookClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(YearbookCloseButton);
        YearbookCloseButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnYearbookCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_YearbookCloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(22.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(95, 68, 18, 255);
            CloseText->EnableShadow(true);
            CloseText->SetShadowOffset(1.f, 1.f);
            CloseText->SetShadowTextColor(246, 219, 117, 150);
            YearbookCloseButton->SetChild(CloseText);
        }

        mYearbookCloseButton = YearbookCloseButton;
    }

    auto YearbookBodyText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookBody", 13).lock();

    if (YearbookBodyText)
    {
        YearbookBodyText->SetText(
            TEXT("종합 만족도: -\n"
                "음식: -\n"
                "보건: -\n"
                "유흥: -\n"
                "신앙: -\n"
                "주거: -\n"
                "직업: -\n"
                "자유: -\n"
                "치안: -\n"
                "무주택자 수: 0명\n"
                "실업자 수: 0명"));
        YearbookBodyText->SetFontSize(14.f);
        YearbookBodyText->SetAlignH(ETextAlignH::Left);
        YearbookBodyText->SetAlignV(ETextAlignV::Top);
        YearbookBodyText->SetTextColor(225, 225, 225, 255);
        YearbookBodyText->EnableShadow(true);
        YearbookBodyText->SetShadowOffset(1.f, 1.f);
        YearbookBodyText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookBodyText = YearbookBodyText;
    }

    auto MenuBackground = CreateWidget<CImage>("BuildMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("BuildMenuBackground",
            GBuildMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuBackground = MenuBackground;
    }

    auto MenuTitleRibbon =
        CreateWidget<CImage>("BuildMenu_TitleRibbon", 7).lock();

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetTexture(
            "BuildMenuTitleRibbonTexture",
            GMenuTitleRibbonTexture);
        MenuTitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuTitleRibbon = MenuTitleRibbon;
    }

    auto MenuGridFrame =
        CreateWidget<CImage>("BuildMenu_GridFrame", 7).lock();

    if (MenuGridFrame)
    {
        MenuGridFrame->SetTexture(
            "BuildMenuGridFrameTexture",
            GMenuGridFrameTexture);
        MenuGridFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        mMenuGridFrame = MenuGridFrame;
    }

    auto MenuDetailFrame =
        CreateWidget<CImage>("BuildMenu_DetailFrame", 7).lock();

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetTexture(
            "BuildMenuDetailFrameTexture",
            GMenuDetailFrameTexture);
        MenuDetailFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        mMenuDetailFrame = MenuDetailFrame;
    }

    auto DetailInfoPanel =
        CreateWidget<CImage>("BuildMenu_DetailInfoPanel", 7).lock();

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetTexture(
            "BuildMenuDetailInfoPanelTexture",
            GDetailInfoPanelTexture);
        DetailInfoPanel->SetTint(1.f, 1.f, 1.f, 0.96f);
        mDetailInfoPanel = DetailInfoPanel;
    }

    auto ScrollTrack =
        CreateWidget<CImage>("BuildMenu_ScrollTrack", 7).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "BuildMenuScrollTrackTexture",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 0.95f);
        mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb =
        CreateWidget<CImage>("BuildMenu_ScrollThumb", 8).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "BuildMenuScrollThumbTexture",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 1.f);
        mScrollThumb = ScrollThumb;
    }

    auto TitleText = CreateWidget<CTextBlock>("BuildMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(CategoryLabels[static_cast<int>(EBuildingCategory::Infrastructure)]);
        TitleText->SetFontSize(32.f);
        TitleText->SetAlignH(ETextAlignH::Center);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(78, 60, 28, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(235, 220, 180, 180);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("BuildMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(16.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(102, 82, 46, 255);
        PageText->EnableShadow(true);
        PageText->SetShadowOffset(1.f, 1.f);
        PageText->SetShadowTextColor(235, 220, 180, 180);
        mPageText = PageText;
    }

    auto MenuCloseButton = CreateWidget<CButton>("BuildMenu_Close", 7).lock();

    if (MenuCloseButton)
    {
        ApplyButtonTextureSet(
            MenuCloseButton,
            "BuildMenuClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(MenuCloseButton);
        MenuCloseButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnMenuCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_CloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(22.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(95, 68, 18, 255);
            CloseText->EnableShadow(true);
            CloseText->SetShadowOffset(1.f, 1.f);
            CloseText->SetShadowTextColor(246, 219, 117, 150);
            MenuCloseButton->SetChild(CloseText);
        }

        mMenuCloseButton = MenuCloseButton;
    }

    auto PrevPageButton = CreateWidget<CButton>("BuildMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ApplyButtonTextureSet(
            PrevPageButton,
            "BuildMenuPrevPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(PrevPageButton);
        PrevPageButton->SetAngle(180.f);
        PrevPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnPrevPageClick);

        mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton = CreateWidget<CButton>("BuildMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ApplyButtonTextureSet(
            NextPageButton,
            "BuildMenuNextPage",
            GDropdownArrowTexture,
            GDropdownArrowHoverTexture,
            GDropdownArrowTexture,
            GDropdownArrowTexture);
        ConfigureIconSlotButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnNextPageClick);

        mNextPageButton = NextPageButton;
    }

    mCategoryButtons.resize(CategoryCount);
    mCategoryButtonIcons.resize(CategoryCount);

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Category_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "BuildMenuCategoryTab_" + std::to_string(i),
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [this, i]()
            {
                SelectCategory(static_cast<EBuildingCategory>(i));
            });

        auto CategoryIcon = CWidget::CreateStaticWidget<CImage>(
            "BuildMenu_CategoryIcon_" + std::to_string(i + 1), mWorld);

        if (CategoryIcon)
        {
            CategoryIcon->SetTexture(
                "BuildMenuCategoryIconTex_" + std::to_string(i + 1),
                GCategoryTabIcons[i]);
            CategoryIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            Button->SetChild(CategoryIcon);
            mCategoryButtonIcons[i] = CategoryIcon;
        }

        mCategoryButtons[i] = Button;
    }

    mBuildingButtons.resize(SlotsPerPage);
    mBuildingButtonIcons.resize(SlotsPerPage);
    mBuildingButtonTexts.resize(SlotsPerPage);

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "BuildMenuSlotCard_" + std::to_string(i),
            GSlotCardTexture,
            GSlotCardHoverTexture,
            GSlotCardSelectedTexture,
            GSlotCardDisabledTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [this, i]()
            {
                StartPlacementBySlot(i);
            });
        Button->SetEventCallback(
            EButtonEventState::Hovered,
            [this, i]()
            {
                PreviewSlot(i);
            });

        auto SlotContent =
            CWidget::CreateStaticWidget<CWidgetContainer>(
                "BuildMenu_SlotContent_" + std::to_string(i + 1),
                mWorld);
        auto SlotIcon = CWidget::CreateStaticWidget<CImage>(
            "BuildMenu_SlotIcon_" + std::to_string(i + 1), mWorld);
        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (SlotContent && SlotIcon && ButtonText)
        {
            SlotIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            SlotContent->AddWidget(SlotIcon);

            ButtonText->SetText(TEXT(""));
            ButtonText->SetFontSize(14.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Bottom);
            ButtonText->SetTextColor(58, 47, 31, 255);
            ButtonText->EnableShadow(true);
            ButtonText->SetShadowOffset(1.f, 1.f);
            ButtonText->SetShadowTextColor(240, 229, 205, 190);
            SlotContent->AddWidget(ButtonText);
            Button->SetChild(SlotContent);
            mBuildingButtonIcons[i] = SlotIcon;
        }

        mBuildingButtons[i] = Button;
        mBuildingButtonTexts[i] = ButtonText;
    }

    auto DetailTitleText =
        CreateWidget<CTextBlock>("BuildMenu_DetailTitle", 7).lock();

    if (DetailTitleText)
    {
        DetailTitleText->SetText(TEXT("건물 정보"));
        DetailTitleText->SetFontSize(26.f);
        DetailTitleText->SetAlignH(ETextAlignH::Left);
        DetailTitleText->SetAlignV(ETextAlignV::Middle);
        DetailTitleText->SetTextColor(100, 72, 28, 255);
        DetailTitleText->EnableShadow(true);
        DetailTitleText->SetShadowOffset(1.f, 1.f);
        DetailTitleText->SetShadowTextColor(245, 235, 205, 180);
        mDetailTitleText = DetailTitleText;
    }

    auto DetailBlueprintIcon =
        CreateWidget<CImage>("BuildMenu_DetailBlueprintIcon", 8).lock();

    if (DetailBlueprintIcon)
    {
        DetailBlueprintIcon->SetTexture(
            "BuildMenu_DetailBlueprintIconTexture",
            GBlueprintCostIconTexture);
        DetailBlueprintIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailBlueprintIcon = DetailBlueprintIcon;
    }

    auto DetailBlueprintCostText =
        CreateWidget<CTextBlock>("BuildMenu_DetailBlueprintCost", 8).lock();

    if (DetailBlueprintCostText)
    {
        DetailBlueprintCostText->SetText(TEXT("-"));
        DetailBlueprintCostText->SetFontSize(18.f);
        DetailBlueprintCostText->SetAlignH(ETextAlignH::Left);
        DetailBlueprintCostText->SetAlignV(ETextAlignV::Middle);
        DetailBlueprintCostText->SetTextColor(62, 116, 204, 255);
        DetailBlueprintCostText->EnableShadow(true);
        DetailBlueprintCostText->SetShadowOffset(1.f, 1.f);
        DetailBlueprintCostText->SetShadowTextColor(240, 240, 240, 170);
        mDetailBlueprintCostText = DetailBlueprintCostText;
    }

    auto DetailConstructionIcon =
        CreateWidget<CImage>("BuildMenu_DetailConstructionIcon", 8).lock();

    if (DetailConstructionIcon)
    {
        DetailConstructionIcon->SetTexture(
            "BuildMenu_DetailConstructionIconTexture",
            GConstructionCostIconTexture);
        DetailConstructionIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mDetailConstructionIcon = DetailConstructionIcon;
    }

    auto DetailConstructionCostText =
        CreateWidget<CTextBlock>("BuildMenu_DetailConstructionCost", 8).lock();

    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetText(TEXT("-"));
        DetailConstructionCostText->SetFontSize(18.f);
        DetailConstructionCostText->SetAlignH(ETextAlignH::Left);
        DetailConstructionCostText->SetAlignV(ETextAlignV::Middle);
        DetailConstructionCostText->SetTextColor(168, 120, 28, 255);
        DetailConstructionCostText->EnableShadow(true);
        DetailConstructionCostText->SetShadowOffset(1.f, 1.f);
        DetailConstructionCostText->SetShadowTextColor(240, 240, 240, 170);
        mDetailConstructionCostText = DetailConstructionCostText;
    }

    auto DetailInfoText =
        CreateWidget<CTextBlock>("BuildMenu_DetailInfoText", 8).lock();

    if (DetailInfoText)
    {
        DetailInfoText->SetText(TEXT("핵심 정보\n- 준비 중"));
        DetailInfoText->SetFontSize(16.f);
        DetailInfoText->SetAlignH(ETextAlignH::Left);
        DetailInfoText->SetAlignV(ETextAlignV::Top);
        DetailInfoText->SetTextColor(56, 56, 56, 255);
        DetailInfoText->EnableShadow(true);
        DetailInfoText->SetShadowOffset(1.f, 1.f);
        DetailInfoText->SetShadowTextColor(255, 255, 255, 150);
        mDetailInfoText = DetailInfoText;
    }

    auto DetailBodyText =
        CreateWidget<CTextBlock>("BuildMenu_DetailBody", 7).lock();

    if (DetailBodyText)
    {
        DetailBodyText->SetText(TEXT("건물을 선택하면 설명과 핵심 정보가 이 영역에 함께 표시됩니다."));
        DetailBodyText->SetFontSize(15.f);
        DetailBodyText->SetAlignH(ETextAlignH::Left);
        DetailBodyText->SetAlignV(ETextAlignV::Top);
        DetailBodyText->SetTextColor(72, 62, 40, 255);
        DetailBodyText->EnableShadow(true);
        DetailBodyText->SetShadowOffset(1.f, 1.f);
        DetailBodyText->SetShadowTextColor(242, 235, 219, 170);
        mDetailBodyText = DetailBodyText;
    }

    if (BuildButton)
        BuildButton->SetEnable(false);
    if (YearbookButton)
        YearbookButton->SetEnable(false);
    if (NpcCountText)
        NpcCountText->SetEnable(false);
    if (BudgetText)
        BudgetText->SetEnable(false);
    if (DateText)
        DateText->SetEnable(false);
    if (DayProgressText)
        DayProgressText->SetEnable(false);
    if (DayProgressBar)
        DayProgressBar->SetEnable(false);
    if (MenuTitleRibbon)
        MenuTitleRibbon->SetEnable(false);
    if (MenuGridFrame)
        MenuGridFrame->SetEnable(false);
    if (MenuDetailFrame)
        MenuDetailFrame->SetEnable(false);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(false);
    if (ScrollTrack)
        ScrollTrack->SetEnable(false);
    if (ScrollThumb)
        ScrollThumb->SetEnable(false);
    if (DetailBlueprintIcon)
        DetailBlueprintIcon->SetEnable(false);
    if (DetailBlueprintCostText)
        DetailBlueprintCostText->SetEnable(false);
    if (DetailConstructionIcon)
        DetailConstructionIcon->SetEnable(false);
    if (DetailConstructionCostText)
        DetailConstructionCostText->SetEnable(false);
    if (DetailInfoText)
        DetailInfoText->SetEnable(false);

    mMenuOpen = false;
    mYearbookOpen = false;
    SelectCategory(EBuildingCategory::Infrastructure);
    ApplyMenuOpenState();
    ApplyYearbookOpenState();
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();
    RefreshLayout();

    return true;
}

void CBuildMenuWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();

    if (mMenuOpen)
    {
        auto World = mWorld.lock();

        if (World)
        {
            auto Input = World->GetInput().lock();

            if (Input)
            {
                const int WheelDelta = Input->GetMouseWheelDelta();

                if (WheelDelta != 0 && IsMouseOverOpenPanel(Input->GetMousePos()))
                    MovePage(WheelDelta < 0 ? 1 : -1);
            }
        }
    }

    RefreshLayout();
}

void CBuildMenuWidget::Render()
{
    CWidgetContainer::Render();
}

bool CBuildMenuWidget::IsMouseOverOpenPanel(const FVector2& MousePos) const
{
    if (!mMenuOpen && !mYearbookOpen)
        return false;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;

    return MousePos.x >= PanelLeft &&
        MousePos.x <= PanelLeft + PanelWidth &&
        MousePos.y >= PanelTop &&
        MousePos.y <= PanelTop + PanelHeight;
}

void CBuildMenuWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(480.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(420.f, ScreenHeight - 100.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 30.f * Scale;
    const float ContentWidth = PanelWidth - HorizontalMargin * 2.f;
    const float HeaderTopPadding = 18.f * Scale;
    const float HeaderHeight = 56.f * Scale;
    const float TitleRibbonWidth = PanelWidth - 140.f * Scale;
    const float TitleRibbonLeft =
        PanelLeft + (PanelWidth - TitleRibbonWidth) * 0.5f;
    const float GridFrameLeft = PanelLeft + HorizontalMargin;
    const float GridFrameTop = PanelTop + HeaderTopPadding + HeaderHeight + 16.f * Scale;
    const float GridFrameWidth = PanelWidth - HorizontalMargin * 2.f;
    const float GridFrameHeight = PanelHeight - 324.f * Scale;
    const float DetailFrameLeft = GridFrameLeft;
    const float DetailFrameTop = GridFrameTop + GridFrameHeight + 18.f * Scale;
    const float DetailFrameWidth = GridFrameWidth;
    const float DetailFrameHeight =
        PanelTop + PanelHeight - DetailFrameTop - 22.f * Scale;

    auto BuildButton = mBuildButton.lock();
    auto YearbookButton = mYearbookButton.lock();

    const float BuildButtonWidth = 220.f;
    const float YearbookButtonWidth = 140.f;
    const float BottomButtonGap = 14.f;
    const float BottomButtonHeight = 58.f;
    const float BottomButtonTop =
        ScreenWidth * 0.5f -
        (BuildButtonWidth + BottomButtonGap + YearbookButtonWidth) * 0.5f;

    if (BuildButton)
    {
        BuildButton->SetPivot(0.f, 1.f);
        BuildButton->SetPos(BottomButtonTop, ScreenHeight - 20.f);
        BuildButton->SetSize(BuildButtonWidth, BottomButtonHeight);
    }

    if (YearbookButton)
    {
        YearbookButton->SetPivot(0.f, 1.f);
        YearbookButton->SetPos(
            BottomButtonTop + BuildButtonWidth + BottomButtonGap,
            ScreenHeight - 20.f);
        YearbookButton->SetSize(YearbookButtonWidth, BottomButtonHeight);
    }

    auto NpcCountText = mNpcCountText.lock();
    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (NpcCountText)
    {
        NpcCountText->SetPivot(0.f, 1.f);
        NpcCountText->SetPos(20.f, ScreenHeight - 20.f);
        NpcCountText->SetSize(380.f, 28.f);
    }

    if (BudgetText)
    {
        BudgetText->SetPivot(0.f, 1.f);
        BudgetText->SetPos(20.f, ScreenHeight - 48.f);
        BudgetText->SetSize(420.f, 28.f);
    }

    if (DateText)
    {
        DateText->SetPivot(0.f, 1.f);
        DateText->SetPos(20.f, ScreenHeight - 76.f);
        DateText->SetSize(420.f, 28.f);
    }

    if (DayProgressText)
    {
        DayProgressText->SetPivot(0.f, 1.f);
        DayProgressText->SetPos(20.f, ScreenHeight - 104.f);
        DayProgressText->SetSize(420.f, 24.f);
    }

    if (DayProgressBar)
    {
        DayProgressBar->SetPivot(0.f, 1.f);
        DayProgressBar->SetPos(20.f, ScreenHeight - 124.f);
        DayProgressBar->SetSize(300.f, 14.f);
    }

    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto YearbookCloseButton = mYearbookCloseButton.lock();

    const float YearbookPanelWidth = PanelWidth;
    const float YearbookPanelHeight = PanelHeight;
    const float YearbookLeft = PanelLeft;
    const float YearbookTop = PanelTop;
    const float YearbookBodyTop =
        YearbookTop + HeaderTopPadding + HeaderHeight + 14.f * Scale;
    const float YearbookBodyHeight = (std::max)(
        40.f, YearbookPanelHeight - (YearbookBodyTop - YearbookTop) - 16.f * Scale);

    if (YearbookPanel)
    {
        YearbookPanel->SetPos(YearbookLeft, YearbookTop);
        YearbookPanel->SetSize(YearbookPanelWidth, YearbookPanelHeight);
    }

    if (YearbookTitleText)
    {
        YearbookTitleText->SetPos(
            YearbookLeft + HorizontalMargin,
            YearbookTop + HeaderTopPadding);
        YearbookTitleText->SetSize(ContentWidth, HeaderHeight);
    }

    if (YearbookCloseButton)
    {
        YearbookCloseButton->SetPos(
            YearbookLeft + YearbookPanelWidth - HorizontalMargin - 44.f * Scale,
            YearbookTop + HeaderTopPadding - 4.f * Scale);
        YearbookCloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    if (YearbookBodyText)
    {
        YearbookBodyText->SetPos(
            YearbookLeft + HorizontalMargin, YearbookBodyTop);
        YearbookBodyText->SetSize(ContentWidth, YearbookBodyHeight);
    }

    auto MenuBackground = mMenuBackground.lock();
    auto MenuTitleRibbon = mMenuTitleRibbon.lock();
    auto MenuGridFrame = mMenuGridFrame.lock();
    auto MenuDetailFrame = mMenuDetailFrame.lock();
    auto DetailInfoPanel = mDetailInfoPanel.lock();
    auto ScrollTrack = mScrollTrack.lock();
    auto ScrollThumb = mScrollThumb.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    if (MenuTitleRibbon)
    {
        MenuTitleRibbon->SetPos(TitleRibbonLeft, PanelTop + HeaderTopPadding);
        MenuTitleRibbon->SetSize(TitleRibbonWidth, HeaderHeight);
    }

    if (MenuGridFrame)
    {
        MenuGridFrame->SetPos(GridFrameLeft, GridFrameTop);
        MenuGridFrame->SetSize(GridFrameWidth, GridFrameHeight);
    }

    if (MenuDetailFrame)
    {
        MenuDetailFrame->SetPos(DetailFrameLeft, DetailFrameTop);
        MenuDetailFrame->SetSize(DetailFrameWidth, DetailFrameHeight);
    }

    auto TitleText = mTitleText.lock();
    auto MenuCloseButton = mMenuCloseButton.lock();

    if (TitleText)
    {
        TitleText->SetFontSize(32.f * Scale);
        TitleText->SetPos(TitleRibbonLeft + 32.f * Scale, PanelTop + HeaderTopPadding);
        TitleText->SetSize(TitleRibbonWidth - 64.f * Scale, HeaderHeight);
    }

    if (MenuCloseButton)
    {
        MenuCloseButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 44.f * Scale,
            PanelTop + HeaderTopPadding - 4.f * Scale);
        MenuCloseButton->SetSize(40.f * Scale, 40.f * Scale);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto PageText = mPageText.lock();
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + SlotsPerPage - 1) / SlotsPerPage);
    const bool ShowPageControls = PageCount > 1;
    const float ScrollTrackLeft =
        GridFrameLeft + GridFrameWidth - 20.f * Scale;
    const float ScrollTrackTop = GridFrameTop + 46.f * Scale;
    const float ScrollTrackHeight = GridFrameHeight - 92.f * Scale;
    const float ScrollTrackWidth = 12.f * Scale;

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + 10.f * Scale);
        PrevPageButton->SetSize(26.f * Scale, 20.f * Scale);
        PrevPageButton->SetEnable(mMenuOpen && ShowPageControls);
    }

    if (PageText)
    {
        PageText->SetFontSize(15.f * Scale);
        PageText->SetPos(
            ScrollTrackLeft - 52.f * Scale,
            GridFrameTop + GridFrameHeight - 30.f * Scale);
        PageText->SetSize(82.f * Scale, 20.f * Scale);
        PageText->SetEnable(mMenuOpen && ShowPageControls);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            ScrollTrackLeft - 7.f * Scale,
            GridFrameTop + GridFrameHeight - 28.f * Scale);
        NextPageButton->SetSize(26.f * Scale, 20.f * Scale);
        NextPageButton->SetEnable(mMenuOpen && ShowPageControls);
    }

    if (ScrollTrack)
    {
        ScrollTrack->SetPos(ScrollTrackLeft, ScrollTrackTop);
        ScrollTrack->SetSize(ScrollTrackWidth, ScrollTrackHeight);
        ScrollTrack->SetEnable(mMenuOpen && ShowPageControls);
    }

    if (ScrollThumb)
    {
        const float ThumbHeight = (std::max)(
            30.f * Scale,
            ShowPageControls ?
            ScrollTrackHeight / static_cast<float>(PageCount) :
            ScrollTrackHeight);
        const float ThumbTravel =
            (std::max)(0.f, ScrollTrackHeight - ThumbHeight);
        const float ThumbRatio =
            PageCount > 1 ?
            static_cast<float>(mCurrentPage) /
            static_cast<float>(PageCount - 1) :
            0.f;

        ScrollThumb->SetPos(
            ScrollTrackLeft,
            ScrollTrackTop + ThumbTravel * ThumbRatio);
        ScrollThumb->SetSize(ScrollTrackWidth, ThumbHeight);
        ScrollThumb->SetEnable(mMenuOpen && ShowPageControls);
    }

    const float CategoryTop = PanelTop - 64.f * Scale;
    const float CategoryGap = 10.f * Scale;
    const float CategoryWidth = 78.f * Scale;
    const float CategoryHeight = 90.f * Scale;
    const float CategoryTotalWidth =
        CategoryWidth * CategoryCount +
        CategoryGap * static_cast<float>(CategoryCount - 1);
    const float CategoryStartX =
        PanelLeft + (PanelWidth - CategoryTotalWidth) * 0.5f;

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();
        auto CategoryIcon = mCategoryButtonIcons[i].lock();

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            CategoryStartX +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);

        if (CategoryIcon)
        {
            CategoryIcon->SetPos(18.f * Scale, 10.f * Scale);
            CategoryIcon->SetSize(42.f * Scale, 42.f * Scale);
        }
    }

    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 14.f * Scale;
    const float SlotLeft = GridFrameLeft + 20.f * Scale;
    const float SlotTop = GridFrameTop + 18.f * Scale;
    const float SlotAreaWidth =
        GridFrameWidth - 56.f * Scale - ScrollTrackWidth;
    const float SlotAreaHeight = GridFrameHeight - 36.f * Scale;
    const float SlotWidth =
        (SlotAreaWidth - SlotGapX * (SlotColumnCount - 1)) /
        static_cast<float>(SlotColumnCount);
    const float SlotHeight =
        (SlotAreaHeight - SlotGapY * (SlotRowCount - 1)) /
        static_cast<float>(SlotRowCount);
    const float SlotIconHorizontalPadding = 16.f * Scale;
    const float SlotIconTopPadding = 12.f * Scale;
    const float SlotTextHeight = 36.f * Scale;
    const float SlotIconHeight =
        (std::max)(28.f * Scale, SlotHeight - SlotTextHeight - 20.f * Scale);

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int Row = i / SlotColumnCount;
        const int Col = i % SlotColumnCount;

        auto SlotButton = mBuildingButtons[i].lock();
        auto SlotIcon = mBuildingButtonIcons[i].lock();
        auto SlotText = mBuildingButtonTexts[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            SlotLeft +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);

        if (SlotIcon)
        {
            SlotIcon->SetPos(
                SlotIconHorizontalPadding,
                SlotIconTopPadding);
            SlotIcon->SetSize(
                SlotWidth - SlotIconHorizontalPadding * 2.f,
                SlotIconHeight);
        }

        if (SlotText)
        {
            SlotText->SetFontSize(15.f * Scale);
            SlotText->SetPos(10.f * Scale, SlotHeight - SlotTextHeight);
            SlotText->SetSize(SlotWidth - 20.f * Scale, SlotTextHeight - 4.f * Scale);
        }
    }

    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBlueprintIcon = mDetailBlueprintIcon.lock();
    auto DetailBlueprintCostText = mDetailBlueprintCostText.lock();
    auto DetailConstructionIcon = mDetailConstructionIcon.lock();
    auto DetailConstructionCostText = mDetailConstructionCostText.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    const float DetailPadding = 18.f * Scale;
    const float DetailColumnGap = 16.f * Scale;
    const float DetailTitleTop = DetailFrameTop + 12.f * Scale;
    const float DetailTitleHeight = 30.f * Scale;
    const float CostTop = DetailTitleTop + DetailTitleHeight + 10.f * Scale;
    const float CostIconSize = 22.f * Scale;
    const float CostTextHeight = 26.f * Scale;
    const float CostLabelGap = 6.f * Scale;
    const float SecondaryCostOffset = 166.f * Scale;
    const float DetailContentTop = CostTop + CostTextHeight + 12.f * Scale;
    const float DetailContentHeight = (std::max)(
        48.f * Scale,
        DetailFrameHeight - (DetailContentTop - DetailFrameTop) - DetailPadding);
    float DetailInfoWidth = (std::max)(
        250.f * Scale,
        DetailFrameWidth * 0.34f);
    DetailInfoWidth = (std::min)(
        DetailInfoWidth,
        DetailFrameWidth - DetailPadding * 2.f - 220.f * Scale);
    const float DetailBodyLeft = DetailFrameLeft + DetailPadding;
    const float DetailInfoLeft =
        DetailFrameLeft + DetailFrameWidth - DetailPadding - DetailInfoWidth;
    const float DetailBodyTop = DetailContentTop;
    const float DetailInfoTop = DetailContentTop;
    const float DetailInfoHeight = DetailContentHeight;
    const float DetailBodyWidth = (std::max)(
        200.f * Scale,
        DetailInfoLeft - DetailBodyLeft - DetailColumnGap);
    const float DetailBodyHeight = DetailContentHeight;
    const float CostLeft = DetailFrameLeft + DetailPadding;

    if (DetailTitleText)
    {
        DetailTitleText->SetFontSize(24.f * Scale);
        DetailTitleText->SetPos(DetailFrameLeft + DetailPadding, DetailTitleTop);
        DetailTitleText->SetSize(
            DetailFrameWidth - DetailPadding * 2.f,
            DetailTitleHeight);
    }

    if (DetailBlueprintIcon)
    {
        DetailBlueprintIcon->SetPos(CostLeft, CostTop);
        DetailBlueprintIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailBlueprintCostText)
    {
        DetailBlueprintCostText->SetFontSize(17.f * Scale);
        DetailBlueprintCostText->SetPos(
            CostLeft + CostIconSize + CostLabelGap,
            CostTop - 1.f * Scale);
        DetailBlueprintCostText->SetSize(132.f * Scale, CostTextHeight);
    }

    if (DetailConstructionIcon)
    {
        DetailConstructionIcon->SetPos(
            CostLeft + SecondaryCostOffset,
            CostTop + 1.f * Scale);
        DetailConstructionIcon->SetSize(CostIconSize, CostIconSize);
    }

    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetFontSize(17.f * Scale);
        DetailConstructionCostText->SetPos(
            CostLeft + SecondaryCostOffset + CostIconSize + CostLabelGap,
            CostTop - 1.f * Scale);
        DetailConstructionCostText->SetSize(180.f * Scale, CostTextHeight);
    }

    if (DetailInfoPanel)
    {
        DetailInfoPanel->SetPos(DetailInfoLeft, DetailInfoTop);
        DetailInfoPanel->SetSize(DetailInfoWidth, DetailInfoHeight);
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetFontSize(16.f * Scale);
        DetailInfoText->SetPos(
            DetailInfoLeft + 16.f * Scale,
            DetailInfoTop + 14.f * Scale);
        DetailInfoText->SetSize(
            DetailInfoWidth - 32.f * Scale,
            DetailInfoHeight - 28.f * Scale);
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetFontSize(15.f * Scale);
        DetailBodyText->SetPos(DetailBodyLeft, DetailBodyTop);
        DetailBodyText->SetSize(DetailBodyWidth, DetailBodyHeight);
    }
}

void CBuildMenuWidget::ApplyMenuOpenState()
{
    auto BuildButton = mBuildButton.lock();
    auto MenuBackground = mMenuBackground.lock();
    auto MenuTitleRibbon = mMenuTitleRibbon.lock();
    auto MenuGridFrame = mMenuGridFrame.lock();
    auto MenuDetailFrame = mMenuDetailFrame.lock();
    auto DetailInfoPanel = mDetailInfoPanel.lock();
    auto DetailBlueprintIcon = mDetailBlueprintIcon.lock();
    auto DetailConstructionIcon = mDetailConstructionIcon.lock();
    auto DetailBlueprintCostText = mDetailBlueprintCostText.lock();
    auto DetailConstructionCostText = mDetailConstructionCostText.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto ScrollTrack = mScrollTrack.lock();
    auto ScrollThumb = mScrollThumb.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBodyText = mDetailBodyText.lock();
    auto MenuCloseButton = mMenuCloseButton.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (MenuBackground)
        MenuBackground->SetEnable(mMenuOpen);
    if (MenuTitleRibbon)
        MenuTitleRibbon->SetEnable(mMenuOpen);
    if (MenuGridFrame)
        MenuGridFrame->SetEnable(mMenuOpen);
    if (MenuDetailFrame)
        MenuDetailFrame->SetEnable(mMenuOpen);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(mMenuOpen);
    if (DetailBlueprintIcon)
        DetailBlueprintIcon->SetEnable(mMenuOpen);
    if (DetailConstructionIcon)
        DetailConstructionIcon->SetEnable(mMenuOpen);
    if (DetailBlueprintCostText)
        DetailBlueprintCostText->SetEnable(mMenuOpen);
    if (DetailConstructionCostText)
        DetailConstructionCostText->SetEnable(mMenuOpen);
    if (DetailInfoText)
        DetailInfoText->SetEnable(mMenuOpen);
    if (ScrollTrack)
        ScrollTrack->SetEnable(mMenuOpen);
    if (ScrollThumb)
        ScrollThumb->SetEnable(mMenuOpen);
    if (TitleText)
        TitleText->SetEnable(mMenuOpen);
    if (PageText)
        PageText->SetEnable(mMenuOpen);
    if (DetailTitleText)
        DetailTitleText->SetEnable(mMenuOpen);
    if (DetailBodyText)
        DetailBodyText->SetEnable(mMenuOpen);
    if (MenuCloseButton)
        MenuCloseButton->SetEnable(mMenuOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(mMenuOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(mMenuOpen);

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }

    for (size_t i = 0; i < mBuildingButtons.size(); ++i)
    {
        auto Button = mBuildingButtons[i].lock();
        const bool HasEntry =
            i < mVisibleEntryIndices.size() &&
            mVisibleEntryIndices[i] >= 0;

        if (Button)
            Button->SetEnable(mMenuOpen && HasEntry);
    }

    if (BuildButton)
    {
        ApplyButtonTextureSet(
            BuildButton,
            "BuildMenuOpenButtonState",
            mMenuOpen ? GBigTextButtonSelectedTexture : GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureIconSlotButtonStyle(BuildButton);
    }

}

void CBuildMenuWidget::ApplyYearbookOpenState()
{
    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto YearbookCloseButton = mYearbookCloseButton.lock();
    auto YearbookButton = mYearbookButton.lock();
    auto YearbookButtonText = mYearbookButtonText.lock();

    if (YearbookPanel)
        YearbookPanel->SetEnable(mYearbookOpen);
    if (YearbookTitleText)
        YearbookTitleText->SetEnable(mYearbookOpen);
    if (YearbookBodyText)
        YearbookBodyText->SetEnable(mYearbookOpen);
    if (YearbookCloseButton)
        YearbookCloseButton->SetEnable(mYearbookOpen);

    if (YearbookButton)
    {
        ApplyButtonTextureSet(
            YearbookButton,
            "BuildMenuYearbookButtonState",
            mYearbookOpen ? GBigTextButtonSelectedTexture : GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureIconSlotButtonStyle(YearbookButton);
    }

    if (YearbookButtonText)
    {
        if (mYearbookOpen)
            YearbookButtonText->SetText(TEXT("연감 ON"));
        else
            YearbookButtonText->SetText(TEXT("연감"));
    }

}

void CBuildMenuWidget::RefreshNpcCountText()
{
    auto NpcCountText = mNpcCountText.lock();
    auto World = mWorld.lock();

    if (!NpcCountText || !World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int NpcCount = 0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (Orb && Orb->GetAlive())
                ++NpcCount;
        }
    }

    wchar_t CountBuffer[64] = {};
    swprintf_s(CountBuffer, L"NPC: %d", NpcCount);
    NpcCountText->SetText(CountBuffer);
}

void CBuildMenuWidget::RefreshEconomyStatus()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);

    if (!MainWorld)
        return;

    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (BudgetText)
    {
        const std::wstring BudgetLabel =
            L"국가 예산: " +
            FormatCurrency(MainWorld->GetNationalBudget());
        BudgetText->SetText(BudgetLabel.c_str());
    }

    const int Year = MainWorld->GetSimulationYear();
    const int Month = MainWorld->GetSimulationMonth();
    const int Day = MainWorld->GetSimulationDay();
    const int MonthDays = MainWorld->GetSimulationMonthDayCount();
    const float MonthProgress = MainWorld->GetSimulationMonthProgress();

    if (DateText)
    {
        wchar_t DateBuffer[64] = {};
        swprintf_s(DateBuffer, L"날짜: %04d-%02d-%02d", Year, Month, Day);
        DateText->SetText(DateBuffer);
    }

    if (DayProgressBar)
        DayProgressBar->SetPercent(MonthProgress);

    if (DayProgressText)
    {
        wchar_t ProgressBuffer[96] = {};
        swprintf_s(
            ProgressBuffer,
            L"월 진행: %d%%  |  %d / %d일",
            static_cast<int>(roundf(MonthProgress * 100.f)),
            Day,
            MonthDays);
        DayProgressText->SetText(ProgressBuffer);
    }
}

void CBuildMenuWidget::RefreshYearbookStatus()
{
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto World = mWorld.lock();

    if (!YearbookBodyText || !World)
        return;

    const auto MainWorld = std::dynamic_pointer_cast<IMainWorldAccess>(World);
    const auto Snapshot = AlmanacDataProvider::BuildSnapshot(
        World,
        MainWorld);
    const std::wstring Body =
        AlmanacDataProvider::BuildYearbookSummaryText(Snapshot);
    YearbookBodyText->SetText(Body.c_str());
}

void CBuildMenuWidget::RefreshCategoryButtons()
{
    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        const bool Selected = i == static_cast<int>(mSelectedCategory);
        ApplyButtonTextureSet(
            CategoryButton,
            "BuildMenuCategoryRefresh_" + std::to_string(i),
            Selected ? GCategoryTabTextureSelected : GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(
            CategoryButton,
            Selected);
    }

    auto TitleText = mTitleText.lock();

    if (TitleText &&
        static_cast<int>(mSelectedCategory) < CategoryCount)
    {
        TitleText->SetText(CategoryLabels[static_cast<int>(mSelectedCategory)]);
    }
}

std::vector<int> CBuildMenuWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Catalog = GetBuildingCatalog();

    for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
    {
        if (Catalog[i].Category != mSelectedCategory)
            continue;

        if (Catalog[i].IsHiddenFromBuildMenu)
            continue;

        Result.push_back(i);
    }

    return Result;
}

void CBuildMenuWidget::RefreshBuildingButtons()
{
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + SlotsPerPage - 1) / SlotsPerPage);

    if (mCurrentPage < 0)
        mCurrentPage = 0;
    else if (mCurrentPage >= PageCount)
        mCurrentPage = PageCount - 1;

    mVisibleEntryIndices.assign(SlotsPerPage, -1);
    const int BeginIndex = mCurrentPage * SlotsPerPage;

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            mVisibleEntryIndices[i] = CategoryEntries[CategoryListIndex];
        }
    }

    bool HasPreviewOnPage = false;

    if (mPreviewEntryIndex >= 0)
    {
        for (int i = 0; i < static_cast<int>(mVisibleEntryIndices.size()); ++i)
        {
            if (mVisibleEntryIndices[i] == mPreviewEntryIndex)
            {
                HasPreviewOnPage = true;
                break;
            }
        }
    }

    if (!HasPreviewOnPage)
        mPreviewEntryIndex = -1;

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        auto Button = mBuildingButtons[i].lock();
        auto ButtonIcon = mBuildingButtonIcons[i].lock();
        auto ButtonText = mBuildingButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        const int EntryIndex = mVisibleEntryIndices[i];

        if (EntryIndex >= 0 &&
            EntryIndex < static_cast<int>(GetBuildingCatalog().size()))
        {
            const auto& Entry = GetBuildingCatalog()[EntryIndex];
            const bool Previewed = EntryIndex == mPreviewEntryIndex;
            const TCHAR* IconPath = GetCatalogEntryIconPath(
                Entry.Category, Entry.CategoryLocalIndex);
            const std::string TextureKey = "BuildMenuSlotCardRefresh_" +
                std::to_string(static_cast<int>(Entry.Category)) + "_" +
                std::to_string(Entry.CategoryLocalIndex);

            ApplyButtonTextureSet(
                Button,
                TextureKey,
                Previewed ? GSlotCardSelectedTexture : GSlotCardTexture,
                Previewed ? GSlotCardSelectedTexture : GSlotCardHoverTexture,
                GSlotCardSelectedTexture,
                GSlotCardDisabledTexture);
            Button->SetEnable(true);
            Button->ButtonEnable(true);
            ButtonText->SetText(Entry.DisplayName.c_str());

            if (ButtonIcon && IconPath)
            {
                ButtonIcon->SetTexture(
                    "BuildMenuSlotIcon_" +
                    std::to_string(static_cast<int>(Entry.Category)) + "_" +
                    std::to_string(Entry.CategoryLocalIndex),
                    IconPath);
                ButtonIcon->SetEnable(true);
                ButtonIcon->SetTint(1.f, 1.f, 1.f, 1.f);
            }
            else if (ButtonIcon)
            {
                ButtonIcon->SetEnable(false);
            }
        }
        else
        {
            Button->SetEnable(false);
            Button->ButtonEnable(false);
            ButtonText->SetText(TEXT(""));

            if (ButtonIcon)
                ButtonIcon->SetEnable(false);
        }
    }

    auto PageText = mPageText.lock();

    if (PageText)
    {
        wchar_t PageBuffer[64] = {};
        swprintf_s(PageBuffer, L"%d / %d", mCurrentPage + 1, PageCount);
        PageText->SetText(PageBuffer);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(mCurrentPage > 0);
    if (NextPageButton)
        NextPageButton->ButtonEnable(mCurrentPage < PageCount - 1);

    RefreshDetailPanel();
}

void CBuildMenuWidget::SelectCategory(EBuildingCategory Category)
{
    mSelectedCategory = Category;
    mCurrentPage = 0;
    mPreviewEntryIndex = -1;
    RefreshCategoryButtons();
    RefreshBuildingButtons();
}

void CBuildMenuWidget::MovePage(int DeltaPage)
{
    if (DeltaPage == 0)
        return;

    mCurrentPage += DeltaPage;
    mPreviewEntryIndex = -1;
    RefreshBuildingButtons();
}

void CBuildMenuWidget::PreviewSlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    if (mPreviewEntryIndex == EntryIndex)
        return;

    mPreviewEntryIndex = EntryIndex;
    RefreshBuildingButtons();
}

void CBuildMenuWidget::RefreshDetailPanel()
{
    auto DetailTitleText = mDetailTitleText.lock();
    auto DetailBlueprintIcon = mDetailBlueprintIcon.lock();
    auto DetailBlueprintCostText = mDetailBlueprintCostText.lock();
    auto DetailConstructionIcon = mDetailConstructionIcon.lock();
    auto DetailConstructionCostText = mDetailConstructionCostText.lock();
    auto DetailInfoPanel = mDetailInfoPanel.lock();
    auto DetailInfoText = mDetailInfoText.lock();
    auto DetailBodyText = mDetailBodyText.lock();

    if (!DetailTitleText && !DetailBodyText)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (mPreviewEntryIndex < 0 ||
        mPreviewEntryIndex >= static_cast<int>(Catalog.size()))
    {
        if (DetailTitleText)
            DetailTitleText->SetText(TEXT("건물 정보"));
        if (DetailBlueprintCostText)
            DetailBlueprintCostText->SetText(TEXT("-"));
        if (DetailConstructionCostText)
            DetailConstructionCostText->SetText(TEXT("-"));
        if (DetailInfoText)
            DetailInfoText->SetText(TEXT("핵심 정보\n- 건물을 선택하면 표시됩니다."));
        if (DetailBodyText)
            DetailBodyText->SetText(TEXT("왼쪽 카드에 마우스를 올리면 건물 설명과 핵심 정보가 함께 표시됩니다."));
        return;
    }

    const auto& Entry = Catalog[mPreviewEntryIndex];
    const FParsedDetailInfo ParsedDetail = ParseDetailInfo(Entry);

    if (DetailTitleText)
        DetailTitleText->SetText(Entry.DisplayName.c_str());

    if (DetailBlueprintIcon)
        DetailBlueprintIcon->SetEnable(true);
    if (DetailConstructionIcon)
        DetailConstructionIcon->SetEnable(true);
    if (DetailInfoPanel)
        DetailInfoPanel->SetEnable(true);

    if (DetailBlueprintCostText)
    {
        DetailBlueprintCostText->SetText(
            ParsedDetail.BlueprintCost.empty() ?
            TEXT("-") :
            ParsedDetail.BlueprintCost.c_str());
    }

    if (DetailConstructionCostText)
    {
        DetailConstructionCostText->SetText(
            ParsedDetail.ConstructionCost.empty() ?
            TEXT("-") :
            ParsedDetail.ConstructionCost.c_str());
    }

    if (DetailInfoText)
    {
        DetailInfoText->SetText(
            BuildHighlightsBlockText(ParsedDetail.Highlights).c_str());
    }

    if (DetailBodyText)
    {
        DetailBodyText->SetText(ParsedDetail.Description.c_str());
    }
}

void CBuildMenuWidget::StartPlacementBySlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetBuildingCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementCtrl =
        World->FindObject<CPlacementController>(GPlacementControllerName).lock();

    if (!PlacementCtrl)
        return;

    const FBuildingCatalogEntry& Entry = Catalog[EntryIndex];

    if (Entry.IsDemolish)
    {
        PlacementCtrl->SetDemolitionMode(true);
        mMenuOpen = false;
        ApplyMenuOpenState();
        return;
    }

    PlacementCtrl->SetDemolitionMode(false);

    const std::string SpriteTexturePath =
        GetCatalogEntryIconPathUtf8(
            Entry.Category, Entry.CategoryLocalIndex);

    const bool Started = PlacementCtrl->BeginBuildPlacement(
        Entry, SpriteTexturePath);

    if (!Started)
        return;

    mMenuOpen = false;
    ApplyMenuOpenState();
}


void CBuildMenuWidget::OnBuildButtonClick()
{
    const bool NextOpen = !mMenuOpen;
    mMenuOpen = NextOpen;

    if (NextOpen)
    {
        mYearbookOpen = false;
        mPreviewEntryIndex = -1;
        RefreshBuildingButtons();

        auto World = mWorld.lock();

        if (World)
        {
            auto UIManager = World->GetUIManager().lock();

            if (UIManager)
            {
                auto EdictWidget =
                    UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();
                auto AlmanacWidget =
                    UIManager->FindWidget<CAlmanacWidget>(
                        GAlmanacWidgetName).lock();

                if (EdictWidget)
                    EdictWidget->SetOpen(false);

                if (AlmanacWidget)
                    AlmanacWidget->SetOpen(false);
            }
        }
    }

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnYearbookButtonClick()
{
    auto World = mWorld.lock();

    if (World)
    {
        auto UIManager = World->GetUIManager().lock();

        if (UIManager)
        {
            auto AlmanacWidget =
                UIManager->FindWidget<CAlmanacWidget>(
                    GAlmanacWidgetName).lock();
            auto EdictWidget =
                UIManager->FindWidget<CEdictWidget>(
                    GEdictWidgetName).lock();

            if (AlmanacWidget)
            {
                mMenuOpen = false;
                mYearbookOpen = false;

                if (EdictWidget)
                    EdictWidget->SetOpen(false);

                ApplyMenuOpenState();
                ApplyYearbookOpenState();
                AlmanacWidget->ToggleOpen();
                return;
            }
        }
    }

    const bool NextOpen = !mYearbookOpen;
    mYearbookOpen = NextOpen;

    if (NextOpen)
    {
        mMenuOpen = false;

        auto World = mWorld.lock();

        if (World)
        {
            auto UIManager = World->GetUIManager().lock();

            if (UIManager)
            {
                auto EdictWidget =
                    UIManager->FindWidget<CEdictWidget>(GEdictWidgetName).lock();

                if (EdictWidget)
                    EdictWidget->SetOpen(false);
            }
        }
    }

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnMenuCloseButtonClick()
{
    SetBuildMenuOpen(false);
}

void CBuildMenuWidget::OnYearbookCloseButtonClick()
{
    SetAlmanacOpen(false);
}

void CBuildMenuWidget::OnPrevPageClick()
{
    MovePage(-1);
}

void CBuildMenuWidget::OnNextPageClick()
{
    MovePage(1);
}

void CBuildMenuWidget::ToggleBuildMenu()
{
    OnBuildButtonClick();
}

void CBuildMenuWidget::ToggleAlmanac()
{
    OnYearbookButtonClick();
}

void CBuildMenuWidget::SetBuildMenuOpen(bool Open)
{
    mMenuOpen = Open;

    if (Open)
    {
        mYearbookOpen = false;
        mPreviewEntryIndex = -1;
        RefreshBuildingButtons();
    }

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::SetAlmanacOpen(bool Open)
{
    auto World = mWorld.lock();

    if (World)
    {
        auto UIManager = World->GetUIManager().lock();

        if (UIManager)
        {
            auto AlmanacWidget =
                UIManager->FindWidget<CAlmanacWidget>(
                    GAlmanacWidgetName).lock();

            if (AlmanacWidget)
            {
                mYearbookOpen = false;

                if (Open)
                    mMenuOpen = false;

                ApplyMenuOpenState();
                ApplyYearbookOpenState();
                AlmanacWidget->SetOpen(Open);
                return;
            }
        }
    }

    mYearbookOpen = Open;

    if (Open)
        mMenuOpen = false;

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}
