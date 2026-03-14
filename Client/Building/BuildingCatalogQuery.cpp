#include "BuildingCatalog.h"
#include "../StringUtils.h"
#include <algorithm>
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

    std::vector<std::wstring> BuildProductionDemandDisplayLabels(
        const FBuildingCatalogEntry& Entry)
    {
        std::vector<std::wstring> DemandLabels =
            BuildProductionInputDisplayLabels(Entry);

        if (Entry.ProducedResourceType == EResourceType::None &&
            Entry.VisitConsumptionResourceType != EResourceType::None)
        {
            const std::wstring VisitLabel =
                GetResourceTypeDisplayName(
                    Entry.VisitConsumptionResourceType);

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
    FBuildingCatalogEntry::EProductionChainStage Stage)
{
    switch (Stage)
    {
    case FBuildingCatalogEntry::EProductionChainStage::Primary:
        return L"1차 자원";
    case FBuildingCatalogEntry::EProductionChainStage::Intermediate:
        return L"중간재";
    case FBuildingCatalogEntry::EProductionChainStage::Final:
        return L"완제품";
    default:
        break;
    }

    return L"";
}

std::wstring BuildProductionChainSummary(
    const FBuildingCatalogEntry& Entry)
{
    if (!Entry.SupplyChainSummary.empty())
        return Entry.SupplyChainSummary;

    const std::wstring OutputLabel =
        GetBuildingProducedResourceDisplayName(Entry);
    const std::vector<std::wstring> DemandLabels =
        BuildProductionDemandDisplayLabels(Entry);

    if (OutputLabel.empty())
    {
        if (DemandLabels.empty() || Entry.DisplayName.empty())
            return std::wstring();

        return JoinLabels(DemandLabels, L" + ") +
            L" -> " +
            Entry.DisplayName;
    }

    if (DemandLabels.empty())
    {
        if (!HasProductionInputs(Entry))
            return OutputLabel + L" 생산";

        return OutputLabel;
    }

    return JoinLabels(DemandLabels, L" + ") +
        L" -> " +
        OutputLabel;
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
