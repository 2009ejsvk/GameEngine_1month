#include "CitizenInfoQueryService.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/ResourceTradePricing.h"
#include "../Economy/TradePolicyRuntime.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);

        if (RequiredCount <= 1)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount - 1);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(),
            static_cast<int>(Text.size()),
            &WideText[0], RequiredCount - 1);
        return WideText;
    }

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

    std::wstring BuildAutoImportSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        switch (Policy.Mode)
        {
        case TradePolicy::EImportPolicyMode::None:
            return L"없음";
        case TradePolicy::EImportPolicyMode::SingleResource:
            if (Policy.SelectedResourceType != EResourceType::None)
                return GetResourceTypeDisplayName(Policy.SelectedResourceType);
            return L"없음";
        case TradePolicy::EImportPolicyMode::AllResources:
        default:
            return L"전체";
        }
    }

    std::wstring BuildImportCapSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        switch (TradePolicy::GetImportMaxUnitsPerResource(Policy))
        {
        case 500:
            return L"낮음 (500)";
        case 1500:
            return L"표준 (1,500)";
        case 3000:
            return L"확대 (3,000)";
        case 6000:
            return L"최대 (6,000)";
        default:
            return L"사용자 지정 (" +
                FormatInteger(
                    TradePolicy::GetImportMaxUnitsPerResource(Policy)) +
                L")";
        }
    }

    std::wstring BuildImportBudgetSelectionText(
        const TradePolicy::FImportTradePolicy& Policy)
    {
        const int BudgetCap = TradePolicy::GetDailyImportBudgetCap(Policy);

        if (BudgetCap <= 0)
            return L"무제한";

        const std::wstring AmountText =
            L"$" + FormatInteger(BudgetCap);

        if (TradePolicy::AllowsEmergencyImports(Policy))
            return L"긴급 대응 (" + AmountText + L")";

        switch (BudgetCap)
        {
        case 12000:
            return L"절약 (" + AmountText + L")";
        case 24000:
            return L"표준 (" + AmountText + L")";
        case 36000:
            return L"대량 (" + AmountText + L")";
        default:
            return L"사용자 지정 (" + AmountText + L")";
        }
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

    std::wstring BuildDomesticReserveSelectionText(
        const TradePolicy::FExportTradePolicy& Policy)
    {
        switch (TradePolicy::GetDomesticReserveBufferUnits(Policy))
        {
        case 0:
            return L"부족분만";
        case 1000:
            return L"표준 (+1,000)";
        case 3000:
            return L"강화 (+3,000)";
        case 6000:
            return L"최우선 (+6,000)";
        default:
            return L"사용자 지정 (+" +
                FormatInteger(
                    TradePolicy::GetDomesticReserveBufferUnits(Policy)) +
                L")";
        }
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

    class CWorldCitizenInfoQuerySource final :
        public CitizenInfoDataProvider::ICitizenInfoQuerySource
    {
    public:
        explicit CWorldCitizenInfoQuerySource(
            const std::shared_ptr<CWorld>& World)
            : mWorld(World)
            , mMainWorldAccess(
                std::dynamic_pointer_cast<IMainWorldBuildMenuAccess>(World))
            , mMainWorldPolicyAccess(
                std::dynamic_pointer_cast<IMainWorldAlmanacAccess>(World))
            , mMainWorldTradeAccess(
                std::dynamic_pointer_cast<IMainWorldTradeAccess>(World))
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
            OutRecord.ProducedResourceType =
                Building->GetProducedResourceType();
            OutRecord.ProducedResourceStock =
                OutRecord.ProducedResourceType == EResourceType::None ?
                    0 :
                    Building->GetResourceStock(OutRecord.ProducedResourceType);
            OutRecord.ProducedPowerMW = Building->GetProducedPowerMW();
            OutRecord.RequiredPowerMW = Building->GetRequiredPowerMW();
            OutRecord.PowerSupplyRatio = Building->GetPowerSupplyRatio();
            OutRecord.HarborShipProgressPercent =
                Building->GetHarborShipProgressPercent();
            OutRecord.ActiveOperationModeIndex =
                Building->GetActiveOperationModeIndex();
            OutRecord.ActiveRuntimeUpgradeIndex =
                Building->GetActiveRuntimeUpgradeIndex();
            OutRecord.ActiveOperationModeText =
                Building->GetActiveOperationModeDisplayName();
            OutRecord.ActiveOperationModeEffectSummary =
                Building->GetActiveOperationModeEffectSummary();
            OutRecord.ActiveRuntimeUpgradeText =
                Building->GetActiveRuntimeUpgradeDisplayName();
            OutRecord.ActiveRuntimeUpgradeEffectSummary =
                Building->GetActiveRuntimeUpgradeEffectSummary();
            OutRecord.RequiredEducationLevel =
                Building->GetRequiredEducationLevel();
            OutRecord.UsesResourceStock =
                OutRecord.ResourceStock > 0 ||
                OutRecord.CanGenerateWorkOutput ||
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
                PopulateHarborTradePolicy(*Building, OutRecord);

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
            const TradePolicy::FImportTradePolicy* ImportPolicy =
                mMainWorldPolicyAccess ?
                    &mMainWorldPolicyAccess->GetGovernmentProfile().
                        ImportTradePolicy :
                    nullptr;
            const TradePolicy::FExportTradePolicy DefaultPolicy;
            const TradePolicy::FImportTradePolicy DefaultImportPolicy;
            const TradePolicy::FExportTradePolicy& ActivePolicy =
                ExportPolicy ? *ExportPolicy : DefaultPolicy;
            const TradePolicy::FImportTradePolicy& ActiveImportPolicy =
                ImportPolicy ? *ImportPolicy : DefaultImportPolicy;
            OutRecord.HarborDomesticReserveSelectionText =
                BuildDomesticReserveSelectionText(ActivePolicy);
            OutRecord.HarborExportSelectionText =
                BuildExportBlockedSelectionText(ActivePolicy);
            OutRecord.HarborImportCapSelectionText =
                BuildImportCapSelectionText(ActiveImportPolicy);
            OutRecord.HarborImportBudgetSelectionText =
                BuildImportBudgetSelectionText(ActiveImportPolicy);
            OutRecord.HarborImportSelectionText =
                BuildAutoImportSelectionText(ActiveImportPolicy);

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
                L"내수 비축 기준: " +
                OutRecord.HarborDomesticReserveSelectionText +
                L" / 부족분 + " +
                FormatInteger(
                    TradePolicy::GetDomesticReserveBufferUnits(
                        ActivePolicy)));

            OutRecord.HarborPolicyLines.push_back(
                L"수출 금지: " + OutRecord.HarborExportSelectionText);
            OutRecord.HarborPolicyLines.push_back(
                L"자동 수입 대상: " +
                OutRecord.HarborImportSelectionText);
            OutRecord.HarborPolicyLines.push_back(
                L"자원별 수입 한도: " +
                OutRecord.HarborImportCapSelectionText);
            OutRecord.HarborPolicyLines.push_back(
                L"일일 수입 예산: " +
                OutRecord.HarborImportBudgetSelectionText);
            OutRecord.HarborPolicyLines.push_back(
                std::wstring(L"긴급 수입: ") +
                (TradePolicy::AllowsEmergencyImports(ActiveImportPolicy) ?
                    L"허용 (1.5배 비용)" :
                    L"일반 수입만"));

            std::wstring ProductionFocusLine =
                ActivePolicy.PrioritizeHighValueCargo ?
                    L"생산 유도: 제조·사치재 수출 우대" :
                    L"생산 유도: 식품·원자재 대량 수출 우대";

            if (TradePolicy::GetDomesticReserveBufferUnits(ActivePolicy) >=
                3000)
            {
                ProductionFocusLine += L" / 내수 우선";
            }

            switch (ActiveImportPolicy.Mode)
            {
            case TradePolicy::EImportPolicyMode::None:
                ProductionFocusLine += L" / 국내 공급망 중심";
                break;
            case TradePolicy::EImportPolicyMode::SingleResource:
                ProductionFocusLine += L" / ";
                ProductionFocusLine +=
                    ActiveImportPolicy.SelectedResourceType !=
                        EResourceType::None ?
                        GetResourceTypeDisplayName(
                            ActiveImportPolicy.SelectedResourceType) :
                        L"선택 자원";
                ProductionFocusLine += L" 투입 산업 우대";
                break;
            case TradePolicy::EImportPolicyMode::AllResources:
            default:
                if (TradePolicy::AllowsEmergencyImports(ActiveImportPolicy))
                    ProductionFocusLine += L" / 수입 의존 산업 확장";
                else
                    ProductionFocusLine += L" / 균형 조달";
                break;
            }

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
                    ActiveImportPolicy,
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

                const int CoveredStock =
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
    };
}

namespace CitizenInfoQueryService
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldCitizenInfoQuerySource>(World);
    }
}
