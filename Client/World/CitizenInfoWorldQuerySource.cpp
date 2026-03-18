#include "CitizenInfoWorldQuerySource.h"
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

namespace
{
    using StringUtils::Utf8ToWide;

    std::wstring Trim(const std::wstring& Text)
    {
        size_t Start = 0;

        while (Start < Text.size() && iswspace(Text[Start]))
            ++Start;

        size_t End = Text.size();

        while (End > Start && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Start, End - Start);
    }

    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long UnsignedValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(UnsignedValue);
        std::wstring Result;
        int GroupCount = 0;

        for (int Index = static_cast<int>(Digits.size()) - 1;
            Index >= 0;
            --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }

            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }

        if (Negative)
            Result.insert(Result.begin(), L'-');

        return Result;
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

    bool StartsWith(const std::wstring& Text, const wchar_t* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = wcslen(Prefix);

        if (Text.size() < PrefixLength)
            return false;

        return Text.compare(0, PrefixLength, Prefix) == 0;
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring Current;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'\r')
                continue;

            if (Ch == L'\n')
            {
                Lines.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Ch);
        }

        if (!Current.empty() || Text.empty())
            Lines.push_back(Current);

        return Lines;
    }

    int ParseLeadingInteger(const std::wstring& Text, int DefaultValue = 0)
    {
        bool Negative = false;
        bool FoundDigit = false;
        int Value = 0;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (!FoundDigit && Ch == L'-')
            {
                Negative = true;
                continue;
            }

            if (Ch < L'0' || Ch > L'9')
            {
                if (FoundDigit)
                    break;

                continue;
            }

            FoundDigit = true;
            Value = Value * 10 + static_cast<int>(Ch - L'0');
        }

        if (!FoundDigit)
            return DefaultValue;

        return Negative ? -Value : Value;
    }

    std::wstring ExtractDetailValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (!StartsWith(Line, Prefix))
                continue;

            return Trim(Line.substr(wcslen(Prefix)));
        }

        return std::wstring();
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
            Result += FormatInteger(SortedEntries[Index].second);
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
            Result += FormatInteger(SortedEntries[Index].second);
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
        if (!TaxEventStatus ||
            !TaxEventStatus->Active ||
            TaxEventStatus->Type == ETaxPolicyEventType::None)
        {
            return 1.f;
        }

        const float Severity = (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(TaxEventStatus->DaysActive + 1) / 6.f));

        switch (TaxEventStatus->Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return 0.74f - 0.30f * Severity;
        case ETaxPolicyEventType::BudgetCrisis:
            return 0.92f - 0.18f * Severity;
        default:
            return 1.f;
        }
    }

    float ResolveWorldCrisisProductionMultiplier(
        const FWorldCrisisStatus* WorldCrisisStatus)
    {
        if (!WorldCrisisStatus ||
            !WorldCrisisStatus->Active ||
            WorldCrisisStatus->Type == EWorldCrisisType::None)
        {
            return 1.f;
        }

        const float Severity = (std::max)(
            0.f,
            (std::min)(
                1.f,
                static_cast<float>(WorldCrisisStatus->DaysActive + 1) / 6.f));

        switch (WorldCrisisStatus->Type)
        {
        case EWorldCrisisType::Raid:
            return 0.90f - 0.14f * Severity;
        case EWorldCrisisType::LaborStrike:
            return 0.78f - 0.24f * Severity;
        case EWorldCrisisType::CrimeWave:
            return 0.92f - 0.12f * Severity;
        case EWorldCrisisType::FiscalEmergency:
            return 0.94f - 0.10f * Severity;
        case EWorldCrisisType::None:
        default:
            return 1.f;
        }
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

    class CWorldCitizenInfoQuerySource final :
        public CitizenInfoDataProvider::ICitizenInfoQuerySource
    {
    public:
        explicit CWorldCitizenInfoQuerySource(
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
        {
        }

    public:
        bool TryGetBuildingRecord(
            const std::string& BuildingName,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const override
        {
            auto Building = FindValidBuilding(BuildingName);

            if (!Building)
                return false;

            OutRecord =
                CitizenInfoDataProvider::FCitizenInfoBuildingRecord();
            OutRecord.Valid = true;
            OutRecord.ObjectName = Utf8ToWide(Building->GetName());
            OutRecord.DisplayName =
                Utf8ToWide(Building->GetBuildingDisplayName());
            OutRecord.CategoryName =
                Utf8ToWide(Building->GetBuildingCategoryName());
            OutRecord.BuildingId = Building->GetBuildingId();
            const FBuildingCatalogEntry* const CatalogEntry =
                FindBuildingCatalogEntry(OutRecord.BuildingId);
            OutRecord.Residential = Building->IsResidential();
            OutRecord.WorkProvider =
                !OutRecord.Residential &&
                Building->GetCapacity() > 0;
            OutRecord.FoodProvider = Building->IsFoodProvider();
            OutRecord.EntertainmentProvider =
                Building->IsEntertainmentProvider();
            OutRecord.HealthProvider = Building->IsHealthProvider();
            OutRecord.FaithProvider = Building->IsFaithProvider();
            OutRecord.Harbor = Building->IsHarbor();
            OutRecord.Warehouse = Building->IsWarehouse();
            OutRecord.IsRoad = Building->IsRoad();
            OutRecord.CanGenerateWorkOutput =
                Building->CanGenerateWorkOutput();
            OutRecord.Capacity = (std::max)(0, Building->GetCapacity());
            OutRecord.BudgetLevel = Building->GetBudgetLevel();
            OutRecord.BudgetScale = Building->GetBudgetSatisfactionScale();
            OutRecord.AccessibilityScore =
                Building->GetAccessibilityScore();
            OutRecord.HousingCap = Building->GetHousingSatisfactionCap();
            OutRecord.JobCap = Building->GetEffectiveJobSatisfactionCap();
            OutRecord.FoodCap = Building->GetFoodSatisfactionCap();
            OutRecord.FunCap = Building->GetFunSatisfactionCap();
            OutRecord.HealthCap = Building->GetHealthSatisfactionCap();
            OutRecord.FaithCap = Building->GetFaithSatisfactionCap();
            OutRecord.ServiceCapacity =
                Building->GetMaxServiceVisitCapacity();
            OutRecord.PollutionOutput = Building->GetPollutionOutput();
            OutRecord.PollutionMitigation =
                Building->GetPollutionMitigation();
            OutRecord.LocalPollutionExposure =
                Building->GetLocalPollutionExposure();
            OutRecord.ResourceStock = Building->GetResourceStock();
            OutRecord.ExportableStock =
                Building->GetExportableResourceStock();
            OutRecord.MaxResourceStock = Building->GetMaxResourceStock();
            PopulateProductionResourceView(*Building, CatalogEntry, OutRecord);
            const CitizenInfoDataProvider::EProductionChainStage RuntimeChainStage =
                CatalogEntry ?
                    BuildRuntimeProductionChainStage(
                        *Building,
                        *CatalogEntry) :
                    CitizenInfoDataProvider::EProductionChainStage::None;
            OutRecord.ChainStage =
                RuntimeChainStage;
            OutRecord.ProducedPowerMW = Building->GetProducedPowerMW();
            OutRecord.RequiredPowerMW = Building->GetRequiredPowerMW();
            OutRecord.PowerSupplyRatio = Building->GetPowerSupplyRatio();
            OutRecord.LastProductionEfficiency =
                Building->GetLastProductionEfficiency();
            OutRecord.DamageEfficiencyMultiplier =
                Building->GetDamageEfficiencyMultiplier();
            OutRecord.HarborShipProgressPercent =
                Building->GetHarborShipProgressPercent();
            OutRecord.ActiveOperationModeIndex =
                Building->GetActiveOperationModeIndex();
            OutRecord.ActiveRuntimeUpgradeIndex =
                Building->GetActiveRuntimeUpgradeIndex();
            OutRecord.DamageLevel = Building->GetDamageLevel();
            OutRecord.RepairCost = Building->GetRepairCost();
            OutRecord.RepairAffordable =
                OutRecord.RepairCost <= 0 ||
                (mMainWorldAccess &&
                    static_cast<long long>(OutRecord.RepairCost) <=
                        mMainWorldAccess->GetNationalBudget());
            OutRecord.ActiveOperationModeText =
                Building->GetActiveOperationModeDisplayName();
            OutRecord.ActiveOperationModeEffectSummary =
                Building->GetActiveOperationModeEffectSummary();
            OutRecord.ActiveRuntimeUpgradeText =
                Building->GetActiveRuntimeUpgradeDisplayName();
            OutRecord.ActiveRuntimeUpgradeEffectSummary =
                Building->GetActiveRuntimeUpgradeEffectSummary();
            OutRecord.KnowledgePoints =
                mMainWorldKnowledgeAccess ?
                    mMainWorldKnowledgeAccess->GetKnowledgePoints() :
                    0;
            OutRecord.DailyKnowledgeGeneration =
                mMainWorldKnowledgeAccess ?
                    mMainWorldKnowledgeAccess->GetDailyKnowledgeGeneration() :
                    0;

            const int OperationModeCount = Building->GetOperationModeCount();

            for (int ModeIndex = 0; ModeIndex < OperationModeCount; ++ModeIndex)
            {
                OutRecord.OperationModeResearchLocked.push_back(
                    Building->IsOperationModeResearchLocked(ModeIndex));
                OutRecord.OperationModeResearchCosts.push_back(
                    Building->GetOperationModeResearchCost(ModeIndex));
                OutRecord.OperationModeResearchLabels.push_back(
                    Building->GetOperationModeResearchLabel(ModeIndex));
            }

            OutRecord.ProductionChainStageText =
                RuntimeChainStage !=
                    CitizenInfoDataProvider::EProductionChainStage::None ?
                    std::wstring(
                        GetProductionChainStageDisplayName(
                            RuntimeChainStage)) :
                    std::wstring();
            OutRecord.SupplyChainSummaryText =
                CatalogEntry ?
                    BuildRuntimeProductionChainSummary(
                        *Building,
                        *CatalogEntry) :
                    std::wstring();
            OutRecord.RequiredEducationLevel =
                Building->GetRequiredEducationLevel();
            OutRecord.UsesResourceStock =
                OutRecord.ResourceStock > 0 ||
                OutRecord.CanGenerateWorkOutput ||
                OutRecord.ProducedResourceType != EResourceType::None ||
                HasProductionInputSlots(OutRecord.ProductionInputs) ||
                HasCatalogProductionIdentity(CatalogEntry) ||
                OutRecord.FoodProvider ||
                OutRecord.Harbor ||
                OutRecord.Warehouse;
            OutRecord.DaysInMonth = mMainWorldAccess ?
                (std::max)(1, mMainWorldAccess->GetSimulationMonthDayCount()) :
                30;
            OutRecord.MonthlyWageCost = Building->GetMonthlyWageCost();
            OutRecord.MonthlyUpkeepCost = Building->GetMonthlyUpkeepCost();
            OutRecord.DailyWageCost =
                Building->GetDailyWageCost(OutRecord.DaysInMonth);
            OutRecord.DailyUpkeepCost =
                Building->GetDailyUpkeepCost(OutRecord.DaysInMonth);
            PopulateProductionFlowMetrics(Building, CatalogEntry, OutRecord);

            if (OutRecord.Warehouse)
            {
                OutRecord.WarehousePolicySelectionText =
                    Building->GetWarehouseStoragePolicyDisplayName();
                OutRecord.WarehousePrioritySelectionText =
                    Building->GetWarehousePriorityDisplayName();

                for (int SlotIndex = 0;
                    SlotIndex < Building->GetWarehouseSlotCount();
                    ++SlotIndex)
                {
                    CitizenInfoDataProvider::FWarehouseSlotRecord SlotRecord;
                    SlotRecord.Type =
                        Building->GetWarehouseSlotType(SlotIndex);
                    SlotRecord.Capacity =
                        Building->GetWarehouseSlotCapacityUnits();
                    SlotRecord.Stock =
                        SlotRecord.Type == EResourceType::None ?
                            0 :
                            Building->GetResourceStock(SlotRecord.Type);
                    if (SlotRecord.Type != EResourceType::None)
                    {
                        SlotRecord.Capacity =
                            Building->GetResourceTypeCapacity(SlotRecord.Type);
                    }
                    OutRecord.WarehouseSlots.push_back(SlotRecord);
                }
            }

            PopulatePowerTotals(OutRecord);
            PopulateCitizenAssignments(BuildingName, *Building, OutRecord);
            PopulateLogisticsLines(Building, OutRecord);

            if (OutRecord.Harbor)
            {
                PopulateHarborTradePolicy(*Building, OutRecord);

                for (int TypeIndex = 1;
                    TypeIndex < static_cast<int>(EResourceType::Count);
                    ++TypeIndex)
                {
                    const EResourceType ResourceType =
                        static_cast<EResourceType>(TypeIndex);
                    const int Stock =
                        Building->GetResourceStock(ResourceType);

                    if (Stock <= 0)
                        continue;

                    CitizenInfoDataProvider::FWarehouseSlotRecord Slot;
                    Slot.Type = ResourceType;
                    Slot.Stock = Stock;
                    Slot.Capacity = 0;
                    OutRecord.HarborResourceSlots.push_back(Slot);
                }
            }

            if (IsCustomsOfficeBuildingId(OutRecord.BuildingId))
            {
                PopulateCustomsTradeSummary(OutRecord);
            }

            return true;
        }

        bool TryGetCitizenRecord(
            const std::string& CitizenName,
            CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord)
            const override
        {
            auto Citizen = FindValidCitizen(CitizenName);

            if (!Citizen)
                return false;

            OutRecord =
                CitizenInfoDataProvider::FCitizenInfoCitizenRecord();
            OutRecord.Valid = true;
            OutRecord.Name = Citizen->GetName();
            OutRecord.Satisfaction = Citizen->GetSatisfaction();
            OutRecord.IdentityProfile = Citizen->GetIdentityProfile();
            OutRecord.PoliticalProfile = Citizen->GetPoliticalProfile();
            OutRecord.State = Citizen->GetCitizenState();
            OutRecord.HomeBuildingName = Citizen->GetHomeBuilding();
            OutRecord.WorkBuildingName = Citizen->GetWorkBuilding();
            OutRecord.FoodBuildingName = Citizen->GetFoodBuilding();
            OutRecord.FoodVisitBuildingName = Citizen->GetFoodVisitBuilding();
            OutRecord.FunBuildingName = Citizen->GetFunBuilding();
            OutRecord.FunVisitBuildingName = Citizen->GetFunVisitBuilding();
            OutRecord.HealthBuildingName = Citizen->GetHealthBuilding();
            OutRecord.HealthVisitBuildingName =
                Citizen->GetHealthVisitBuilding();
            OutRecord.FaithBuildingName = Citizen->GetFaithBuilding();
            OutRecord.FaithVisitBuildingName =
                Citizen->GetFaithVisitBuilding();
            return true;
        }

        std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const override
        {
            if (BuildingName.empty())
                return L"-";

            auto Building = FindValidBuilding(BuildingName);

            if (!Building)
                return Utf8ToWide(BuildingName);

            const std::wstring DisplayName =
                Utf8ToWide(Building->GetBuildingDisplayName());
            return DisplayName.empty() ?
                Utf8ToWide(BuildingName) :
                DisplayName;
        }

    private:
        int CountActiveCitizenOrbs() const
        {
            if (!mWorld)
                return 0;

            std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

            if (!mWorld->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
                return 0;

            int Count = 0;

            for (size_t Index = 0; Index < OrbList.size(); ++Index)
            {
                const auto Orb = OrbList[Index].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                ++Count;
            }

            return Count;
        }

        void PopulateProductionFlowMetrics(
            const std::shared_ptr<CPlacementAreaObject>& Building,
            const FBuildingCatalogEntry* CatalogEntry,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!Building)
                return;

            OutRecord.CurrentWorkerOccupancy = (std::max)(
                0,
                Building->GetCurrentWorkerOccupancy());

            if (!Building->CanGenerateWorkOutput() ||
                Building->GetProducedResourceType() == EResourceType::None)
            {
                return;
            }

            const float BaseUnitsPerSecond =
                ResolveBaseProductionUnitsPerSecond(*Building);

            if (BaseUnitsPerSecond <= 0.f)
                return;

            std::array<EResourceType, GProductionInputSlotCount> InputTypes = {};
            const FBuildingOperationModeEffect RuntimeEffect =
                ResolveCatalogRuntimeEffect(
                    CatalogEntry,
                    Building->GetActiveOperationModeIndex(),
                    Building->GetActiveRuntimeUpgradeIndex());

            for (int SlotIndex = 0;
                SlotIndex < GProductionInputSlotCount;
                ++SlotIndex)
            {
                InputTypes[static_cast<size_t>(SlotIndex)] =
                    SlotIndex < Building->GetProductionInputCount() ?
                        Building->GetProductionInputType(SlotIndex) :
                        EResourceType::None;
            }

            FGovernmentEdictModifiers EdictModifiers;
            const FGovernmentProfile* const GovernmentProfile =
                mMainWorldPolicyAccess ?
                    &mMainWorldPolicyAccess->GetGovernmentProfile() :
                    nullptr;
            const FTaxPolicyEventStatus* const TaxEventStatus =
                mMainWorldPolicyAccess ?
                    &mMainWorldPolicyAccess->GetTaxPolicyEventStatus() :
                    nullptr;
            const FWorldCrisisStatus* const WorldCrisisStatus =
                mMainWorldPolicyAccess ?
                    &mMainWorldPolicyAccess->GetWorldCrisisStatus() :
                    nullptr;

            if (mMainWorldPolicyAccess)
            {
                EdictModifiers =
                    EdictSystem::CalculateEdictModifiers(
                        mMainWorldPolicyAccess->GetGovernmentEdictStates(),
                        CountActiveCitizenOrbs());
            }

            const float TradePolicyProductionMultiplier =
                GovernmentProfile ?
                    TradePolicyRuntime::ComputeBuildingProductionMultiplier(
                        Building->GetProducedResourceType(),
                        InputTypes,
                        GovernmentProfile->ExportTradePolicy,
                        GovernmentProfile->ImportTradePolicy) :
                    1.f;
            const float NominalUnitsPerSecond =
                BaseUnitsPerSecond *
                (std::max)(0.f, EdictModifiers.ProductionMultiplier) *
                ResolveTaxEventProductionMultiplier(TaxEventStatus) *
                ResolveWorldCrisisProductionMultiplier(WorldCrisisStatus) *
                TradePolicyProductionMultiplier *
                (std::max)(0.f, RuntimeEffect.ProductionMultiplier) *
                (std::max)(0.f, Building->GetBudgetSatisfactionScale()) *
                Building->GetDamageEfficiencyMultiplier() *
                ResolveUiPowerOperationalMultiplier(*Building);
            const float EffectiveProductionEfficiency =
                (OutRecord.CurrentWorkerOccupancy > 0 &&
                    OutRecord.Capacity > 0) ?
                    (std::max)(
                        0.f,
                        (std::min)(
                            1.f,
                            Building->GetLastProductionEfficiency())) :
                    0.f;

            OutRecord.CurrentProductionUnitsPerSecond =
                NominalUnitsPerSecond * EffectiveProductionEfficiency;
            const float DailyProductionUnits =
                OutRecord.CurrentProductionUnitsPerSecond *
                MainWorldConfig::GSecondsPerSimulationDay;
            OutRecord.EstimatedDailyProductionUnits = (std::max)(
                0,
                static_cast<int>(roundf(DailyProductionUnits)));
            OutRecord.EstimatedMonthlyProductionUnits = (std::max)(
                0,
                static_cast<int>(roundf(
                    DailyProductionUnits *
                    static_cast<float>((std::max)(1, OutRecord.DaysInMonth)))));

            const float EffectiveInputConsumptionMultiplier = (std::max)(
                0.f,
                RuntimeEffect.InputConsumptionMultiplier);

            for (size_t Index = 0; Index < OutRecord.ProductionInputs.size();
                ++Index)
            {
                CitizenInfoDataProvider::FProductionInputSlotView& InputRecord =
                    OutRecord.ProductionInputs[Index];
                if (!IsValidProductionInputSlot(InputRecord))
                    continue;
                const float ConsumptionUnitsPerSecond =
                    OutRecord.CurrentProductionUnitsPerSecond *
                    static_cast<float>((std::max)(1, InputRecord.RequiredAmount)) *
                    EffectiveInputConsumptionMultiplier;

                InputRecord.ConsumptionUnitsPerSecond =
                    ConsumptionUnitsPerSecond;
                InputRecord.EstimatedDailyConsumptionUnits = (std::max)(
                    0,
                    static_cast<int>(roundf(
                        ConsumptionUnitsPerSecond *
                        MainWorldConfig::GSecondsPerSimulationDay)));
                InputRecord.EstimatedMonthlyConsumptionUnits = (std::max)(
                    0,
                    static_cast<int>(roundf(
                        static_cast<float>(
                            InputRecord.EstimatedDailyConsumptionUnits) *
                        static_cast<float>((std::max)(
                            1,
                            OutRecord.DaysInMonth)))));
            }
        }

        void PopulateCustomsTradeSummary(
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            OutRecord.LastDailyExportIncome =
                mMainWorldPolicyAccess ?
                    mMainWorldPolicyAccess->GetLastDailyExportIncome() :
                    0;
            OutRecord.LastDailyImportExpense =
                mMainWorldPolicyAccess ?
                    mMainWorldPolicyAccess->GetLastDailyImportExpense() :
                    0;

            if (mMainWorldTradeAccess)
            {
                auto AccumulateRoute =
                    [&](bool Completed,
                        bool ImportRoute,
                        int FulfilledUnits,
                        int ContractUnits)
                {
                    (void)Completed;

                    if (ImportRoute)
                    {
                        OutRecord.TradeRouteImportFulfilledUnits +=
                            (std::max)(0, FulfilledUnits);
                    }
                    else
                    {
                        OutRecord.TradeRouteExportFulfilledUnits +=
                            (std::max)(0, FulfilledUnits);
                        OutRecord.TradeRouteExportContractUnits +=
                            (std::max)(0, ContractUnits);
                    }
                };

                const auto& ActiveRoutes =
                    mMainWorldTradeAccess->GetActiveTradeRoutes();

                for (size_t Index = 0; Index < ActiveRoutes.size(); ++Index)
                {
                    const FTradeRouteRuntimeState& Route =
                        ActiveRoutes[Index];
                    AccumulateRoute(
                        false,
                        Route.ImportRoute,
                        Route.FulfilledUnits,
                        Route.ContractUnits);
                }

                const auto& CompletedRoutes =
                    mMainWorldTradeAccess->GetCompletedTradeRoutes();

                for (size_t Index = 0;
                    Index < CompletedRoutes.size();
                    ++Index)
                {
                    const FTradeRouteCompletionRecord& Route =
                        CompletedRoutes[Index];
                    AccumulateRoute(
                        true,
                        Route.ImportRoute,
                        Route.FulfilledUnits,
                        Route.ContractUnits);
                }
            }

            if (!mWorld)
                return;

            std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

            if (!mWorld->FindObjectListByType<CPlacementAreaObject>(BuildingList))
                return;

            for (size_t Index = 0; Index < BuildingList.size(); ++Index)
            {
                auto Building = BuildingList[Index].lock();

                if (!IsOperationalBuilding(Building))
                    continue;

                const FBuildingCatalogEntry* const Entry =
                    FindBuildingCatalogEntry(Building->GetBuildingId());

                if (!Entry || Entry->Category != EBuildingCategory::Tourism)
                    continue;

                for (int ServiceIndex = 0;
                    ServiceIndex < GBuildingServiceTypeCount;
                    ++ServiceIndex)
                {
                    OutRecord.TourismArrivalCount +=
                        (std::max)(
                            0,
                            Building->GetActiveServiceVisitorCount(
                                static_cast<EBuildingServiceType>(
                                    ServiceIndex)));
                }
            }
        }

    private:
        void PopulateHarborTradePolicy(
            CPlacementAreaObject& HarborBuilding,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            const TradePolicy::FExportTradePolicy* ExportPolicy =
                mMainWorldPolicyAccess ?
                    &mMainWorldPolicyAccess->GetGovernmentProfile().
                        ExportTradePolicy :
                    nullptr;
            const TradePolicy::FExportTradePolicy DefaultPolicy;
            const TradePolicy::FExportTradePolicy& ActivePolicy =
                ExportPolicy ? *ExportPolicy : DefaultPolicy;
            OutRecord.HarborExportSelectionText =
                BuildExportBlockedSelectionText(ActivePolicy);

            OutRecord.HarborPolicyLines.push_back(
                std::wstring(L"선적 방식: ") +
                (ActivePolicy.PrioritizeHighValueCargo ?
                    L"고가 상품 우선" :
                    L"재고량 우선"));
            OutRecord.HarborPolicyLines.push_back(
                L"1회 선적 한도: " +
                FormatInteger(
                    TradePolicy::GetHarborExportShipCapacityUnits(
                        ActivePolicy)));
            OutRecord.HarborPolicyLines.push_back(
                L"수출 금지: " + OutRecord.HarborExportSelectionText);

            std::wstring ProductionFocusLine =
                ActivePolicy.PrioritizeHighValueCargo ?
                    L"생산 유도: 제조·사치재 수출 우대" :
                    L"생산 유도: 식품·원자재 대량 수출 우대";

            auto FormatSignedCurrency = [&](long long Value) -> std::wstring
            {
                if (Value == 0)
                    return L"$0";

                const bool Positive = Value > 0;
                const unsigned long long AbsoluteValue = Positive ?
                    static_cast<unsigned long long>(Value) :
                    static_cast<unsigned long long>(-Value);
                return std::wstring(Positive ? L"+$" : L"-$") +
                    FormatInteger(static_cast<long long>(AbsoluteValue));
            };

            const long long ForecastBudgetDelta =
                TradePolicyRuntime::ComputeDailyTradePolicyBudgetDelta(
                    ActivePolicy,
                    TradePolicy::FImportTradePolicy(),
                    mMainWorldPolicyAccess ?
                        mMainWorldPolicyAccess->GetLastDailyExportIncome() :
                        0,
                    mMainWorldPolicyAccess ?
                        mMainWorldPolicyAccess->GetLastDailyImportExpense() :
                        0);

            OutRecord.HarborPolicyLines.push_back(ProductionFocusLine);
            OutRecord.HarborPolicyLines.push_back(
                L"예산 전략: 전일 무역량 기준 " +
                FormatSignedCurrency(ForecastBudgetDelta) +
                L"/일");

            int TradeBiasSampleCount = 0;
            int TotalDiplomacyExportBias = 0;
            int TotalDiplomacyImportBias = 0;
            int TotalEdictExportBias = 0;
            int TotalEdictImportBias = 0;

            auto FormatSignedPercent = [](int Value) -> std::wstring
            {
                return std::wstring(Value > 0 ? L"+" : L"") +
                    std::to_wstring(Value) +
                    L"%";
            };

            for (int ResourceIndex = 1;
                ResourceIndex < static_cast<int>(EResourceType::Count);
                ++ResourceIndex)
            {
                const EResourceType ResourceType =
                    static_cast<EResourceType>(ResourceIndex);

                if (!IsExportableResourceType(ResourceType))
                    continue;

                ++TradeBiasSampleCount;
                TotalDiplomacyExportBias +=
                    ResourceTradePricing::GetDiplomacyExportBiasPercent(
                        ResourceType);
                TotalDiplomacyImportBias +=
                    ResourceTradePricing::GetDiplomacyImportBiasPercent(
                        ResourceType);
                TotalEdictExportBias +=
                    ResourceTradePricing::GetEdictExportBiasPercent(
                        ResourceType);
                TotalEdictImportBias +=
                    ResourceTradePricing::GetEdictImportBiasPercent(
                        ResourceType);
            }

            if (TradeBiasSampleCount > 0)
            {
                OutRecord.HarborPolicyLines.push_back(
                    L"외교 보정: 수출 " +
                    FormatSignedPercent(
                        static_cast<int>(std::lround(
                            static_cast<double>(TotalDiplomacyExportBias) /
                            static_cast<double>(TradeBiasSampleCount)))) +
                    L" / 수입 " +
                    FormatSignedPercent(
                        static_cast<int>(std::lround(
                            static_cast<double>(TotalDiplomacyImportBias) /
                            static_cast<double>(TradeBiasSampleCount)))));
                OutRecord.HarborPolicyLines.push_back(
                    L"칙령 보정: 수출 " +
                    FormatSignedPercent(
                        static_cast<int>(std::lround(
                            static_cast<double>(TotalEdictExportBias) /
                            static_cast<double>(TradeBiasSampleCount)))) +
                    L" / 수입 " +
                    FormatSignedPercent(
                        static_cast<int>(std::lround(
                            static_cast<double>(TotalEdictImportBias) /
                            static_cast<double>(TradeBiasSampleCount)))));
            }

            struct FPriorityEntry
            {
                EResourceType Type = EResourceType::None;
                int Stock = 0;
                int UnitPrice = 0;
            };

            std::vector<FPriorityEntry> PriorityEntries;

            for (int ResourceIndex = 1;
                ResourceIndex < static_cast<int>(EResourceType::Count);
                ++ResourceIndex)
            {
                const EResourceType ResourceType =
                    static_cast<EResourceType>(ResourceIndex);

                if (!TradePolicy::IsResourceExportAllowed(
                        ActivePolicy,
                        ResourceType))
                {
                    continue;
                }

                const int Stock =
                    HarborBuilding.GetResourceStock(ResourceType);

                if (Stock <= 0)
                    continue;

                FPriorityEntry Entry;
                Entry.Type = ResourceType;
                Entry.Stock = Stock;
                Entry.UnitPrice =
                    ResourceTradePricing::GetExportPricePerStockUnit(
                        ResourceType);
                PriorityEntries.push_back(Entry);
            }

            std::sort(
                PriorityEntries.begin(),
                PriorityEntries.end(),
                [&](const FPriorityEntry& A, const FPriorityEntry& B)
                {
                    if (ActivePolicy.PrioritizeHighValueCargo &&
                        A.UnitPrice != B.UnitPrice)
                    {
                        return A.UnitPrice > B.UnitPrice;
                    }

                    if (A.Stock != B.Stock)
                        return A.Stock > B.Stock;

                    return static_cast<int>(A.Type) <
                        static_cast<int>(B.Type);
                });

            if (PriorityEntries.size() > 5)
                PriorityEntries.resize(5);

            for (size_t Index = 0; Index < PriorityEntries.size(); ++Index)
            {
                const FPriorityEntry& Entry = PriorityEntries[Index];
                std::wstring Line =
                    std::to_wstring(static_cast<int>(Index) + 1) +
                    L". " +
                    std::wstring(GetResourceTypeDisplayName(Entry.Type)) +
                    L" " +
                    FormatInteger(Entry.Stock) +
                    L" (단가 $" +
                    FormatInteger(Entry.UnitPrice) +
                    L")";
                OutRecord.HarborPriorityLines.push_back(std::move(Line));
            }
        }

    private:
        std::shared_ptr<CPlacementAreaObject> FindValidBuilding(
            const std::string& BuildingName) const
        {
            if (!mWorld || BuildingName.empty())
                return nullptr;

            auto Building =
                mWorld->FindObject<CPlacementAreaObject>(BuildingName).lock();

            if (!Building || !Building->GetAlive() || !Building->GetEnable())
                return nullptr;

            return Building;
        }

        std::shared_ptr<CBuildingMarkerOrb> FindValidCitizen(
            const std::string& CitizenName) const
        {
            if (!mWorld || CitizenName.empty())
                return nullptr;

            auto Citizen =
                mWorld->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

            if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
                return nullptr;

            return Citizen;
        }

        void PopulatePowerTotals(
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!mWorld)
                return;

            std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

            if (!mWorld->FindObjectListByType<CPlacementAreaObject>(BuildingList))
                return;

            for (size_t Index = 0; Index < BuildingList.size(); ++Index)
            {
                auto OtherBuilding = BuildingList[Index].lock();

                if (!OtherBuilding ||
                    !OtherBuilding->GetAlive() ||
                    !OtherBuilding->GetEnable() ||
                    !OtherBuilding->HasPlacedArea())
                {
                    continue;
                }

                const FBuildingCatalogEntry* Entry =
                    FindBuildingCatalogEntry(OtherBuilding->GetBuildingId());

                if (!Entry)
                    continue;

                OutRecord.TotalProducedPowerMW +=
                    (std::max)(0, OtherBuilding->GetProducedPowerMW());
                OutRecord.TotalRequiredPowerMW +=
                    (std::max)(0, OtherBuilding->GetRequiredPowerMW());
            }
        }

        void PopulateLogisticsLines(
            const std::shared_ptr<CPlacementAreaObject>& Building,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!Building)
                return;

            auto AppendLine = [&](const std::wstring& Line)
            {
                if (!Line.empty())
                    OutRecord.LogisticsLines.push_back(Line);
            };

            auto AppendOutputLine = [&](EResourceType Type)
            {
                if (Type == EResourceType::None)
                    return;

                AppendLine(
                    L"생산 대기: " +
                    std::wstring(GetResourceTypeDisplayName(Type)) +
                    L" 사용 가능 " +
                    FormatInteger(Building->GetAvailableResourceStock(Type)) +
                    L" / 픽업 예약 " +
                    FormatInteger(
                        Building->GetReservedResourcePickupAmount(Type)));
            };

            auto AppendDemandLine = [&](
                const wchar_t* Prefix,
                EResourceType Type)
            {
                if (!Prefix || Type == EResourceType::None)
                    return;

                const bool VisitConsumptionDemand =
                    Type == Building->GetVisitConsumptionResourceType();
                const bool CompatibleProductionInputDemand =
                    !VisitConsumptionDemand &&
                    Type == EResourceType::FeedCrops;
                const int CoveredStock =
                    VisitConsumptionDemand ?
                        Building->GetVisitConsumptionCompatibleResourceStock(
                            Type) +
                            Building
                                ->GetVisitConsumptionCompatibleReservedIncomingResourceAmount(
                                    Type) :
                    CompatibleProductionInputDemand ?
                        Building->GetProductionInputCompatibleResourceStock(
                            Type) +
                            Building
                                ->GetProductionInputCompatibleReservedIncomingResourceAmount(
                                    Type) :
                        Building->GetResourceStock(Type) +
                            Building->GetReservedIncomingResourceAmount(Type);
                const int ShortageAmount = (std::max)(
                    0,
                    GameConstants::Orb::TeamsterConsumerTargetStock -
                        CoveredStock);
                AppendLine(
                    std::wstring(Prefix) +
                    L": " +
                    GetResourceTypeDisplayName(Type) +
                    L" 재고+입고 " +
                    FormatInteger(CoveredStock) +
                    L" / 부족 " +
                    FormatInteger(ShortageAmount));
            };

            AppendOutputLine(Building->GetProducedResourceType());
            AppendDemandLine(
                L"소비 보급",
                Building->GetVisitConsumptionResourceType());

            for (int SlotIndex = 0;
                SlotIndex < Building->GetProductionInputCount();
                ++SlotIndex)
            {
                const EResourceType InputType =
                    Building->GetProductionInputType(SlotIndex);

                if (InputType == EResourceType::None ||
                    InputType == Building->GetVisitConsumptionResourceType())
                {
                    continue;
                }

                AppendDemandLine(L"투입 보급", InputType);
            }

            if (OutRecord.Warehouse)
            {
                int ActiveSlots = 0;
                int EmptySlots = 0;
                int TotalReservedIncoming = 0;
                int TotalFreeCapacity = 0;
                const int SlotCapacityUnits =
                    Building->GetWarehouseSlotCapacityUnits();

                for (int SlotIndex = 0;
                    SlotIndex < Building->GetWarehouseSlotCount();
                    ++SlotIndex)
                {
                    const EResourceType SlotType =
                        Building->GetWarehouseSlotType(SlotIndex);

                    if (SlotType == EResourceType::None)
                    {
                        ++EmptySlots;
                        TotalFreeCapacity += SlotCapacityUnits;
                        continue;
                    }

                    ++ActiveSlots;
                    TotalReservedIncoming +=
                        Building->GetReservedIncomingResourceAmount(SlotType);
                    TotalFreeCapacity +=
                        Building->GetAvailableIncomingCapacity(SlotType);
                }

                AppendLine(
                    L"창고 여유: 사용 슬롯 " +
                    std::to_wstring(ActiveSlots) +
                    L" / " +
                    std::to_wstring(Building->GetWarehouseSlotCount()) +
                    L", 빈 슬롯 " +
                    std::to_wstring(EmptySlots) +
                    L", 여유 " +
                    FormatInteger(TotalFreeCapacity));
                AppendLine(
                    L"창고 입고 예약: " +
                    FormatInteger(TotalReservedIncoming));
                AppendLine(
                    L"슬롯당 용량: " +
                    FormatInteger(SlotCapacityUnits) +
                    L" x " +
                    std::to_wstring(Building->GetWarehouseSlotCount()));
                AppendLine(
                    L"보관 정책: " +
                    Building->GetWarehouseStoragePolicyDisplayName() +
                    L" / " +
                    Building->GetWarehousePriorityDisplayName());

                if (Building->GetLastDailyWarehouseStorageLoss() > 0)
                {
                    AppendLine(
                        L"장기 보관 손실: 전일 " +
                        FormatInteger(
                            Building->GetLastDailyWarehouseStorageLoss()));
                }
            }

            if (OutRecord.Harbor)
            {
                AppendLine(
                    L"선적 대기: 사용 가능 " +
                    FormatInteger(Building->GetAvailableExportableResourceStock()) +
                    L" / 선적 예약 " +
                    FormatInteger(Building->GetReservedExportPickupAmount()));
            }

            if (Building->IsTransportOffice() && mWorld)
            {
                std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

                if (mWorld->FindObjectListByType<CPlacementAreaObject>(
                        BuildingList))
                {
                    std::vector<std::shared_ptr<CPlacementAreaObject>>
                        OfficeBuildings;
                    int ProducerCount = 0;
                    int ConsumerCount = 0;
                    int WarehouseCount = 0;
                    int HarborCount = 0;
                    int CoveredPickupReserved = 0;
                    int CoveredIncomingReserved = 0;
                    int CoveredExportReserved = 0;
                    int CoveredPickupWaiting = 0;
                    int CoveredShortageWaiting = 0;
                    int CoverageGapCount = 0;
                    int AssignedTeamsters = 0;
                    int InTransitTeamsters = 0;
                    int WaitingTeamsters = 0;
                    std::vector<std::pair<EResourceType, int>>
                        CoverageShortages;
                    std::vector<std::pair<EResourceType, int>>
                        CoveragePickupReservations;
                    std::vector<std::pair<EResourceType, int>>
                        CoverageIncomingReservations;
                    std::vector<std::pair<EResourceType, int>>
                        CoveragePickupWaitingByType;
                    std::vector<std::pair<std::wstring, int>>
                        CoverageGapBuildings;

                    for (int ResourceIndex = 1;
                        ResourceIndex < static_cast<int>(EResourceType::Count);
                        ++ResourceIndex)
                    {
                        CoverageShortages.push_back(
                            {
                                static_cast<EResourceType>(ResourceIndex),
                                0
                            });
                        CoveragePickupReservations.push_back(
                            {
                                static_cast<EResourceType>(ResourceIndex),
                                0
                            });
                        CoverageIncomingReservations.push_back(
                            {
                                static_cast<EResourceType>(ResourceIndex),
                                0
                            });
                        CoveragePickupWaitingByType.push_back(
                            {
                                static_cast<EResourceType>(ResourceIndex),
                                0
                            });
                    }

                    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
                    {
                        auto OtherBuilding = BuildingList[Index].lock();

                        if (!IsOperationalBuilding(OtherBuilding) ||
                            !OtherBuilding->IsTransportOffice())
                        {
                            continue;
                        }

                        OfficeBuildings.push_back(OtherBuilding);
                    }

                    for (size_t Index = 0; Index < BuildingList.size(); ++Index)
                    {
                        auto OtherBuilding = BuildingList[Index].lock();

                        if (!IsOperationalBuilding(OtherBuilding) ||
                            OtherBuilding == Building)
                        {
                            continue;
                        }

                        const bool CoveredByCurrentOffice =
                            IsWithinTeamsterCoverage(Building, OtherBuilding);
                        const bool CoveredByAnyOffice =
                            IsCoveredByAnyTransportOffice(
                                OtherBuilding,
                                OfficeBuildings);

                        int CoverageGapMetric = 0;

                        for (int ResourceIndex = 1;
                            ResourceIndex <
                                static_cast<int>(EResourceType::Count);
                            ++ResourceIndex)
                        {
                            const EResourceType ResourceType =
                                static_cast<EResourceType>(ResourceIndex);
                            const bool CanPickupFromBuilding =
                                OtherBuilding->IsWarehouse() ||
                                OtherBuilding->IsHarbor() ||
                                (OtherBuilding->SupportsTeamsterPickup() &&
                                    OtherBuilding->GetProducedResourceType() ==
                                        ResourceType);

                            if (CanPickupFromBuilding)
                            {
                                CoverageGapMetric +=
                                    OtherBuilding->GetAvailableResourceStock(
                                        ResourceType);
                            }

                            if (BuildingConsumesResource(
                                    OtherBuilding,
                                    ResourceType))
                            {
                                CoverageGapMetric += (std::max)(
                                    0,
                                    GameConstants::Orb::
                                        TeamsterConsumerTargetStock -
                                        (OtherBuilding->GetResourceStock(
                                            ResourceType) +
                                        OtherBuilding->
                                            GetReservedIncomingResourceAmount(
                                                ResourceType)));
                            }
                        }

                        if (!CoveredByAnyOffice && CoverageGapMetric > 0)
                        {
                            ++CoverageGapCount;
                            CoverageGapBuildings.push_back(
                                {
                                    BuildBuildingCoverageLabel(OtherBuilding),
                                    CoverageGapMetric
                                });
                        }

                        if (!CoveredByCurrentOffice)
                            continue;

                        if (OtherBuilding->SupportsTeamsterPickup() &&
                            OtherBuilding->GetProducedResourceType() !=
                                EResourceType::None)
                        {
                            ++ProducerCount;
                        }

                        if (OtherBuilding->IsWarehouse())
                            ++WarehouseCount;

                        if (OtherBuilding->IsHarbor())
                        {
                            ++HarborCount;
                            CoveredExportReserved +=
                                OtherBuilding->GetReservedExportPickupAmount();
                        }

                        bool CountedConsumer = false;

                        for (int ResourceIndex = 1;
                            ResourceIndex <
                                static_cast<int>(EResourceType::Count);
                            ++ResourceIndex)
                        {
                            const EResourceType ResourceType =
                                static_cast<EResourceType>(ResourceIndex);
                            const int ReservedPickup =
                                OtherBuilding->GetReservedResourcePickupAmount(
                                    ResourceType);
                            const int ReservedIncoming =
                                OtherBuilding->GetReservedIncomingResourceAmount(
                                    ResourceType);
                            const bool CanPickupFromBuilding =
                                OtherBuilding->IsWarehouse() ||
                                OtherBuilding->IsHarbor() ||
                                (OtherBuilding->SupportsTeamsterPickup() &&
                                    OtherBuilding->GetProducedResourceType() ==
                                        ResourceType);
                            const int PickupWaitingAmount =
                                CanPickupFromBuilding ?
                                    OtherBuilding->GetAvailableResourceStock(
                                        ResourceType) :
                                    0;

                            CoveredPickupReserved += ReservedPickup;
                            CoveredIncomingReserved += ReservedIncoming;
                            CoveredPickupWaiting += PickupWaitingAmount;
                            CoveragePickupReservations[
                                static_cast<size_t>(ResourceIndex - 1)].second +=
                                    ReservedPickup;
                            CoverageIncomingReservations[
                                static_cast<size_t>(ResourceIndex - 1)].second +=
                                    ReservedIncoming;
                            CoveragePickupWaitingByType[
                                static_cast<size_t>(ResourceIndex - 1)].second +=
                                    PickupWaitingAmount;

                            if (!BuildingConsumesResource(
                                    OtherBuilding,
                                    ResourceType))
                            {
                                continue;
                            }

                            if (!CountedConsumer)
                            {
                                ++ConsumerCount;
                                CountedConsumer = true;
                            }

                            const int ShortageAmount = (std::max)(
                                0,
                                GameConstants::Orb::
                                    TeamsterConsumerTargetStock -
                                    (OtherBuilding->GetResourceStock(
                                        ResourceType) +
                                    OtherBuilding->
                                        GetReservedIncomingResourceAmount(
                                            ResourceType)));
                            CoverageShortages[
                                static_cast<size_t>(ResourceIndex - 1)].second +=
                                    ShortageAmount;
                            CoveredShortageWaiting += ShortageAmount;
                        }
                    }

                    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

                    if (mWorld->FindObjectListByType<CBuildingMarkerOrb>(
                            OrbList))
                    {
                        for (size_t Index = 0; Index < OrbList.size(); ++Index)
                        {
                            auto Orb = OrbList[Index].lock();

                            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                                continue;

                            if (Orb->GetWorkBuilding() != Building->GetName())
                                continue;

                            ++AssignedTeamsters;

                            const ECitizenState State =
                                Orb->GetCitizenState();

                            if (IsTeamsterTransitState(State))
                                ++InTransitTeamsters;
                            else if (State == ECitizenState::AtWork)
                                ++WaitingTeamsters;
                        }
                    }

                    AppendLine(
                        L"서비스 반경: " +
                        FormatInteger(static_cast<int>(roundf(
                            GameConstants::Orb::TeamsterCoverageRadiusTiles))) +
                        L"타일 | 생산 " +
                        FormatInteger(ProducerCount) +
                        L" / 소비 " +
                        FormatInteger(ConsumerCount) +
                        L" / 창고 " +
                        FormatInteger(WarehouseCount) +
                        L" / 항구 " +
                        FormatInteger(HarborCount));

                    const std::wstring ShortageSummary =
                        BuildResourceAmountSummary(CoverageShortages, 2);
                    AppendLine(
                        L"관할 부족: " +
                        (ShortageSummary.empty() ?
                            std::wstring(L"안정") :
                            ShortageSummary));

                    const std::wstring PickupReservedSummary =
                        BuildResourceAmountSummary(
                            CoveragePickupReservations,
                            2);
                    const std::wstring IncomingReservedSummary =
                        BuildResourceAmountSummary(
                            CoverageIncomingReservations,
                            2);
                    AppendLine(
                        L"관할 예약: 픽업 " +
                        FormatInteger(CoveredPickupReserved) +
                        L" / 입고 " +
                        FormatInteger(CoveredIncomingReserved) +
                        L" / 선적 " +
                        FormatInteger(CoveredExportReserved) +
                        ((PickupReservedSummary.empty() &&
                            IncomingReservedSummary.empty()) ?
                                std::wstring() :
                                (L" (" +
                                    (PickupReservedSummary.empty() ?
                                        std::wstring() :
                                        L"픽업 " + PickupReservedSummary) +
                                    (!PickupReservedSummary.empty() &&
                                        !IncomingReservedSummary.empty() ?
                                        L" | " :
                                        std::wstring()) +
                                    (IncomingReservedSummary.empty() ?
                                        std::wstring() :
                                        L"입고 " + IncomingReservedSummary) +
                                    L")")));

                    const std::wstring PickupWaitingSummary =
                        BuildResourceAmountSummary(
                            CoveragePickupWaitingByType,
                            2);
                    AppendLine(
                        L"관할 대기: 수거 " +
                        FormatInteger(CoveredPickupWaiting) +
                        L" / 소비 부족 " +
                        FormatInteger(CoveredShortageWaiting) +
                        (PickupWaitingSummary.empty() ?
                            std::wstring() :
                            (L" (" + PickupWaitingSummary + L")")));
                    AppendLine(
                        L"팀스터 상태: 배정 " +
                        FormatInteger(AssignedTeamsters) +
                        L" / 운송 중 " +
                        FormatInteger(InTransitTeamsters) +
                        L" / 사무소 대기 " +
                        FormatInteger(WaitingTeamsters));

                    const std::wstring CoverageGapSummary =
                        BuildBuildingMetricSummary(
                            CoverageGapBuildings,
                            3);
                    AppendLine(
                        L"커버리지 사각: " +
                        (CoverageGapSummary.empty() ?
                            std::wstring(L"없음") :
                            (CoverageGapSummary +
                                L" / 총 " +
                                FormatInteger(CoverageGapCount) +
                                L"곳")));
                }
            }
        }

        void PopulateCitizenAssignments(
            const std::string& BuildingName,
            CPlacementAreaObject& Building,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!mWorld)
                return;

            std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

            if (!mWorld->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
                return;

            for (size_t Index = 0; Index < OrbList.size(); ++Index)
            {
                auto Orb = OrbList[Index].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                const std::string OrbName = Orb->GetName();

                if (Orb->GetHomeBuilding() == BuildingName)
                    PushUnique(OutRecord.Residents, OrbName);

                if (OutRecord.WorkProvider &&
                    Orb->GetWorkBuilding() == BuildingName)
                {
                    PushUnique(OutRecord.AssignedEmployees, OrbName);

                    if (Orb->GetCitizenState() == ECitizenState::AtWork)
                    {
                        PushUnique(
                            OutRecord.WorkingEmployees,
                            OrbName);
                    }
                }

                if (OutRecord.EntertainmentProvider &&
                    Orb->GetFunBuilding() == BuildingName)
                {
                    PushUnique(OutRecord.AssignedVisitors, OrbName);

                    if (Orb->GetCitizenState() == ECitizenState::AtFun)
                        PushUnique(OutRecord.ArrivedVisitors, OrbName);
                }

                if (!OutRecord.FoodProvider)
                    continue;

                const ECitizenState OrbState = Orb->GetCitizenState();

                if (OrbState == ECitizenState::AtFood)
                {
                    const std::string& VisitFoodBuilding =
                        Orb->GetFoodVisitBuilding();
                    const bool IsVisitBuildingMatched =
                        VisitFoodBuilding == BuildingName ||
                        (VisitFoodBuilding.empty() &&
                            Orb->GetFoodBuilding() == BuildingName);

                    if (!IsVisitBuildingMatched)
                        continue;

                    PushUnique(OutRecord.AssignedVisitors, OrbName);
                    PushUnique(OutRecord.ArrivedVisitors, OrbName);
                    continue;
                }

                if (OrbState != ECitizenState::GoingToFood ||
                    Orb->GetFoodBuilding() != BuildingName)
                {
                    continue;
                }

                FVector3 MarkerPos = FVector3::Zero;

                if (!Building.GetClosestMarkerWorldPos(
                    Orb->GetWorldPos(),
                    MarkerPos))
                {
                    continue;
                }

                FVector3 OrbPos = Orb->GetWorldPos();
                OrbPos.z = MarkerPos.z;

                const float NearDistance =
                    (std::max)(8.f, Orb->GetArrivalDistance() * 2.f);

                if (OrbPos.Distance(MarkerPos) > NearDistance)
                    continue;

                PushUnique(OutRecord.AssignedVisitors, OrbName);
                PushUnique(OutRecord.IncomingVisitors, OrbName);
            }

            std::sort(OutRecord.Residents.begin(), OutRecord.Residents.end());
            std::sort(
                OutRecord.AssignedEmployees.begin(),
                OutRecord.AssignedEmployees.end());
            std::sort(
                OutRecord.WorkingEmployees.begin(),
                OutRecord.WorkingEmployees.end());
            std::sort(
                OutRecord.AssignedVisitors.begin(),
                OutRecord.AssignedVisitors.end());
            std::sort(
                OutRecord.ArrivedVisitors.begin(),
                OutRecord.ArrivedVisitors.end());
            std::sort(
                OutRecord.IncomingVisitors.begin(),
                OutRecord.IncomingVisitors.end());
        }

    private:
        std::shared_ptr<CWorld> mWorld;
        std::shared_ptr<IMainWorldBuildMenuAccess> mMainWorldAccess;
        std::shared_ptr<IMainWorldAlmanacAccess> mMainWorldPolicyAccess;
        std::shared_ptr<IMainWorldTradeAccess> mMainWorldTradeAccess;
        std::shared_ptr<IMainWorldKnowledgeAccess> mMainWorldKnowledgeAccess;
    };
}

namespace CitizenInfoWorldQuerySource
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldCitizenInfoQuerySource>(World);
    }
}

