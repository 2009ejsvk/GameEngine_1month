#include "UILayoutConfig.h"
#include "RuntimeConfigRegistry.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "World/WorldUIManager.h"
#include <Windows.h>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace UIConfig
{
    // ── 기본값 정의 (INI 파일이 없을 때 이 값으로 동작) ────

    // 상단 상태바
    float StatusBarX              = 14.f;
    float StatusBarY              = 10.f;
    float StatusBarHeight         = 94.f;
    float StatusBarPaddingX       = 20.f;
    float StatusBudgetBlockWidth  = 218.f;
    float StatusNpcBlockWidth     = 128.f;
    float StatusSupportBlockWidth = 132.f;
    float StatusBlockGap          = 18.f;
    float StatusIconSize          = 26.f;
    float StatusIconTextGap       = 10.f;
    float StatusLabelOffsetY      = 12.f;
    float StatusValueOffsetY      = 30.f;
    float StatusLabelHeight       = 16.f;
    float StatusValueHeight       = 30.f;
    float StatusLabelFontSize     = 12.f;
    float StatusValueFontSize     = 22.f;

    // 블록별 미세 위치 조정
    float BudgetBlockOffsetX  = 0.f;
    float BudgetBlockOffsetY  = 0.f;
    float NpcBlockOffsetX     = 0.f;
    float NpcBlockOffsetY     = 0.f;
    float SupportBlockOffsetX = 0.f;
    float SupportBlockOffsetY = 0.f;

    // 하단 좌측 패널
    float SpeedPanelX            = 16.f;
    float SpeedPanelWidth        = 388.f;
    float SpeedPanelHeight       = 182.f;
    float SpeedPanelBottomMargin = 18.f;
    float SpeedPanelMinY         = 12.f;
    float PanelTextOffsetX       = 74.f;
    float TimeBarOffsetY         = 16.f;
    float TimeBarWidth           = 226.f;
    float TimeBarHeight          = 14.f;
    float DateTextOffsetY        = 34.f;
    float DateTextWidth          = 240.f;
    float DateTextHeight         = 28.f;
    float DateFontSize           = 22.f;

    // 게임 속도 버튼
    float PlayPauseButtonSize           = 44.f;
    float PlayPauseButtonOffsetX        = 24.f;
    float PlayPauseButtonOffsetY        = 0.f;
    float PlayPauseButtonBottomMargin   = 18.f;
    float SpeedMultiplierButtonSize     = 44.f;
    float SpeedMultiplierButtonOffsetX  = 76.f;
    float SpeedMultiplierButtonOffsetY  = 0.f;
    float SpeedMultiplierButtonBottomMargin = 18.f;

    // 레거시 공용 키 호환용
    float SpeedButtonSize         = 44.f;
    float SpeedButtonStep         = 52.f;
    float SpeedButtonOffsetX      = 24.f;
    float SpeedButtonBottomMargin = 18.f;

    // 하단 메뉴 아이콘 버튼
    float MenuButtonSize        = 60.f;
    float MenuButtonGap         = 10.f;
    float MenuLabelGap          = 8.f;
    float MenuButtonStartOffsetX = 26.f;
    float MenuButtonOffsetY     = 8.f;
    float MenuMinWidth          = 120.f;
    float MenuRightMargin       = 14.f;
    float MenuMinScaleFactor    = 0.70f;
    float MenuLabelBaseFontSize = 12.5f;
    float MenuLabelBaseHeight   = 18.f;

    // 칙령 UI
    float EdictPanelWidth           = 1120.f;
    float EdictPanelHeight          = 760.f;
    float EdictHeaderTopPadding     = 40.f;
    float EdictHeaderHeight         = 48.f;
    float EdictHorizontalMargin     = 24.f;
    float EdictVerticalMargin       = 18.f;
    float EdictGridFrameHeight      = 404.f;
    float EdictGridGapFromHeader    = 10.f;
    float EdictDetailGapFromGrid    = 12.f;
    float EdictTitleFontSize        = 30.f;
    float EdictCategoryWidth        = 74.f;
    float EdictCategoryHeight       = 90.f;
    float EdictCategoryGap          = 12.f;
    float EdictSlotPaddingLeft      = 18.f;
    float EdictSlotPaddingTop       = 26.f;
    float EdictSlotGapX             = 12.f;
    float EdictSlotGapY             = 14.f;
    float EdictDetailTitleFontSize  = 23.f;
    float EdictDetailBodyFontSize   = 15.f;
    float EdictDetailCostFontSize   = 18.f;
    float EdictTitleTextOffsetX     = 0.f;
    float EdictTitleTextOffsetY     = 0.f;
    float EdictSlotTextOffsetX      = 0.f;
    float EdictSlotTextOffsetY      = 0.f;
    float EdictDetailTitleOffsetX   = 0.f;
    float EdictDetailTitleOffsetY   = 0.f;
    float EdictDetailCostRowWidth   = 140.f;
    float EdictDetailCostRowOffsetX = 0.f;
    float EdictDetailCostRowOffsetY = 0.f;
    float EdictDetailCostIconSize   = 24.f;
    float EdictDetailCostIconOffsetX = 0.f;
    float EdictDetailCostIconOffsetY = 0.f;
    float EdictDetailCostOffsetX    = 0.f;
    float EdictDetailCostOffsetY    = 0.f;
    float EdictDetailCostTextWidth  = 108.f;
    float EdictDetailCostTextHeight = 30.f;
    bool EdictDetailInfoInlineWithCost = true;
    float EdictDetailInfoInlineGapX = 12.f;
    float EdictDetailInfoPanelOffsetX = 0.f;
    float EdictDetailInfoPanelOffsetY = 0.f;
    float EdictDetailInfoPanelWidthAdjust = 0.f;
    float EdictDetailInfoPanelHeightAdjust = 0.f;
    float EdictDetailInfoTextWidth = 0.f;
    float EdictDetailInfoTextHeight = 20.f;
    float EdictDetailInfoOffsetX    = 0.f;
    float EdictDetailInfoOffsetY    = 0.f;
    float EdictFeedbackOffsetX      = 0.f;
    float EdictFeedbackOffsetY      = 0.f;
    float EdictDetailBodyOffsetX    = 0.f;
    float EdictDetailBodyOffsetY    = 0.f;
    float EdictRequirementOffsetX   = 0.f;
    float EdictRequirementOffsetY   = 0.f;
    float EdictApplyButtonOffsetX   = 0.f;
    float EdictApplyButtonOffsetY   = 0.f;
    float EdictApplyButtonTextOffsetX = 0.f;
    float EdictApplyButtonTextOffsetY = 0.f;
    float EdictTaxPolicyTitleOffsetX = 0.f;
    float EdictTaxPolicyTitleOffsetY = 0.f;
    float EdictTaxPolicyRowTextOffsetX = 0.f;
    float EdictTaxPolicyRowTextOffsetY = 0.f;
    float EdictTaxPolicySummaryOffsetX = 0.f;
    float EdictTaxPolicySummaryOffsetY = 0.f;
    float EdictApplyButtonWidth     = 140.f;
    float EdictApplyButtonHeight    = 38.f;
    float EdictScrollTrackWidth     = 10.f;
    float EdictCloseButtonSize      = 40.f;
    bool EdictEnableTaxPolicyPanel  = false;
    float EdictTaxPolicyPanelWidth  = 296.f;
    float EdictTaxPolicyPanelHeight = 168.f;
    float EdictTaxPolicySummaryHeight = 40.f;

    // 연감 UI
    float AlmanacPanelWidth         = 1120.f;
    float AlmanacPanelHeight        = 756.f;
    float AlmanacPanelTopOffset     = 18.f;
    float AlmanacHeaderHeight       = 82.f;
    float AlmanacHeaderPadding      = 30.f;
    float AlmanacRibbonTopOffset    = 18.f;
    float AlmanacFrameInsetX        = 16.f;
    float AlmanacFrameHeaderOverlap = 10.f;
    float AlmanacFrameBottomInset   = 14.f;
    float AlmanacRailLeftInset      = 8.f;
    float AlmanacRailTopInset       = 22.f;
    float AlmanacRailBottomInset    = 46.f;
    float AlmanacRailThumbTopOffset = 10.f;
    float AlmanacRailThumbMinHeight = 88.f;
    float AlmanacRailThumbExpand    = 1.f;
    float AlmanacRailToContentGap   = 14.f;
    float AlmanacContentMarginX     = 42.f;
    float AlmanacContentMarginTop   = 20.f;
    float AlmanacContentMarginBottom = 30.f;
    float AlmanacContentTopInset    = 18.f;
    float AlmanacContentBottomInset = 18.f;
    float AlmanacTabSize            = 64.f;
    float AlmanacTabGap             = 10.f;
    float AlmanacTabBaseOffsetY     = 22.f;
    float AlmanacTabSelectedOffsetY = 6.f;
    float AlmanacTitleFontSize      = 30.f;
    float AlmanacTitlePaddingX      = 34.f;
    float AlmanacTitlePaddingY      = 1.f;
    float AlmanacCloseButtonSize    = 40.f;
    float AlmanacCloseButtonOffsetX = 44.f;
    float AlmanacCloseButtonOffsetY = 10.f;
    float AlmanacMetricRowHeight    = 46.f;
    float AlmanacMetricRowGap       = 6.f;
    float AlmanacDetailRowHeight    = 42.f;
    float AlmanacDetailRowGap       = 6.f;
    float AlmanacCardColumns        = 3.f;
    float AlmanacCardGapX           = 18.f;
    float AlmanacCardGapY           = 18.f;
    float AlmanacLeftPanelRatio     = 0.47f;
    float AlmanacPageColumnGap      = 22.f;
    float AlmanacWidePageColumnGap  = 24.f;
    float AlmanacPageTitleHeight    = 28.f;
    float AlmanacPageFrameTop       = 34.f;
    float AlmanacOffscreenHideOffset = 200.f;

    // 건물 선택 UI
    float BuildingPanelWidthRatio          = 0.24f;
    float BuildingPanelMinWidth            = 320.f;
    float BuildingPanelMaxWidth            = 410.f;
    float BuildingPanelTopOffset           = 58.f;
    float BuildingPanelBottomMargin        = 10.f;
    float BuildingPanelRightInset          = 10.f;
    float BuildingPanelMinHeight           = 420.f;
    float BuildingPanelInnerMarginX        = 18.f;
    float BuildingPanelInnerMarginTop      = 16.f;
    float BuildingPanelInnerBottomInset    = 28.f;
    float BuildingTabWidth                 = 56.f;
    float BuildingTabHeight                = 56.f;
    float BuildingTabGap                   = 10.f;
    float BuildingTabLabelFontSize         = 14.f;
    float BuildingTabLabelOffsetX          = 0.f;
    float BuildingTabLabelOffsetY          = 0.f;
    float BuildingTitleFontSize            = 26.f;
    float BuildingSubtitleFontSize         = 15.f;
    float BuildingBodyFontSize             = 18.f;
    float BuildingPageTitleFontSize        = 21.f;
    float BuildingBudgetTextFontSize       = 16.f;
    float BuildingOverviewWorkModeLabelFontSize = 18.f;
    float BuildingOverviewWorkModeValueFontSize = 17.f;
    float BuildingOverviewBudgetLabelFontSize = 18.f;
    float BuildingOverviewBudgetValueFontSize = 18.f;
    float BuildingOverviewOccupancyLabelFontSize = 18.f;
    float BuildingOverviewOccupancyValueFontSize = 18.f;
    float BuildingOverviewMetricLabelFontSize = 17.f;
    float BuildingOverviewMetricSectionHeaderFontSize = 18.f;
    float BuildingOverviewMetricValueFontSize = 17.f;
    float BuildingUpgradeTitleFontSize    = 18.f;
    float BuildingUpgradeDescriptionFontSize = 16.f;
    float BuildingInformationAccentFontSize = 28.f;
    float BuildingInformationBodyFontSize = 17.f;
    float BuildingBudgetButtonFontSize    = 16.f;
    float BuildingActionButtonFontSize    = 18.f;
    float BuildingOverviewCommandButtonFontSize = 18.f;
    float BuildingTitleIconInsetX          = 6.f;
    float BuildingTitleIconOffsetY         = 0.f;
    float BuildingTitleTextInsetX          = 14.f;
    float BuildingTitleIconGap             = 6.f;
    float BuildingTitleTextOffsetX         = 0.f;
    float BuildingTitleTextOffsetY         = 0.f;
    float BuildingTitleTextWidthAdjust     = 0.f;
    float BuildingTitleTextHeightAdjust    = 0.f;
    float BuildingSubtitleOffsetY          = 4.f;
    float BuildingTitleRibbonOffsetX       = 40.f;
    float BuildingTitleRibbonOffsetY       = 18.f;
    float BuildingTitleRibbonHeight        = 48.f;
    float BuildingSectionRibbonHeight      = 34.f;
    float BuildingSectionRibbonOffsetY     = 28.f;
    float BuildingCollapsedSectionGap      = 6.f;
    float BuildingCloseButtonOffsetX       = 38.f;
    float BuildingCloseButtonOffsetY       = 4.f;
    float BuildingCloseButtonSize          = 34.f;
    float BuildingCompactControlHeight     = 22.f;
    float BuildingCompactBudgetButtonWidth = 36.f;
    float BuildingSectionDividerWidth      = 172.f;
    float BuildingSectionDividerHeight     = 14.f;
    float BuildingBudgetBaseOffsetY        = 36.f;
    float BuildingBudgetLabelOffsetY       = 2.f;
    float BuildingOverviewWorkModeLabelOffsetX = 0.f;
    float BuildingOverviewWorkModeLabelOffsetY = 0.f;
    float BuildingOverviewWorkModeLabelWidthAdjust = 0.f;
    float BuildingOverviewWorkModeBackgroundOffsetX = 0.f;
    float BuildingOverviewWorkModeBackgroundOffsetY = 0.f;
    float BuildingOverviewWorkModeBackgroundWidthAdjust = 0.f;
    float BuildingOverviewWorkModeBackgroundHeightAdjust = 0.f;
    float BuildingOverviewWorkModeTextOffsetX = 0.f;
    float BuildingOverviewWorkModeTextOffsetY = 0.f;
    float BuildingOverviewWorkModeTextWidthAdjust = 0.f;
    float BuildingOverviewWorkModeTextHeightAdjust = 0.f;
    float BuildingOverviewBudgetLabelOffsetX = 0.f;
    float BuildingOverviewBudgetLabelOffsetY = 0.f;
    float BuildingOverviewBudgetLabelWidthAdjust = 0.f;
    float BuildingOverviewBudgetValueOffsetX = 0.f;
    float BuildingOverviewBudgetValueOffsetY = 0.f;
    float BuildingOverviewBudgetValueWidthAdjust = 0.f;
    float BuildingBudgetCustomButtonsOffsetY = 20.f;
    float BuildingBudgetWorkButtonsOffsetY = 78.f;
    float BuildingBudgetDefaultButtonsOffsetY = 26.f;
    float BuildingBudgetCompactGap         = 6.f;
    float BuildingBudgetDefaultGap         = 8.f;
    float BuildingOccupancyGapY            = 18.f;
    float BuildingActionButtonHeight       = 34.f;
    float BuildingActionButtonWidth        = 124.f;
    float BuildingActionButtonBottomMargin = 52.f;
    float BuildingActionCompactIconSize    = 34.f;
    float BuildingActionCompactIconOffsetY = 2.f;
    float BuildingMoveCompactRightOffset   = 82.f;
    float BuildingFocusCompactRightOffset  = 40.f;
    float BuildingOverviewCommandGap       = 10.f;
    float BuildingBudgetButtonHeight       = 30.f;
    float BuildingScrollTrackWidth         = 15.f;
    float BuildingScrollBottomInset        = 52.f;
    float BuildingScrollThumbHeight        = 94.f;
    float BuildingScrollThumbTopOffset     = 14.f;
    float BuildingBodyGapAfterSection      = 8.f;
    float BuildingBodyGapAfterActions      = 14.f;
    float BuildingBodyGapBeforeActions     = 12.f;
    float BuildingBodyFallbackOffset       = 4.f;
    float BuildingBodyBottomInset          = 22.f;
    float BuildingIconSize                 = 36.f;

    // NPC 선택 UI
    float CitizenPanelWidthRatio      = 0.24f;
    float CitizenPanelMinWidth        = 320.f;
    float CitizenPanelMaxWidth        = 410.f;
    float CitizenPanelTopOffset       = 58.f;
    float CitizenPanelBottomMargin    = 10.f;
    float CitizenPanelRightInset      = 10.f;
    float CitizenPanelMinHeight       = 420.f;
    float CitizenPanelInnerMarginX    = 18.f;
    float CitizenPanelInnerTopOffset  = 16.f;
    float CitizenPanelInnerBottomInset = 28.f;
    float CitizenTitleFontSize        = 26.f;
    float CitizenSubtitleFontSize     = 15.f;
    float CitizenBodyFontSize         = 18.f;
    float CitizenPageTitleFontSize    = 21.f;
    float CitizenTitleRibbonHeight    = 44.f;
    float CitizenSectionRibbonHeight  = 34.f;
    float CitizenSectionRibbonOffsetY = 28.f;
    float CitizenScrollTrackWidth     = 15.f;
    float CitizenScrollBottomInset    = 52.f;
    float CitizenScrollThumbHeight    = 94.f;
    float CitizenScrollThumbTopOffset = 14.f;
    float CitizenCloseButtonSize      = 34.f;
    float CitizenCloseButtonOffsetY   = 4.f;
    float CitizenBudgetBaseOffsetY    = 36.f;
    float CitizenActionStackTopOffset = 238.f;
    float CitizenFooterBottomInset    = 28.f;
    float CitizenBodyBottomInset      = 22.f;

    // 게임 오버 팝업
    float GameOverPanelWidth        = 720.f;
    float GameOverPanelHeight       = 390.f;
    float GameOverTitlePaddingX     = 70.f;
    float GameOverTitleOffsetY      = 70.f;
    float GameOverTitleHeight       = 48.f;
    float GameOverBodyPaddingX      = 84.f;
    float GameOverBodyOffsetY       = 138.f;
    float GameOverBodyBottomPadding = 210.f;

    void RegisterRuntimeConfig();

    namespace
    {
        constexpr const wchar_t* GPrimaryConfigFile = L"UILayout.ini";
        constexpr const wchar_t* GSplitConfigDirectory = L"UILayout\\";
        constexpr const wchar_t* GWidgetPathDumpFile = L"UIWidgetPaths.txt";
        constexpr const wchar_t* GWidgetTemplateFile =
            L"UIWidgetOverrideTemplate.ini";
        constexpr const char* GWidgetTemplateBeginMarker =
            "# >>> AUTO-GENERATED WIDGET OVERRIDE TEMPLATE BEGIN <<<";
        constexpr const char* GWidgetTemplateEndMarker =
            "# >>> AUTO-GENERATED WIDGET OVERRIDE TEMPLATE END <<<";

        template <typename T>
        struct TOptionalOverrideValue
        {
            bool Set = false;
            T Value = T();
        };

        struct FWidgetOverrideRule
        {
            TOptionalOverrideValue<float> PosX;
            TOptionalOverrideValue<float> PosY;
            TOptionalOverrideValue<float> OffsetX;
            TOptionalOverrideValue<float> OffsetY;
            TOptionalOverrideValue<float> Width;
            TOptionalOverrideValue<float> Height;
            TOptionalOverrideValue<float> WidthAdd;
            TOptionalOverrideValue<float> HeightAdd;
            TOptionalOverrideValue<float> PivotX;
            TOptionalOverrideValue<float> PivotY;
            TOptionalOverrideValue<float> Opacity;
            TOptionalOverrideValue<float> ZOrder;
            TOptionalOverrideValue<float> TintR;
            TOptionalOverrideValue<float> TintG;
            TOptionalOverrideValue<float> TintB;
            TOptionalOverrideValue<float> TintA;
            TOptionalOverrideValue<float> FontSize;
            TOptionalOverrideValue<float> FontSizeAdd;
            TOptionalOverrideValue<float> ShadowOffsetX;
            TOptionalOverrideValue<float> ShadowOffsetY;
            TOptionalOverrideValue<bool> Enable;
        };

        std::unordered_map<std::string, FWidgetOverrideRule> GWidgetPathOverrides;
        std::unordered_map<std::string, FWidgetOverrideRule> GWidgetNameOverrides;
        unsigned long long GLastDumpedWidgetPathGeneration = 0;
        unsigned long long GLastWidgetTreeSignature = 0;
        unsigned long long GConfigGeneration = 0;
        constexpr float GConfigCheckIntervalSeconds = 0.5f;
        float GConfigCheckCooldown = 0.f;
        bool GConfigRegistered = false;

        struct FTrackedConfigFile
        {
            std::wstring Path;
            unsigned long long WriteTime = 0;
        };

        std::vector<FTrackedConfigFile> GTrackedConfigFiles;

        void TrimString(std::string& S);
        void ResetToDefaults();
        bool LoadFile(const std::wstring& Path);

        template <typename T>
        void SetOverrideValue(TOptionalOverrideValue<T>& Field, const T& Value)
        {
            Field.Set = true;
            Field.Value = Value;
        }

        template <typename T>
        void MergeOverrideValue(
            TOptionalOverrideValue<T>& Destination,
            const TOptionalOverrideValue<T>& Source)
        {
            if (Source.Set)
                Destination = Source;
        }

        void MergeOverrideRule(
            FWidgetOverrideRule& Destination,
            const FWidgetOverrideRule& Source)
        {
            MergeOverrideValue(Destination.PosX, Source.PosX);
            MergeOverrideValue(Destination.PosY, Source.PosY);
            MergeOverrideValue(Destination.OffsetX, Source.OffsetX);
            MergeOverrideValue(Destination.OffsetY, Source.OffsetY);
            MergeOverrideValue(Destination.Width, Source.Width);
            MergeOverrideValue(Destination.Height, Source.Height);
            MergeOverrideValue(Destination.WidthAdd, Source.WidthAdd);
            MergeOverrideValue(Destination.HeightAdd, Source.HeightAdd);
            MergeOverrideValue(Destination.PivotX, Source.PivotX);
            MergeOverrideValue(Destination.PivotY, Source.PivotY);
            MergeOverrideValue(Destination.Opacity, Source.Opacity);
            MergeOverrideValue(Destination.ZOrder, Source.ZOrder);
            MergeOverrideValue(Destination.TintR, Source.TintR);
            MergeOverrideValue(Destination.TintG, Source.TintG);
            MergeOverrideValue(Destination.TintB, Source.TintB);
            MergeOverrideValue(Destination.TintA, Source.TintA);
            MergeOverrideValue(Destination.FontSize, Source.FontSize);
            MergeOverrideValue(Destination.FontSizeAdd, Source.FontSizeAdd);
            MergeOverrideValue(
                Destination.ShadowOffsetX,
                Source.ShadowOffsetX);
            MergeOverrideValue(
                Destination.ShadowOffsetY,
                Source.ShadowOffsetY);
            MergeOverrideValue(Destination.Enable, Source.Enable);
        }

        bool ParseWidgetOverrideKey(
            const std::string& Key,
            bool& OutByName,
            std::string& OutTarget,
            std::string& OutProperty)
        {
            size_t PrefixLength = 0;

            if (Key.rfind("Widget.", 0) == 0)
            {
                OutByName = false;
                PrefixLength = 7;
            }
            else if (Key.rfind("WidgetName.", 0) == 0)
            {
                OutByName = true;
                PrefixLength = 11;
            }
            else
            {
                return false;
            }

            const size_t PropertyPos = Key.rfind('.');

            if (PropertyPos == std::string::npos ||
                PropertyPos <= PrefixLength ||
                PropertyPos + 1 >= Key.size())
            {
                return false;
            }

            OutTarget = Key.substr(PrefixLength, PropertyPos - PrefixLength);
            OutProperty = Key.substr(PropertyPos + 1);
            TrimString(OutTarget);
            TrimString(OutProperty);
            return !OutTarget.empty() && !OutProperty.empty();
        }

        FWidgetOverrideRule* FindOverrideRule(
            bool ByName,
            const std::string& Target)
        {
            auto& SourceMap = ByName ? GWidgetNameOverrides : GWidgetPathOverrides;
            return &SourceMap[Target];
        }

        bool ApplyWidgetOverrideValue(const std::string& Key, float Val)
        {
            bool ByName = false;
            std::string Target;
            std::string Property;

            if (!ParseWidgetOverrideKey(Key, ByName, Target, Property))
                return false;

            FWidgetOverrideRule* const Rule =
                FindOverrideRule(ByName, Target);

            if (!Rule)
                return false;

            if      (Property == "PosX")          SetOverrideValue(Rule->PosX, Val);
            else if (Property == "PosY")          SetOverrideValue(Rule->PosY, Val);
            else if (Property == "OffsetX")       SetOverrideValue(Rule->OffsetX, Val);
            else if (Property == "OffsetY")       SetOverrideValue(Rule->OffsetY, Val);
            else if (Property == "Width")         SetOverrideValue(Rule->Width, Val);
            else if (Property == "Height")        SetOverrideValue(Rule->Height, Val);
            else if (Property == "WidthAdd")      SetOverrideValue(Rule->WidthAdd, Val);
            else if (Property == "HeightAdd")     SetOverrideValue(Rule->HeightAdd, Val);
            else if (Property == "PivotX")        SetOverrideValue(Rule->PivotX, Val);
            else if (Property == "PivotY")        SetOverrideValue(Rule->PivotY, Val);
            else if (Property == "Opacity")       SetOverrideValue(Rule->Opacity, Val);
            else if (Property == "ZOrder")        SetOverrideValue(Rule->ZOrder, Val);
            else if (Property == "TintR")         SetOverrideValue(Rule->TintR, Val);
            else if (Property == "TintG")         SetOverrideValue(Rule->TintG, Val);
            else if (Property == "TintB")         SetOverrideValue(Rule->TintB, Val);
            else if (Property == "TintA")         SetOverrideValue(Rule->TintA, Val);
            else if (Property == "FontSize")      SetOverrideValue(Rule->FontSize, Val);
            else if (Property == "FontSizeAdd")   SetOverrideValue(Rule->FontSizeAdd, Val);
            else if (Property == "ShadowOffsetX") SetOverrideValue(Rule->ShadowOffsetX, Val);
            else if (Property == "ShadowOffsetY") SetOverrideValue(Rule->ShadowOffsetY, Val);
            else return false;

            return true;
        }

        bool ApplyWidgetOverrideFlag(const std::string& Key, bool Val)
        {
            bool ByName = false;
            std::string Target;
            std::string Property;

            if (!ParseWidgetOverrideKey(Key, ByName, Target, Property))
                return false;

            if (Property != "Enable")
                return false;

            SetOverrideValue(FindOverrideRule(ByName, Target)->Enable, Val);
            return true;
        }

        const char* GetWidgetTypeName(const std::shared_ptr<CWidget>& Widget)
        {
            if (std::dynamic_pointer_cast<CTextBlock>(Widget))
                return "TextBlock";

            if (std::dynamic_pointer_cast<CButton>(Widget))
                return "Button";

            if (std::dynamic_pointer_cast<CImage>(Widget))
                return "Image";

            if (std::dynamic_pointer_cast<CWidgetContainer>(Widget))
                return "Container";

            return "Widget";
        }

        bool BuildCombinedOverrideRule(
            const std::string& WidgetPath,
            const std::string& WidgetName,
            FWidgetOverrideRule& OutRule)
        {
            bool HasOverride = false;
            const auto NameIt = GWidgetNameOverrides.find(WidgetName);
            const auto PathIt = GWidgetPathOverrides.find(WidgetPath);

            if (NameIt != GWidgetNameOverrides.end())
            {
                MergeOverrideRule(OutRule, NameIt->second);
                HasOverride = true;
            }

            if (PathIt != GWidgetPathOverrides.end())
            {
                MergeOverrideRule(OutRule, PathIt->second);
                HasOverride = true;
            }

            return HasOverride;
        }

        void ApplyOverrideRuleToWidget(
            const std::shared_ptr<CWidget>& Widget,
            const FWidgetOverrideRule& Rule)
        {
            if (!Widget)
                return;

            if (Rule.Enable.Set)
                Widget->SetEnable(Rule.Enable.Value);

            FVector3 Position = Widget->GetPos();

            if (Rule.PosX.Set)
                Position.x = Rule.PosX.Value;

            if (Rule.PosY.Set)
                Position.y = Rule.PosY.Value;

            if (Rule.OffsetX.Set)
                Position.x += Rule.OffsetX.Value;

            if (Rule.OffsetY.Set)
                Position.y += Rule.OffsetY.Value;

            Widget->SetPos(Position);

            FVector3 Size = Widget->GetSize();

            if (Rule.Width.Set)
                Size.x = Rule.Width.Value;

            if (Rule.Height.Set)
                Size.y = Rule.Height.Value;

            if (Rule.WidthAdd.Set)
                Size.x += Rule.WidthAdd.Value;

            if (Rule.HeightAdd.Set)
                Size.y += Rule.HeightAdd.Value;

            Size.x = (std::max)(0.f, Size.x);
            Size.y = (std::max)(0.f, Size.y);
            Widget->SetSize(Size);

            FVector3 Pivot = Widget->GetPivot();

            if (Rule.PivotX.Set)
                Pivot.x = Rule.PivotX.Value;

            if (Rule.PivotY.Set)
                Pivot.y = Rule.PivotY.Value;

            Widget->SetPivot(Pivot);

            if (Rule.ZOrder.Set)
                Widget->SetZOrder(static_cast<int>(Rule.ZOrder.Value));

            if (Rule.TintR.Set || Rule.TintG.Set || Rule.TintB.Set || Rule.TintA.Set)
            {
                FVector4 Tint = Widget->GetWidgetColor();

                if (Rule.TintR.Set)
                    Tint.x = Rule.TintR.Value;

                if (Rule.TintG.Set)
                    Tint.y = Rule.TintG.Value;

                if (Rule.TintB.Set)
                    Tint.z = Rule.TintB.Value;

                if (Rule.TintA.Set)
                    Tint.w = Rule.TintA.Value;

                Widget->SetWidgetColor(Tint);
            }

            if (Rule.Opacity.Set)
                Widget->SetOpacity(Rule.Opacity.Value);

            const auto Text = std::dynamic_pointer_cast<CTextBlock>(Widget);

            if (!Text)
                return;

            float FontSize = Text->GetFontSize();

            if (Rule.FontSize.Set)
                FontSize = Rule.FontSize.Value;

            if (Rule.FontSizeAdd.Set)
                FontSize += Rule.FontSizeAdd.Value;

            if (Rule.FontSize.Set || Rule.FontSizeAdd.Set)
                Text->SetFontSize((std::max)(0.f, FontSize));

            if (Rule.ShadowOffsetX.Set || Rule.ShadowOffsetY.Set)
            {
                FVector2 ShadowOffset = Text->GetShadowOffset();

                if (Rule.ShadowOffsetX.Set)
                    ShadowOffset.x = Rule.ShadowOffsetX.Value;

                if (Rule.ShadowOffsetY.Set)
                    ShadowOffset.y = Rule.ShadowOffsetY.Value;

                Text->SetShadowOffset(ShadowOffset);
            }
        }

        std::wstring BuildPrimaryConfigPath()
        {
            return RuntimeConfigRegistry::BuildExeRelativePath(
                GPrimaryConfigFile);
        }

        std::wstring BuildSplitConfigDirectoryPath()
        {
            return RuntimeConfigRegistry::BuildExeRelativePath(
                GSplitConfigDirectory);
        }

        std::wstring BuildWidgetTemplatePath()
        {
            return RuntimeConfigRegistry::BuildExeRelativePath(
                GWidgetTemplateFile);
        }

        unsigned long long ConvertFileTimeToTicks(const FILETIME& Time)
        {
            ULARGE_INTEGER Value = {};
            Value.LowPart = Time.dwLowDateTime;
            Value.HighPart = Time.dwHighDateTime;
            return Value.QuadPart;
        }

        bool TryGetTrackedFileWriteTime(
            const std::wstring& Path,
            unsigned long long& OutWriteTime)
        {
            WIN32_FILE_ATTRIBUTE_DATA Attributes = {};

            if (!GetFileAttributesExW(
                    Path.c_str(),
                    GetFileExInfoStandard,
                    &Attributes) ||
                (Attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                OutWriteTime = 0;
                return false;
            }

            OutWriteTime = ConvertFileTimeToTicks(Attributes.ftLastWriteTime);
            return true;
        }

        std::vector<FTrackedConfigFile> BuildTrackedConfigFiles()
        {
            std::vector<FTrackedConfigFile> Result;
            unsigned long long WriteTime = 0;
            const std::wstring PrimaryPath = BuildPrimaryConfigPath();

            if (TryGetTrackedFileWriteTime(PrimaryPath, WriteTime))
                Result.push_back({ PrimaryPath, WriteTime });

            const std::wstring DirectoryPath = BuildSplitConfigDirectoryPath();
            WIN32_FIND_DATAW FindData = {};
            const HANDLE FindHandle = FindFirstFileW(
                (DirectoryPath + L"*.ini").c_str(),
                &FindData);

            if (FindHandle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) !=
                        0)
                    {
                        continue;
                    }

                    Result.push_back(
                        {
                            DirectoryPath + FindData.cFileName,
                            ConvertFileTimeToTicks(FindData.ftLastWriteTime)
                        });
                } while (FindNextFileW(FindHandle, &FindData));

                FindClose(FindHandle);
            }

            if (Result.size() > 1)
            {
                std::sort(
                    Result.begin() + 1,
                    Result.end(),
                    [](const FTrackedConfigFile& Left,
                        const FTrackedConfigFile& Right)
                    {
                        return Left.Path < Right.Path;
                    });
            }

            return Result;
        }

        bool TrackedConfigFilesMatch(
            const std::vector<FTrackedConfigFile>& Left,
            const std::vector<FTrackedConfigFile>& Right)
        {
            if (Left.size() != Right.size())
                return false;

            for (size_t Index = 0; Index < Left.size(); ++Index)
            {
                if (Left[Index].Path != Right[Index].Path ||
                    Left[Index].WriteTime != Right[Index].WriteTime)
                {
                    return false;
                }
            }

            return true;
        }

        void StripUtf8Bom(std::string& Line)
        {
            constexpr char GUtf8Bom[] = "\xEF\xBB\xBF";

            if (Line.rfind(GUtf8Bom, 0) == 0)
                Line.erase(0, 3);
        }

        bool ReadBinaryFile(const std::wstring& Path, std::string& OutContent)
        {
            std::ifstream File(Path, std::ios::binary);

            if (!File.is_open())
                return false;

            std::ostringstream Buffer;
            Buffer << File.rdbuf();
            OutContent = Buffer.str();
            return true;
        }

        bool WriteBinaryFile(const std::wstring& Path, const std::string& Content)
        {
            std::ofstream File(Path, std::ios::binary | std::ios::trunc);

            if (!File.is_open())
                return false;

            File.write(Content.data(), Content.size());
            return true;
        }

        void ReloadTrackedConfigFiles(
            const std::vector<FTrackedConfigFile>& ConfigFiles)
        {
            ResetToDefaults();

            for (size_t Index = 0; Index < ConfigFiles.size(); ++Index)
                LoadFile(ConfigFiles[Index].Path);

            GTrackedConfigFiles = ConfigFiles;
            GConfigCheckCooldown = GConfigCheckIntervalSeconds;
            GConfigRegistered = true;
            ++GConfigGeneration;
        }

        std::string DetectLineEnding(const std::string& Text)
        {
            return Text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
        }

        std::string FormatFloatValue(float Value)
        {
            if (!std::isfinite(Value))
                Value = 0.f;

            if (std::fabs(Value) < 0.0005f)
                Value = 0.f;

            std::ostringstream Stream;
            Stream << std::fixed << std::setprecision(3) << Value;
            std::string Result = Stream.str();

            while (!Result.empty() && Result.back() == '0')
                Result.pop_back();

            if (!Result.empty() && Result.back() == '.')
                Result.pop_back();

            return Result.empty() ? "0" : Result;
        }

        std::string FormatBoolValue(bool Value)
        {
            return Value ? "1" : "0";
        }

        std::string BuildWidgetTemplateIntro(const std::string& NewLine)
        {
            std::ostringstream Stream;
            Stream << GWidgetTemplateBeginMarker << NewLine;
            Stream << "# Auto-generated widget override reference."
                << NewLine;
            Stream << "# Runtime-loaded overrides live in Binary/UILayout/*.ini."
                << NewLine;
            Stream << "# Copy a line from this file into a loaded config file,"
                << " then remove the leading '# ' to activate it." << NewLine;
            Stream << "# UIWidgetPaths.txt also lists every widget path."
                << NewLine;
            Stream << "# Full-path syntax: Widget.TopHudWidget/TopHud_DateText"
                << ".OffsetY = -4" << NewLine;
            Stream << "# Supported props: PosX PosY OffsetX OffsetY Width Height"
                << " WidthAdd HeightAdd PivotX PivotY Opacity ZOrder TintR"
                << " TintG TintB TintA FontSize FontSizeAdd ShadowOffsetX"
                << " ShadowOffsetY Enable" << NewLine;
            Stream << NewLine;
            return Stream.str();
        }

        void CollectManagedWidgetOverrideKeys(
            const std::string& SectionText,
            std::unordered_set<std::string>& OutKeys)
        {
            std::istringstream Stream(SectionText);
            std::string Line;

            while (std::getline(Stream, Line))
            {
                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                TrimString(Line);

                if (Line.empty())
                    continue;

                if (Line[0] == '#' || Line[0] == ';')
                {
                    Line.erase(Line.begin());
                    TrimString(Line);
                }

                if (Line.empty())
                    continue;

                const size_t EqPos = Line.find('=');
                std::string Key =
                    EqPos == std::string::npos ? Line : Line.substr(0, EqPos);
                TrimString(Key);

                if (Key.rfind("Widget.", 0) == 0 ||
                    Key.rfind("WidgetName.", 0) == 0)
                {
                    OutKeys.insert(Key);
                }
            }
        }

        void AppendCommentedWidgetOverrideLine(
            std::ostringstream& Stream,
            const std::string& NewLine,
            const std::string& Key,
            const std::string& Value)
        {
            Stream << "# " << Key << " = " << Value << NewLine;
        }

        void AppendWidgetOverrideTemplateRecursive(
            const std::shared_ptr<CWidget>& Widget,
            const std::string& WidgetPath,
            const std::unordered_set<std::string>& ExistingKeys,
            const std::string& NewLine,
            std::ostringstream& Stream)
        {
            if (!Widget)
                return;

            const FVector3 Position = Widget->GetPos();
            const FVector3 Size = Widget->GetSize();
            const FVector3 Pivot = Widget->GetPivot();
            const FVector4 Tint = Widget->GetWidgetColor();

            std::vector<std::pair<std::string, std::string>> Entries;
            const std::string KeyPrefix = "Widget." + WidgetPath + ".";

            Entries.emplace_back(
                KeyPrefix + "PosX",
                FormatFloatValue(Position.x));
            Entries.emplace_back(
                KeyPrefix + "PosY",
                FormatFloatValue(Position.y));
            Entries.emplace_back(KeyPrefix + "OffsetX", "0");
            Entries.emplace_back(KeyPrefix + "OffsetY", "0");
            Entries.emplace_back(
                KeyPrefix + "Width",
                FormatFloatValue(Size.x));
            Entries.emplace_back(
                KeyPrefix + "Height",
                FormatFloatValue(Size.y));
            Entries.emplace_back(KeyPrefix + "WidthAdd", "0");
            Entries.emplace_back(KeyPrefix + "HeightAdd", "0");
            Entries.emplace_back(
                KeyPrefix + "PivotX",
                FormatFloatValue(Pivot.x));
            Entries.emplace_back(
                KeyPrefix + "PivotY",
                FormatFloatValue(Pivot.y));
            Entries.emplace_back(
                KeyPrefix + "Opacity",
                FormatFloatValue(Tint.w));
            Entries.emplace_back(
                KeyPrefix + "ZOrder",
                std::to_string(Widget->GetZOrder()));
            Entries.emplace_back(
                KeyPrefix + "TintR",
                FormatFloatValue(Tint.x));
            Entries.emplace_back(
                KeyPrefix + "TintG",
                FormatFloatValue(Tint.y));
            Entries.emplace_back(
                KeyPrefix + "TintB",
                FormatFloatValue(Tint.z));
            Entries.emplace_back(
                KeyPrefix + "TintA",
                FormatFloatValue(Tint.w));
            Entries.emplace_back(
                KeyPrefix + "Enable",
                FormatBoolValue(Widget->GetEnable()));

            if (const auto Text = std::dynamic_pointer_cast<CTextBlock>(Widget))
            {
                const FVector2 ShadowOffset = Text->GetShadowOffset();

                Entries.emplace_back(
                    KeyPrefix + "FontSize",
                    FormatFloatValue(Text->GetFontSize()));
                Entries.emplace_back(KeyPrefix + "FontSizeAdd", "0");
                Entries.emplace_back(
                    KeyPrefix + "ShadowOffsetX",
                    FormatFloatValue(ShadowOffset.x));
                Entries.emplace_back(
                    KeyPrefix + "ShadowOffsetY",
                    FormatFloatValue(ShadowOffset.y));
            }

            bool HasMissingEntry = false;

            for (size_t Index = 0; Index < Entries.size(); ++Index)
            {
                if (ExistingKeys.find(Entries[Index].first) == ExistingKeys.end())
                {
                    HasMissingEntry = true;
                    break;
                }
            }

            if (HasMissingEntry)
            {
                Stream << "# " << WidgetPath << " ["
                    << GetWidgetTypeName(Widget) << "]" << NewLine;

                for (size_t Index = 0; Index < Entries.size(); ++Index)
                {
                    if (ExistingKeys.find(Entries[Index].first) !=
                        ExistingKeys.end())
                    {
                        continue;
                    }

                    AppendCommentedWidgetOverrideLine(
                        Stream,
                        NewLine,
                        Entries[Index].first,
                        Entries[Index].second);
                }

                Stream << NewLine;
            }

            if (const auto Button = std::dynamic_pointer_cast<CButton>(Widget))
            {
                const auto Child = Button->GetChild();

                if (Child)
                {
                    AppendWidgetOverrideTemplateRecursive(
                        Child,
                        WidgetPath + "/" + Child->GetName(),
                        ExistingKeys,
                        NewLine,
                        Stream);
                }
            }

            const auto Container = std::dynamic_pointer_cast<CWidgetContainer>(
                Widget);

            if (!Container)
                return;

            const auto& Children = Container->GetChildList();

            for (size_t Index = 0; Index < Children.size(); ++Index)
            {
                const auto& Child = Children[Index];

                if (!Child)
                    continue;

                AppendWidgetOverrideTemplateRecursive(
                    Child,
                    WidgetPath + "/" + Child->GetName(),
                    ExistingKeys,
                    NewLine,
                    Stream);
            }
        }

        std::string BuildMissingWidgetTemplateText(
            const std::shared_ptr<CWorldUIManager>& UIManager,
            const std::unordered_set<std::string>& ExistingKeys,
            const std::string& NewLine)
        {
            if (!UIManager)
                return std::string();

            std::ostringstream Stream;
            const auto& WidgetList = UIManager->GetWidgetList();

            for (size_t Index = 0; Index < WidgetList.size(); ++Index)
            {
                const auto& RootWidget = WidgetList[Index];

                if (!RootWidget)
                    continue;

                AppendWidgetOverrideTemplateRecursive(
                    RootWidget,
                    RootWidget->GetName(),
                    ExistingKeys,
                    NewLine,
                    Stream);
            }

            return Stream.str();
        }

        void SyncWidgetTemplateSection(
            const std::shared_ptr<CWorldUIManager>& UIManager)
        {
            if (!UIManager)
                return;

            const std::string NewLine = "\r\n";
            const std::unordered_set<std::string> ExistingKeys;
            const std::string TemplateBody = BuildMissingWidgetTemplateText(
                UIManager,
                ExistingKeys,
                NewLine);

            if (TemplateBody.empty())
                return;

            std::string UpdatedContent = "\xEF\xBB\xBF";
            UpdatedContent += BuildWidgetTemplateIntro(NewLine);
            UpdatedContent += TemplateBody;
            UpdatedContent += GWidgetTemplateEndMarker;
            UpdatedContent += NewLine;

            std::string ExistingContent;

            if (ReadBinaryFile(BuildWidgetTemplatePath(), ExistingContent) &&
                ExistingContent == UpdatedContent)
            {
                return;
            }

            WriteBinaryFile(BuildWidgetTemplatePath(), UpdatedContent);
        }

        void HashWidgetPathString(
            unsigned long long& Signature,
            const std::string& Text)
        {
            for (size_t Index = 0; Index < Text.size(); ++Index)
            {
                Signature ^= static_cast<unsigned long long>(
                    static_cast<unsigned char>(Text[Index]));
                Signature *= 1099511628211ull;
            }
        }

        void BuildWidgetTreeSignatureRecursive(
            const std::shared_ptr<CWidget>& Widget,
            const std::string& WidgetPath,
            unsigned long long& Signature)
        {
            if (!Widget)
                return;

            HashWidgetPathString(Signature, WidgetPath);
            HashWidgetPathString(Signature, GetWidgetTypeName(Widget));

            if (const auto Button = std::dynamic_pointer_cast<CButton>(Widget))
            {
                const auto Child = Button->GetChild();

                if (Child)
                {
                    BuildWidgetTreeSignatureRecursive(
                        Child,
                        WidgetPath + "/" + Child->GetName(),
                        Signature);
                }
            }

            const auto Container = std::dynamic_pointer_cast<CWidgetContainer>(
                Widget);

            if (!Container)
                return;

            const auto& Children = Container->GetChildList();

            for (size_t Index = 0; Index < Children.size(); ++Index)
            {
                const auto& Child = Children[Index];

                if (!Child)
                    continue;

                BuildWidgetTreeSignatureRecursive(
                    Child,
                    WidgetPath + "/" + Child->GetName(),
                    Signature);
            }
        }

        unsigned long long BuildWidgetTreeSignature(
            const std::shared_ptr<CWorldUIManager>& UIManager)
        {
            if (!UIManager)
                return 0;

            unsigned long long Signature = 1469598103934665603ull;
            const auto& WidgetList = UIManager->GetWidgetList();

            for (size_t Index = 0; Index < WidgetList.size(); ++Index)
            {
                const auto& RootWidget = WidgetList[Index];

                if (!RootWidget)
                    continue;

                BuildWidgetTreeSignatureRecursive(
                    RootWidget,
                    RootWidget->GetName(),
                    Signature);
            }

            return Signature;
        }

        void DumpWidgetPathsRecursive(
            const std::shared_ptr<CWidget>& Widget,
            const std::string& WidgetPath,
            std::ofstream& File)
        {
            if (!Widget)
                return;

            File << WidgetPath << " [" << GetWidgetTypeName(Widget) << "]\n";

            if (const auto Button = std::dynamic_pointer_cast<CButton>(Widget))
            {
                const auto Child = Button->GetChild();

                if (Child)
                {
                    DumpWidgetPathsRecursive(
                        Child,
                        WidgetPath + "/" + Child->GetName(),
                        File);
                }
            }

            const auto Container = std::dynamic_pointer_cast<CWidgetContainer>(Widget);

            if (!Container)
                return;

            const auto& Children = Container->GetChildList();

            for (size_t Index = 0; Index < Children.size(); ++Index)
            {
                const auto& Child = Children[Index];

                if (!Child)
                    continue;

                DumpWidgetPathsRecursive(
                    Child,
                    WidgetPath + "/" + Child->GetName(),
                    File);
            }
        }

        void DumpWidgetPaths(const std::shared_ptr<CWorldUIManager>& UIManager)
        {
            if (!UIManager)
                return;

            std::ofstream File(
                RuntimeConfigRegistry::BuildExeRelativePath(GWidgetPathDumpFile));

            if (!File.is_open())
                return;

            File << "# Auto-generated widget path list for UILayout.ini\n";
            File << "# Full-path syntax: Widget.TopHudWidget/TopHud_DateText.OffsetY = -4\n";
            File << "# Name-wide syntax: WidgetName.TopHud_DateText.OffsetY = -4\n";
            File << "# Supported props: PosX PosY OffsetX OffsetY Width Height WidthAdd HeightAdd\n";
            File << "#                  PivotX PivotY Opacity ZOrder TintR TintG TintB TintA\n";
            File << "#                  FontSize FontSizeAdd ShadowOffsetX ShadowOffsetY Enable\n\n";

            const auto& WidgetList = UIManager->GetWidgetList();

            for (size_t Index = 0; Index < WidgetList.size(); ++Index)
            {
                const auto& RootWidget = WidgetList[Index];

                if (!RootWidget)
                    continue;

                DumpWidgetPathsRecursive(
                    RootWidget,
                    RootWidget->GetName(),
                    File);
            }
        }

        void ApplyWidgetOverridesRecursive(
            const std::shared_ptr<CWidget>& Widget,
            const std::string& WidgetPath)
        {
            if (!Widget)
                return;

            FWidgetOverrideRule Rule;

            if (BuildCombinedOverrideRule(WidgetPath, Widget->GetName(), Rule))
                ApplyOverrideRuleToWidget(Widget, Rule);

            if (const auto Button = std::dynamic_pointer_cast<CButton>(Widget))
            {
                const auto Child = Button->GetChild();

                if (Child)
                {
                    ApplyWidgetOverridesRecursive(
                        Child,
                        WidgetPath + "/" + Child->GetName());
                }
            }

            const auto Container = std::dynamic_pointer_cast<CWidgetContainer>(Widget);

            if (!Container)
                return;

            const auto& Children = Container->GetChildList();

            for (size_t Index = 0; Index < Children.size(); ++Index)
            {
                const auto& Child = Children[Index];

                if (!Child)
                    continue;

                ApplyWidgetOverridesRecursive(
                    Child,
                    WidgetPath + "/" + Child->GetName());
            }
        }

        void ResetToDefaults()
        {
            GWidgetPathOverrides.clear();
            GWidgetNameOverrides.clear();

            StatusBarX              = 14.f;
            StatusBarY              = 10.f;
            StatusBarHeight         = 94.f;
            StatusBarPaddingX       = 20.f;
            StatusBudgetBlockWidth  = 218.f;
            StatusNpcBlockWidth     = 128.f;
            StatusSupportBlockWidth = 132.f;
            StatusBlockGap          = 18.f;
            StatusIconSize          = 26.f;
            StatusIconTextGap       = 10.f;
            StatusLabelOffsetY      = 12.f;
            StatusValueOffsetY      = 30.f;
            StatusLabelHeight       = 16.f;
            StatusValueHeight       = 30.f;
            StatusLabelFontSize     = 12.f;
            StatusValueFontSize     = 22.f;

            BudgetBlockOffsetX  = 0.f;
            BudgetBlockOffsetY  = 0.f;
            NpcBlockOffsetX     = 0.f;
            NpcBlockOffsetY     = 0.f;
            SupportBlockOffsetX = 0.f;
            SupportBlockOffsetY = 0.f;

            SpeedPanelX             = 16.f;
            SpeedPanelWidth         = 388.f;
            SpeedPanelHeight        = 182.f;
            SpeedPanelBottomMargin  = 18.f;
            SpeedPanelMinY          = 12.f;
            PanelTextOffsetX        = 74.f;
            TimeBarOffsetY          = 16.f;
            TimeBarWidth            = 226.f;
            TimeBarHeight           = 14.f;
            DateTextOffsetY         = 34.f;
            DateTextWidth           = 240.f;
            DateTextHeight          = 28.f;
            DateFontSize            = 22.f;

            PlayPauseButtonSize           = 44.f;
            PlayPauseButtonOffsetX        = 24.f;
            PlayPauseButtonOffsetY        = 0.f;
            PlayPauseButtonBottomMargin   = 18.f;
            SpeedMultiplierButtonSize     = 44.f;
            SpeedMultiplierButtonOffsetX  = 76.f;
            SpeedMultiplierButtonOffsetY  = 0.f;
            SpeedMultiplierButtonBottomMargin = 18.f;

            SpeedButtonSize         = 44.f;
            SpeedButtonStep         = 52.f;
            SpeedButtonOffsetX      = 24.f;
            SpeedButtonBottomMargin = 18.f;

            MenuButtonSize         = 60.f;
            MenuButtonGap          = 10.f;
            MenuLabelGap           = 8.f;
            MenuButtonStartOffsetX = 26.f;
            MenuButtonOffsetY      = 8.f;
            MenuMinWidth           = 120.f;
            MenuRightMargin        = 14.f;
            MenuMinScaleFactor     = 0.70f;
            MenuLabelBaseFontSize  = 12.5f;
            MenuLabelBaseHeight    = 18.f;

            EdictPanelWidth            = 1120.f;
            EdictPanelHeight           = 760.f;
            EdictHeaderTopPadding      = 40.f;
            EdictHeaderHeight          = 48.f;
            EdictHorizontalMargin      = 24.f;
            EdictVerticalMargin        = 18.f;
            EdictGridFrameHeight       = 404.f;
            EdictGridGapFromHeader     = 10.f;
            EdictDetailGapFromGrid     = 12.f;
            EdictTitleFontSize         = 30.f;
            EdictCategoryWidth         = 74.f;
            EdictCategoryHeight        = 90.f;
            EdictCategoryGap           = 12.f;
            EdictSlotPaddingLeft       = 18.f;
            EdictSlotPaddingTop        = 26.f;
            EdictSlotGapX              = 12.f;
            EdictSlotGapY              = 14.f;
            EdictDetailTitleFontSize   = 23.f;
            EdictDetailBodyFontSize    = 15.f;
            EdictDetailCostFontSize    = 18.f;
            EdictTitleTextOffsetX      = 0.f;
            EdictTitleTextOffsetY      = 0.f;
            EdictSlotTextOffsetX       = 0.f;
            EdictSlotTextOffsetY       = 0.f;
            EdictDetailTitleOffsetX    = 0.f;
            EdictDetailTitleOffsetY    = 0.f;
            EdictDetailCostRowWidth    = 140.f;
            EdictDetailCostRowOffsetX  = 0.f;
            EdictDetailCostRowOffsetY  = 0.f;
            EdictDetailCostIconSize    = 24.f;
            EdictDetailCostIconOffsetX = 0.f;
            EdictDetailCostIconOffsetY = 0.f;
            EdictDetailCostOffsetX     = 0.f;
            EdictDetailCostOffsetY     = 0.f;
            EdictDetailCostTextWidth   = 108.f;
            EdictDetailCostTextHeight  = 30.f;
            EdictDetailInfoInlineWithCost = true;
            EdictDetailInfoInlineGapX  = 12.f;
            EdictDetailInfoPanelOffsetX = 0.f;
            EdictDetailInfoPanelOffsetY = 0.f;
            EdictDetailInfoPanelWidthAdjust = 0.f;
            EdictDetailInfoPanelHeightAdjust = 0.f;
            EdictDetailInfoTextWidth  = 0.f;
            EdictDetailInfoTextHeight = 20.f;
            EdictDetailInfoOffsetX     = 0.f;
            EdictDetailInfoOffsetY     = 0.f;
            EdictFeedbackOffsetX       = 0.f;
            EdictFeedbackOffsetY       = 0.f;
            EdictDetailBodyOffsetX     = 0.f;
            EdictDetailBodyOffsetY     = 0.f;
            EdictRequirementOffsetX    = 0.f;
            EdictRequirementOffsetY    = 0.f;
            EdictApplyButtonOffsetX    = 0.f;
            EdictApplyButtonOffsetY    = 0.f;
            EdictApplyButtonTextOffsetX = 0.f;
            EdictApplyButtonTextOffsetY = 0.f;
            EdictTaxPolicyTitleOffsetX = 0.f;
            EdictTaxPolicyTitleOffsetY = 0.f;
            EdictTaxPolicyRowTextOffsetX = 0.f;
            EdictTaxPolicyRowTextOffsetY = 0.f;
            EdictTaxPolicySummaryOffsetX = 0.f;
            EdictTaxPolicySummaryOffsetY = 0.f;
            EdictApplyButtonWidth      = 140.f;
            EdictApplyButtonHeight     = 38.f;
            EdictScrollTrackWidth      = 10.f;
            EdictCloseButtonSize       = 40.f;
            EdictEnableTaxPolicyPanel  = false;
            EdictTaxPolicyPanelWidth   = 296.f;
            EdictTaxPolicyPanelHeight  = 168.f;
            EdictTaxPolicySummaryHeight = 40.f;

            AlmanacPanelWidth          = 1120.f;
            AlmanacPanelHeight         = 756.f;
            AlmanacPanelTopOffset      = 18.f;
            AlmanacHeaderHeight        = 82.f;
            AlmanacHeaderPadding       = 30.f;
            AlmanacRibbonTopOffset     = 18.f;
            AlmanacFrameInsetX         = 16.f;
            AlmanacFrameHeaderOverlap  = 10.f;
            AlmanacFrameBottomInset    = 14.f;
            AlmanacRailLeftInset       = 8.f;
            AlmanacRailTopInset        = 22.f;
            AlmanacRailBottomInset     = 46.f;
            AlmanacRailThumbTopOffset  = 10.f;
            AlmanacRailThumbMinHeight  = 88.f;
            AlmanacRailThumbExpand     = 1.f;
            AlmanacRailToContentGap    = 14.f;
            AlmanacContentMarginX      = 42.f;
            AlmanacContentMarginTop    = 20.f;
            AlmanacContentMarginBottom = 30.f;
            AlmanacContentTopInset     = 18.f;
            AlmanacContentBottomInset  = 18.f;
            AlmanacTabSize             = 64.f;
            AlmanacTabGap              = 10.f;
            AlmanacTabBaseOffsetY      = 22.f;
            AlmanacTabSelectedOffsetY  = 6.f;
            AlmanacTitleFontSize       = 30.f;
            AlmanacTitlePaddingX       = 34.f;
            AlmanacTitlePaddingY       = 1.f;
            AlmanacCloseButtonSize     = 40.f;
            AlmanacCloseButtonOffsetX  = 44.f;
            AlmanacCloseButtonOffsetY  = 10.f;
            AlmanacMetricRowHeight     = 46.f;
            AlmanacMetricRowGap        = 6.f;
            AlmanacDetailRowHeight     = 42.f;
            AlmanacDetailRowGap        = 6.f;
            AlmanacCardColumns         = 3.f;
            AlmanacCardGapX            = 18.f;
            AlmanacCardGapY            = 18.f;
            AlmanacLeftPanelRatio      = 0.47f;
            AlmanacPageColumnGap       = 22.f;
            AlmanacWidePageColumnGap   = 24.f;
            AlmanacPageTitleHeight     = 28.f;
            AlmanacPageFrameTop        = 34.f;
            AlmanacOffscreenHideOffset = 200.f;

            BuildingPanelWidthRatio          = 0.24f;
            BuildingPanelMinWidth            = 320.f;
            BuildingPanelMaxWidth            = 410.f;
            BuildingPanelTopOffset           = 58.f;
            BuildingPanelBottomMargin        = 10.f;
            BuildingPanelRightInset          = 10.f;
            BuildingPanelMinHeight           = 420.f;
            BuildingPanelInnerMarginX        = 18.f;
            BuildingPanelInnerMarginTop      = 16.f;
            BuildingPanelInnerBottomInset    = 28.f;
            BuildingTabWidth                 = 56.f;
            BuildingTabHeight                = 56.f;
            BuildingTabGap                   = 10.f;
            BuildingTabLabelFontSize         = 14.f;
            BuildingTabLabelOffsetX          = 0.f;
            BuildingTabLabelOffsetY          = 0.f;
            BuildingTitleFontSize            = 26.f;
            BuildingSubtitleFontSize         = 15.f;
            BuildingBodyFontSize             = 18.f;
            BuildingPageTitleFontSize        = 21.f;
            BuildingBudgetTextFontSize       = 16.f;
            BuildingOverviewWorkModeLabelFontSize = 18.f;
            BuildingOverviewWorkModeValueFontSize = 17.f;
            BuildingOverviewBudgetLabelFontSize = 18.f;
            BuildingOverviewBudgetValueFontSize = 18.f;
            BuildingOverviewOccupancyLabelFontSize = 18.f;
            BuildingOverviewOccupancyValueFontSize = 18.f;
            BuildingOverviewMetricLabelFontSize = 17.f;
            BuildingOverviewMetricSectionHeaderFontSize = 18.f;
            BuildingOverviewMetricValueFontSize = 17.f;
            BuildingUpgradeTitleFontSize    = 18.f;
            BuildingUpgradeDescriptionFontSize = 16.f;
            BuildingInformationAccentFontSize = 28.f;
            BuildingInformationBodyFontSize = 17.f;
            BuildingBudgetButtonFontSize    = 16.f;
            BuildingActionButtonFontSize    = 18.f;
            BuildingOverviewCommandButtonFontSize = 18.f;
            BuildingTitleIconInsetX          = 6.f;
            BuildingTitleIconOffsetY         = 0.f;
            BuildingTitleTextInsetX          = 14.f;
            BuildingTitleIconGap             = 6.f;
            BuildingTitleTextOffsetX         = 0.f;
            BuildingTitleTextOffsetY         = 0.f;
            BuildingTitleTextWidthAdjust     = 0.f;
            BuildingTitleTextHeightAdjust    = 0.f;
            BuildingSubtitleOffsetY          = 4.f;
            BuildingTitleRibbonOffsetX       = 40.f;
            BuildingTitleRibbonOffsetY       = 18.f;
            BuildingTitleRibbonHeight        = 48.f;
            BuildingSectionRibbonHeight      = 34.f;
            BuildingSectionRibbonOffsetY     = 28.f;
            BuildingCollapsedSectionGap      = 6.f;
            BuildingCloseButtonOffsetX       = 38.f;
            BuildingCloseButtonOffsetY       = 4.f;
            BuildingCloseButtonSize          = 34.f;
            BuildingCompactControlHeight     = 22.f;
            BuildingCompactBudgetButtonWidth = 36.f;
            BuildingSectionDividerWidth      = 172.f;
            BuildingSectionDividerHeight     = 14.f;
            BuildingBudgetBaseOffsetY        = 36.f;
            BuildingBudgetLabelOffsetY       = 2.f;
            BuildingOverviewWorkModeLabelOffsetX = 0.f;
            BuildingOverviewWorkModeLabelOffsetY = 0.f;
            BuildingOverviewWorkModeLabelWidthAdjust = 0.f;
            BuildingOverviewWorkModeBackgroundOffsetX = 0.f;
            BuildingOverviewWorkModeBackgroundOffsetY = 0.f;
            BuildingOverviewWorkModeBackgroundWidthAdjust = 0.f;
            BuildingOverviewWorkModeBackgroundHeightAdjust = 0.f;
            BuildingOverviewWorkModeTextOffsetX = 0.f;
            BuildingOverviewWorkModeTextOffsetY = 0.f;
            BuildingOverviewWorkModeTextWidthAdjust = 0.f;
            BuildingOverviewWorkModeTextHeightAdjust = 0.f;
            BuildingOverviewBudgetLabelOffsetX = 0.f;
            BuildingOverviewBudgetLabelOffsetY = 0.f;
            BuildingOverviewBudgetLabelWidthAdjust = 0.f;
            BuildingOverviewBudgetValueOffsetX = 0.f;
            BuildingOverviewBudgetValueOffsetY = 0.f;
            BuildingOverviewBudgetValueWidthAdjust = 0.f;
            BuildingBudgetCustomButtonsOffsetY = 20.f;
            BuildingBudgetWorkButtonsOffsetY = 78.f;
            BuildingBudgetDefaultButtonsOffsetY = 26.f;
            BuildingBudgetCompactGap         = 6.f;
            BuildingBudgetDefaultGap         = 8.f;
            BuildingOccupancyGapY            = 18.f;
            BuildingActionButtonHeight       = 34.f;
            BuildingActionButtonWidth        = 124.f;
            BuildingActionButtonBottomMargin = 52.f;
            BuildingActionCompactIconSize    = 34.f;
            BuildingActionCompactIconOffsetY = 2.f;
            BuildingMoveCompactRightOffset   = 82.f;
            BuildingFocusCompactRightOffset  = 40.f;
            BuildingOverviewCommandGap       = 10.f;
            BuildingBudgetButtonHeight       = 30.f;
            BuildingScrollTrackWidth         = 15.f;
            BuildingScrollBottomInset        = 52.f;
            BuildingScrollThumbHeight        = 94.f;
            BuildingScrollThumbTopOffset     = 14.f;
            BuildingBodyGapAfterSection      = 8.f;
            BuildingBodyGapAfterActions      = 14.f;
            BuildingBodyGapBeforeActions     = 12.f;
            BuildingBodyFallbackOffset       = 4.f;
            BuildingBodyBottomInset          = 22.f;
            BuildingIconSize                 = 36.f;

            CitizenPanelWidthRatio       = 0.24f;
            CitizenPanelMinWidth         = 320.f;
            CitizenPanelMaxWidth         = 410.f;
            CitizenPanelTopOffset        = 58.f;
            CitizenPanelBottomMargin     = 10.f;
            CitizenPanelRightInset       = 10.f;
            CitizenPanelMinHeight        = 420.f;
            CitizenPanelInnerMarginX     = 18.f;
            CitizenPanelInnerTopOffset   = 16.f;
            CitizenPanelInnerBottomInset = 28.f;
            CitizenTitleFontSize         = 26.f;
            CitizenSubtitleFontSize      = 15.f;
            CitizenBodyFontSize          = 18.f;
            CitizenPageTitleFontSize     = 21.f;
            CitizenTitleRibbonHeight     = 44.f;
            CitizenSectionRibbonHeight   = 34.f;
            CitizenSectionRibbonOffsetY  = 28.f;
            CitizenScrollTrackWidth      = 15.f;
            CitizenScrollBottomInset     = 52.f;
            CitizenScrollThumbHeight     = 94.f;
            CitizenScrollThumbTopOffset  = 14.f;
            CitizenCloseButtonSize       = 34.f;
            CitizenCloseButtonOffsetY    = 4.f;
            CitizenBudgetBaseOffsetY     = 36.f;
            CitizenActionStackTopOffset  = 238.f;
            CitizenFooterBottomInset     = 28.f;
            CitizenBodyBottomInset       = 22.f;

            GameOverPanelWidth        = 720.f;
            GameOverPanelHeight       = 390.f;
            GameOverTitlePaddingX     = 70.f;
            GameOverTitleOffsetY      = 70.f;
            GameOverTitleHeight       = 48.f;
            GameOverBodyPaddingX      = 84.f;
            GameOverBodyOffsetY       = 138.f;
            GameOverBodyBottomPadding = 210.f;
        }

        void TrimString(std::string& S)
        {
            S.erase(S.begin(), std::find_if(S.begin(), S.end(),
                [](unsigned char C) { return !std::isspace(C); }));
            S.erase(std::find_if(S.rbegin(), S.rend(),
                [](unsigned char C) { return !std::isspace(C); }).base(),
                S.end());
        }

        bool TryParseBoolValue(std::string Value, bool& Result)
        {
            TrimString(Value);
            std::transform(
                Value.begin(),
                Value.end(),
                Value.begin(),
                [](unsigned char C)
                {
                    return static_cast<char>(std::tolower(C));
                });

            if (Value == "1" ||
                Value == "true" ||
                Value == "on" ||
                Value == "yes")
            {
                Result = true;
                return true;
            }

            if (Value == "0" ||
                Value == "false" ||
                Value == "off" ||
                Value == "no")
            {
                Result = false;
                return true;
            }

            return false;
        }

        // ── 서브 함수: 상태바 / 패널 / 버튼 ──────────────────
        bool ApplyValue_HUD(const std::string& Key, float Val)
        {
            if      (Key == "StatusBarX")              StatusBarX              = Val;
            else if (Key == "StatusBarY")              StatusBarY              = Val;
            else if (Key == "StatusBarHeight")         StatusBarHeight         = Val;
            else if (Key == "StatusBarPaddingX")       StatusBarPaddingX       = Val;
            else if (Key == "StatusBudgetBlockWidth")  StatusBudgetBlockWidth  = Val;
            else if (Key == "StatusNpcBlockWidth")     StatusNpcBlockWidth     = Val;
            else if (Key == "StatusSupportBlockWidth") StatusSupportBlockWidth = Val;
            else if (Key == "StatusBlockGap")          StatusBlockGap          = Val;
            else if (Key == "StatusIconSize")          StatusIconSize          = Val;
            else if (Key == "StatusIconTextGap")       StatusIconTextGap       = Val;
            else if (Key == "StatusLabelOffsetY")      StatusLabelOffsetY      = Val;
            else if (Key == "StatusValueOffsetY")      StatusValueOffsetY      = Val;
            else if (Key == "StatusLabelHeight")       StatusLabelHeight       = Val;
            else if (Key == "StatusValueHeight")       StatusValueHeight       = Val;
            else if (Key == "StatusLabelFontSize")     StatusLabelFontSize     = Val;
            else if (Key == "StatusValueFontSize")     StatusValueFontSize     = Val;
            else if (Key == "BudgetBlockOffsetX")  BudgetBlockOffsetX  = Val;
            else if (Key == "BudgetBlockOffsetY")  BudgetBlockOffsetY  = Val;
            else if (Key == "NpcBlockOffsetX")     NpcBlockOffsetX     = Val;
            else if (Key == "NpcBlockOffsetY")     NpcBlockOffsetY     = Val;
            else if (Key == "SupportBlockOffsetX") SupportBlockOffsetX = Val;
            else if (Key == "SupportBlockOffsetY") SupportBlockOffsetY = Val;
            else return false;
            return true;
        }

        bool ApplyValue_Panel(const std::string& Key, float Val)
        {
            if      (Key == "SpeedPanelX")            SpeedPanelX            = Val;
            else if (Key == "SpeedPanelWidth")        SpeedPanelWidth        = Val;
            else if (Key == "SpeedPanelHeight")       SpeedPanelHeight       = Val;
            else if (Key == "SpeedPanelBottomMargin") SpeedPanelBottomMargin = Val;
            else if (Key == "SpeedPanelMinY")         SpeedPanelMinY         = Val;
            else if (Key == "PanelTextOffsetX")       PanelTextOffsetX       = Val;
            else if (Key == "TimeBarOffsetY")         TimeBarOffsetY         = Val;
            else if (Key == "TimeBarWidth")           TimeBarWidth           = Val;
            else if (Key == "TimeBarHeight")          TimeBarHeight          = Val;
            else if (Key == "DateTextOffsetY")        DateTextOffsetY        = Val;
            else if (Key == "DateTextWidth")          DateTextWidth          = Val;
            else if (Key == "DateTextHeight")         DateTextHeight         = Val;
            else if (Key == "DateFontSize")           DateFontSize           = Val;
            else if (Key == "PlayPauseButtonSize")       PlayPauseButtonSize       = Val;
            else if (Key == "PlayPauseButtonOffsetX")    PlayPauseButtonOffsetX    = Val;
            else if (Key == "PlayPauseButtonOffsetY")    PlayPauseButtonOffsetY    = Val;
            else if (Key == "PlayPauseButtonBottomMargin") PlayPauseButtonBottomMargin = Val;
            else if (Key == "SpeedMultiplierButtonSize") SpeedMultiplierButtonSize = Val;
            else if (Key == "SpeedMultiplierButtonOffsetX") SpeedMultiplierButtonOffsetX = Val;
            else if (Key == "SpeedMultiplierButtonOffsetY") SpeedMultiplierButtonOffsetY = Val;
            else if (Key == "SpeedMultiplierButtonBottomMargin") SpeedMultiplierButtonBottomMargin = Val;
            else if (Key == "SpeedButtonSize")
            {
                SpeedButtonSize = Val;
                PlayPauseButtonSize = Val;
                SpeedMultiplierButtonSize = Val;
            }
            else if (Key == "SpeedButtonStep")
            {
                SpeedButtonStep = Val;
                SpeedMultiplierButtonOffsetX =
                    SpeedButtonOffsetX + SpeedButtonStep;
            }
            else if (Key == "SpeedButtonOffsetX")
            {
                SpeedButtonOffsetX = Val;
                PlayPauseButtonOffsetX = Val;
                SpeedMultiplierButtonOffsetX =
                    SpeedButtonOffsetX + SpeedButtonStep;
            }
            else if (Key == "SpeedButtonBottomMargin")
            {
                SpeedButtonBottomMargin = Val;
                PlayPauseButtonBottomMargin = Val;
                SpeedMultiplierButtonBottomMargin = Val;
            }
            else return false;
            return true;
        }

        bool ApplyValue_Menu(const std::string& Key, float Val)
        {
            if      (Key == "MenuButtonSize")         MenuButtonSize         = Val;
            else if (Key == "MenuButtonGap")          MenuButtonGap          = Val;
            else if (Key == "MenuLabelGap")           MenuLabelGap           = Val;
            else if (Key == "MenuButtonStartOffsetX") MenuButtonStartOffsetX = Val;
            else if (Key == "MenuButtonOffsetY")      MenuButtonOffsetY      = Val;
            else if (Key == "MenuMinWidth")           MenuMinWidth           = Val;
            else if (Key == "MenuRightMargin")        MenuRightMargin        = Val;
            else if (Key == "MenuMinScaleFactor")     MenuMinScaleFactor     = Val;
            else if (Key == "MenuLabelBaseFontSize")  MenuLabelBaseFontSize  = Val;
            else if (Key == "MenuLabelBaseHeight")    MenuLabelBaseHeight    = Val;
            else if (Key == "GameOverPanelWidth")        GameOverPanelWidth        = Val;
            else if (Key == "GameOverPanelHeight")       GameOverPanelHeight       = Val;
            else if (Key == "GameOverTitlePaddingX")     GameOverTitlePaddingX     = Val;
            else if (Key == "GameOverTitleOffsetY")      GameOverTitleOffsetY      = Val;
            else if (Key == "GameOverTitleHeight")       GameOverTitleHeight       = Val;
            else if (Key == "GameOverBodyPaddingX")      GameOverBodyPaddingX      = Val;
            else if (Key == "GameOverBodyOffsetY")       GameOverBodyOffsetY       = Val;
            else if (Key == "GameOverBodyBottomPadding") GameOverBodyBottomPadding = Val;
            else return false;
            return true;
        }

        bool ApplyValue_Edict(const std::string& Key, float Val)
        {
            if      (Key == "EdictPanelWidth")          EdictPanelWidth          = Val;
            else if (Key == "EdictPanelHeight")         EdictPanelHeight         = Val;
            else if (Key == "EdictHeaderTopPadding")    EdictHeaderTopPadding    = Val;
            else if (Key == "EdictHeaderHeight")        EdictHeaderHeight        = Val;
            else if (Key == "EdictHorizontalMargin")    EdictHorizontalMargin    = Val;
            else if (Key == "EdictVerticalMargin")      EdictVerticalMargin      = Val;
            else if (Key == "EdictGridFrameHeight")     EdictGridFrameHeight     = Val;
            else if (Key == "EdictGridGapFromHeader")   EdictGridGapFromHeader   = Val;
            else if (Key == "EdictDetailGapFromGrid")   EdictDetailGapFromGrid   = Val;
            else if (Key == "EdictTitleFontSize")       EdictTitleFontSize       = Val;
            else if (Key == "EdictCategoryWidth")       EdictCategoryWidth       = Val;
            else if (Key == "EdictCategoryHeight")      EdictCategoryHeight      = Val;
            else if (Key == "EdictCategoryGap")         EdictCategoryGap         = Val;
            else if (Key == "EdictSlotPaddingLeft")     EdictSlotPaddingLeft     = Val;
            else if (Key == "EdictSlotPaddingTop")      EdictSlotPaddingTop      = Val;
            else if (Key == "EdictSlotGapX")            EdictSlotGapX            = Val;
            else if (Key == "EdictSlotGapY")            EdictSlotGapY            = Val;
            else if (Key == "EdictDetailTitleFontSize") EdictDetailTitleFontSize = Val;
            else if (Key == "EdictDetailBodyFontSize")  EdictDetailBodyFontSize  = Val;
            else if (Key == "EdictDetailCostFontSize")  EdictDetailCostFontSize  = Val;
            else if (Key == "EdictTitleTextOffsetX")    EdictTitleTextOffsetX    = Val;
            else if (Key == "EdictTitleTextOffsetY")    EdictTitleTextOffsetY    = Val;
            else if (Key == "EdictSlotTextOffsetX")     EdictSlotTextOffsetX     = Val;
            else if (Key == "EdictSlotTextOffsetY")     EdictSlotTextOffsetY     = Val;
            else if (Key == "EdictDetailTitleOffsetX")  EdictDetailTitleOffsetX  = Val;
            else if (Key == "EdictDetailTitleOffsetY")  EdictDetailTitleOffsetY  = Val;
            else if (Key == "EdictDetailCostRowWidth")  EdictDetailCostRowWidth  = Val;
            else if (Key == "EdictDetailCostRowOffsetX") EdictDetailCostRowOffsetX = Val;
            else if (Key == "EdictDetailCostRowOffsetY") EdictDetailCostRowOffsetY = Val;
            else if (Key == "EdictDetailCostIconSize")  EdictDetailCostIconSize  = Val;
            else if (Key == "EdictDetailCostIconOffsetX") EdictDetailCostIconOffsetX = Val;
            else if (Key == "EdictDetailCostIconOffsetY") EdictDetailCostIconOffsetY = Val;
            else if (Key == "EdictDetailCostOffsetX")   EdictDetailCostOffsetX   = Val;
            else if (Key == "EdictDetailCostOffsetY")   EdictDetailCostOffsetY   = Val;
            else if (Key == "EdictDetailCostTextWidth") EdictDetailCostTextWidth = Val;
            else if (Key == "EdictDetailCostTextHeight") EdictDetailCostTextHeight = Val;
            else if (Key == "EdictDetailInfoInlineGapX") EdictDetailInfoInlineGapX = Val;
            else if (Key == "EdictDetailInfoPanelOffsetX") EdictDetailInfoPanelOffsetX = Val;
            else if (Key == "EdictDetailInfoPanelOffsetY") EdictDetailInfoPanelOffsetY = Val;
            else if (Key == "EdictDetailInfoPanelWidthAdjust") EdictDetailInfoPanelWidthAdjust = Val;
            else if (Key == "EdictDetailInfoPanelHeightAdjust") EdictDetailInfoPanelHeightAdjust = Val;
            else if (Key == "EdictDetailInfoTextWidth") EdictDetailInfoTextWidth = Val;
            else if (Key == "EdictDetailInfoTextHeight") EdictDetailInfoTextHeight = Val;
            else if (Key == "EdictDetailInfoOffsetX")   EdictDetailInfoOffsetX   = Val;
            else if (Key == "EdictDetailInfoOffsetY")   EdictDetailInfoOffsetY   = Val;
            else if (Key == "EdictFeedbackOffsetX")     EdictFeedbackOffsetX     = Val;
            else if (Key == "EdictFeedbackOffsetY")     EdictFeedbackOffsetY     = Val;
            else if (Key == "EdictDetailBodyOffsetX")   EdictDetailBodyOffsetX   = Val;
            else if (Key == "EdictDetailBodyOffsetY")   EdictDetailBodyOffsetY   = Val;
            else if (Key == "EdictRequirementOffsetX")  EdictRequirementOffsetX  = Val;
            else if (Key == "EdictRequirementOffsetY")  EdictRequirementOffsetY  = Val;
            else if (Key == "EdictApplyButtonOffsetX")  EdictApplyButtonOffsetX  = Val;
            else if (Key == "EdictApplyButtonOffsetY")  EdictApplyButtonOffsetY  = Val;
            else if (Key == "EdictApplyButtonTextOffsetX") EdictApplyButtonTextOffsetX = Val;
            else if (Key == "EdictApplyButtonTextOffsetY") EdictApplyButtonTextOffsetY = Val;
            else if (Key == "EdictTaxPolicyTitleOffsetX") EdictTaxPolicyTitleOffsetX = Val;
            else if (Key == "EdictTaxPolicyTitleOffsetY") EdictTaxPolicyTitleOffsetY = Val;
            else if (Key == "EdictTaxPolicyRowTextOffsetX") EdictTaxPolicyRowTextOffsetX = Val;
            else if (Key == "EdictTaxPolicyRowTextOffsetY") EdictTaxPolicyRowTextOffsetY = Val;
            else if (Key == "EdictTaxPolicySummaryOffsetX") EdictTaxPolicySummaryOffsetX = Val;
            else if (Key == "EdictTaxPolicySummaryOffsetY") EdictTaxPolicySummaryOffsetY = Val;
            else if (Key == "EdictApplyButtonWidth")    EdictApplyButtonWidth    = Val;
            else if (Key == "EdictApplyButtonHeight")   EdictApplyButtonHeight   = Val;
            else if (Key == "EdictScrollTrackWidth")    EdictScrollTrackWidth    = Val;
            else if (Key == "EdictCloseButtonSize")     EdictCloseButtonSize     = Val;
            else if (Key == "EdictTaxPolicyPanelWidth") EdictTaxPolicyPanelWidth = Val;
            else if (Key == "EdictTaxPolicyPanelHeight") EdictTaxPolicyPanelHeight = Val;
            else if (Key == "EdictTaxPolicySummaryHeight") EdictTaxPolicySummaryHeight = Val;
            else return false;
            return true;
        }

        bool ApplyValue_EdictFlags(const std::string& Key, bool Val)
        {
            if (Key == "EdictEnableTaxPolicyPanel")
                EdictEnableTaxPolicyPanel = Val;
            else if (Key == "EdictDetailInfoInlineWithCost")
                EdictDetailInfoInlineWithCost = Val;
            else
                return false;

            return true;
        }

        bool ApplyValue_Almanac(const std::string& Key, float Val)
        {
            if      (Key == "AlmanacPanelWidth")          AlmanacPanelWidth          = Val;
            else if (Key == "AlmanacPanelHeight")         AlmanacPanelHeight         = Val;
            else if (Key == "AlmanacPanelTopOffset")      AlmanacPanelTopOffset      = Val;
            else if (Key == "AlmanacHeaderHeight")        AlmanacHeaderHeight        = Val;
            else if (Key == "AlmanacHeaderPadding")       AlmanacHeaderPadding       = Val;
            else if (Key == "AlmanacRibbonTopOffset")     AlmanacRibbonTopOffset     = Val;
            else if (Key == "AlmanacFrameInsetX")         AlmanacFrameInsetX         = Val;
            else if (Key == "AlmanacFrameHeaderOverlap")  AlmanacFrameHeaderOverlap  = Val;
            else if (Key == "AlmanacFrameBottomInset")    AlmanacFrameBottomInset    = Val;
            else if (Key == "AlmanacRailLeftInset")       AlmanacRailLeftInset       = Val;
            else if (Key == "AlmanacRailTopInset")        AlmanacRailTopInset        = Val;
            else if (Key == "AlmanacRailBottomInset")     AlmanacRailBottomInset     = Val;
            else if (Key == "AlmanacRailThumbTopOffset")  AlmanacRailThumbTopOffset  = Val;
            else if (Key == "AlmanacRailThumbMinHeight")  AlmanacRailThumbMinHeight  = Val;
            else if (Key == "AlmanacRailThumbExpand")     AlmanacRailThumbExpand     = Val;
            else if (Key == "AlmanacRailToContentGap")    AlmanacRailToContentGap    = Val;
            else if (Key == "AlmanacContentMarginX")      AlmanacContentMarginX      = Val;
            else if (Key == "AlmanacContentMarginTop")    AlmanacContentMarginTop    = Val;
            else if (Key == "AlmanacContentMarginBottom") AlmanacContentMarginBottom = Val;
            else if (Key == "AlmanacContentTopInset")     AlmanacContentTopInset     = Val;
            else if (Key == "AlmanacContentBottomInset")  AlmanacContentBottomInset  = Val;
            else if (Key == "AlmanacTabSize")             AlmanacTabSize             = Val;
            else if (Key == "AlmanacTabGap")              AlmanacTabGap              = Val;
            else if (Key == "AlmanacTabBaseOffsetY")      AlmanacTabBaseOffsetY      = Val;
            else if (Key == "AlmanacTabSelectedOffsetY")  AlmanacTabSelectedOffsetY  = Val;
            else if (Key == "AlmanacTitleFontSize")       AlmanacTitleFontSize       = Val;
            else if (Key == "AlmanacTitlePaddingX")       AlmanacTitlePaddingX       = Val;
            else if (Key == "AlmanacTitlePaddingY")       AlmanacTitlePaddingY       = Val;
            else if (Key == "AlmanacCloseButtonSize")     AlmanacCloseButtonSize     = Val;
            else if (Key == "AlmanacCloseButtonOffsetX")  AlmanacCloseButtonOffsetX  = Val;
            else if (Key == "AlmanacCloseButtonOffsetY")  AlmanacCloseButtonOffsetY  = Val;
            else if (Key == "AlmanacMetricRowHeight")     AlmanacMetricRowHeight     = Val;
            else if (Key == "AlmanacMetricRowGap")        AlmanacMetricRowGap        = Val;
            else if (Key == "AlmanacDetailRowHeight")     AlmanacDetailRowHeight     = Val;
            else if (Key == "AlmanacDetailRowGap")        AlmanacDetailRowGap        = Val;
            else if (Key == "AlmanacCardColumns")         AlmanacCardColumns         = Val;
            else if (Key == "AlmanacCardGapX")            AlmanacCardGapX            = Val;
            else if (Key == "AlmanacCardGapY")            AlmanacCardGapY            = Val;
            else if (Key == "AlmanacLeftPanelRatio")      AlmanacLeftPanelRatio      = Val;
            else if (Key == "AlmanacPageColumnGap")       AlmanacPageColumnGap       = Val;
            else if (Key == "AlmanacWidePageColumnGap")   AlmanacWidePageColumnGap   = Val;
            else if (Key == "AlmanacPageTitleHeight")     AlmanacPageTitleHeight     = Val;
            else if (Key == "AlmanacPageFrameTop")        AlmanacPageFrameTop        = Val;
            else if (Key == "AlmanacOffscreenHideOffset") AlmanacOffscreenHideOffset = Val;
            else return false;
            return true;
        }

        bool ApplyValue_BuildingCitizenLayout(const std::string& Key, float Val)
        {
            if      (Key == "BuildingTitleIconInsetX")          BuildingTitleIconInsetX          = Val;
            else if (Key == "BuildingTitleIconOffsetY")         BuildingTitleIconOffsetY         = Val;
            else if (Key == "BuildingTitleTextInsetX")          BuildingTitleTextInsetX          = Val;
            else if (Key == "BuildingTitleIconGap")             BuildingTitleIconGap             = Val;
            else if (Key == "BuildingTitleTextOffsetX")         BuildingTitleTextOffsetX         = Val;
            else if (Key == "BuildingTitleTextOffsetY")         BuildingTitleTextOffsetY         = Val;
            else if (Key == "BuildingTitleTextWidthAdjust")     BuildingTitleTextWidthAdjust     = Val;
            else if (Key == "BuildingTitleTextHeightAdjust")    BuildingTitleTextHeightAdjust    = Val;
            else if (Key == "BuildingSubtitleOffsetY")          BuildingSubtitleOffsetY          = Val;
            else if (Key == "BuildingTitleRibbonOffsetX")       BuildingTitleRibbonOffsetX       = Val;
            else if (Key == "BuildingTitleRibbonOffsetY")       BuildingTitleRibbonOffsetY       = Val;
            else if (Key == "BuildingTitleRibbonHeight")        BuildingTitleRibbonHeight        = Val;
            else if (Key == "BuildingSectionRibbonHeight")      BuildingSectionRibbonHeight      = Val;
            else if (Key == "BuildingSectionRibbonOffsetY")     BuildingSectionRibbonOffsetY     = Val;
            else if (Key == "BuildingCollapsedSectionGap")      BuildingCollapsedSectionGap      = Val;
            else if (Key == "BuildingCloseButtonOffsetX")       BuildingCloseButtonOffsetX       = Val;
            else if (Key == "BuildingCloseButtonOffsetY")       BuildingCloseButtonOffsetY       = Val;
            else if (Key == "BuildingCloseButtonSize")          BuildingCloseButtonSize          = Val;
            else if (Key == "BuildingCompactControlHeight")     BuildingCompactControlHeight     = Val;
            else if (Key == "BuildingCompactBudgetButtonWidth") BuildingCompactBudgetButtonWidth = Val;
            else if (Key == "BuildingSectionDividerWidth")      BuildingSectionDividerWidth      = Val;
            else if (Key == "BuildingSectionDividerHeight")     BuildingSectionDividerHeight     = Val;
            else if (Key == "BuildingBudgetBaseOffsetY")        BuildingBudgetBaseOffsetY        = Val;
            else if (Key == "BuildingBudgetLabelOffsetY")       BuildingBudgetLabelOffsetY       = Val;
            else if (Key == "BuildingOverviewWorkModeLabelOffsetX") BuildingOverviewWorkModeLabelOffsetX = Val;
            else if (Key == "BuildingOverviewWorkModeLabelOffsetY") BuildingOverviewWorkModeLabelOffsetY = Val;
            else if (Key == "BuildingOverviewWorkModeLabelWidthAdjust") BuildingOverviewWorkModeLabelWidthAdjust = Val;
            else if (Key == "BuildingOverviewWorkModeBackgroundOffsetX") BuildingOverviewWorkModeBackgroundOffsetX = Val;
            else if (Key == "BuildingOverviewWorkModeBackgroundOffsetY") BuildingOverviewWorkModeBackgroundOffsetY = Val;
            else if (Key == "BuildingOverviewWorkModeBackgroundWidthAdjust") BuildingOverviewWorkModeBackgroundWidthAdjust = Val;
            else if (Key == "BuildingOverviewWorkModeBackgroundHeightAdjust") BuildingOverviewWorkModeBackgroundHeightAdjust = Val;
            else if (Key == "BuildingOverviewWorkModeTextOffsetX") BuildingOverviewWorkModeTextOffsetX = Val;
            else if (Key == "BuildingOverviewWorkModeTextOffsetY") BuildingOverviewWorkModeTextOffsetY = Val;
            else if (Key == "BuildingOverviewWorkModeTextWidthAdjust") BuildingOverviewWorkModeTextWidthAdjust = Val;
            else if (Key == "BuildingOverviewWorkModeTextHeightAdjust") BuildingOverviewWorkModeTextHeightAdjust = Val;
            else if (Key == "BuildingOverviewBudgetLabelOffsetX") BuildingOverviewBudgetLabelOffsetX = Val;
            else if (Key == "BuildingOverviewBudgetLabelOffsetY") BuildingOverviewBudgetLabelOffsetY = Val;
            else if (Key == "BuildingOverviewBudgetLabelWidthAdjust") BuildingOverviewBudgetLabelWidthAdjust = Val;
            else if (Key == "BuildingOverviewBudgetValueOffsetX") BuildingOverviewBudgetValueOffsetX = Val;
            else if (Key == "BuildingOverviewBudgetValueOffsetY") BuildingOverviewBudgetValueOffsetY = Val;
            else if (Key == "BuildingOverviewBudgetValueWidthAdjust") BuildingOverviewBudgetValueWidthAdjust = Val;
            else if (Key == "BuildingBudgetCustomButtonsOffsetY") BuildingBudgetCustomButtonsOffsetY = Val;
            else if (Key == "BuildingBudgetWorkButtonsOffsetY") BuildingBudgetWorkButtonsOffsetY = Val;
            else if (Key == "BuildingBudgetDefaultButtonsOffsetY") BuildingBudgetDefaultButtonsOffsetY = Val;
            else if (Key == "BuildingBudgetCompactGap")         BuildingBudgetCompactGap         = Val;
            else if (Key == "BuildingBudgetDefaultGap")         BuildingBudgetDefaultGap         = Val;
            else if (Key == "BuildingOccupancyGapY")            BuildingOccupancyGapY            = Val;
            else if (Key == "BuildingActionButtonHeight")       BuildingActionButtonHeight       = Val;
            else if (Key == "BuildingActionButtonWidth")        BuildingActionButtonWidth        = Val;
            else if (Key == "BuildingActionButtonBottomMargin") BuildingActionButtonBottomMargin = Val;
            else if (Key == "BuildingActionCompactIconSize")    BuildingActionCompactIconSize    = Val;
            else if (Key == "BuildingActionCompactIconOffsetY") BuildingActionCompactIconOffsetY = Val;
            else if (Key == "BuildingMoveCompactRightOffset")   BuildingMoveCompactRightOffset   = Val;
            else if (Key == "BuildingFocusCompactRightOffset")  BuildingFocusCompactRightOffset  = Val;
            else if (Key == "BuildingOverviewCommandGap")       BuildingOverviewCommandGap       = Val;
            else if (Key == "BuildingBudgetButtonHeight")       BuildingBudgetButtonHeight       = Val;
            else if (Key == "BuildingScrollTrackWidth")         BuildingScrollTrackWidth         = Val;
            else if (Key == "BuildingScrollBottomInset")        BuildingScrollBottomInset        = Val;
            else if (Key == "BuildingScrollThumbHeight")        BuildingScrollThumbHeight        = Val;
            else if (Key == "BuildingScrollThumbTopOffset")     BuildingScrollThumbTopOffset     = Val;
            else if (Key == "BuildingBodyGapAfterSection")      BuildingBodyGapAfterSection      = Val;
            else if (Key == "BuildingBodyGapAfterActions")      BuildingBodyGapAfterActions      = Val;
            else if (Key == "BuildingBodyGapBeforeActions")     BuildingBodyGapBeforeActions     = Val;
            else if (Key == "BuildingBodyFallbackOffset")       BuildingBodyFallbackOffset       = Val;
            else if (Key == "BuildingBodyBottomInset")          BuildingBodyBottomInset          = Val;
            else if (Key == "BuildingIconSize")                 BuildingIconSize                 = Val;
            else return false;

            return true;
        }

        bool ApplyValue_CitizenPanel(const std::string& Key, float Val)
        {
            if      (Key == "CitizenPanelWidthRatio")     CitizenPanelWidthRatio     = Val;
            else if (Key == "CitizenPanelMinWidth")       CitizenPanelMinWidth       = Val;
            else if (Key == "CitizenPanelMaxWidth")       CitizenPanelMaxWidth       = Val;
            else if (Key == "CitizenPanelTopOffset")      CitizenPanelTopOffset      = Val;
            else if (Key == "CitizenPanelBottomMargin")   CitizenPanelBottomMargin   = Val;
            else if (Key == "CitizenPanelRightInset")     CitizenPanelRightInset     = Val;
            else if (Key == "CitizenPanelMinHeight")      CitizenPanelMinHeight      = Val;
            else if (Key == "CitizenPanelInnerMarginX")   CitizenPanelInnerMarginX   = Val;
            else if (Key == "CitizenPanelInnerTopOffset") CitizenPanelInnerTopOffset = Val;
            else if (Key == "CitizenPanelInnerBottomInset") CitizenPanelInnerBottomInset = Val;
            else if (Key == "CitizenTitleFontSize")       CitizenTitleFontSize       = Val;
            else if (Key == "CitizenSubtitleFontSize")    CitizenSubtitleFontSize    = Val;
            else if (Key == "CitizenBodyFontSize")        CitizenBodyFontSize        = Val;
            else if (Key == "CitizenPageTitleFontSize")   CitizenPageTitleFontSize   = Val;
            else if (Key == "CitizenTitleRibbonHeight")   CitizenTitleRibbonHeight   = Val;
            else if (Key == "CitizenSectionRibbonHeight") CitizenSectionRibbonHeight = Val;
            else if (Key == "CitizenSectionRibbonOffsetY") CitizenSectionRibbonOffsetY = Val;
            else if (Key == "CitizenScrollTrackWidth")    CitizenScrollTrackWidth    = Val;
            else if (Key == "CitizenScrollBottomInset")   CitizenScrollBottomInset   = Val;
            else if (Key == "CitizenScrollThumbHeight")   CitizenScrollThumbHeight   = Val;
            else if (Key == "CitizenScrollThumbTopOffset") CitizenScrollThumbTopOffset = Val;
            else if (Key == "CitizenCloseButtonSize")     CitizenCloseButtonSize     = Val;
            else if (Key == "CitizenCloseButtonOffsetY")  CitizenCloseButtonOffsetY  = Val;
            else if (Key == "CitizenBudgetBaseOffsetY")   CitizenBudgetBaseOffsetY   = Val;
            else if (Key == "CitizenActionStackTopOffset") CitizenActionStackTopOffset = Val;
            else if (Key == "CitizenFooterBottomInset")   CitizenFooterBottomInset   = Val;
            else if (Key == "CitizenBodyBottomInset")     CitizenBodyBottomInset     = Val;
            else return false;

            return true;
        }

        bool ApplyValue_BuildingCitizen(const std::string& Key, float Val)
        {
            if      (Key == "BuildingPanelWidthRatio")          BuildingPanelWidthRatio          = Val;
            else if (Key == "BuildingPanelMinWidth")            BuildingPanelMinWidth            = Val;
            else if (Key == "BuildingPanelMaxWidth")            BuildingPanelMaxWidth            = Val;
            else if (Key == "BuildingPanelTopOffset")           BuildingPanelTopOffset           = Val;
            else if (Key == "BuildingPanelBottomMargin")        BuildingPanelBottomMargin        = Val;
            else if (Key == "BuildingPanelRightInset")          BuildingPanelRightInset          = Val;
            else if (Key == "BuildingPanelMinHeight")           BuildingPanelMinHeight           = Val;
            else if (Key == "BuildingPanelInnerMarginX")        BuildingPanelInnerMarginX        = Val;
            else if (Key == "BuildingPanelInnerMarginTop")      BuildingPanelInnerMarginTop      = Val;
            else if (Key == "BuildingPanelInnerBottomInset")    BuildingPanelInnerBottomInset    = Val;
            else if (Key == "BuildingTabWidth")                 BuildingTabWidth                 = Val;
            else if (Key == "BuildingTabHeight")                BuildingTabHeight                = Val;
            else if (Key == "BuildingTabGap")                   BuildingTabGap                   = Val;
            else if (Key == "BuildingTabLabelFontSize")         BuildingTabLabelFontSize         = Val;
            else if (Key == "BuildingTabLabelOffsetX")          BuildingTabLabelOffsetX          = Val;
            else if (Key == "BuildingTabLabelOffsetY")          BuildingTabLabelOffsetY          = Val;
            else if (Key == "BuildingTitleFontSize")            BuildingTitleFontSize            = Val;
            else if (Key == "BuildingSubtitleFontSize")         BuildingSubtitleFontSize         = Val;
            else if (Key == "BuildingBodyFontSize")             BuildingBodyFontSize             = Val;
            else if (Key == "BuildingPageTitleFontSize")        BuildingPageTitleFontSize        = Val;
            else if (Key == "BuildingBudgetTextFontSize")       BuildingBudgetTextFontSize       = Val;
            else if (Key == "BuildingOverviewWorkModeLabelFontSize") BuildingOverviewWorkModeLabelFontSize = Val;
            else if (Key == "BuildingOverviewWorkModeValueFontSize") BuildingOverviewWorkModeValueFontSize = Val;
            else if (Key == "BuildingOverviewBudgetLabelFontSize") BuildingOverviewBudgetLabelFontSize = Val;
            else if (Key == "BuildingOverviewBudgetValueFontSize") BuildingOverviewBudgetValueFontSize = Val;
            else if (Key == "BuildingOverviewOccupancyLabelFontSize") BuildingOverviewOccupancyLabelFontSize = Val;
            else if (Key == "BuildingOverviewOccupancyValueFontSize") BuildingOverviewOccupancyValueFontSize = Val;
            else if (Key == "BuildingOverviewMetricLabelFontSize") BuildingOverviewMetricLabelFontSize = Val;
            else if (Key == "BuildingOverviewMetricSectionHeaderFontSize") BuildingOverviewMetricSectionHeaderFontSize = Val;
            else if (Key == "BuildingOverviewMetricValueFontSize") BuildingOverviewMetricValueFontSize = Val;
            else if (Key == "BuildingUpgradeTitleFontSize")    BuildingUpgradeTitleFontSize    = Val;
            else if (Key == "BuildingUpgradeDescriptionFontSize") BuildingUpgradeDescriptionFontSize = Val;
            else if (Key == "BuildingInformationAccentFontSize") BuildingInformationAccentFontSize = Val;
            else if (Key == "BuildingInformationBodyFontSize") BuildingInformationBodyFontSize = Val;
            else if (Key == "BuildingBudgetButtonFontSize")    BuildingBudgetButtonFontSize    = Val;
            else if (Key == "BuildingActionButtonFontSize")    BuildingActionButtonFontSize    = Val;
            else if (Key == "BuildingOverviewCommandButtonFontSize") BuildingOverviewCommandButtonFontSize = Val;
            else if (ApplyValue_BuildingCitizenLayout(Key, Val)) return true;
            else if (ApplyValue_CitizenPanel(Key, Val)) return true;
            else return false;
            return true;
        }

        // 키 이름으로 값 적용 (서브함수들 위임)
        void ApplyFloatValue(const std::string& Key, float Val)
        {
            if (ApplyWidgetOverrideValue(Key, Val)) return;
            if (ApplyValue_HUD(Key, Val))            return;
            if (ApplyValue_Panel(Key, Val))          return;
            if (ApplyValue_Menu(Key, Val))           return;
            if (ApplyValue_Edict(Key, Val))          return;
            if (ApplyValue_Almanac(Key, Val))        return;
            if (ApplyValue_BuildingCitizen(Key, Val)) return;
        }

        bool LoadFile(const std::wstring& Path)
        {
            std::ifstream File(Path, std::ios::binary);

            if (!File.is_open())
                return false;

            std::string Line;

            while (std::getline(File, Line))
            {
                StripUtf8Bom(Line);
                TrimString(Line);

                // 주석(#, ;)과 빈 줄 무시
                if (Line.empty() || Line[0] == '#' || Line[0] == ';')
                    continue;

                const auto EqPos = Line.find('=');

                if (EqPos == std::string::npos)
                    continue;

                std::string Key   = Line.substr(0, EqPos);
                std::string Value = Line.substr(EqPos + 1);
                TrimString(Key);
                TrimString(Value);

                if (Key.empty() || Value.empty())
                    continue;

                try
                {
                    bool BoolValue = false;

                    if (TryParseBoolValue(Value, BoolValue) &&
                        (ApplyValue_EdictFlags(Key, BoolValue) ||
                            ApplyWidgetOverrideFlag(Key, BoolValue)))
                    {
                        continue;
                    }

                    ApplyFloatValue(Key, std::stof(Value));
                }
                catch (...)
                {
                    // 숫자로 변환 불가한 줄은 조용히 무시
                }
            }

            return true;
        }
    }

    void RegisterRuntimeConfig()
    {
        if (GConfigRegistered)
            return;

        ReloadTrackedConfigFiles(BuildTrackedConfigFiles());
    }

    void ApplyWidgetOverrides(const std::shared_ptr<CWorldUIManager>& UIManager)
    {
        if (!UIManager)
            return;

        const unsigned long long Generation = GConfigGeneration;
        const unsigned long long WidgetTreeSignature =
            BuildWidgetTreeSignature(UIManager);

        if (Generation != GLastDumpedWidgetPathGeneration ||
            WidgetTreeSignature != GLastWidgetTreeSignature)
        {
            DumpWidgetPaths(UIManager);
            SyncWidgetTemplateSection(UIManager);
            GLastDumpedWidgetPathGeneration = Generation;
            GLastWidgetTreeSignature = WidgetTreeSignature;
        }

        if (GWidgetPathOverrides.empty() && GWidgetNameOverrides.empty())
            return;

        const auto& WidgetList = UIManager->GetWidgetList();

        for (size_t Index = 0; Index < WidgetList.size(); ++Index)
        {
            const auto& RootWidget = WidgetList[Index];

            if (!RootWidget)
                continue;

            ApplyWidgetOverridesRecursive(
                RootWidget,
                RootWidget->GetName());
        }
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        RegisterRuntimeConfig();
        GConfigCheckCooldown -= (std::max)(0.f, DeltaTime);

        if (GConfigCheckCooldown > 0.f)
            return false;

        GConfigCheckCooldown = GConfigCheckIntervalSeconds;
        const std::vector<FTrackedConfigFile> ConfigFiles =
            BuildTrackedConfigFiles();

        if (TrackedConfigFilesMatch(ConfigFiles, GTrackedConfigFiles))
            return false;

        ReloadTrackedConfigFiles(ConfigFiles);
        return true;
    }

} // namespace UIConfig
