#pragma once

// ============================================================
//  UI 레이아웃 런타임 설정
//
//  게임을 실행한 채로 Binary 폴더 안의 UILayout.ini 파일을
//  메모장 등으로 수정하고 저장하면 약 0.5초 뒤 화면에 바로
//  반영됩니다. 빌드(F7)가 필요 없습니다.
//
//  각 항목의 설명은 UILayout.ini 파일 안의 주석을 참고하세요.
// ============================================================

namespace UIConfig
{
    // ── 상단 상태바 ──────────────────────────────────────────
    extern float StatusBarX;
    extern float StatusBarY;
    extern float StatusBarHeight;
    extern float StatusBarPaddingX;
    extern float StatusBudgetBlockWidth;
    extern float StatusNpcBlockWidth;
    extern float StatusSupportBlockWidth;
    extern float StatusBlockGap;
    extern float StatusIconSize;
    extern float StatusIconTextGap;
    extern float StatusLabelOffsetY;
    extern float StatusValueOffsetY;
    extern float StatusLabelHeight;
    extern float StatusValueHeight;
    extern float StatusLabelFontSize;
    extern float StatusValueFontSize;

    // 블록별 미세 위치 조정 (기본 자동배치 위치에서 추가 이동)
    extern float BudgetBlockOffsetX;  // 예산 블록 X 보정
    extern float BudgetBlockOffsetY;  // 예산 블록 Y 보정
    extern float NpcBlockOffsetX;     // 인구 블록 X 보정
    extern float NpcBlockOffsetY;     // 인구 블록 Y 보정
    extern float SupportBlockOffsetX; // 지지율 블록 X 보정
    extern float SupportBlockOffsetY; // 지지율 블록 Y 보정

    // ── 하단 좌측 패널 ───────────────────────────────────────
    extern float SpeedPanelX;
    extern float SpeedPanelWidth;
    extern float SpeedPanelHeight;
    extern float SpeedPanelBottomMargin;
    extern float SpeedPanelMinY;
    extern float PanelTextOffsetX;
    extern float TimeBarOffsetY;
    extern float TimeBarWidth;
    extern float TimeBarHeight;
    extern float DateTextOffsetY;
    extern float DateTextWidth;
    extern float DateTextHeight;
    extern float DateFontSize;

    // ── 게임 속도 버튼 ───────────────────────────────────────
    extern float SpeedButtonSize;
    extern float SpeedButtonStep;
    extern float SpeedButtonOffsetX;
    extern float SpeedButtonBottomMargin;

    // ── 하단 메뉴 아이콘 버튼 ────────────────────────────────
    extern float MenuButtonSize;
    extern float MenuButtonGap;
    extern float MenuLabelGap;
    extern float MenuButtonStartOffsetX;
    extern float MenuButtonOffsetY;
    extern float MenuMinWidth;
    extern float MenuRightMargin;
    extern float MenuMinScaleFactor;
    extern float MenuLabelBaseFontSize;
    extern float MenuLabelBaseHeight;

    // ── 게임 오버 팝업 ───────────────────────────────────────
    extern float GameOverPanelWidth;
    extern float GameOverPanelHeight;
    extern float GameOverTitlePaddingX;
    extern float GameOverTitleOffsetY;
    extern float GameOverTitleHeight;
    extern float GameOverBodyPaddingX;
    extern float GameOverBodyOffsetY;
    extern float GameOverBodyBottomPadding;

    // ── 칙령 UI ──────────────────────────────────────────────
    extern float EdictPanelWidth;
    extern float EdictPanelHeight;
    extern float EdictHeaderTopPadding;
    extern float EdictHeaderHeight;
    extern float EdictHorizontalMargin;
    extern float EdictVerticalMargin;
    extern float EdictGridFrameHeight;
    extern float EdictGridGapFromHeader;
    extern float EdictDetailGapFromGrid;
    extern float EdictTitleFontSize;
    extern float EdictCategoryWidth;
    extern float EdictCategoryHeight;
    extern float EdictCategoryGap;
    extern float EdictSlotPaddingLeft;
    extern float EdictSlotPaddingTop;
    extern float EdictSlotGapX;
    extern float EdictSlotGapY;
    extern float EdictDetailTitleFontSize;
    extern float EdictDetailBodyFontSize;
    extern float EdictDetailCostFontSize;
    extern float EdictApplyButtonWidth;
    extern float EdictApplyButtonHeight;
    extern float EdictScrollTrackWidth;
    extern float EdictCloseButtonSize;
    extern bool EdictEnableTaxPolicyPanel;
    extern float EdictTaxPolicyPanelWidth;
    extern float EdictTaxPolicyPanelHeight;
    extern float EdictTaxPolicySummaryHeight;

    // ── 연감 UI ──────────────────────────────────────────────
    extern float AlmanacPanelWidth;
    extern float AlmanacPanelHeight;
    extern float AlmanacPanelTopOffset;
    extern float AlmanacHeaderHeight;
    extern float AlmanacHeaderPadding;
    extern float AlmanacRibbonTopOffset;
    extern float AlmanacFrameInsetX;
    extern float AlmanacFrameHeaderOverlap;
    extern float AlmanacFrameBottomInset;
    extern float AlmanacRailLeftInset;
    extern float AlmanacRailTopInset;
    extern float AlmanacRailBottomInset;
    extern float AlmanacRailThumbTopOffset;
    extern float AlmanacRailThumbMinHeight;
    extern float AlmanacRailThumbExpand;
    extern float AlmanacRailToContentGap;
    extern float AlmanacContentMarginX;
    extern float AlmanacContentMarginTop;
    extern float AlmanacContentMarginBottom;
    extern float AlmanacContentTopInset;
    extern float AlmanacContentBottomInset;
    extern float AlmanacTabSize;
    extern float AlmanacTabGap;
    extern float AlmanacTabBaseOffsetY;
    extern float AlmanacTabSelectedOffsetY;
    extern float AlmanacTitleFontSize;
    extern float AlmanacTitlePaddingX;
    extern float AlmanacTitlePaddingY;
    extern float AlmanacCloseButtonSize;
    extern float AlmanacCloseButtonOffsetX;
    extern float AlmanacCloseButtonOffsetY;
    extern float AlmanacMetricRowHeight;
    extern float AlmanacMetricRowGap;
    extern float AlmanacDetailRowHeight;
    extern float AlmanacDetailRowGap;
    extern float AlmanacCardColumns;
    extern float AlmanacCardGapX;
    extern float AlmanacCardGapY;
    extern float AlmanacLeftPanelRatio;
    extern float AlmanacPageColumnGap;
    extern float AlmanacWidePageColumnGap;
    extern float AlmanacPageTitleHeight;
    extern float AlmanacPageFrameTop;
    extern float AlmanacOffscreenHideOffset;

    // ── 건물 선택 UI ─────────────────────────────────────────
    extern float BuildingPanelWidthRatio;
    extern float BuildingPanelMinWidth;
    extern float BuildingPanelMaxWidth;
    extern float BuildingPanelTopOffset;
    extern float BuildingPanelInnerMarginX;
    extern float BuildingPanelInnerMarginTop;
    extern float BuildingTabWidth;
    extern float BuildingTabHeight;
    extern float BuildingTabGap;
    extern float BuildingTitleFontSize;
    extern float BuildingSubtitleFontSize;
    extern float BuildingBodyFontSize;
    extern float BuildingTitleRibbonOffsetX;
    extern float BuildingTitleRibbonOffsetY;
    extern float BuildingTitleRibbonHeight;
    extern float BuildingCloseButtonOffsetX;
    extern float BuildingCloseButtonSize;
    extern float BuildingActionButtonHeight;
    extern float BuildingActionButtonWidth;
    extern float BuildingActionButtonBottomMargin;
    extern float BuildingBudgetButtonHeight;
    extern float BuildingScrollTrackWidth;
    extern float BuildingIconSize;

    // ── NPC 선택 UI ──────────────────────────────────────────
    extern float CitizenPanelWidthRatio;
    extern float CitizenPanelMinWidth;
    extern float CitizenPanelMaxWidth;
    extern float CitizenPanelTopOffset;
    extern float CitizenPanelBottomMargin;
    extern float CitizenPanelRightInset;
    extern float CitizenPanelMinHeight;
    extern float CitizenPanelInnerMarginX;
    extern float CitizenPanelInnerTopOffset;
    extern float CitizenPanelInnerBottomInset;
    extern float CitizenTitleFontSize;
    extern float CitizenSubtitleFontSize;
    extern float CitizenBodyFontSize;
    extern float CitizenTitleRibbonHeight;
    extern float CitizenSectionRibbonHeight;
    extern float CitizenSectionRibbonOffsetY;
    extern float CitizenScrollTrackWidth;
    extern float CitizenScrollBottomInset;
    extern float CitizenScrollThumbHeight;
    extern float CitizenScrollThumbTopOffset;
    extern float CitizenCloseButtonSize;
    extern float CitizenCloseButtonOffsetY;
    extern float CitizenBudgetBaseOffsetY;
    extern float CitizenActionStackTopOffset;
    extern float CitizenFooterBottomInset;
    extern float CitizenBodyBottomInset;

    void RegisterRuntimeConfig();

    // ── 런타임 리로드 ────────────────────────────────────────
    // 공용 RuntimeConfigRegistry를 쓰지 않을 때만 수동 호출하세요.
    // INI 파일이 변경되었으면 다시 읽고 true 를 반환합니다.
    bool ReloadIfChanged(float DeltaTime);

} // namespace UIConfig
