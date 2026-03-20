#include "CitizenInfoWorldQuerySourceInternal.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradePolicyRuntime.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/EdictSystem.h"
#include "../StringUtils.h"
#include "../World/MainWorldAccess.h"
#include "../World/MainWorldConfig.h"
#include "World/World.h"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cwchar>
#include <vector>

namespace CitizenInfoWorldQuerySourceInternal
{
    using StringUtils::ExtractDetailValue;
    using StringUtils::ParseLeadingInteger;
    using StringUtils::SplitLines;
    using StringUtils::StartsWith;
    using StringUtils::Trim;
    using StringUtils::Utf8ToWide;

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

    std::wstring BuildExportBlockedSelectionText(
        const TradePolicy::FExportTradePolicy& Policy)
    {
        std::vector<std::wstring> BlockedResources;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const EResourceType ResourceType =
                static_cast<EResourceType>(ResourceIndex);

            if (!IsExportableResourceType(ResourceType) ||
                !IsImmediateProductionScopeResourceType(ResourceType) ||
                TradePolicy::IsResourceExportAllowed(Policy, ResourceType))
            {
                continue;
            }

            BlockedResources.push_back(
                GetResourceTypeDisplayName(ResourceType));
        }

        if (BlockedResources.empty())
            return L"없음";

        return JoinLabels(BlockedResources, L", ");
    }

    int ExtractPowerValueMW(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        return ParseLeadingInteger(
            ExtractDetailValue(DetailText, Prefix),
            0);
    }

    void PushUnique(std::vector<std::string>& Names, const std::string& Name)
    {
        if (Name.empty())
            return;

        if (std::find(Names.begin(), Names.end(), Name) == Names.end())
            Names.push_back(Name);
    }

    bool IsOperationalBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea();
    }

    bool TryGetCoverageDistanceSq(
        const std::shared_ptr<CPlacementAreaObject>& Office,
        const std::shared_ptr<CPlacementAreaObject>& Building,
        float& OutDistSq)
    {
        OutDistSq = FLT_MAX;

        if (!Office || !Building)
            return false;

        int OfficeGridX = 0;
        int OfficeGridY = 0;
        int BuildingGridX = 0;
        int BuildingGridY = 0;

        if (!Office->GetPlacedCenterGridCoords(OfficeGridX, OfficeGridY) ||
            !Building->GetPlacedCenterGridCoords(BuildingGridX, BuildingGridY))
        {
            return false;
        }

        const float dx = static_cast<float>(OfficeGridX - BuildingGridX);
        const float dy = static_cast<float>(OfficeGridY - BuildingGridY);
        OutDistSq = dx * dx + dy * dy;
        return true;
    }

    bool IsWithinTeamsterCoverage(
        const std::shared_ptr<CPlacementAreaObject>& Office,
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        float DistSq = FLT_MAX;

        if (!TryGetCoverageDistanceSq(Office, Building, DistSq))
            return false;

        const float CoverageRadius =
            GameConstants::Orb::TeamsterCoverageRadiusTiles;
        if (CoverageRadius <= 0.f)
            return true;
        return DistSq <= CoverageRadius * CoverageRadius;
    }

    bool BuildingConsumesResource(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType Type)
    {
        if (!IsOperationalBuilding(Building) ||
            Type == EResourceType::None ||
            Building->IsRoad() ||
            Building->IsBusStop() ||
            Building->IsHarbor() ||
            Building->IsTransportOffice() ||
            Building->IsWarehouse())
        {
            return false;
        }

        if (Building->GetVisitConsumptionResourceType() == Type &&
            Building->GetProducedResourceType() != Type)
        {
            return true;
        }

        return Building->UsesProductionInputResource(Type);
    }

    std::wstring BuildResourceAmountSummary(
        const std::vector<std::pair<EResourceType, int>>& Entries,
        size_t MaxCount)
    {
        std::vector<std::pair<EResourceType, int>> SortedEntries = Entries;
        std::sort(
            SortedEntries.begin(),
            SortedEntries.end(),
            [](const std::pair<EResourceType, int>& A,
                const std::pair<EResourceType, int>& B)
            {
                if (A.second != B.second)
                    return A.second > B.second;

                return static_cast<int>(A.first) <
                    static_cast<int>(B.first);
            });

        std::wstring Result;
        const size_t SafeMaxCount = (std::max)(size_t(1), MaxCount);

        for (size_t Index = 0;
            Index < SortedEntries.size() && Index < SafeMaxCount;
            ++Index)
        {
            if (SortedEntries[Index].second <= 0)
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += GetResourceTypeDisplayName(SortedEntries[Index].first);
            Result += L" ";
            Result += StringUtils::FormatIntegerWithCommas(SortedEntries[Index].second);
        }

        return Result;
    }

    std::wstring BuildBuildingMetricSummary(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        size_t MaxCount)
    {
        std::vector<std::pair<std::wstring, int>> SortedEntries = Entries;
        std::sort(
            SortedEntries.begin(),
            SortedEntries.end(),
            [](const std::pair<std::wstring, int>& A,
                const std::pair<std::wstring, int>& B)
            {
                if (A.second != B.second)
                    return A.second > B.second;

                return A.first < B.first;
            });

        std::wstring Result;
        const size_t SafeMaxCount = (std::max)(size_t(1), MaxCount);

        for (size_t Index = 0;
            Index < SortedEntries.size() && Index < SafeMaxCount;
            ++Index)
        {
            if (SortedEntries[Index].second <= 0)
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += SortedEntries[Index].first;
            Result += L" ";
            Result += StringUtils::FormatIntegerWithCommas(SortedEntries[Index].second);
        }

        return Result;
    }

    std::wstring BuildBuildingCoverageLabel(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        if (!Building)
            return std::wstring();

        std::wstring Label = Utf8ToWide(Building->GetBuildingDisplayName());

        if (Label.empty())
            Label = Utf8ToWide(Building->GetName());

        int GridX = 0;
        int GridY = 0;

        if (Building->GetPlacedCenterGridCoords(GridX, GridY))
        {
            Label += L" [";
            Label += std::to_wstring(GridX);
            Label += L",";
            Label += std::to_wstring(GridY);
            Label += L"]";
        }

        return Label;
    }

    bool IsCoveredByAnyTransportOffice(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        const std::vector<std::shared_ptr<CPlacementAreaObject>>& Offices)
    {
        if (!IsOperationalBuilding(Building))
            return false;

        for (size_t Index = 0; Index < Offices.size(); ++Index)
        {
            if (IsWithinTeamsterCoverage(Offices[Index], Building))
                return true;
        }

        return false;
    }

    bool IsTeamsterTransitState(ECitizenState State)
    {
        return State == ECitizenState::GoingToTeamsterSource ||
            State == ECitizenState::GoingToTeamsterHarbor ||
            State == ECitizenState::GoingToTeamsterConsumerSource ||
            State == ECitizenState::GoingToTeamsterConsumerTarget ||
            State == ECitizenState::GoingToTeamsterOffice;
    }

    float ResolveTaxEventProductionMultiplier(
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        static_cast<void>(TaxEventStatus);
        return 1.f;
    }

    float ResolveWorldCrisisProductionMultiplier(
        const FWorldCrisisStatus* WorldCrisisStatus)
    {
        static_cast<void>(WorldCrisisStatus);
        return 1.f;
    }

    float ResolveBaseProductionUnitsPerSecond(
        const CPlacementAreaObject& Building)
    {
        return ResolveBuildingBaseProductionUnitsPerSecond(
            Building.GetBuildingId(),
            Building.GetBuildingCategory(),
            Building.GetProducedResourceType());
    }

    bool HasCatalogProductionIdentity(const FBuildingCatalogEntry* CatalogEntry)
    {
        if (!CatalogEntry)
            return false;

        if (CatalogEntry->ProducedResourceType != EResourceType::None ||
            CatalogEntry->UsesRecipeTable ||
            CatalogEntry->ProductionChainStage !=
                EBuildingProductionChainStage::None)
        {
            return true;
        }

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            if (CatalogEntry->ProductionInputTypes[
                    static_cast<size_t>(SlotIndex)] != EResourceType::None &&
                CatalogEntry->ProductionInputAmounts[
                    static_cast<size_t>(SlotIndex)] > 0)
            {
                return true;
            }
        }

        return false;
    }

    bool IsValidProductionInputSlot(
        const CitizenInfoDataProvider::FProductionInputSlotView& InputSlot)
    {
        return InputSlot.Type != EResourceType::None &&
            InputSlot.RequiredAmount > 0;
    }

    bool HasProductionInputSlots(
        const std::array<CitizenInfoDataProvider::FProductionInputSlotView,
            GProductionInputSlotCount>& InputSlots)
    {
        for (size_t Index = 0; Index < InputSlots.size(); ++Index)
        {
            if (IsValidProductionInputSlot(InputSlots[Index]))
                return true;
        }

        return false;
    }

    int ResolveProductionInputCurrentStock(
        const CPlacementAreaObject& Building,
        EResourceType InputType)
    {
        if (InputType == EResourceType::None ||
            InputType == EResourceType::Count)
        {
            return 0;
        }

        if (InputType == EResourceType::FeedCrops)
        {
            return Building.GetProductionInputCompatibleResourceStock(
                InputType);
        }

        return Building.GetResourceStock(InputType);
    }

    int ResolveProductionInputMaxStock(
        const CPlacementAreaObject& Building,
        EResourceType InputType)
    {
        if (InputType == EResourceType::None ||
            InputType == EResourceType::Count)
        {
            return 0;
        }

        if (InputType == EResourceType::FeedCrops)
        {
            return Building.GetResourceTypeCapacity(EResourceType::Corn) +
                Building.GetResourceTypeCapacity(EResourceType::Sugar);
        }

        return Building.GetResourceTypeCapacity(InputType);
    }

    void PopulateProductionResourceView(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry* CatalogEntry,
        CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
    {
        OutRecord.ProducedResourceType =
            Building.GetProducedResourceType() != EResourceType::None ?
                Building.GetProducedResourceType() :
                (CatalogEntry ?
                    CatalogEntry->ProducedResourceType :
                    EResourceType::None);
        OutRecord.ProducedResourceStock =
            OutRecord.ProducedResourceType == EResourceType::None ?
                0 :
                Building.GetResourceStock(OutRecord.ProducedResourceType);

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            CitizenInfoDataProvider::FProductionInputSlotView& InputRecord =
                OutRecord.ProductionInputs[static_cast<size_t>(SlotIndex)];
            InputRecord = CitizenInfoDataProvider::FProductionInputSlotView();

            EResourceType InputType = Building.GetProductionInputType(SlotIndex);
            int RequiredAmount = Building.GetProductionInputAmount(SlotIndex);

            if ((InputType == EResourceType::None || RequiredAmount <= 0) &&
                CatalogEntry)
            {
                InputType =
                    CatalogEntry->ProductionInputTypes[
                        static_cast<size_t>(SlotIndex)];
                RequiredAmount =
                    CatalogEntry->ProductionInputAmounts[
                        static_cast<size_t>(SlotIndex)];
            }

            InputRecord.Type = InputType;
            InputRecord.RequiredAmount = (std::max)(0, RequiredAmount);

            if (!IsValidProductionInputSlot(InputRecord))
                continue;

            InputRecord.CurrentStock = ResolveProductionInputCurrentStock(
                Building,
                InputRecord.Type);
            InputRecord.MaxStock = ResolveProductionInputMaxStock(
                Building,
                InputRecord.Type);
        }
    }

    FBuildingOperationModeEffect ResolveCatalogRuntimeEffect(
        const FBuildingCatalogEntry* CatalogEntry,
        int ActiveOperationModeIndex,
        int ActiveRuntimeUpgradeIndex)
    {
        FBuildingOperationModeEffect Result;

        if (!CatalogEntry)
            return Result;

        if (ActiveOperationModeIndex >= 0 &&
            ActiveOperationModeIndex <
                static_cast<int>(CatalogEntry->OperationModeDefs.size()))
        {
            Result =
                CatalogEntry->OperationModeDefs[
                    static_cast<size_t>(ActiveOperationModeIndex)].Effect;
        }

        if (ActiveRuntimeUpgradeIndex >= 0 &&
            ActiveRuntimeUpgradeIndex <
                static_cast<int>(CatalogEntry->RuntimeUpgradeDefs.size()))
        {
            const FBuildingOperationModeEffect& UpgradeEffect =
                CatalogEntry->RuntimeUpgradeDefs[
                    static_cast<size_t>(ActiveRuntimeUpgradeIndex)].Effect;

            if (UpgradeEffect.HasProducedResourceTypeOverride)
            {
                Result.HasProducedResourceTypeOverride = true;
                Result.ProducedResourceTypeOverride =
                    UpgradeEffect.ProducedResourceTypeOverride;
            }

            if (UpgradeEffect.HasProductionInputTypesOverride)
            {
                Result.HasProductionInputTypesOverride = true;
                Result.ProductionInputTypesOverride =
                    UpgradeEffect.ProductionInputTypesOverride;
                Result.ProductionInputAmountsOverride =
                    UpgradeEffect.ProductionInputAmountsOverride;
            }

            if (UpgradeEffect.HasVisitConsumptionTypeOverride)
            {
                Result.HasVisitConsumptionTypeOverride = true;
                Result.VisitConsumptionTypeOverride =
                    UpgradeEffect.VisitConsumptionTypeOverride;
            }

            if (UpgradeEffect.HasVisitConsumptionAcceptedTypesOverride)
            {
                Result.HasVisitConsumptionAcceptedTypesOverride = true;
                Result.VisitConsumptionAcceptedTypesOverride =
                    UpgradeEffect.VisitConsumptionAcceptedTypesOverride;
            }

            Result.ProductionMultiplier *= UpgradeEffect.ProductionMultiplier;
            Result.InputConsumptionMultiplier *=
                UpgradeEffect.InputConsumptionMultiplier;
        }

        return Result;
    }

    float ResolveUiPowerOperationalMultiplier(
        const CPlacementAreaObject& Building)
    {
        if (Building.GetRequiredPowerMW() <= 0)
            return 1.f;

        if (Building.GetPowerSupplyRatio() <= 0.05f)
            return 0.f;

        return (std::max)(
            0.f,
            (std::min)(1.f, Building.GetPowerSupplyRatio()));
    }

    CWorldCitizenInfoBuildingQuery::CWorldCitizenInfoBuildingQuery(
        const CWorldCitizenInfoQuerySource& Owner)
        : mOwner(Owner)
    {
    }

    CWorldCitizenInfoCitizenQuery::CWorldCitizenInfoCitizenQuery(
        const CWorldCitizenInfoQuerySource& Owner)
        : mOwner(Owner)
    {
    }

    CWorldCitizenInfoTradeQuery::CWorldCitizenInfoTradeQuery(
        const CWorldCitizenInfoQuerySource& Owner)
        : mOwner(Owner)
    {
    }

    CWorldCitizenInfoQuerySource::CWorldCitizenInfoQuerySource(
        const std::shared_ptr<CWorld>& World)
        : mWorld(World)
        , mMainWorldAccess(
            ResolveMainWorldBuildMenuAccess(World))
        , mMainWorldPolicyAccess(
            ResolveMainWorldAlmanacAccess(World))
        , mMainWorldTradeAccess(
            ResolveMainWorldTradeAccess(World))
        , mMainWorldKnowledgeAccess(
            ResolveMainWorldKnowledgeAccess(World))
        , mBuildingQuery(*this)
        , mCitizenQuery(*this)
        , mTradeQuery(*this)
    {
    }
}
namespace CitizenInfoWorldQuerySource
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CitizenInfoWorldQuerySourceInternal::CWorldCitizenInfoQuerySource>(World);
    }
}

