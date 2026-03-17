#pragma once

#include "../UI/UIStrings.h"
#include "Vector4.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <vector>

// 건물 카탈로그 카테고리 번호.
// BuildingCategoryInfo.h 의 카테고리 메타데이터와 순서가 일치해야 한다.
enum class EBuildingCategory
{
    Infrastructure  = 0,  // 교통 및 기반시설
    FoodResource    = 1,  // 음식 및 자원
    Industry        = 2,  // 산업
    Housing         = 3,  // 주거지
    Entertainment   = 4,  // 오락
    MediaEducation  = 5,  // 미디어 및 교육
    Tourism         = 6,  // 관광업
    PublicService   = 7,  // 공익 서비스
    LuxuryEntertainment = 8,   // 호화 오락
    Military       = 9,   // 습격 및 군사
    GovernmentFinance = 10, // 정부 및 재정
    Count           = 11
};

enum class EBuildingEra
{
    Colonial = 0,
    WorldWars,
    ColdWar,
    Modern
};

inline int GetBuildingEraRank(EBuildingEra Era)
{
    return static_cast<int>(Era);
}

inline EBuildingEra GetNextBuildingEra(EBuildingEra Era)
{
    switch (Era)
    {
    case EBuildingEra::Colonial:
        return EBuildingEra::WorldWars;
    case EBuildingEra::WorldWars:
        return EBuildingEra::ColdWar;
    case EBuildingEra::ColdWar:
        return EBuildingEra::Modern;
    case EBuildingEra::Modern:
    default:
        return EBuildingEra::Modern;
    }
}

inline bool HasNextBuildingEra(EBuildingEra Era)
{
    return Era != EBuildingEra::Modern;
}

inline bool IsBuildingEraUnlocked(
    EBuildingEra CurrentEra,
    EBuildingEra RequiredEra)
{
    return GetBuildingEraRank(CurrentEra) >=
        GetBuildingEraRank(RequiredEra);
}

inline const wchar_t* GetBuildingEraDisplayName(EBuildingEra Era)
{
    switch (Era)
    {
    case EBuildingEra::Colonial:
        return UIStrings::Get(L"building.era.colonial").c_str();
    case EBuildingEra::WorldWars:
        return UIStrings::Get(L"building.era.world_wars").c_str();
    case EBuildingEra::ColdWar:
        return UIStrings::Get(L"building.era.cold_war").c_str();
    case EBuildingEra::Modern:
        return UIStrings::Get(L"building.era.modern").c_str();
    default:
        return UIStrings::Get(L"building.era.unknown").c_str();
    }
}

struct FEraUnlockRequirement
{
    int MinPopulation = 0;
    int MinTotalBuildings = 0;
    int MinFoodProviders = 0;
    int MinIndustryBuildings = 0;
    int MinPublicServiceBuildings = 0;
    int MinEntertainmentBuildings = 0;
    int MinPowerMW = 0;
};

struct FEraProgressState
{
    EBuildingEra CurrentEra = EBuildingEra::Colonial;
    bool HasNextEra = true;
    EBuildingEra NextEra = EBuildingEra::WorldWars;
    FEraUnlockRequirement NextRequirement;
    bool NextEraReady = false;
    int Population = 0;
    int TotalBuildings = 0;
    int FoodProviders = 0;
    int IndustryBuildings = 0;
    int PublicServiceBuildings = 0;
    int EntertainmentBuildings = 0;
    int PowerMW = 0;
};

enum class EEraTransitionStage
{
    None = 0,
    Available,
    PendingChoice,
    InCeremony,
    Cooldown
};

enum class EEraTransitionChoice
{
    None = 0,
    Confirm
};

struct FEraTransitionState
{
    EEraTransitionStage Stage = EEraTransitionStage::None;
    EEraTransitionChoice Choice = EEraTransitionChoice::None;
    EBuildingEra CurrentEra = EBuildingEra::Colonial;
    EBuildingEra TargetEra = EBuildingEra::WorldWars;
    bool CanStart = false;
    int AvailableSinceYear = 0;
    int AvailableSinceMonth = 0;
    int AvailableSinceDay = 0;
    int NotificationDays = 0;
    std::wstring Title;
    std::wstring Summary;
    std::wstring ConfirmText;
};

enum class EBuildingHousingClass
{
    None = 0,
    Collective,
    Standard,
    Elite
};

enum class EBuildingLeisureClass
{
    None = 0,
    General,
    Cultural,
    Luxury
};

enum class ETouristPreference
{
    None = 0,
    Cultural,
    Family,
    Backpacker,
    Relaxation,
    ThrillSeeker,
    Celebrity
};

constexpr int GTouristPreferenceCount =
    static_cast<int>(ETouristPreference::Celebrity) + 1;

inline bool HasTouristPreference(ETouristPreference Preference)
{
    return Preference != ETouristPreference::None;
}

inline const wchar_t* GetTouristPreferenceDisplayName(
    ETouristPreference Preference)
{
    switch (Preference)
    {
    case ETouristPreference::Cultural:
        return UIStrings::Get(L"building.tourist_preference.cultural").c_str();
    case ETouristPreference::Family:
        return UIStrings::Get(L"building.tourist_preference.family").c_str();
    case ETouristPreference::Backpacker:
        return UIStrings::Get(
            L"building.tourist_preference.backpacker").c_str();
    case ETouristPreference::Relaxation:
        return UIStrings::Get(
            L"building.tourist_preference.relaxation").c_str();
    case ETouristPreference::ThrillSeeker:
        return UIStrings::Get(
            L"building.tourist_preference.thrill_seeker").c_str();
    case ETouristPreference::Celebrity:
        return UIStrings::Get(
            L"building.tourist_preference.celebrity").c_str();
    default:
        return UIStrings::Get(L"building.tourist_preference.unknown").c_str();
    }
}

constexpr unsigned int GBuildingWealthMaskNone = 0u;
constexpr unsigned int GBuildingWealthMaskBroke = 1u << 0;
constexpr unsigned int GBuildingWealthMaskPoor = 1u << 1;
constexpr unsigned int GBuildingWealthMaskWellOff = 1u << 2;
constexpr unsigned int GBuildingWealthMaskRich = 1u << 3;
constexpr unsigned int GBuildingWealthMaskFilthyRich = 1u << 4;
constexpr unsigned int GBuildingWealthMaskAll =
    GBuildingWealthMaskBroke |
    GBuildingWealthMaskPoor |
    GBuildingWealthMaskWellOff |
    GBuildingWealthMaskRich |
    GBuildingWealthMaskFilthyRich;

enum class EResourceType
{
    None = 0,
    Coconuts,
    Logs,
    Fish,
    Crops,
    AnimalProducts,
    Ore,
    Oil,
    FarmedFish,
    // Project-local family placeholder for Hydroponic Plantation only.
    // This is not a concrete Tropico 6 trade good, and it must not be used
    // as a proxy for Vertical Farm outputs.
    HydroponicProduce,
    FactoryLivestock,
    FeedCrops,
    Banana,
    Cocoa,
    Coffee,
    Corn,
    Cotton,
    Pineapple,
    Rubber,
    Sugar,
    Tobacco,
    Meat,
    Milk,
    Hides,
    Wool,
    Coal,
    Iron,
    Gold,
    Nickel,
    Aluminum,
    Uranium,
    Planks,
    Rum,
    Leather,
    CannedGoods,
    Cheese,
    Cigars,
    Boats,
    Steel,
    Textiles,
    Weapons,
    Chocolate,
    Furniture,
    Jewelry,
    Plastic,
    Cars,
    Electronics,
    Apparel,
    Pharmaceuticals,
    Juice,
    SpecialChocolate,
    Goldnuts,
    BS,
    Count
};

enum class EResourceMarketClass
{
    None = 0,
    Food,
    RawGoods,
    ManufacturedGoods,
    LuxuryGoods
};

enum class EBuildingServiceType
{
    Food = 0,
    Fun,
    Health,
    Faith,
    Count
};

constexpr int GProductionInputSlotCount = 4;
constexpr int GBuildingServiceTypeCount =
    static_cast<int>(EBuildingServiceType::Count);

inline const wchar_t* GetResourceTypeDisplayName(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Coconuts:
        return UIStrings::Get(L"building.resource_type.coconuts").c_str();
    case EResourceType::Logs:
        return UIStrings::Get(L"building.resource_type.logs").c_str();
    case EResourceType::Fish:
        return UIStrings::Get(L"building.resource_type.fish").c_str();
    case EResourceType::Crops:
        return UIStrings::Get(L"building.resource_type.crops").c_str();
    case EResourceType::AnimalProducts:
        return UIStrings::Get(
            L"building.resource_type.animal_products").c_str();
    case EResourceType::Ore:
        return UIStrings::Get(L"building.resource_type.ore").c_str();
    case EResourceType::Oil:
        return UIStrings::Get(L"building.resource_type.oil").c_str();
    case EResourceType::FarmedFish:
        return UIStrings::Get(L"building.resource_type.farmed_fish").c_str();
    case EResourceType::HydroponicProduce:
        return UIStrings::Get(
            L"building.resource_type.hydroponic_produce").c_str();
    case EResourceType::FactoryLivestock:
        return UIStrings::Get(
            L"building.resource_type.factory_livestock").c_str();
    case EResourceType::FeedCrops:
        return UIStrings::Get(L"building.resource_type.feed_crops").c_str();
    case EResourceType::Banana:
        return UIStrings::Get(L"building.resource_type.banana").c_str();
    case EResourceType::Cocoa:
        return UIStrings::Get(L"building.resource_type.cocoa").c_str();
    case EResourceType::Coffee:
        return UIStrings::Get(L"building.resource_type.coffee").c_str();
    case EResourceType::Corn:
        return UIStrings::Get(L"building.resource_type.corn").c_str();
    case EResourceType::Cotton:
        return UIStrings::Get(L"building.resource_type.cotton").c_str();
    case EResourceType::Pineapple:
        return UIStrings::Get(L"building.resource_type.pineapple").c_str();
    case EResourceType::Rubber:
        return UIStrings::Get(L"building.resource_type.rubber").c_str();
    case EResourceType::Sugar:
        return UIStrings::Get(L"building.resource_type.sugar").c_str();
    case EResourceType::Tobacco:
        return UIStrings::Get(L"building.resource_type.tobacco").c_str();
    case EResourceType::Meat:
        return UIStrings::Get(L"building.resource_type.meat").c_str();
    case EResourceType::Milk:
        return UIStrings::Get(L"building.resource_type.milk").c_str();
    case EResourceType::Hides:
        return UIStrings::Get(L"building.resource_type.hides").c_str();
    case EResourceType::Wool:
        return UIStrings::Get(L"building.resource_type.wool").c_str();
    case EResourceType::Coal:
        return UIStrings::Get(L"building.resource_type.coal").c_str();
    case EResourceType::Iron:
        return UIStrings::Get(L"building.resource_type.iron").c_str();
    case EResourceType::Gold:
        return UIStrings::Get(L"building.resource_type.gold").c_str();
    case EResourceType::Nickel:
        return UIStrings::Get(L"building.resource_type.nickel").c_str();
    case EResourceType::Aluminum:
        return UIStrings::Get(L"building.resource_type.aluminum").c_str();
    case EResourceType::Uranium:
        return UIStrings::Get(L"building.resource_type.uranium").c_str();
    case EResourceType::Planks:
        return UIStrings::Get(L"building.resource_type.planks").c_str();
    case EResourceType::Rum:
        return UIStrings::Get(L"building.resource_type.rum").c_str();
    case EResourceType::Leather:
        return UIStrings::Get(L"building.resource_type.leather").c_str();
    case EResourceType::CannedGoods:
        return UIStrings::Get(
            L"building.resource_type.canned_goods").c_str();
    case EResourceType::Cheese:
        return UIStrings::Get(L"building.resource_type.cheese").c_str();
    case EResourceType::Cigars:
        return UIStrings::Get(L"building.resource_type.cigars").c_str();
    case EResourceType::Boats:
        return UIStrings::Get(L"building.resource_type.boats").c_str();
    case EResourceType::Steel:
        return UIStrings::Get(L"building.resource_type.steel").c_str();
    case EResourceType::Textiles:
        return UIStrings::Get(L"building.resource_type.textiles").c_str();
    case EResourceType::Weapons:
        return UIStrings::Get(L"building.resource_type.weapons").c_str();
    case EResourceType::Chocolate:
        return UIStrings::Get(L"building.resource_type.chocolate").c_str();
    case EResourceType::Furniture:
        return UIStrings::Get(L"building.resource_type.furniture").c_str();
    case EResourceType::Jewelry:
        return UIStrings::Get(L"building.resource_type.jewelry").c_str();
    case EResourceType::Plastic:
        return UIStrings::Get(L"building.resource_type.plastic").c_str();
    case EResourceType::Cars:
        return UIStrings::Get(L"building.resource_type.cars").c_str();
    case EResourceType::Electronics:
        return UIStrings::Get(L"building.resource_type.electronics").c_str();
    case EResourceType::Apparel:
        return UIStrings::Get(L"building.resource_type.apparel").c_str();
    case EResourceType::Pharmaceuticals:
        return UIStrings::Get(
            L"building.resource_type.pharmaceuticals").c_str();
    case EResourceType::Juice:
        return UIStrings::Get(L"building.resource_type.juice").c_str();
    case EResourceType::SpecialChocolate:
        return UIStrings::Get(
            L"building.resource_type.special_chocolate").c_str();
    case EResourceType::Goldnuts:
        return UIStrings::Get(L"building.resource_type.goldnuts").c_str();
    case EResourceType::BS:
        return UIStrings::Get(L"building.resource_type.bs").c_str();
    default:
        return UIStrings::Get(L"building.resource_type.none").c_str();
    }
}

inline EResourceMarketClass GetResourceMarketClass(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Coconuts:
    case EResourceType::Fish:
    case EResourceType::Crops:
    case EResourceType::AnimalProducts:
    case EResourceType::FarmedFish:
    case EResourceType::HydroponicProduce:
    case EResourceType::FactoryLivestock:
    case EResourceType::FeedCrops:
    case EResourceType::Banana:
    case EResourceType::Corn:
    case EResourceType::Pineapple:
    case EResourceType::Meat:
    case EResourceType::Milk:
    case EResourceType::CannedGoods:
    case EResourceType::Cheese:
    case EResourceType::Juice:
    case EResourceType::BS:
        return EResourceMarketClass::Food;
    case EResourceType::Logs:
    case EResourceType::Ore:
    case EResourceType::Oil:
    case EResourceType::Cocoa:
    case EResourceType::Coffee:
    case EResourceType::Cotton:
    case EResourceType::Rubber:
    case EResourceType::Sugar:
    case EResourceType::Tobacco:
    case EResourceType::Hides:
    case EResourceType::Wool:
    case EResourceType::Coal:
    case EResourceType::Iron:
    case EResourceType::Gold:
    case EResourceType::Nickel:
    case EResourceType::Aluminum:
    case EResourceType::Uranium:
        return EResourceMarketClass::RawGoods;
    case EResourceType::Planks:
    case EResourceType::Rum:
    case EResourceType::Leather:
    case EResourceType::Steel:
    case EResourceType::Textiles:
    case EResourceType::Plastic:
        return EResourceMarketClass::ManufacturedGoods;
    case EResourceType::Cigars:
    case EResourceType::Boats:
    case EResourceType::Weapons:
    case EResourceType::Chocolate:
    case EResourceType::SpecialChocolate:
    case EResourceType::Furniture:
    case EResourceType::Jewelry:
    case EResourceType::Goldnuts:
    case EResourceType::Cars:
    case EResourceType::Electronics:
    case EResourceType::Apparel:
    case EResourceType::Pharmaceuticals:
        return EResourceMarketClass::LuxuryGoods;
    default:
        return EResourceMarketClass::None;
    }
}

inline bool IsCropOutputResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Banana:
    case EResourceType::Cocoa:
    case EResourceType::Coffee:
    case EResourceType::Corn:
    case EResourceType::Cotton:
    case EResourceType::Pineapple:
    case EResourceType::Rubber:
    case EResourceType::Sugar:
    case EResourceType::Tobacco:
        return true;
    default:
        return false;
    }
}

inline bool IsAnimalOutputResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Meat:
    case EResourceType::Milk:
    case EResourceType::Hides:
    case EResourceType::Wool:
        return true;
    default:
        return false;
    }
}

inline bool IsMineOutputResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Coal:
    case EResourceType::Iron:
    case EResourceType::Gold:
    case EResourceType::Nickel:
    case EResourceType::Aluminum:
    case EResourceType::Uranium:
        return true;
    default:
        return false;
    }
}

inline bool IsHydroponicPlantationPlaceholderResourceType(EResourceType Type)
{
    return Type == EResourceType::HydroponicProduce;
}

template <typename TVisitor>
inline void ForEachCropOutputResourceType(
    const TVisitor& Visitor)
{
    Visitor(EResourceType::Banana);
    Visitor(EResourceType::Cocoa);
    Visitor(EResourceType::Coffee);
    Visitor(EResourceType::Corn);
    Visitor(EResourceType::Cotton);
    Visitor(EResourceType::Pineapple);
    Visitor(EResourceType::Rubber);
    Visitor(EResourceType::Sugar);
    Visitor(EResourceType::Tobacco);
}

template <typename TVisitor>
inline void ForEachAnimalOutputResourceType(
    const TVisitor& Visitor)
{
    Visitor(EResourceType::Meat);
    Visitor(EResourceType::Milk);
    Visitor(EResourceType::Hides);
    Visitor(EResourceType::Wool);
}

template <typename TVisitor>
inline void ForEachMineOutputResourceType(
    const TVisitor& Visitor)
{
    Visitor(EResourceType::Coal);
    Visitor(EResourceType::Iron);
    Visitor(EResourceType::Gold);
    Visitor(EResourceType::Nickel);
    Visitor(EResourceType::Aluminum);
    Visitor(EResourceType::Uranium);
}

// IMPORTANT:
// Compatibility rules are intentionally split by purpose.
// Do not merge industrial input, food visit, feed, policy, and
// supply-chain family logic back into a single generic helper.

// Industrial input families are only for recipe/chain interpretation.
// Runtime stock, transport, import, and export should stay exact-type based.
template <typename TVisitor>
inline void ForEachIndustrialInputCompatibleResourceType(
    EResourceType DemandType,
    const TVisitor& Visitor)
{
    if (DemandType == EResourceType::None ||
        DemandType == EResourceType::Count)
    {
        return;
    }

    switch (DemandType)
    {
    case EResourceType::Crops:
    case EResourceType::HydroponicProduce:
        // HydroponicProduce is a Hydroponic Plantation family alias only.
        // Vertical Farm should enter the economy through exact crops / BS,
        // never through this placeholder type.
        ForEachCropOutputResourceType(Visitor);
        return;
    case EResourceType::AnimalProducts:
    case EResourceType::FactoryLivestock:
        ForEachAnimalOutputResourceType(Visitor);
        return;
    case EResourceType::Ore:
        ForEachMineOutputResourceType(Visitor);
        return;
    default:
        Visitor(DemandType);
        return;
    }
}

inline bool IsFeedInputDemandResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::FeedCrops:
    case EResourceType::Corn:
    case EResourceType::Sugar:
    case EResourceType::Fish:
    case EResourceType::FarmedFish:
        return true;
    default:
        return false;
    }
}

// Feed compatibility is a separate domain from generic crop compatibility.
// FeedCrops is a local recipe family that expands only to its accepted exact
// feed resources.
template <typename TVisitor>
inline void ForEachFeedCompatibleResourceType(
    EResourceType DemandType,
    const TVisitor& Visitor)
{
    if (DemandType == EResourceType::None ||
        DemandType == EResourceType::Count)
    {
        return;
    }

    switch (DemandType)
    {
    case EResourceType::FeedCrops:
        Visitor(EResourceType::Corn);
        Visitor(EResourceType::Sugar);
        return;
    case EResourceType::Fish:
    case EResourceType::FarmedFish:
    case EResourceType::Corn:
    case EResourceType::Sugar:
        Visitor(DemandType);
        return;
    default:
        Visitor(DemandType);
        return;
    }
}

inline const wchar_t* GetResourceMarketClassDisplayName(
    EResourceMarketClass MarketClass)
{
    switch (MarketClass)
    {
    case EResourceMarketClass::Food:
        return L"식품";
    case EResourceMarketClass::RawGoods:
        return L"원자재";
    case EResourceMarketClass::ManufacturedGoods:
        return L"가공재";
    case EResourceMarketClass::LuxuryGoods:
        return L"사치재";
    default:
        return L"전체";
    }
}

inline bool IsFoodResourceType(EResourceType Type)
{
    return GetResourceMarketClass(Type) == EResourceMarketClass::Food;
}

inline bool IsSummaryResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Crops:
    case EResourceType::AnimalProducts:
    case EResourceType::Ore:
    case EResourceType::HydroponicProduce:
    case EResourceType::FactoryLivestock:
    case EResourceType::FeedCrops:
        return true;
    default:
        return false;
    }
}

inline bool IsConcreteEconomicResourceType(EResourceType Type)
{
    return Type != EResourceType::None &&
        Type != EResourceType::Count &&
        !IsSummaryResourceType(Type);
}

// Phase-2 special goods. Keep enum ids reserved so saves/data remain stable,
// but do not surface these in the immediate vanilla-core scope.
inline bool IsPhaseTwoSpecialResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::SpecialChocolate:
    case EResourceType::Goldnuts:
    case EResourceType::BS:
        return true;
    default:
        return false;
    }
}

// Immediate project scope is vanilla core production + source-backed major
// workmode switches. Future/DLC special goods and the Vertical Farm /
// Synthetic Food Lab economy stay out of policy/trade UI until their full
// production loops are source-backed.
inline bool IsImmediateProductionScopeResourceType(EResourceType Type)
{
    return IsConcreteEconomicResourceType(Type) &&
        !IsPhaseTwoSpecialResourceType(Type);
}

inline bool IsEdibleResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Coconuts:
    case EResourceType::Fish:
    case EResourceType::FarmedFish:
    case EResourceType::Banana:
    case EResourceType::Corn:
    case EResourceType::Pineapple:
    case EResourceType::Meat:
    case EResourceType::Milk:
    case EResourceType::Cheese:
    case EResourceType::CannedGoods:
    case EResourceType::Juice:
    case EResourceType::BS:
        return true;
    default:
        return false;
    }
}

inline bool ShouldVisitConsumptionUseSelfProducedFoodOnly(
    const std::string& BuildingId)
{
    return BuildingId == "build_2_4" ||
        BuildingId == "build_2_6" ||
        BuildingId == "build_2_11" ||
        BuildingId == "build_2_12";
}

template <typename TVisitor>
inline void ForEachFoodVisitCompatibleResourceType(
    const std::string& BuildingId,
    EResourceType ProducedType,
    EResourceType VisitConsumptionType,
    const TVisitor& Visitor)
{
    if (VisitConsumptionType == EResourceType::None ||
        VisitConsumptionType == EResourceType::Count)
    {
        return;
    }

    if (!IsSummaryResourceType(VisitConsumptionType))
    {
        Visitor(VisitConsumptionType);
        return;
    }

    if (ShouldVisitConsumptionUseSelfProducedFoodOnly(BuildingId))
    {
        if (IsEdibleResourceType(ProducedType))
            Visitor(ProducedType);

        return;
    }

    (void)BuildingId;
}

// Policy families are presentation/selection aliases only.
inline EResourceType GetPolicyFamilyResourceType(EResourceType Type)
{
    if (IsHydroponicPlantationPlaceholderResourceType(Type) ||
        IsCropOutputResourceType(Type))
    {
        return EResourceType::Crops;
    }

    if (Type == EResourceType::FactoryLivestock ||
        IsAnimalOutputResourceType(Type))
    {
        return EResourceType::AnimalProducts;
    }

    if (IsMineOutputResourceType(Type))
        return EResourceType::Ore;

    return Type;
}

inline EResourceMarketClass GetPolicyFamilyMarketClass(EResourceType Type)
{
    return GetResourceMarketClass(GetPolicyFamilyResourceType(Type));
}

// Supply-chain families are also presentation aliases only.
inline EResourceType GetSupplyChainFamilyResourceType(EResourceType Type)
{
    if (IsHydroponicPlantationPlaceholderResourceType(Type) ||
        IsCropOutputResourceType(Type))
    {
        return EResourceType::Crops;
    }

    if (Type == EResourceType::FactoryLivestock ||
        IsAnimalOutputResourceType(Type))
    {
        return EResourceType::AnimalProducts;
    }

    if (IsMineOutputResourceType(Type))
        return EResourceType::Ore;

    return Type;
}

template <typename TVisitor>
inline void ForEachSupplyChainDemandKeyResourceType(
    EResourceType Type,
    const TVisitor& Visitor)
{
    if (Type == EResourceType::None ||
        Type == EResourceType::Count)
    {
        return;
    }

    Visitor(Type);

    const EResourceType FamilyType =
        GetSupplyChainFamilyResourceType(Type);
    if (FamilyType != Type)
        Visitor(FamilyType);
}

inline bool IsExportableResourceType(EResourceType Type)
{
    return IsConcreteEconomicResourceType(Type);
}

inline float ResolveBuildingBaseProductionUnitsPerSecond(
    const std::string& BuildingId,
    EBuildingCategory Category,
    EResourceType ProducedType)
{
    if (ProducedType == EResourceType::None ||
        ProducedType == EResourceType::Count)
    {
        return 0.f;
    }

    // Stateful primary producers should keep a stable family-level base rate
    // even when their active output changes between exact resource types.
    if (BuildingId == "build_2_4" ||
        BuildingId == "build_2_11" ||
        BuildingId == "build_2_6" ||
        BuildingId == "build_2_12")
    {
        return 40.f;
    }

    if (BuildingId == "build_2_7" ||
        BuildingId == "build_3_1")
    {
        return 10.f;
    }

    const bool PrimaryAgricultureOutput =
        Category == EBuildingCategory::FoodResource &&
        (IsCropOutputResourceType(ProducedType) ||
            IsAnimalOutputResourceType(ProducedType));

    if (PrimaryAgricultureOutput)
        return 40.f;

    switch (GetResourceMarketClass(ProducedType))
    {
    case EResourceMarketClass::Food:
        return Category == EBuildingCategory::FoodResource ? 40.f : 8.f;
    case EResourceMarketClass::RawGoods:
        return 10.f;
    case EResourceMarketClass::ManufacturedGoods:
        return 6.f;
    case EResourceMarketClass::LuxuryGoods:
        return 4.f;
    default:
        return 0.f;
    }
}

struct FResourceInventory
{
    std::array<int, static_cast<size_t>(EResourceType::Count)> Amounts = {};

    int Get(EResourceType Type) const
    {
        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return 0;

        return Amounts[Index];
    }

    int GetTotal() const
    {
        int Total = 0;

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
            Total += Amounts[Index];

        return Total;
    }

    int GetExportableTotal() const
    {
        int Total = 0;

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type))
                continue;

            Total += Amounts[Index];
        }

        return Total;
    }

    void Add(EResourceType Type, int Amount, int MaxPerType)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return;

        Amounts[Index] = (std::min)(MaxPerType, Amounts[Index] + Amount);
    }

    bool Consume(EResourceType Type, int Amount)
    {
        if (Amount <= 0)
            return true;

        if (Type == EResourceType::None)
            return false;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return false;

        if (Amounts[Index] < Amount)
            return false;

        Amounts[Index] -= Amount;
        return true;
    }

    bool ConsumeAnyExportable(int Amount, EResourceType& OutType)
    {
        if (Amount <= 0)
        {
            OutType = EResourceType::None;
            return true;
        }

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type) ||
                Amounts[Index] < Amount)
            {
                continue;
            }

            Amounts[Index] -= Amount;
            OutType = Type;
            return true;
        }

        OutType = EResourceType::None;
        return false;
    }

    bool ConsumeExportableAmount(int Amount)
    {
        if (Amount <= 0)
            return true;

        if (GetExportableTotal() < Amount)
            return false;

        int Remaining = Amount;

        for (size_t Index = 0; Index < Amounts.size() && Remaining > 0; ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type) ||
                Amounts[Index] <= 0)
            {
                continue;
            }

            const int Consumed = (std::min)(Amounts[Index], Remaining);
            Amounts[Index] -= Consumed;
            Remaining -= Consumed;
        }

        return Remaining == 0;
    }
};

// PlacementController / PlacementBuildingVisual 이 참조하는 건물 비주얼 분기 타입.
// Texture 파일 경로는 SpriteTexturePath 가 담당하고, 이 enum 은
// "어떤 종류의 비주얼 규칙을 적용할지"만 결정한다.
enum class EPlacementBuildingKind
{
    Structure,          // 일반 구조물 기본 분기
    Road,               // 도로 전용 분기
    TransportOffice,    // 운송업자 사무소 전용 분기
    Harbor              // 항구 전용 분기
};

// 배치 시 점유하는 타일 영역의 모양·크기 프리셋.
// PlacementAreaObject::MakePlacementTemplate() 에서 각 값에 대응하는
// FPlacementTemplate 을 생성한다.
//
//   이름 규칙: Diamond{논리지름}x{논리지름}{마커수}Marker
//   DiamondRadius : 1 → 3×3, 2 → 5×5, 3 → 7×7
//
//   새 크기가 필요하면 이 enum 에 값을 추가하고
//   PlacementAreaObject::MakePlacementTemplate() 의 switch 에도 케이스를 추가한다.
enum class EPlacementTemplateType
{
    SingleTileMarker,       // 1x1 타일, 마커 1개 (도로 등)
    Diamond3x3SingleMarker,  // 반지름 1, 마커 1개, 입구 Gap 있음 (기본·소형)
    Diamond5x5TwoMarker,     // 반지름 2, 마커 2개 (중형)
    Diamond5x5FourMarker,    // 반지름 2, 마커 4개 (중형·4방향 입구)
    Diamond7x7ThreeMarker    // 반지름 3, 마커 3개 (대형)
};

// 배치 마커(입구·장식 오브젝트) 하나의 논리 좌표 오프셋.
// 아이소메트릭 논리 좌표계 기준 (x: 오른쪽, y: 아래).
// PlacementAreaObject 가 중심 타일의 논리 좌표에 이 오프셋을 더해 마커 위치를 결정한다.
struct FPlacementMarkerAnchor
{
    float LogicalOffsetX = 0.f;  // 중심으로부터 논리 X 오프셋
    float LogicalOffsetY = 0.f;  // 중심으로부터 논리 Y 오프셋
};

// 배치 영역 전체를 기술하는 런타임 데이터.
// BuildingCatalog 의 EPlacementTemplateType → PlacementAreaObject::MakePlacementTemplate()
// 을 통해 생성되며, PlacementAreaObject 가 보유·사용한다.
struct FPlacementTemplate
{
    EPlacementTemplateType Type =
        EPlacementTemplateType::Diamond3x3SingleMarker;  // 원본 프리셋 타입 (참조용)

    // 아이소메트릭 다이아몬드 반지름.
    // 실제 점유 타일은 |dx|+|dy| <= DiamondRadius 조건을 만족하는 격자 셀.
    // 1 → 3×3(약 9칸), 2 → 5×5(약 25칸), 3 → 7×7(약 49칸)
    int DiamondRadius = 1;

    // 건물 입구·장식 마커 앵커 목록.
    // PlacementController 가 이 위치에 BuildingMarkerOrb 를 생성한다.
    std::vector<FPlacementMarkerAnchor> MarkerAnchors;

    FVector4 AreaColor = FVector4::Blue;  // 배치 가능 영역 타일 표시 색상

    // true 면 회전 방향에 따라 영역 외곽 타일 1칸을 비워 "입구" 공간을 만든다.
    // Diamond3x3SingleMarker 에만 기본 활성화, 나머지는 false.
    bool HasDirectionalGap = true;

    // 예상 점유 타일 수를 반환한다 (Gap 1칸 제외 포함).
    // 주로 유효성 검증에 사용.
    int GetExpectedTileCount() const
    {
        const int Side = DiamondRadius * 2 + 1;
        return Side * Side - (HasDirectionalGap ? 1 : 0);
    }
};
