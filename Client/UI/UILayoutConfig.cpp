#include "UILayoutConfig.h"
#include <Windows.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

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

    // 건물 선택 UI
    float BuildingPanelWidthRatio          = 0.24f;
    float BuildingPanelMinWidth            = 320.f;
    float BuildingPanelMaxWidth            = 410.f;
    float BuildingPanelTopOffset           = 58.f;
    float BuildingPanelInnerMarginX        = 18.f;
    float BuildingPanelInnerMarginTop      = 16.f;
    float BuildingTabWidth                 = 56.f;
    float BuildingTabHeight                = 56.f;
    float BuildingTabGap                   = 10.f;
    float BuildingTitleFontSize            = 26.f;
    float BuildingSubtitleFontSize         = 15.f;
    float BuildingBodyFontSize             = 18.f;
    float BuildingTitleRibbonOffsetX       = 40.f;
    float BuildingTitleRibbonOffsetY       = 18.f;
    float BuildingTitleRibbonHeight        = 48.f;
    float BuildingCloseButtonOffsetX       = 38.f;
    float BuildingCloseButtonSize          = 34.f;
    float BuildingActionButtonHeight       = 34.f;
    float BuildingActionButtonWidth        = 124.f;
    float BuildingActionButtonBottomMargin = 52.f;
    float BuildingBudgetButtonHeight       = 30.f;
    float BuildingScrollTrackWidth         = 15.f;
    float BuildingIconSize                 = 36.f;

    // NPC 선택 UI
    float CitizenPanelWidthRatio      = 0.24f;
    float CitizenPanelMinWidth        = 320.f;
    float CitizenPanelMaxWidth        = 410.f;
    float CitizenPanelTopOffset       = 58.f;
    float CitizenPanelBottomMargin    = 10.f;
    float CitizenPanelInnerMarginX    = 18.f;
    float CitizenPanelInnerTopOffset  = 16.f;
    float CitizenPanelInnerBottomInset = 28.f;
    float CitizenTitleFontSize        = 26.f;
    float CitizenSubtitleFontSize     = 15.f;
    float CitizenBodyFontSize         = 18.f;
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

    // ── 내부 구현 ────────────────────────────────────────────

    namespace
    {
        // exe 옆에 있는 UILayout.ini 경로를 반환
        std::wstring GetConfigPath()
        {
            wchar_t ExePath[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, ExePath, MAX_PATH);

            std::wstring Path(ExePath);
            const auto Slash = Path.rfind(L'\\');

            if (Slash != std::wstring::npos)
                Path = Path.substr(0, Slash + 1);

            return Path + L"UILayout.ini";
        }

        // 마지막으로 읽은 파일 수정 시각
        __time64_t LastWriteTime = 0;

        // 파일 체크 주기 (초)
        float CheckInterval  = 0.5f;
        float CheckCooldown  = 0.f;

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
            else if (Key == "SpeedButtonSize")         SpeedButtonSize         = Val;
            else if (Key == "SpeedButtonStep")         SpeedButtonStep         = Val;
            else if (Key == "SpeedButtonOffsetX")      SpeedButtonOffsetX      = Val;
            else if (Key == "SpeedButtonBottomMargin") SpeedButtonBottomMargin = Val;
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
            else return false;
            return true;
        }

        bool ApplyValue_BuildingCitizen(const std::string& Key, float Val)
        {
            if      (Key == "BuildingPanelWidthRatio")          BuildingPanelWidthRatio          = Val;
            else if (Key == "BuildingPanelMinWidth")            BuildingPanelMinWidth            = Val;
            else if (Key == "BuildingPanelMaxWidth")            BuildingPanelMaxWidth            = Val;
            else if (Key == "BuildingPanelTopOffset")           BuildingPanelTopOffset           = Val;
            else if (Key == "BuildingPanelInnerMarginX")        BuildingPanelInnerMarginX        = Val;
            else if (Key == "BuildingPanelInnerMarginTop")      BuildingPanelInnerMarginTop      = Val;
            else if (Key == "BuildingTabWidth")                 BuildingTabWidth                 = Val;
            else if (Key == "BuildingTabHeight")                BuildingTabHeight                = Val;
            else if (Key == "BuildingTabGap")                   BuildingTabGap                   = Val;
            else if (Key == "BuildingTitleFontSize")            BuildingTitleFontSize            = Val;
            else if (Key == "BuildingSubtitleFontSize")         BuildingSubtitleFontSize         = Val;
            else if (Key == "BuildingBodyFontSize")             BuildingBodyFontSize             = Val;
            else if (Key == "BuildingTitleRibbonOffsetX")       BuildingTitleRibbonOffsetX       = Val;
            else if (Key == "BuildingTitleRibbonOffsetY")       BuildingTitleRibbonOffsetY       = Val;
            else if (Key == "BuildingTitleRibbonHeight")        BuildingTitleRibbonHeight        = Val;
            else if (Key == "BuildingCloseButtonOffsetX")       BuildingCloseButtonOffsetX       = Val;
            else if (Key == "BuildingCloseButtonSize")          BuildingCloseButtonSize          = Val;
            else if (Key == "BuildingActionButtonHeight")       BuildingActionButtonHeight       = Val;
            else if (Key == "BuildingActionButtonWidth")        BuildingActionButtonWidth        = Val;
            else if (Key == "BuildingActionButtonBottomMargin") BuildingActionButtonBottomMargin = Val;
            else if (Key == "BuildingBudgetButtonHeight")       BuildingBudgetButtonHeight       = Val;
            else if (Key == "BuildingScrollTrackWidth")         BuildingScrollTrackWidth         = Val;
            else if (Key == "BuildingIconSize")                 BuildingIconSize                 = Val;
            else if (Key == "CitizenPanelWidthRatio")     CitizenPanelWidthRatio     = Val;
            else if (Key == "CitizenPanelMinWidth")       CitizenPanelMinWidth       = Val;
            else if (Key == "CitizenPanelMaxWidth")       CitizenPanelMaxWidth       = Val;
            else if (Key == "CitizenPanelTopOffset")      CitizenPanelTopOffset      = Val;
            else if (Key == "CitizenPanelBottomMargin")   CitizenPanelBottomMargin   = Val;
            else if (Key == "CitizenPanelInnerMarginX")   CitizenPanelInnerMarginX   = Val;
            else if (Key == "CitizenPanelInnerTopOffset") CitizenPanelInnerTopOffset = Val;
            else if (Key == "CitizenPanelInnerBottomInset") CitizenPanelInnerBottomInset = Val;
            else if (Key == "CitizenTitleFontSize")       CitizenTitleFontSize       = Val;
            else if (Key == "CitizenSubtitleFontSize")    CitizenSubtitleFontSize    = Val;
            else if (Key == "CitizenBodyFontSize")        CitizenBodyFontSize        = Val;
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

        // 키 이름으로 값 적용 (서브함수들 위임)
        void ApplyFloatValue(const std::string& Key, float Val)
        {
            if (ApplyValue_HUD(Key, Val))            return;
            if (ApplyValue_Panel(Key, Val))          return;
            if (ApplyValue_Menu(Key, Val))           return;
            if (ApplyValue_Edict(Key, Val))          return;
            if (ApplyValue_Almanac(Key, Val))        return;
            if (ApplyValue_BuildingCitizen(Key, Val)) return;
        }

        void LoadFile(const std::wstring& Path)
        {
            std::ifstream File(Path);

            if (!File.is_open())
                return;

            std::string Line;

            while (std::getline(File, Line))
            {
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
                        ApplyValue_EdictFlags(Key, BoolValue))
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
        }
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        CheckCooldown -= DeltaTime;

        if (CheckCooldown > 0.f)
            return false;

        CheckCooldown = CheckInterval;

        const std::wstring Path = GetConfigPath();

        struct _stat64 Stat = {};

        if (_wstat64(Path.c_str(), &Stat) != 0)
            return false; // 파일 없음

        if (Stat.st_mtime == LastWriteTime)
            return false; // 변경 없음

        LastWriteTime = Stat.st_mtime;
        LoadFile(Path);
        return true;
    }

} // namespace UIConfig
