#pragma once

#include "CitizenInfoWorldQuerySource.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/TradePolicyRuntime.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace CitizenInfoWorldQuerySourceInternal
{
    std::wstring BuildExportBlockedSelectionText(
        const TradePolicy::FExportTradePolicy& Policy);
    void PushUnique(
        std::vector<std::string>& Names,
        const std::string& Name);
    bool IsOperationalBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building);
    bool IsWithinTeamsterCoverage(
        const std::shared_ptr<CPlacementAreaObject>& Office,
        const std::shared_ptr<CPlacementAreaObject>& Building);
    bool BuildingConsumesResource(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EResourceType Type);
    std::wstring BuildResourceAmountSummary(
        const std::vector<std::pair<EResourceType, int>>& Entries,
        size_t MaxCount);
    std::wstring BuildBuildingMetricSummary(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        size_t MaxCount);
    std::wstring BuildBuildingCoverageLabel(
        const std::shared_ptr<CPlacementAreaObject>& Building);
    bool IsCoveredByAnyTransportOffice(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        const std::vector<std::shared_ptr<CPlacementAreaObject>>& Offices);
    bool IsTeamsterTransitState(ECitizenState State);
    float ResolveTaxEventProductionMultiplier(
        const FTaxPolicyEventStatus* TaxEventStatus);
    float ResolveWorldCrisisProductionMultiplier(
        const FWorldCrisisStatus* WorldCrisisStatus);
    float ResolveBaseProductionUnitsPerSecond(
        const CPlacementAreaObject& Building);
    bool HasCatalogProductionIdentity(
        const FBuildingCatalogEntry* CatalogEntry);
    bool IsValidProductionInputSlot(
        const CitizenInfoDataProvider::FProductionInputSlotView& InputSlot);
    bool HasProductionInputSlots(
        const std::array<CitizenInfoDataProvider::FProductionInputSlotView,
            GProductionInputSlotCount>& InputSlots);
    void PopulateProductionResourceView(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry* CatalogEntry,
        CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord);
    FBuildingOperationModeEffect ResolveCatalogRuntimeEffect(
        const FBuildingCatalogEntry* CatalogEntry,
        int ActiveOperationModeIndex,
        int ActiveRuntimeUpgradeIndex);
    float ResolveUiPowerOperationalMultiplier(
        const CPlacementAreaObject& Building);

    class CWorldCitizenInfoQuerySource;

    class CWorldCitizenInfoBuildingQuery final
    {
    public:
        explicit CWorldCitizenInfoBuildingQuery(
            const CWorldCitizenInfoQuerySource& Owner);

        bool TryGetBuildingRecord(
            const std::string& BuildingName,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;
        std::shared_ptr<CPlacementAreaObject> FindValidBuilding(
            const std::string& BuildingName) const;
        int CountActiveCitizenOrbs() const;
        void PopulateProductionFlowMetrics(
            const std::shared_ptr<CPlacementAreaObject>& Building,
            const FBuildingCatalogEntry* CatalogEntry,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;
        void PopulatePowerTotals(
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;

    private:
        const CWorldCitizenInfoQuerySource& mOwner;
    };

    class CWorldCitizenInfoCitizenQuery final
    {
    public:
        explicit CWorldCitizenInfoCitizenQuery(
            const CWorldCitizenInfoQuerySource& Owner);

        bool TryGetCitizenRecord(
            const std::string& CitizenName,
            CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord)
            const;
        std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const;
        std::shared_ptr<CPlacementAreaObject> FindValidBuilding(
            const std::string& BuildingName) const;
        std::shared_ptr<CBuildingMarkerOrb> FindValidCitizen(
            const std::string& CitizenName) const;
        void PopulateCitizenAssignments(
            const std::string& BuildingName,
            CPlacementAreaObject& Building,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;

    private:
        const CWorldCitizenInfoQuerySource& mOwner;
    };

    class CWorldCitizenInfoTradeQuery final
    {
    public:
        explicit CWorldCitizenInfoTradeQuery(
            const CWorldCitizenInfoQuerySource& Owner);

        void PopulateCustomsTradeSummary(
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;
        void PopulateHarborTradePolicy(
            CPlacementAreaObject& HarborBuilding,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;
        void PopulateLogisticsLines(
            const std::shared_ptr<CPlacementAreaObject>& Building,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const;

    private:
        const CWorldCitizenInfoQuerySource& mOwner;
    };

    class CWorldCitizenInfoQuerySource final :
        public CitizenInfoDataProvider::ICitizenInfoQuerySource
    {
    public:
        explicit CWorldCitizenInfoQuerySource(
            const std::shared_ptr<CWorld>& World);

        bool TryGetBuildingRecord(
            const std::string& BuildingName,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const override;
        bool TryGetCitizenRecord(
            const std::string& CitizenName,
            CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord)
            const override;
        std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const override;

    private:
        friend class CWorldCitizenInfoBuildingQuery;
        friend class CWorldCitizenInfoCitizenQuery;
        friend class CWorldCitizenInfoTradeQuery;

    private:
        std::shared_ptr<CWorld> mWorld;
        std::shared_ptr<IMainWorldBuildMenuAccess> mMainWorldAccess;
        std::shared_ptr<IMainWorldAlmanacAccess> mMainWorldPolicyAccess;
        std::shared_ptr<IMainWorldTradeAccess> mMainWorldTradeAccess;
        std::shared_ptr<IMainWorldKnowledgeAccess> mMainWorldKnowledgeAccess;
        CWorldCitizenInfoBuildingQuery mBuildingQuery;
        CWorldCitizenInfoCitizenQuery mCitizenQuery;
        CWorldCitizenInfoTradeQuery mTradeQuery;
    };
}
