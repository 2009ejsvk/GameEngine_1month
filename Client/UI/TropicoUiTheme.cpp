#include "TropicoUiTheme.h"
#include "../RuntimeConfigRegistry.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <string>

namespace TropicoUiTheme
{
    FVector4 GButtonDefaultNormalTint(0.20f, 0.22f, 0.26f, 0.92f);
    FVector4 GButtonDefaultHoverTint(0.26f, 0.30f, 0.35f, 0.95f);
    FVector4 GButtonDefaultClickTint(0.14f, 0.16f, 0.20f, 0.98f);
    FVector4 GButtonDefaultDisableTint(0.10f, 0.10f, 0.12f, 0.70f);

    FVector4 GButtonHighlightNormalTint(0.10f, 0.32f, 0.52f, 0.95f);
    FVector4 GButtonHighlightHoverTint(0.16f, 0.40f, 0.62f, 0.98f);
    FVector4 GButtonHighlightClickTint(0.08f, 0.24f, 0.40f, 0.98f);
    FVector4 GButtonHighlightDisableTint(0.08f, 0.24f, 0.40f, 0.70f);

    FVector4 GButtonIconSlotNormalTint(1.f, 1.f, 1.f, 0.96f);
    FVector4 GButtonIconSlotHoverTint(1.f, 1.f, 1.f, 1.f);
    FVector4 GButtonIconSlotClickTint(0.80f, 0.80f, 0.80f, 1.f);
    FVector4 GButtonIconSlotDisableTint(0.35f, 0.35f, 0.35f, 0.75f);

    FVector4 GButtonCategorySelectedNormalTint(1.f, 1.f, 1.f, 1.f);
    FVector4 GButtonCategorySelectedHoverTint(1.f, 1.f, 1.f, 1.f);
    FVector4 GButtonCategorySelectedClickTint(0.90f, 0.90f, 0.90f, 1.f);
    FVector4 GButtonCategorySelectedDisableTint(0.50f, 0.50f, 0.50f, 0.75f);
    FVector4 GButtonCategoryNormalTint(0.74f, 0.84f, 0.98f, 0.96f);
    FVector4 GButtonCategoryHoverTint(0.88f, 0.95f, 1.f, 1.f);
    FVector4 GButtonCategoryClickTint(0.62f, 0.74f, 0.92f, 1.f);
    FVector4 GButtonCategoryDisableTint(0.50f, 0.50f, 0.50f, 0.70f);

    FVector4 GStatusDangerTint(0.82f, 0.24f, 0.18f, 1.f);
    FVector4 GStatusWarningTint(0.84f, 0.48f, 0.12f, 1.f);
    FVector4 GStatusCautionTint(0.78f, 0.68f, 0.18f, 1.f);
    FVector4 GStatusSuccessTint(0.20f, 0.56f, 0.20f, 1.f);
    FVector4 GTextMutedTint(0.31f, 0.27f, 0.21f, 1.f);

    FVector4 GEdictSlotActiveTint(1.15f, 0.90f, 0.20f, 1.f);
    FVector4 GEdictSlotCoolingDownTint(0.86f, 0.88f, 0.92f, 1.f);
    FVector4 GEdictSlotUnavailableTint(0.90f, 0.90f, 0.90f, 1.f);
    FVector4 GEdictSlotFocusedTint(1.03f, 0.98f, 0.84f, 1.f);
    FVector4 GOverlayDisabledTint(0.60f, 0.60f, 0.60f, 0.75f);

    FVector4 GCitizenInfoTitleTint(0.37f, 0.26f, 0.10f, 1.f);
    FVector4 GCitizenInfoSubtitleTint(0.35f, 0.30f, 0.22f, 1.f);
    FVector4 GCitizenInfoBodyTint(0.22f, 0.22f, 0.22f, 1.f);
    FVector4 GCitizenInfoSectionHeaderTint(0.24f, 0.24f, 0.24f, 1.f);
    FVector4 GCitizenInfoValueTint(0.28f, 0.24f, 0.20f, 1.f);
    FVector4 GCitizenInfoStrongValueTint(0.32f, 0.20f, 0.10f, 1.f);
    FVector4 GCitizenInfoAccentTint(0.26f, 0.64f, 0.82f, 1.f);
    FVector4 GCitizenInfoActionTextTint(0.29f, 0.22f, 0.12f, 1.f);
    FVector4 GCitizenInfoDividerTint(0.86f, 0.98f, 1.08f, 0.96f);
    FVector4 GCitizenInfoPanelHighlightTint(1.f, 0.98f, 0.92f, 0.94f);
    FVector4 GCitizenInfoProgressRailTint(0.90f, 0.89f, 0.82f, 0.92f);
    FVector4 GCitizenInfoProgressFillTint(0.22f, 0.53f, 0.90f, 0.95f);
    FVector4 GCitizenInfoSupportNegativeTint(0.92f, 0.62f, 0.48f, 0.95f);
    FVector4 GCitizenInfoSupportNeutralTint(0.82f, 0.82f, 0.82f, 0.95f);
    FVector4 GCitizenInfoSupportPositiveTint(0.42f, 0.72f, 0.36f, 0.98f);
    FVector4 GCitizenInfoFooterTint(0.58f, 0.84f, 0.88f, 0.92f);
    FVector4 GCitizenInfoTabLabelSelectedTint(0.27f, 0.17f, 0.06f, 1.f);
    FVector4 GCitizenInfoTabLabelNormalTint(0.22f, 0.20f, 0.17f, 1.f);
    FVector4 GCitizenInfoTabIconSelectedTint(1.f, 1.f, 1.f, 1.f);
    FVector4 GCitizenInfoTabIconNormalTint(0.92f, 0.92f, 0.92f, 0.95f);
    FVector4 GCitizenInfoBudgetLabelSelectedTint(0.36f, 0.22f, 0.08f, 1.f);
    FVector4 GCitizenInfoBudgetLabelNormalTint(0.30f, 0.22f, 0.12f, 1.f);
    FVector4 GCitizenInfoMetricProfileTint(0.33f, 0.30f, 0.26f, 1.f);
    FVector4 GCitizenInfoMetricHeaderTint(0.24f, 0.24f, 0.24f, 1.f);
    FVector4 GCitizenInfoMetricLabelTint(0.28f, 0.28f, 0.28f, 1.f);
    FVector4 GCitizenInfoMetricValueTint(0.27f, 0.27f, 0.27f, 1.f);
    FVector4 GCitizenInfoMetricAccentTint(0.33f, 0.62f, 0.88f, 1.f);
    FVector4 GCitizenInfoEmptyResidentTint(1.00f, 0.90f, 0.35f, 0.85f);
    FVector4 GCitizenInfoDisabledVisitorTint(0.72f, 0.72f, 0.72f, 0.65f);
}

namespace
{
    constexpr const wchar_t* GConfigId = L"UI.Theme";
    constexpr size_t GAlmanacSatisfactionTintCount = 9;

    std::array<FVector4, GAlmanacSatisfactionTintCount> GAlmanacSatisfactionTints =
    {{
        FVector4(0.96f, 0.80f, 0.12f, 0.98f),
        FVector4(0.94f, 0.66f, 0.16f, 0.98f),
        FVector4(0.48f, 0.74f, 0.40f, 0.98f),
        FVector4(0.86f, 0.56f, 0.18f, 0.98f),
        FVector4(0.72f, 0.56f, 0.78f, 0.98f),
        FVector4(0.74f, 0.64f, 0.34f, 0.98f),
        FVector4(0.64f, 0.46f, 0.22f, 0.98f),
        FVector4(0.62f, 0.72f, 0.92f, 0.98f),
        FVector4(0.86f, 0.72f, 0.24f, 0.98f)
    }};
    FVector4 GAlmanacSatisfactionFallbackTint(0.90f, 0.72f, 0.18f, 0.95f);

    struct FColorBinding
    {
        const char* Section;
        const char* Key;
        FVector4* Value;
        FVector4 DefaultValue;
    };

    const FColorBinding GColorBindings[] =
    {
        { "button.default", "normal_tint", &TropicoUiTheme::GButtonDefaultNormalTint, FVector4(0.20f, 0.22f, 0.26f, 0.92f) },
        { "button.default", "hover_tint", &TropicoUiTheme::GButtonDefaultHoverTint, FVector4(0.26f, 0.30f, 0.35f, 0.95f) },
        { "button.default", "click_tint", &TropicoUiTheme::GButtonDefaultClickTint, FVector4(0.14f, 0.16f, 0.20f, 0.98f) },
        { "button.default", "disable_tint", &TropicoUiTheme::GButtonDefaultDisableTint, FVector4(0.10f, 0.10f, 0.12f, 0.70f) },
        { "button.highlight", "normal_tint", &TropicoUiTheme::GButtonHighlightNormalTint, FVector4(0.10f, 0.32f, 0.52f, 0.95f) },
        { "button.highlight", "hover_tint", &TropicoUiTheme::GButtonHighlightHoverTint, FVector4(0.16f, 0.40f, 0.62f, 0.98f) },
        { "button.highlight", "click_tint", &TropicoUiTheme::GButtonHighlightClickTint, FVector4(0.08f, 0.24f, 0.40f, 0.98f) },
        { "button.highlight", "disable_tint", &TropicoUiTheme::GButtonHighlightDisableTint, FVector4(0.08f, 0.24f, 0.40f, 0.70f) },
        { "button.icon_slot", "normal_tint", &TropicoUiTheme::GButtonIconSlotNormalTint, FVector4(1.f, 1.f, 1.f, 0.96f) },
        { "button.icon_slot", "hover_tint", &TropicoUiTheme::GButtonIconSlotHoverTint, FVector4(1.f, 1.f, 1.f, 1.f) },
        { "button.icon_slot", "click_tint", &TropicoUiTheme::GButtonIconSlotClickTint, FVector4(0.80f, 0.80f, 0.80f, 1.f) },
        { "button.icon_slot", "disable_tint", &TropicoUiTheme::GButtonIconSlotDisableTint, FVector4(0.35f, 0.35f, 0.35f, 0.75f) },
        { "button.category_selected", "normal_tint", &TropicoUiTheme::GButtonCategorySelectedNormalTint, FVector4(1.f, 1.f, 1.f, 1.f) },
        { "button.category_selected", "hover_tint", &TropicoUiTheme::GButtonCategorySelectedHoverTint, FVector4(1.f, 1.f, 1.f, 1.f) },
        { "button.category_selected", "click_tint", &TropicoUiTheme::GButtonCategorySelectedClickTint, FVector4(0.90f, 0.90f, 0.90f, 1.f) },
        { "button.category_selected", "disable_tint", &TropicoUiTheme::GButtonCategorySelectedDisableTint, FVector4(0.50f, 0.50f, 0.50f, 0.75f) },
        { "button.category", "normal_tint", &TropicoUiTheme::GButtonCategoryNormalTint, FVector4(0.74f, 0.84f, 0.98f, 0.96f) },
        { "button.category", "hover_tint", &TropicoUiTheme::GButtonCategoryHoverTint, FVector4(0.88f, 0.95f, 1.f, 1.f) },
        { "button.category", "click_tint", &TropicoUiTheme::GButtonCategoryClickTint, FVector4(0.62f, 0.74f, 0.92f, 1.f) },
        { "button.category", "disable_tint", &TropicoUiTheme::GButtonCategoryDisableTint, FVector4(0.50f, 0.50f, 0.50f, 0.70f) },
        { "status", "danger_tint", &TropicoUiTheme::GStatusDangerTint, FVector4(0.82f, 0.24f, 0.18f, 1.f) },
        { "status", "warning_tint", &TropicoUiTheme::GStatusWarningTint, FVector4(0.84f, 0.48f, 0.12f, 1.f) },
        { "status", "caution_tint", &TropicoUiTheme::GStatusCautionTint, FVector4(0.78f, 0.68f, 0.18f, 1.f) },
        { "status", "success_tint", &TropicoUiTheme::GStatusSuccessTint, FVector4(0.20f, 0.56f, 0.20f, 1.f) },
        { "status", "text_muted_tint", &TropicoUiTheme::GTextMutedTint, FVector4(0.31f, 0.27f, 0.21f, 1.f) },
        { "edict", "active_tint", &TropicoUiTheme::GEdictSlotActiveTint, FVector4(1.15f, 0.90f, 0.20f, 1.f) },
        { "edict", "cooling_down_tint", &TropicoUiTheme::GEdictSlotCoolingDownTint, FVector4(0.86f, 0.88f, 0.92f, 1.f) },
        { "edict", "unavailable_tint", &TropicoUiTheme::GEdictSlotUnavailableTint, FVector4(0.90f, 0.90f, 0.90f, 1.f) },
        { "edict", "focused_tint", &TropicoUiTheme::GEdictSlotFocusedTint, FVector4(1.03f, 0.98f, 0.84f, 1.f) },
        { "overlay", "disabled_tint", &TropicoUiTheme::GOverlayDisabledTint, FVector4(0.60f, 0.60f, 0.60f, 0.75f) },
        { "citizen_info", "title_tint", &TropicoUiTheme::GCitizenInfoTitleTint, FVector4(0.37f, 0.26f, 0.10f, 1.f) },
        { "citizen_info", "subtitle_tint", &TropicoUiTheme::GCitizenInfoSubtitleTint, FVector4(0.35f, 0.30f, 0.22f, 1.f) },
        { "citizen_info", "body_tint", &TropicoUiTheme::GCitizenInfoBodyTint, FVector4(0.22f, 0.22f, 0.22f, 1.f) },
        { "citizen_info", "section_header_tint", &TropicoUiTheme::GCitizenInfoSectionHeaderTint, FVector4(0.24f, 0.24f, 0.24f, 1.f) },
        { "citizen_info", "value_tint", &TropicoUiTheme::GCitizenInfoValueTint, FVector4(0.28f, 0.24f, 0.20f, 1.f) },
        { "citizen_info", "strong_value_tint", &TropicoUiTheme::GCitizenInfoStrongValueTint, FVector4(0.32f, 0.20f, 0.10f, 1.f) },
        { "citizen_info", "accent_tint", &TropicoUiTheme::GCitizenInfoAccentTint, FVector4(0.26f, 0.64f, 0.82f, 1.f) },
        { "citizen_info", "action_text_tint", &TropicoUiTheme::GCitizenInfoActionTextTint, FVector4(0.29f, 0.22f, 0.12f, 1.f) },
        { "citizen_info", "divider_tint", &TropicoUiTheme::GCitizenInfoDividerTint, FVector4(0.86f, 0.98f, 1.08f, 0.96f) },
        { "citizen_info", "panel_highlight_tint", &TropicoUiTheme::GCitizenInfoPanelHighlightTint, FVector4(1.f, 0.98f, 0.92f, 0.94f) },
        { "citizen_info", "progress_rail_tint", &TropicoUiTheme::GCitizenInfoProgressRailTint, FVector4(0.90f, 0.89f, 0.82f, 0.92f) },
        { "citizen_info", "progress_fill_tint", &TropicoUiTheme::GCitizenInfoProgressFillTint, FVector4(0.22f, 0.53f, 0.90f, 0.95f) },
        { "citizen_info", "support_negative_tint", &TropicoUiTheme::GCitizenInfoSupportNegativeTint, FVector4(0.92f, 0.62f, 0.48f, 0.95f) },
        { "citizen_info", "support_neutral_tint", &TropicoUiTheme::GCitizenInfoSupportNeutralTint, FVector4(0.82f, 0.82f, 0.82f, 0.95f) },
        { "citizen_info", "support_positive_tint", &TropicoUiTheme::GCitizenInfoSupportPositiveTint, FVector4(0.42f, 0.72f, 0.36f, 0.98f) },
        { "citizen_info", "footer_tint", &TropicoUiTheme::GCitizenInfoFooterTint, FVector4(0.58f, 0.84f, 0.88f, 0.92f) },
        { "citizen_info", "tab_label_selected_tint", &TropicoUiTheme::GCitizenInfoTabLabelSelectedTint, FVector4(0.27f, 0.17f, 0.06f, 1.f) },
        { "citizen_info", "tab_label_normal_tint", &TropicoUiTheme::GCitizenInfoTabLabelNormalTint, FVector4(0.22f, 0.20f, 0.17f, 1.f) },
        { "citizen_info", "tab_icon_selected_tint", &TropicoUiTheme::GCitizenInfoTabIconSelectedTint, FVector4(1.f, 1.f, 1.f, 1.f) },
        { "citizen_info", "tab_icon_normal_tint", &TropicoUiTheme::GCitizenInfoTabIconNormalTint, FVector4(0.92f, 0.92f, 0.92f, 0.95f) },
        { "citizen_info", "budget_label_selected_tint", &TropicoUiTheme::GCitizenInfoBudgetLabelSelectedTint, FVector4(0.36f, 0.22f, 0.08f, 1.f) },
        { "citizen_info", "budget_label_normal_tint", &TropicoUiTheme::GCitizenInfoBudgetLabelNormalTint, FVector4(0.30f, 0.22f, 0.12f, 1.f) },
        { "citizen_info", "metric_profile_tint", &TropicoUiTheme::GCitizenInfoMetricProfileTint, FVector4(0.33f, 0.30f, 0.26f, 1.f) },
        { "citizen_info", "metric_header_tint", &TropicoUiTheme::GCitizenInfoMetricHeaderTint, FVector4(0.24f, 0.24f, 0.24f, 1.f) },
        { "citizen_info", "metric_label_tint", &TropicoUiTheme::GCitizenInfoMetricLabelTint, FVector4(0.28f, 0.28f, 0.28f, 1.f) },
        { "citizen_info", "metric_value_tint", &TropicoUiTheme::GCitizenInfoMetricValueTint, FVector4(0.27f, 0.27f, 0.27f, 1.f) },
        { "citizen_info", "metric_accent_tint", &TropicoUiTheme::GCitizenInfoMetricAccentTint, FVector4(0.33f, 0.62f, 0.88f, 1.f) },
        { "citizen_info", "empty_resident_tint", &TropicoUiTheme::GCitizenInfoEmptyResidentTint, FVector4(1.00f, 0.90f, 0.35f, 0.85f) },
        { "citizen_info", "disabled_visitor_tint", &TropicoUiTheme::GCitizenInfoDisabledVisitorTint, FVector4(0.72f, 0.72f, 0.72f, 0.65f) }
    };

    void TrimString(std::string& Value)
    {
        Value.erase(
            Value.begin(),
            std::find_if(
                Value.begin(),
                Value.end(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }));
        Value.erase(
            std::find_if(
                Value.rbegin(),
                Value.rend(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }).base(),
            Value.end());
    }

    std::string ToLowerCopy(std::string Value)
    {
        std::transform(
            Value.begin(),
            Value.end(),
            Value.begin(),
            [](unsigned char Character)
            {
                return static_cast<char>(std::tolower(Character));
            });
        return Value;
    }

    bool TryParseSectionHeader(
        const std::string& Line,
        std::string& OutSection)
    {
        if (Line.size() < 3 || Line.front() != '[' || Line.back() != ']')
            return false;

        OutSection = Line.substr(1, Line.size() - 2);
        TrimString(OutSection);
        OutSection = ToLowerCopy(OutSection);
        return !OutSection.empty();
    }

    bool TrySplitSectionAndKey(
        const std::string& CurrentSection,
        const std::string& RawKey,
        std::string& OutSection,
        std::string& OutKey)
    {
        OutSection = CurrentSection;
        OutKey = RawKey;

        const size_t DotPos = RawKey.find('.');

        if (DotPos != std::string::npos)
        {
            OutSection = RawKey.substr(0, DotPos);
            OutKey = RawKey.substr(DotPos + 1);
        }

        TrimString(OutSection);
        TrimString(OutKey);
        OutSection = ToLowerCopy(OutSection);
        OutKey = ToLowerCopy(OutKey);
        return !OutSection.empty() && !OutKey.empty();
    }

    bool TryParseFloat(const std::string& RawValue, float& OutValue)
    {
        try
        {
            size_t ParsedLength = 0;
            const float ParsedValue = std::stof(RawValue, &ParsedLength);

            if (ParsedLength != RawValue.size())
                return false;

            OutValue = ParsedValue;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool TryParseTint(const std::string& RawValue, FVector4& OutTint)
    {
        std::array<float, 4> Components = {};
        size_t Start = 0;

        for (size_t Index = 0; Index < Components.size(); ++Index)
        {
            const size_t Delimiter =
                Index + 1 < Components.size() ?
                    RawValue.find(',', Start) :
                    std::string::npos;
            std::string Part =
                Delimiter == std::string::npos ?
                    RawValue.substr(Start) :
                    RawValue.substr(Start, Delimiter - Start);
            TrimString(Part);

            if (!TryParseFloat(Part, Components[Index]))
                return false;

            if (Delimiter == std::string::npos)
            {
                if (Index + 1 != Components.size())
                    return false;
            }
            else
            {
                Start = Delimiter + 1;
            }
        }

        OutTint = FVector4(
            Components[0],
            Components[1],
            Components[2],
            Components[3]);
        return true;
    }

    void ResetToDefaults()
    {
        for (const FColorBinding& Binding : GColorBindings)
            *Binding.Value = Binding.DefaultValue;

        GAlmanacSatisfactionTints =
        {{
            FVector4(0.96f, 0.80f, 0.12f, 0.98f),
            FVector4(0.94f, 0.66f, 0.16f, 0.98f),
            FVector4(0.48f, 0.74f, 0.40f, 0.98f),
            FVector4(0.86f, 0.56f, 0.18f, 0.98f),
            FVector4(0.72f, 0.56f, 0.78f, 0.98f),
            FVector4(0.74f, 0.64f, 0.34f, 0.98f),
            FVector4(0.64f, 0.46f, 0.22f, 0.98f),
            FVector4(0.62f, 0.72f, 0.92f, 0.98f),
            FVector4(0.86f, 0.72f, 0.24f, 0.98f)
        }};
        GAlmanacSatisfactionFallbackTint = FVector4(0.90f, 0.72f, 0.18f, 0.95f);
    }

    bool ApplyTintValue(
        const std::string& Section,
        const std::string& Key,
        const FVector4& Value)
    {
        for (const FColorBinding& Binding : GColorBindings)
        {
            if (Section == Binding.Section && Key == Binding.Key)
            {
                *Binding.Value = Value;
                return true;
            }
        }

        if (Section == "almanac.satisfaction")
        {
            if (Key == "fallback_tint")
            {
                GAlmanacSatisfactionFallbackTint = Value;
                return true;
            }

            for (size_t Index = 0; Index < GAlmanacSatisfactionTints.size(); ++Index)
            {
                if (Key == ("tier" + std::to_string(Index) + "_tint"))
                {
                    GAlmanacSatisfactionTints[Index] = Value;
                    return true;
                }
            }
        }

        return false;
    }

    bool LoadFile(const std::wstring& Path)
    {
        std::ifstream File(Path);

        if (!File.is_open())
            return false;

        std::string CurrentSection;
        std::string Line;

        while (std::getline(File, Line))
        {
            if (!Line.empty() && Line.back() == '\r')
                Line.pop_back();

            std::string TrimmedLine = Line;
            TrimString(TrimmedLine);

            if (TrimmedLine.empty() ||
                TrimmedLine[0] == '#' ||
                TrimmedLine[0] == ';')
            {
                continue;
            }

            std::string ParsedSection;

            if (TryParseSectionHeader(TrimmedLine, ParsedSection))
            {
                CurrentSection = ParsedSection;
                continue;
            }

            const size_t EqualsPos = TrimmedLine.find('=');

            if (EqualsPos == std::string::npos)
                continue;

            std::string RawKey = TrimmedLine.substr(0, EqualsPos);
            std::string RawValue = TrimmedLine.substr(EqualsPos + 1);
            TrimString(RawKey);
            TrimString(RawValue);

            if (RawKey.empty() || RawValue.empty())
                continue;

            std::string Section;
            std::string Key;

            if (!TrySplitSectionAndKey(CurrentSection, RawKey, Section, Key))
                continue;

            FVector4 ParsedTint;

            if (!TryParseTint(RawValue, ParsedTint))
                continue;

            ApplyTintValue(Section, Key, ParsedTint);
        }

        return true;
    }
}

namespace TropicoUiTheme
{
    void RegisterRuntimeConfig()
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                RuntimeConfigRegistry::BuildExeRelativePath(L"UITheme.ini"),
                0.5f,
                &ResetToDefaults,
                &LoadFile,
                nullptr
            });
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::PollSource(GConfigId, DeltaTime);
    }

    unsigned long long GetRuntimeConfigGeneration()
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::GetSourceGeneration(GConfigId);
    }

    const FVector4& GetAlmanacSatisfactionTint(int Index)
    {
        if (Index < 0 ||
            Index >= static_cast<int>(GAlmanacSatisfactionTints.size()))
        {
            return GAlmanacSatisfactionFallbackTint;
        }

        return GAlmanacSatisfactionTints[static_cast<size_t>(Index)];
    }
}
