#include "BuildingCatalog.h"
#include "../StringUtils.h"
#include "../Map/PlacementAreaObject.h"
#include <algorithm>
#include <array>
#include <vector>

namespace
{
    using StringUtils::WideToUtf8;

    bool HasProductionInputs(const FBuildingCatalogEntry& Entry)
    {
        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            const size_t Index = static_cast<size_t>(SlotIndex);

            if (Entry.ProductionInputTypes[Index] != EResourceType::None &&
                Entry.ProductionInputAmounts[Index] > 0)
            {
                return true;
            }
        }

        return false;
    }

    std::vector<std::wstring> BuildProductionInputDisplayLabels(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> InputLabels;

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            const std::wstring InputLabel =
                GetBuildingProductionInputDisplayName(Entry, SlotIndex);

            if (InputLabel.empty())
                continue;

            std::wstring DisplayLabel = InputLabel;
            const size_t Index = static_cast<size_t>(SlotIndex);

            if (Entry.ProductionInputLabels[Index].empty() &&
                Entry.ProductionInputAmounts[Index] > 1)
            {
                DisplayLabel += L" x";
                DisplayLabel +=
                    std::to_wstring(Entry.ProductionInputAmounts[Index]);
            }

            InputLabels.push_back(DisplayLabel);
        }

        return InputLabels;
    }

    std::vector<std::wstring> BuildAcceptedResourceDisplayLabels(
        const std::vector<EResourceType>& AcceptedTypes)
    {
        std::vector<std::wstring> AcceptedLabels;

        for (size_t Index = 0; Index < AcceptedTypes.size(); ++Index)
        {
            const EResourceType AcceptedType = AcceptedTypes[Index];

            if (AcceptedType == EResourceType::None ||
                AcceptedType == EResourceType::Count ||
                IsSummaryResourceType(AcceptedType))
            {
                continue;
            }

            const std::wstring Label =
                std::wstring(GetResourceTypeDisplayName(AcceptedType));

            if (Label.empty())
                continue;

            if (std::find(
                    AcceptedLabels.begin(),
                    AcceptedLabels.end(),
                    Label) == AcceptedLabels.end())
            {
                AcceptedLabels.push_back(Label);
            }
        }

        return AcceptedLabels;
    }

    std::wstring BuildVisitConsumptionDemandLabel(
        EResourceType VisitType,
        const std::vector<EResourceType>& AcceptedTypes)
    {
        const std::vector<std::wstring> AcceptedLabels =
            BuildAcceptedResourceDisplayLabels(AcceptedTypes);

        if (!AcceptedLabels.empty())
        {
            std::wstring JoinedAcceptedLabels;

            for (size_t Index = 0; Index < AcceptedLabels.size(); ++Index)
            {
                if (!JoinedAcceptedLabels.empty())
                    JoinedAcceptedLabels += L"/";

                JoinedAcceptedLabels += AcceptedLabels[Index];
            }

            return L"소비 품목(" + JoinedAcceptedLabels + L")";
        }

        return std::wstring(GetResourceTypeDisplayName(VisitType));
    }

    std::vector<std::wstring> BuildProductionDemandDisplayLabels(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> DemandLabels =
            BuildProductionInputDisplayLabels(Entry);

        if (Entry.ProducedResourceType == EResourceType::None &&
            Entry.VisitConsumptionResourceType != EResourceType::None)
        {
            const std::wstring VisitLabel =
                BuildVisitConsumptionDemandLabel(
                    Entry.VisitConsumptionResourceType,
                    Entry.VisitConsumptionAcceptedResourceTypes);

            if (!VisitLabel.empty() &&
                std::find(
                    DemandLabels.begin(),
                    DemandLabels.end(),
                    VisitLabel) == DemandLabels.end())
            {
                DemandLabels.insert(DemandLabels.begin(), VisitLabel);
            }
        }

        return DemandLabels;
    }

    std::wstring BuildProductionConsumerDisplayLabel(
        const FBuildingCatalogEntry& Entry)
    {
        const std::wstring OutputLabel =
            GetBuildingProducedResourceDisplayName(Entry);

        if (!OutputLabel.empty())
            return OutputLabel;

        return Entry.DisplayName;
    }

    std::wstring JoinLabels(
        const std::vector<std::wstring>& Labels,
        const wchar_t* Separator)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Labels.size(); ++Index)
        {
            if (Labels[Index].empty())
                continue;

            if (!Result.empty() && Separator)
                Result += Separator;

            Result += Labels[Index];
        }

        return Result;
    }

    std::wstring BuildOperationModeSummary(
        const FBuildingOperationModeDef& ModeDef)
    {
        std::vector<std::wstring> Segments;

        if (ModeDef.HasUnlockEra)
        {
            Segments.push_back(
                std::wstring(GetBuildingEraDisplayName(ModeDef.UnlockEra)));
        }

        if (!ModeDef.RequiredResearch.empty())
            Segments.push_back(ModeDef.RequiredResearch);

        if (!ModeDef.EffectSummary.empty())
            Segments.push_back(ModeDef.EffectSummary);

        return JoinLabels(Segments, L" / ");
    }

    void AppendUniqueLabel(
        std::vector<std::wstring>& Labels,
        const std::wstring& Label)
    {
        if (Label.empty())
            return;

        if (std::find(Labels.begin(), Labels.end(), Label) == Labels.end())
            Labels.push_back(Label);
    }

    bool HasRuntimeProductionInputs(
        const CPlacementAreaObject& Building)
    {
        for (int SlotIndex = 0;
            SlotIndex < Building.GetProductionInputCount();
            ++SlotIndex)
        {
            if (Building.GetProductionInputType(SlotIndex) !=
                    EResourceType::None &&
                Building.GetProductionInputAmount(SlotIndex) > 0)
            {
                return true;
            }
        }

        return false;
    }

    std::wstring BuildProducedResourceDisplayNameForType(
        const FBuildingCatalogEntry& Entry,
        EResourceType EffectiveProducedType)
    {
        if (EffectiveProducedType == EResourceType::None)
            return std::wstring();

        if (EffectiveProducedType == Entry.ProducedResourceType)
            return GetBuildingProducedResourceDisplayName(Entry);

        return std::wstring(GetResourceTypeDisplayName(EffectiveProducedType));
    }

    std::wstring ComposeProductionChainSummary(
        const std::wstring& OutputLabel,
        const std::vector<std::wstring>& DemandLabels,
        bool EntryHasInputs,
        const std::vector<std::wstring>& DownstreamLabels,
        const std::wstring& FallbackDisplayName)
    {
        if (OutputLabel.empty())
        {
            if (DemandLabels.empty() || FallbackDisplayName.empty())
                return std::wstring();

            return JoinLabels(DemandLabels, L" + ") +
                L" -> " +
                FallbackDisplayName;
        }

        std::wstring Summary;

        if (!DemandLabels.empty())
        {
            Summary += JoinLabels(DemandLabels, L" + ");
            Summary += L" -> ";
        }

        Summary += OutputLabel;

        if (EntryHasInputs && !DownstreamLabels.empty())
        {
            Summary += L" -> ";
            Summary += JoinLabels(DownstreamLabels, L", ");
        }
        else if (!EntryHasInputs)
        {
            Summary += L" 생산";
        }

        return Summary;
    }

    EBuildingProductionChainStage ComposeProductionChainStage(
        const std::wstring& OutputLabel,
        bool EntryHasInputs,
        const std::vector<std::wstring>& DownstreamLabels)
    {
        if (OutputLabel.empty())
            return EBuildingProductionChainStage::None;

        if (!EntryHasInputs)
            return EBuildingProductionChainStage::Primary;

        return !DownstreamLabels.empty() ?
            EBuildingProductionChainStage::Intermediate :
            EBuildingProductionChainStage::Final;
    }

    std::wstring BuildRuntimeProducedResourceDisplayName(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry& Entry)
    {
        const EResourceType RuntimeProducedType =
            Building.GetProducedResourceType();

        if (RuntimeProducedType == EResourceType::None)
            return std::wstring();

        if (RuntimeProducedType == Entry.ProducedResourceType)
            return GetBuildingProducedResourceDisplayName(Entry);

        return std::wstring(GetResourceTypeDisplayName(RuntimeProducedType));
    }

    std::vector<std::wstring> BuildRuntimeProductionDemandDisplayLabels(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> DemandLabels;

        for (int SlotIndex = 0;
            SlotIndex < Building.GetProductionInputCount();
            ++SlotIndex)
        {
            const EResourceType InputType =
                Building.GetProductionInputType(SlotIndex);
            const int InputAmount =
                Building.GetProductionInputAmount(SlotIndex);

            if (InputType == EResourceType::None || InputAmount <= 0)
                continue;

            std::wstring DisplayLabel;

            if (SlotIndex < GProductionInputSlotCount &&
                InputType ==
                    Entry.ProductionInputTypes[
                        static_cast<size_t>(SlotIndex)] &&
                !Entry.ProductionInputLabels[
                    static_cast<size_t>(SlotIndex)].empty())
            {
                DisplayLabel =
                    Entry.ProductionInputLabels[
                        static_cast<size_t>(SlotIndex)];
            }
            else
            {
                DisplayLabel =
                    std::wstring(GetResourceTypeDisplayName(InputType));

                if (InputAmount > 1)
                {
                    DisplayLabel += L" x";
                    DisplayLabel += std::to_wstring(InputAmount);
                }
            }

            DemandLabels.push_back(DisplayLabel);
        }

        if (Building.GetProducedResourceType() == EResourceType::None &&
            Building.GetVisitConsumptionResourceType() != EResourceType::None)
        {
            const std::vector<EResourceType> RuntimeAcceptedTypes =
                Building.GetVisitConsumptionAcceptedResourceTypes();
            const std::wstring VisitLabel =
                BuildVisitConsumptionDemandLabel(
                    Building.GetVisitConsumptionResourceType(),
                    RuntimeAcceptedTypes);

            if (!VisitLabel.empty() &&
                std::find(
                    DemandLabels.begin(),
                    DemandLabels.end(),
                    VisitLabel) == DemandLabels.end())
            {
                DemandLabels.insert(DemandLabels.begin(), VisitLabel);
            }
        }

        return DemandLabels;
    }

    std::vector<std::wstring> BuildRuntimeDownstreamConsumerLabels(
        EResourceType ProducedType)
    {
        std::vector<std::wstring> DownstreamLabels;

        if (ProducedType == EResourceType::None ||
            ProducedType == EResourceType::Count)
        {
            return DownstreamLabels;
        }

        std::array<bool, static_cast<size_t>(EResourceType::Count)> MatchKeys =
            {};

        ForEachSupplyChainDemandKeyResourceType(
            ProducedType,
            [&](EResourceType KeyType)
            {
                const size_t ResourceIndex = static_cast<size_t>(KeyType);

                if (ResourceIndex < MatchKeys.size())
                    MatchKeys[ResourceIndex] = true;
            });

        auto MatchesProducedType = [&](EResourceType DemandType)
        {
            bool Matches = false;

            ForEachSupplyChainDemandKeyResourceType(
                DemandType,
                [&](EResourceType KeyType)
                {
                    const size_t ResourceIndex =
                        static_cast<size_t>(KeyType);

                    if (ResourceIndex < MatchKeys.size() &&
                        MatchKeys[ResourceIndex])
                    {
                        Matches = true;
                    }
                });

            return Matches;
        };

        const std::vector<FBuildingCatalogEntry>& Catalog = GetBuildingCatalog();

        for (size_t EntryIndex = 0; EntryIndex < Catalog.size(); ++EntryIndex)
        {
            const FBuildingCatalogEntry& ConsumerEntry = Catalog[EntryIndex];
            const std::wstring ConsumerLabel =
                BuildProductionConsumerDisplayLabel(ConsumerEntry);

            if (ConsumerLabel.empty())
                continue;

            bool UsesProducedType = false;

            if (ConsumerEntry.VisitConsumptionResourceType !=
                    EResourceType::None &&
                ConsumerEntry.VisitConsumptionResourceType !=
                    ConsumerEntry.ProducedResourceType)
            {
                const EResourceType VisitType =
                    ConsumerEntry.VisitConsumptionResourceType;

                if (!ConsumerEntry.VisitConsumptionAcceptedResourceTypes.empty())
                {
                    for (size_t AcceptedIndex = 0;
                        AcceptedIndex <
                            ConsumerEntry.VisitConsumptionAcceptedResourceTypes.size();
                        ++AcceptedIndex)
                    {
                        if (MatchesProducedType(
                                ConsumerEntry
                                    .VisitConsumptionAcceptedResourceTypes[
                                        AcceptedIndex]))
                        {
                            UsesProducedType = true;
                            break;
                        }
                    }
                }
                else if (ConsumerEntry.FoodProvider &&
                    IsSummaryResourceType(VisitType))
                {
                    ForEachFoodVisitCompatibleResourceType(
                        ConsumerEntry.Id,
                        ConsumerEntry.ProducedResourceType,
                        VisitType,
                        [&](EResourceType CompatibleType)
                        {
                            if (MatchesProducedType(CompatibleType))
                                UsesProducedType = true;
                        });
                }
                else if (MatchesProducedType(VisitType))
                {
                    UsesProducedType = true;
                }
            }

            for (int SlotIndex = 0;
                !UsesProducedType &&
                    SlotIndex < GProductionInputSlotCount;
                ++SlotIndex)
            {
                const size_t InputIndex = static_cast<size_t>(SlotIndex);
                const EResourceType InputType =
                    ConsumerEntry.ProductionInputTypes[InputIndex];

                if (InputType == EResourceType::None ||
                    ConsumerEntry.ProductionInputAmounts[InputIndex] <= 0 ||
                    InputType == ConsumerEntry.VisitConsumptionResourceType)
                {
                    continue;
                }

                const bool FeedInput =
                    IsFeedInputDemandResourceType(InputType);

                if (FeedInput)
                {
                    ForEachFeedCompatibleResourceType(
                        InputType,
                        [&](EResourceType CompatibleType)
                        {
                            if (MatchesProducedType(CompatibleType))
                                UsesProducedType = true;
                        });
                }
                else
                {
                    ForEachIndustrialInputCompatibleResourceType(
                        InputType,
                        [&](EResourceType CompatibleType)
                        {
                            if (MatchesProducedType(CompatibleType))
                                UsesProducedType = true;
                        });
                }
            }

            if (UsesProducedType)
                AppendUniqueLabel(DownstreamLabels, ConsumerLabel);
        }

        return DownstreamLabels;
    }
} // namespace

bool IsCustomsOfficeCatalogEntry(const FBuildingCatalogEntry& Entry)
{
    return Entry.IsCustomsOffice;
}

bool IsCustomsOfficeBuildingId(const std::string& EntryId)
{
    const FBuildingCatalogEntry* const Entry =
        FindBuildingCatalogEntry(EntryId);
    return Entry && IsCustomsOfficeCatalogEntry(*Entry);
}

EBuildingCategory GetEffectiveBuildMenuCategory(
    const FBuildingCatalogEntry& Entry)
{
    if (Entry.HasBuildMenuCategoryOverride)
        return Entry.BuildMenuCategoryOverride;

    if (Entry.Category == EBuildingCategory::Entertainment &&
        Entry.CategoryLocalIndex >= 12)
    {
        return EBuildingCategory::LuxuryEntertainment;
    }

    if (Entry.Category == EBuildingCategory::PublicService &&
        Entry.IsCustomsOffice)
    {
        return EBuildingCategory::GovernmentFinance;
    }

    return Entry.Category;
}

std::string GetCatalogEntryIconPathUtf8(const FBuildingCatalogEntry& Entry)
{
    const wchar_t* const IconPath = GetCatalogEntryIconPath(Entry);
    return IconPath ? WideToUtf8(std::wstring(IconPath)) : std::string();
}

std::string GetCatalogEntryIconPathUtf8(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    const wchar_t* const IconPath =
        GetCatalogEntryIconPath(Category, CategoryLocalIndex);
    return IconPath ? WideToUtf8(std::wstring(IconPath)) : std::string();
}

std::string GetCatalogEntrySpriteTexturePathUtf8(
    const FBuildingCatalogEntry& Entry)
{
    const wchar_t* const SpriteTexturePath =
        GetCatalogEntrySpriteTexturePath(Entry);
    return SpriteTexturePath ?
        WideToUtf8(std::wstring(SpriteTexturePath)) :
        std::string();
}

std::string GetCatalogEntrySpriteTexturePathUtf8(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    const wchar_t* const SpriteTexturePath =
        GetCatalogEntrySpriteTexturePath(Category, CategoryLocalIndex);
    return SpriteTexturePath ?
        WideToUtf8(std::wstring(SpriteTexturePath)) :
        std::string();
}

std::wstring GetBuildingProducedResourceDisplayName(
    const FBuildingCatalogEntry& Entry)
{
    if (Entry.ProducedResourceType == EResourceType::None)
        return std::wstring();

    if (!Entry.ProducedResourceLabel.empty())
        return Entry.ProducedResourceLabel;

    return std::wstring(GetResourceTypeDisplayName(Entry.ProducedResourceType));
}

std::wstring GetBuildingProductionInputDisplayName(
    const FBuildingCatalogEntry& Entry,
    int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= GProductionInputSlotCount)
        return std::wstring();

    const size_t Index = static_cast<size_t>(SlotIndex);
    const EResourceType InputType = Entry.ProductionInputTypes[Index];
    const int InputAmount = Entry.ProductionInputAmounts[Index];

    if (InputType == EResourceType::None || InputAmount <= 0)
        return std::wstring();

    if (!Entry.ProductionInputLabels[Index].empty())
        return Entry.ProductionInputLabels[Index];

    return std::wstring(GetResourceTypeDisplayName(InputType));
}

const wchar_t* GetProductionChainStageDisplayName(
    EBuildingProductionChainStage Stage)
{
    switch (Stage)
    {
    case EBuildingProductionChainStage::Primary:
        return L"1차 자원";
    case EBuildingProductionChainStage::Intermediate:
        return L"중간재";
    case EBuildingProductionChainStage::Final:
        return L"완제품";
    default:
        break;
    }

    return L"";
}

std::wstring BuildProductionChainSummary(
    const FBuildingCatalogEntry& Entry,
    EResourceType EffectiveProducedType)
{
    if (EffectiveProducedType == EResourceType::None)
        EffectiveProducedType = Entry.ProducedResourceType;

    if (EffectiveProducedType == Entry.ProducedResourceType &&
        !Entry.SupplyChainSummary.empty())
    {
        return Entry.SupplyChainSummary;
    }

    const std::wstring OutputLabel =
        BuildProducedResourceDisplayNameForType(
            Entry,
            EffectiveProducedType);
    const std::vector<std::wstring> DemandLabels =
        BuildProductionDemandDisplayLabels(Entry);
    const bool EntryHasInputs = HasProductionInputs(Entry);
    const std::vector<std::wstring> DownstreamLabels =
        EffectiveProducedType != EResourceType::None ?
            BuildRuntimeDownstreamConsumerLabels(EffectiveProducedType) :
            std::vector<std::wstring>();

    return ComposeProductionChainSummary(
        OutputLabel,
        DemandLabels,
        EntryHasInputs,
        DownstreamLabels,
        Entry.DisplayName);
}

std::wstring BuildRuntimeProductionChainSummary(
    const CPlacementAreaObject& Building,
    const FBuildingCatalogEntry& Entry)
{
    const std::wstring OutputLabel =
        BuildRuntimeProducedResourceDisplayName(Building, Entry);
    const std::vector<std::wstring> DemandLabels =
        BuildRuntimeProductionDemandDisplayLabels(Building, Entry);

    const bool EntryHasInputs = HasRuntimeProductionInputs(Building);
    const std::vector<std::wstring> DownstreamLabels =
        BuildRuntimeDownstreamConsumerLabels(Building.GetProducedResourceType());

    return ComposeProductionChainSummary(
        OutputLabel,
        DemandLabels,
        EntryHasInputs,
        DownstreamLabels,
        Entry.DisplayName);
}

EBuildingProductionChainStage BuildRuntimeProductionChainStage(
    const CPlacementAreaObject& Building,
    const FBuildingCatalogEntry& Entry)
{
    const std::wstring OutputLabel =
        BuildRuntimeProducedResourceDisplayName(Building, Entry);
    const bool EntryHasInputs = HasRuntimeProductionInputs(Building);
    const std::vector<std::wstring> DownstreamLabels =
        BuildRuntimeDownstreamConsumerLabels(Building.GetProducedResourceType());

    return ComposeProductionChainStage(
        OutputLabel,
        EntryHasInputs,
        DownstreamLabels);
}

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)].DisplayName;
}

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogRuntimeData& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)].DisplayName;
}

std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return BuildOperationModeSummary(
        Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)]);
}

std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogRuntimeData& Entry,
    int ModeIndex)
{
    if (ModeIndex < 0 ||
        ModeIndex >= static_cast<int>(Entry.OperationModeDefs.size()))
    {
        return std::wstring();
    }

    return BuildOperationModeSummary(
        Entry.OperationModeDefs[static_cast<size_t>(ModeIndex)]);
}

std::wstring GetOperationModeTransitionNotice(
    const FBuildingCatalogEntry& Entry)
{
    for (size_t Index = 0; Index < Entry.OperationModeDefs.size(); ++Index)
    {
        const FBuildingOperationModeEffect& Effect =
            Entry.OperationModeDefs[Index].Effect;

        if (!Effect.HasProductionRecipeOverride() &&
            !Effect.HasVisitConsumptionOverride())
        {
            continue;
        }

        return
            L"전환 후 진행 중 화물은 자동으로 새 품목으로 바뀌지 않습니다. "
            L"이미 실린 화물은 기존 품목으로 도착하고, 예약만 된 물량만 "
            L"남은 재고 기준으로 다시 픽업됩니다.";
    }

    return std::wstring();
}

std::wstring GetOperationModeTransitionNotice(
    const FBuildingCatalogRuntimeData& Entry)
{
    for (size_t Index = 0; Index < Entry.OperationModeDefs.size(); ++Index)
    {
        const FBuildingOperationModeEffect& Effect =
            Entry.OperationModeDefs[Index].Effect;

        if (!Effect.HasProductionRecipeOverride() &&
            !Effect.HasVisitConsumptionOverride())
        {
            continue;
        }

        return
            L"전환 후 진행 중 화물은 자동으로 새 품목으로 바뀌지 않습니다. "
            L"이미 실린 화물은 기존 품목으로 도착하고, 예약만 된 물량만 "
            L"남은 재고 기준으로 다시 픽업됩니다.";
    }

    return std::wstring();
}

std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        DisplayName;
}

std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogRuntimeData& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        DisplayName;
}

std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        EffectSummary;
}

std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogRuntimeData& Entry,
    int UpgradeIndex)
{
    if (UpgradeIndex < 0 ||
        UpgradeIndex >= static_cast<int>(Entry.RuntimeUpgradeDefs.size()))
    {
        return std::wstring();
    }

    return Entry.RuntimeUpgradeDefs[static_cast<size_t>(UpgradeIndex)].
        EffectSummary;
}

