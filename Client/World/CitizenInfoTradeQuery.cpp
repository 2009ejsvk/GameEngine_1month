#include "CitizenInfoWorldQuerySourceInternal.h"
#include "../Economy/ResourceTradePricing.h"
#include "../GameConstants.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>

using namespace CitizenInfoWorldQuerySourceInternal;

void CWorldCitizenInfoTradeQuery::PopulateCustomsTradeSummary(
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    OutRecord.LastDailyExportIncome =
        mOwner.mMainWorldPolicyAccess ?
            mOwner.mMainWorldPolicyAccess->GetLastDailyExportIncome() :
            0;
    OutRecord.LastDailyImportExpense =
        mOwner.mMainWorldPolicyAccess ?
            mOwner.mMainWorldPolicyAccess->GetLastDailyImportExpense() :
            0;

    if (mOwner.mMainWorldTradeAccess)
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
            mOwner.mMainWorldTradeAccess->GetActiveTradeRoutes();

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
            mOwner.mMainWorldTradeAccess->GetCompletedTradeRoutes();

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

    if (!mOwner.mWorld)
        return;

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!mOwner.mWorld->FindObjectListByType<CPlacementAreaObject>(BuildingList))
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

void CWorldCitizenInfoTradeQuery::PopulateHarborTradePolicy(
    CPlacementAreaObject& HarborBuilding,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    const TradePolicy::FExportTradePolicy* ExportPolicy =
        mOwner.mMainWorldPolicyAccess ?
            &mOwner.mMainWorldPolicyAccess->GetGovernmentProfile().
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
        StringUtils::FormatIntegerWithCommas(
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
            StringUtils::FormatIntegerWithCommas(
                static_cast<long long>(AbsoluteValue));
    };

    const long long ForecastBudgetDelta =
        TradePolicyRuntime::ComputeDailyTradePolicyBudgetDelta(
            ActivePolicy,
            TradePolicy::FImportTradePolicy(),
            mOwner.mMainWorldPolicyAccess ?
                mOwner.mMainWorldPolicyAccess->GetLastDailyExportIncome() :
                0,
            mOwner.mMainWorldPolicyAccess ?
                mOwner.mMainWorldPolicyAccess->GetLastDailyImportExpense() :
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
            StringUtils::FormatIntegerWithCommas(Entry.Stock) +
            L" (단가 $" +
            StringUtils::FormatIntegerWithCommas(Entry.UnitPrice) +
            L")";
        OutRecord.HarborPriorityLines.push_back(std::move(Line));
    }
}

void CWorldCitizenInfoTradeQuery::PopulateLogisticsLines(
    const std::shared_ptr<CPlacementAreaObject>& Building,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
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
            StringUtils::FormatIntegerWithCommas(
                Building->GetAvailableResourceStock(Type)) +
            L" / 픽업 예약 " +
            StringUtils::FormatIntegerWithCommas(
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
            StringUtils::FormatIntegerWithCommas(CoveredStock) +
            L" / 부족 " +
            StringUtils::FormatIntegerWithCommas(ShortageAmount));
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
            StringUtils::FormatIntegerWithCommas(TotalFreeCapacity));
        AppendLine(
            L"창고 입고 예약: " +
            StringUtils::FormatIntegerWithCommas(TotalReservedIncoming));
        AppendLine(
            L"슬롯당 용량: " +
            StringUtils::FormatIntegerWithCommas(SlotCapacityUnits) +
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
                StringUtils::FormatIntegerWithCommas(
                    Building->GetLastDailyWarehouseStorageLoss()));
        }
    }

    if (OutRecord.Harbor)
    {
        AppendLine(
            L"선적 대기: 사용 가능 " +
            StringUtils::FormatIntegerWithCommas(
                Building->GetAvailableExportableResourceStock()) +
            L" / 선적 예약 " +
            StringUtils::FormatIntegerWithCommas(
                Building->GetReservedExportPickupAmount()));
    }

    if (Building->IsTransportOffice() && mOwner.mWorld)
    {
        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (mOwner.mWorld->FindObjectListByType<CPlacementAreaObject>(
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

            if (mOwner.mWorld->FindObjectListByType<CBuildingMarkerOrb>(
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
                StringUtils::FormatIntegerWithCommas(
                    static_cast<int>(roundf(
                        GameConstants::Orb::TeamsterCoverageRadiusTiles))) +
                L"타일 | 생산 " +
                StringUtils::FormatIntegerWithCommas(ProducerCount) +
                L" / 소비 " +
                StringUtils::FormatIntegerWithCommas(ConsumerCount) +
                L" / 창고 " +
                StringUtils::FormatIntegerWithCommas(WarehouseCount) +
                L" / 항구 " +
                StringUtils::FormatIntegerWithCommas(HarborCount));

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
                StringUtils::FormatIntegerWithCommas(CoveredPickupReserved) +
                L" / 입고 " +
                StringUtils::FormatIntegerWithCommas(CoveredIncomingReserved) +
                L" / 선적 " +
                StringUtils::FormatIntegerWithCommas(CoveredExportReserved) +
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
                StringUtils::FormatIntegerWithCommas(CoveredPickupWaiting) +
                L" / 소비 부족 " +
                StringUtils::FormatIntegerWithCommas(CoveredShortageWaiting) +
                (PickupWaitingSummary.empty() ?
                    std::wstring() :
                    (L" (" + PickupWaitingSummary + L")")));
            AppendLine(
                L"팀스터 상태: 배정 " +
                StringUtils::FormatIntegerWithCommas(AssignedTeamsters) +
                L" / 운송 중 " +
                StringUtils::FormatIntegerWithCommas(InTransitTeamsters) +
                L" / 사무소 대기 " +
                StringUtils::FormatIntegerWithCommas(WaitingTeamsters));

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
                        StringUtils::FormatIntegerWithCommas(CoverageGapCount) +
                        L"곳")));
        }
    }
}
