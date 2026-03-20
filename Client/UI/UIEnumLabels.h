#pragma once

#include "UIStrings.h"
#include "../Building/BuildingTypes.h"
#include "../Map/PlacementAreaRuntimeState.h"
#include "../Politics/PoliticalTypes.h"

namespace UIEnumLabels
{
    inline const std::wstring& GetPoliticalDemandStageLabel(
        EPoliticalDemandStage Stage)
    {
        switch (Stage)
        {
        case EPoliticalDemandStage::Warning:
            return UIStrings::Get(L"escalation.stage.warning");
        case EPoliticalDemandStage::Ultimatum:
            return UIStrings::Get(L"escalation.stage.ultimatum");
        case EPoliticalDemandStage::Revolt:
            return UIStrings::Get(L"escalation.stage.revolt");
        case EPoliticalDemandStage::Demand:
        default:
            return UIStrings::Get(L"escalation.stage.demand");
        }
    }

    inline const std::wstring& GetBuildingDamageLevelDisplayName(
        EBuildingDamageLevel Level)
    {
        switch (Level)
        {
        case EBuildingDamageLevel::Damaged:
            return UIStrings::Get(L"citizen_info.damage.damaged");
        case EBuildingDamageLevel::Critical:
            return UIStrings::Get(L"citizen_info.damage.critical");
        case EBuildingDamageLevel::None:
        default:
            return UIStrings::Get(L"citizen_info.damage.none");
        }
    }

    inline const std::wstring& GetHousingClassDisplayName(
        EBuildingHousingClass HousingClass)
    {
        switch (HousingClass)
        {
        case EBuildingHousingClass::Collective:
            return UIStrings::Get(
                L"citizen_info.building.housing_class.collective");
        case EBuildingHousingClass::Standard:
            return UIStrings::Get(
                L"citizen_info.building.housing_class.standard");
        case EBuildingHousingClass::Elite:
            return UIStrings::Get(
                L"citizen_info.building.housing_class.elite");
        default:
            return UIStrings::Get(
                L"citizen_info.building.housing_class.default");
        }
    }

    inline const std::wstring& GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass)
    {
        switch (LeisureClass)
        {
        case EBuildingLeisureClass::Cultural:
            return UIStrings::Get(
                L"citizen_info.building.leisure_class.cultural");
        case EBuildingLeisureClass::Luxury:
            return UIStrings::Get(
                L"citizen_info.building.leisure_class.luxury");
        case EBuildingLeisureClass::General:
            return UIStrings::Get(
                L"citizen_info.building.leisure_class.general");
        default:
            return UIStrings::Get(
                L"citizen_info.building.leisure_class.unknown");
        }
    }
}
