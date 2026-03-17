#pragma once

#include "Vector4.h"

namespace TropicoUiTheme
{
    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);
    unsigned long long GetRuntimeConfigGeneration();

    extern FVector4 GButtonDefaultNormalTint;
    extern FVector4 GButtonDefaultHoverTint;
    extern FVector4 GButtonDefaultClickTint;
    extern FVector4 GButtonDefaultDisableTint;

    extern FVector4 GButtonHighlightNormalTint;
    extern FVector4 GButtonHighlightHoverTint;
    extern FVector4 GButtonHighlightClickTint;
    extern FVector4 GButtonHighlightDisableTint;

    extern FVector4 GButtonIconSlotNormalTint;
    extern FVector4 GButtonIconSlotHoverTint;
    extern FVector4 GButtonIconSlotClickTint;
    extern FVector4 GButtonIconSlotDisableTint;

    extern FVector4 GButtonCategorySelectedNormalTint;
    extern FVector4 GButtonCategorySelectedHoverTint;
    extern FVector4 GButtonCategorySelectedClickTint;
    extern FVector4 GButtonCategorySelectedDisableTint;
    extern FVector4 GButtonCategoryNormalTint;
    extern FVector4 GButtonCategoryHoverTint;
    extern FVector4 GButtonCategoryClickTint;
    extern FVector4 GButtonCategoryDisableTint;

    extern FVector4 GStatusDangerTint;
    extern FVector4 GStatusWarningTint;
    extern FVector4 GStatusCautionTint;
    extern FVector4 GStatusSuccessTint;
    extern FVector4 GTextMutedTint;

    extern FVector4 GEdictSlotActiveTint;
    extern FVector4 GEdictSlotCoolingDownTint;
    extern FVector4 GEdictSlotUnavailableTint;
    extern FVector4 GEdictSlotFocusedTint;
    extern FVector4 GOverlayDisabledTint;

    extern FVector4 GCitizenInfoTitleTint;
    extern FVector4 GCitizenInfoSubtitleTint;
    extern FVector4 GCitizenInfoBodyTint;
    extern FVector4 GCitizenInfoSectionHeaderTint;
    extern FVector4 GCitizenInfoValueTint;
    extern FVector4 GCitizenInfoStrongValueTint;
    extern FVector4 GCitizenInfoAccentTint;
    extern FVector4 GCitizenInfoActionTextTint;
    extern FVector4 GCitizenInfoDividerTint;
    extern FVector4 GCitizenInfoPanelHighlightTint;
    extern FVector4 GCitizenInfoProgressRailTint;
    extern FVector4 GCitizenInfoProgressFillTint;
    extern FVector4 GCitizenInfoSupportNegativeTint;
    extern FVector4 GCitizenInfoSupportNeutralTint;
    extern FVector4 GCitizenInfoSupportPositiveTint;
    extern FVector4 GCitizenInfoFooterTint;
    extern FVector4 GCitizenInfoTabLabelSelectedTint;
    extern FVector4 GCitizenInfoTabLabelNormalTint;
    extern FVector4 GCitizenInfoTabIconSelectedTint;
    extern FVector4 GCitizenInfoTabIconNormalTint;
    extern FVector4 GCitizenInfoBudgetLabelSelectedTint;
    extern FVector4 GCitizenInfoBudgetLabelNormalTint;
    extern FVector4 GCitizenInfoMetricProfileTint;
    extern FVector4 GCitizenInfoMetricHeaderTint;
    extern FVector4 GCitizenInfoMetricLabelTint;
    extern FVector4 GCitizenInfoMetricValueTint;
    extern FVector4 GCitizenInfoMetricAccentTint;
    extern FVector4 GCitizenInfoEmptyResidentTint;
    extern FVector4 GCitizenInfoDisabledVisitorTint;

    const FVector4& GetAlmanacSatisfactionTint(int Index);
}
