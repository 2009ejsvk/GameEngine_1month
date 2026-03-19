#include "PlacementAreaObject.h"
#include "PlacementBuildingRoleResolver.h"
#include "../Building/BuildingCatalog.h"
#include "../Economy/TradePolicyRuntime.h"
#include "../StringUtils.h"
#include "../World/MainWorldBuildingControlAccess.h"
#include "../World/MainWorldInfrastructureAccess.h"
#include "../World/MainWorldSystemAccess.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/Input.h"
#include "World/World.h"

namespace PlacementAreaObjectInternal
{
    using StringUtils::Utf8ToWide;
    using StringUtils::WideToUtf8;

    void FlushPlacementTopologyUpdates(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!Building || !Building->GetAlive() || !Building->GetEnable())
                continue;

            Building->RefreshAccessibilityScore();
        }

        if (auto RoadNetworkAccess =
                ResolveMainWorldRoadNetworkAccess(World.get()))
        {
            RoadNetworkAccess->RebuildRoadNetwork();
        }

        if (auto RefreshAccess =
                ResolveMainWorldRuntimeRefreshAccess(World.get()))
        {
            RefreshAccess->RefreshRuntimeBuildingState();
        }
    }
} // namespace PlacementAreaObjectInternal

namespace
{
    struct FOperationModeResearchMetadata
    {
        bool Valid = false;
        std::wstring Key;
        std::wstring Label;
        int Cost = 0;
    };

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

    int ResolveOperationModeResearchCost(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars:
            return 60;
        case EBuildingEra::ColdWar:
            return 90;
        case EBuildingEra::Modern:
            return 130;
        case EBuildingEra::Colonial:
        default:
            return 40;
        }
    }

    std::shared_ptr<IMainWorldKnowledgeAccess> ResolveKnowledgeAccess(
        const std::shared_ptr<CWorld>& World)
    {
        return ResolveMainWorldKnowledgeAccess(World);
    }

    bool TryBuildOperationModeResearchMetadata(
        const CPlacementAreaObject& Building,
        int ModeIndex,
        FOperationModeResearchMetadata& OutMetadata)
    {
        OutMetadata = FOperationModeResearchMetadata();
        const FBuildingCatalogEntry* const Entry =
            FindBuildingCatalogEntry(Building.GetBuildingId());

        if (!Entry ||
            ModeIndex < 0 ||
            ModeIndex >= static_cast<int>(Entry->OperationModeDefs.size()))
        {
            return false;
        }

        const FBuildingOperationModeDef& ModeDef =
            Entry->OperationModeDefs[static_cast<size_t>(ModeIndex)];

        if (ModeDef.RequiredResearch.empty())
            return false;

        const EBuildingEra ResearchEra =
            ModeDef.HasUnlockEra ?
                ModeDef.UnlockEra :
                Entry->UnlockEra;
        const bool GenericResearchLabel =
            ModeDef.RequiredResearch == L"연구 필요";
        OutMetadata.Valid = true;
        OutMetadata.Label =
            GenericResearchLabel ?
                (!ModeDef.DisplayName.empty() ?
                    ModeDef.DisplayName + L" 연구" :
                    std::wstring(L"운영 모드 연구")) :
                ModeDef.RequiredResearch;
        OutMetadata.Key =
            GenericResearchLabel ?
                (L"operation_mode:" +
                    StringUtils::Utf8ToWide(Building.GetBuildingId()) +
                    L":" +
                    std::to_wstring(ModeIndex)) :
                (L"research:" + ModeDef.RequiredResearch);
        OutMetadata.Cost = ResolveOperationModeResearchCost(ResearchEra);
        return true;
    }

    void BuildOperationModeSelectionMessage(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry& Entry,
        bool UnlockedResearchNow,
        const FOperationModeResearchMetadata* ResearchMetadata,
        std::wstring& OutMessage)
    {
        std::wstring ModeLabel = Building.GetActiveOperationModeDisplayName();

        if (ModeLabel.empty())
            ModeLabel = L"-";

        OutMessage =
            StringUtils::Utf8ToWide(Building.GetBuildingDisplayName()) +
            L": " +
            ModeLabel;

        const std::wstring EffectSummary =
            Building.GetActiveOperationModeEffectSummary();

        if (!EffectSummary.empty())
        {
            OutMessage += L" (";
            OutMessage += EffectSummary;
            OutMessage += L")";
        }

        if (UnlockedResearchNow && ResearchMetadata)
        {
            OutMessage += L"\n연구 해금: ";
            OutMessage += ResearchMetadata->Label;
            OutMessage += L" (-";
            OutMessage += std::to_wstring((std::max)(0, ResearchMetadata->Cost));
            OutMessage += L" 지식)";
        }

        const std::wstring TransitionNotice =
            GetOperationModeTransitionNotice(Entry);

        if (!TransitionNotice.empty())
        {
            OutMessage += L"\n";
            OutMessage += TransitionNotice;
        }
    }

    void BuildOperationModeResearchFailureMessage(
        const CPlacementAreaObject& Building,
        const FOperationModeResearchMetadata& ResearchMetadata,
        std::wstring& OutMessage)
    {
        OutMessage =
            StringUtils::Utf8ToWide(Building.GetBuildingDisplayName()) +
            L": " +
            ResearchMetadata.Label +
            L" 해금에 지식 " +
            std::to_wstring((std::max)(0, ResearchMetadata.Cost)) +
            L" 필요";
    }
}

using StringUtils::Utf8ToWide;
using StringUtils::WideToUtf8;

CPlacementAreaObject::FScopedTopologyBatch::~FScopedTopologyBatch()
{
    Flush();
}

void CPlacementAreaObject::FScopedTopologyBatch::MarkDirty(
    const std::shared_ptr<CWorld>& World)
{
    if (!World)
        return;

    auto CurrentWorld = mWorld.lock();

    if (CurrentWorld && CurrentWorld != World)
        PlacementAreaObjectInternal::FlushPlacementTopologyUpdates(CurrentWorld);

    mPending = true;
    mWorld = World;
}

void CPlacementAreaObject::FScopedTopologyBatch::Flush()
{
    if (!mPending)
        return;

    mPending = false;
    auto World = mWorld.lock();
    mWorld.reset();

    PlacementAreaObjectInternal::FlushPlacementTopologyUpdates(World);
}

CPlacementAreaObject::CPlacementAreaObject()
{
    SetClassType<CPlacementAreaObject>();
    mTemplate = CreateTemplateByType(
        EPlacementTemplateType::Diamond3x3SingleMarker);
}

CPlacementAreaObject::CPlacementAreaObject(
    const CPlacementAreaObject& ref) :
    CGameObject(ref)
{
}

CPlacementAreaObject::CPlacementAreaObject(
    CPlacementAreaObject&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CPlacementAreaObject::~CPlacementAreaObject()
{
}

void CPlacementAreaObject::SetBuildingDisplayInfo(
    const std::string& DisplayName,
    const std::string& CategoryName,
    bool Residential,
    int Capacity,
    bool FoodProvider,
    bool EntertainmentProvider,
    bool HealthProvider,
    bool FaithProvider,
    int HousingSatisfactionCap,
    int JobSatisfactionCap,
    int FoodSatisfactionCap,
    int FunSatisfactionCap,
    int HealthSatisfactionCap,
    int FaithSatisfactionCap,
    int ServiceCapacity,
    int BaseMonthlyWage,
    int BaseMonthlyUpkeep)
{
    mBuildingDisplayName = DisplayName;
    mBuildingCategoryName = CategoryName;
    mServiceProfile.ConfigureDisplay(
        Residential,
        Capacity,
        FoodProvider,
        EntertainmentProvider,
        HealthProvider,
        FaithProvider,
        HousingSatisfactionCap,
        JobSatisfactionCap,
        FoodSatisfactionCap,
        FunSatisfactionCap,
        HealthSatisfactionCap,
        FaithSatisfactionCap,
        ServiceCapacity);
    mOperations.ConfigureEconomy(
        mServiceProfile,
        IsTransportOffice(),
        IsHarbor(),
        BaseMonthlyWage,
        BaseMonthlyUpkeep);
    mOperations.ConfigureServiceBehavior(mServiceProfile);
}

void CPlacementAreaObject::ApplyCatalogEntry(
    const FBuildingCatalogEntry& Entry)
{
    SetBuildingId(Entry.Id);
    SetBuildingCategory(Entry.Category);
    SetHouseholdCapacity(Entry.HouseholdCapacity);
    SetLeisureClass(Entry.LeisureClass);
    SetPrimaryTouristPreference(Entry.PrimaryTouristPreference);
    SetPoliticalSignals(Entry.PoliticalSignals);
    SetBuildingKind(Entry.BuildingKind);
    mRuntime.ResetForCatalog(ResolvePlacementBuildingRoleState(Entry));
    mOperations.ConfigureStorageBehavior(IsWarehouse());
    RefreshWarehouseStorageRuntime();
    SetRequiredEducationLevel(Entry.RequiredEducationLevel);
    SetAllowedWealthMask(Entry.AllowedWealthMask);
    SetBuildingDisplayInfo(
        WideToUtf8(Entry.DisplayName),
        WideToUtf8(Entry.CategoryName),
        Entry.Residential,
        Entry.Capacity,
        Entry.FoodProvider,
        Entry.EntertainmentProvider,
        Entry.HealthProvider,
        Entry.FaithProvider,
        Entry.HousingSatisfactionCap,
        Entry.JobSatisfactionCap,
        Entry.FoodSatisfactionCap,
        Entry.FunSatisfactionCap,
        Entry.HealthSatisfactionCap,
        Entry.FaithSatisfactionCap,
        Entry.ServiceCapacity);
    SetPlacementTemplateType(Entry.TemplateType);
    ApplyRuntimeResourceBehavior();
}

bool CPlacementAreaObject::Init()
{
    CGameObject::Init();
    CreateComponent<CSceneComponent>("Root");
    return true;
}

void CPlacementAreaObject::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);
    EnsurePlacementObject();
    if (HasPlacedArea())
    {
        mOperations.TickServiceStock(
            DeltaTime,
            ResolveOperationModeServiceThroughputMultiplier() *
                ResolvePowerOperationalMultiplier() *
                ResolveDamageOperationalMultiplier(),
            ResolveEffectiveServiceCapacityDelta());

        if (CanGenerateWorkOutput() &&
            mOperations.ProducedResourceType != EResourceType::None)
        {
            const auto World = mWorld.lock();
            const IMainWorldCitizenPolicyAccess* MainWorld =
                ResolveMainWorldCitizenPolicyAccess(World.get());
            const FGovernmentProfile* GovernmentProfile =
                MainWorld ? &MainWorld->GetGovernmentProfile() : nullptr;
            const FGovernmentEdictModifiers* EdictModifiers =
                MainWorld ? &MainWorld->GetEdictModifiers() : nullptr;
            const FTaxPolicyEventStatus* TaxEventStatus =
                MainWorld ? &MainWorld->GetTaxPolicyEventStatus() : nullptr;
            const FWorldCrisisStatus* WorldCrisisStatus =
                MainWorld ? &MainWorld->GetWorldCrisisStatus() : nullptr;

            std::array<EResourceType, GProductionInputSlotCount> InputTypes = {};
            for (int SlotIndex = 0;
                SlotIndex < GProductionInputSlotCount;
                ++SlotIndex)
            {
                InputTypes[static_cast<size_t>(SlotIndex)] =
                    SlotIndex < GetProductionInputCount() ?
                        GetProductionInputType(SlotIndex) :
                        EResourceType::None;
            }

            const float TradePolicyProductionMultiplier =
                GovernmentProfile ?
                    TradePolicyRuntime::ComputeBuildingProductionMultiplier(
                        GetProducedResourceType(),
                        InputTypes,
                        GovernmentProfile->ExportTradePolicy,
                        GovernmentProfile->ImportTradePolicy) :
                    1.f;
            const float GovernmentProductionMultiplier =
                EdictModifiers ? EdictModifiers->ProductionMultiplier : 1.f;
            const float ProductionPerSec =
                ResolveBuildingBaseProductionUnitsPerSecond(
                    GetBuildingId(),
                    GetBuildingCategory(),
                    GetProducedResourceType()) *
                GovernmentProductionMultiplier *
                ResolveTaxEventProductionMultiplier(TaxEventStatus) *
                ResolveWorldCrisisProductionMultiplier(WorldCrisisStatus) *
                TradePolicyProductionMultiplier;

            AddProduction(ProductionPerSec, DeltaTime);
        }
    }

    if (mTileMapPrepared)
    {
        std::shared_ptr<CTileMapComponent> TileMap;

        if (AcquireTileMap(TileMap))
        {
            ApplyPlacedAreaColor(TileMap);

            if (mDemolitionHoverActive &&
                !mPrimaryPlacedIndices.empty())
            {
                SetAreaColor(
                    TileMap, mPrimaryPlacedIndices, FVector4::Red);
            }
        }
    }

    if (!mMovePreviewActive)
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    UpdatePlacementPreviewFromMouse(Input->GetMouseWorldPos());
}

bool CPlacementAreaObject::CycleOperationMode(std::wstring& OutMessage)
{
    OutMessage.clear();

    const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

    if (!Entry || Entry->OperationModeDefs.empty())
        return false;

    const int ModeCount = static_cast<int>(Entry->OperationModeDefs.size());
    const int CurrentModeIndex = ResolveActiveOperationModeIndex(Entry);
    const auto World = mWorld.lock();
    const auto KnowledgeAccess = ResolveKnowledgeAccess(World);
    bool FoundBlockedResearch = false;
    FOperationModeResearchMetadata BlockedResearchMetadata;

    for (int Offset = 1; Offset < ModeCount; ++Offset)
    {
        const int CandidateModeIndex = (CurrentModeIndex + Offset) % ModeCount;
        FOperationModeResearchMetadata ResearchMetadata;
        bool UnlockedResearchNow = false;

        if (TryBuildOperationModeResearchMetadata(
                *this,
                CandidateModeIndex,
                ResearchMetadata) &&
            !ResearchMetadata.Key.empty())
        {
            if (!KnowledgeAccess)
            {
                if (!FoundBlockedResearch)
                {
                    FoundBlockedResearch = true;
                    BlockedResearchMetadata = ResearchMetadata;
                }

                continue;
            }

            if (!KnowledgeAccess->IsResearchUnlocked(ResearchMetadata.Key))
            {
                if (!KnowledgeAccess->TryUnlockResearch(
                        ResearchMetadata.Key,
                        ResearchMetadata.Cost))
                {
                    if (!FoundBlockedResearch)
                    {
                        FoundBlockedResearch = true;
                        BlockedResearchMetadata = ResearchMetadata;
                    }

                    continue;
                }

                UnlockedResearchNow = true;
            }
        }

        mRuntime.ActiveOperationModeIndex = CandidateModeIndex;
        ApplyRuntimeResourceBehavior();
        RefreshWarehouseStorageRuntime();

        if (auto RefreshAccess =
                ResolveMainWorldRuntimeRefreshAccess(World))
        {
            RefreshAccess->RefreshRuntimeBuildingState();
        }

        BuildOperationModeSelectionMessage(
            *this,
            *Entry,
            UnlockedResearchNow,
            ResearchMetadata.Valid ? &ResearchMetadata : nullptr,
            OutMessage);
        return true;
    }

    if (ModeCount <= 1)
    {
        BuildOperationModeSelectionMessage(
            *this,
            *Entry,
            false,
            nullptr,
            OutMessage);
        return true;
    }

    if (FoundBlockedResearch)
    {
        BuildOperationModeResearchFailureMessage(
            *this,
            BlockedResearchMetadata,
            OutMessage);
    }

    return false;
}

bool CPlacementAreaObject::SetActiveOperationMode(
    int ModeIndex,
    std::wstring& OutMessage)
{
    OutMessage.clear();

    const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

    if (!Entry || Entry->OperationModeDefs.empty())
        return false;

    const int ModeCount = static_cast<int>(Entry->OperationModeDefs.size());

    if (ModeIndex < 0 || ModeIndex >= ModeCount)
        return false;

    const int SafeModeIndex = ResolveActiveOperationModeIndex(Entry);

    if (SafeModeIndex == ModeIndex)
    {
        BuildOperationModeSelectionMessage(
            *this,
            *Entry,
            false,
            nullptr,
            OutMessage);
        return true;
    }

    const auto World = mWorld.lock();
    const auto KnowledgeAccess = ResolveKnowledgeAccess(World);
    FOperationModeResearchMetadata ResearchMetadata;
    bool UnlockedResearchNow = false;

    if (TryBuildOperationModeResearchMetadata(*this, ModeIndex, ResearchMetadata) &&
        !ResearchMetadata.Key.empty())
    {
        if (!KnowledgeAccess)
        {
            BuildOperationModeResearchFailureMessage(
                *this,
                ResearchMetadata,
                OutMessage);
            return false;
        }

        if (!KnowledgeAccess->IsResearchUnlocked(ResearchMetadata.Key))
        {
            if (!KnowledgeAccess->TryUnlockResearch(
                    ResearchMetadata.Key,
                    ResearchMetadata.Cost))
            {
                BuildOperationModeResearchFailureMessage(
                    *this,
                    ResearchMetadata,
                    OutMessage);
                return false;
            }

            UnlockedResearchNow = true;
        }
    }

    mRuntime.ActiveOperationModeIndex = ModeIndex;
    ApplyRuntimeResourceBehavior();
    RefreshWarehouseStorageRuntime();

    if (auto RefreshAccess =
            ResolveMainWorldRuntimeRefreshAccess(World))
    {
        RefreshAccess->RefreshRuntimeBuildingState();
    }

    BuildOperationModeSelectionMessage(
        *this,
        *Entry,
        UnlockedResearchNow,
        ResearchMetadata.Valid ? &ResearchMetadata : nullptr,
        OutMessage);
    return true;
}

bool CPlacementAreaObject::IsOperationModeResearchLocked(int ModeIndex) const
{
    FOperationModeResearchMetadata ResearchMetadata;

    if (!TryBuildOperationModeResearchMetadata(*this, ModeIndex, ResearchMetadata) ||
        ResearchMetadata.Key.empty())
    {
        return false;
    }

    const auto KnowledgeAccess = ResolveKnowledgeAccess(mWorld.lock());
    return !KnowledgeAccess ||
        !KnowledgeAccess->IsResearchUnlocked(ResearchMetadata.Key);
}

bool CPlacementAreaObject::IsOperationModeResearchUnlocked(int ModeIndex) const
{
    return !IsOperationModeResearchLocked(ModeIndex);
}

int CPlacementAreaObject::GetOperationModeResearchCost(int ModeIndex) const
{
    FOperationModeResearchMetadata ResearchMetadata;
    return TryBuildOperationModeResearchMetadata(*this, ModeIndex, ResearchMetadata) ?
        (std::max)(0, ResearchMetadata.Cost) :
        0;
}

std::wstring CPlacementAreaObject::GetOperationModeResearchLabel(int ModeIndex) const
{
    FOperationModeResearchMetadata ResearchMetadata;
    return TryBuildOperationModeResearchMetadata(*this, ModeIndex, ResearchMetadata) ?
        ResearchMetadata.Label :
        std::wstring();
}

bool CPlacementAreaObject::CycleRuntimeUpgrade(std::wstring& OutMessage)
{
    OutMessage.clear();

    const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

    if (!Entry || Entry->RuntimeUpgradeDefs.empty())
        return false;

    const int UpgradeCount = static_cast<int>(Entry->RuntimeUpgradeDefs.size());
    const int CurrentIndex = ResolveActiveRuntimeUpgradeIndex(Entry);
    mRuntime.ActiveRuntimeUpgradeIndex =
        CurrentIndex < 0 ? 0 :
        (CurrentIndex + 1 < UpgradeCount ? CurrentIndex + 1 : -1);
    ApplyRuntimeResourceBehavior();
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            ResolveMainWorldRuntimeRefreshAccess(World);

        if (RefreshAccess)
            RefreshAccess->RefreshRuntimeBuildingState();
    }

    const std::wstring ActiveUpgrade =
        GetActiveRuntimeUpgradeDisplayName();
    OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
        L": " +
        (ActiveUpgrade.empty() ? L"업그레이드 없음" : ActiveUpgrade);

    const std::wstring EffectSummary =
        GetActiveRuntimeUpgradeEffectSummary();

    if (!EffectSummary.empty())
    {
        OutMessage += L" (";
        OutMessage += EffectSummary;
        OutMessage += L")";
    }

    return true;
}

bool CPlacementAreaObject::CycleWarehouseStoragePolicy(std::wstring& OutMessage)
{
    OutMessage.clear();

    if (!IsWarehouse())
        return false;

    mRuntime.WarehouseStoragePolicy =
        mRuntime.WarehouseStoragePolicy == EWarehouseStoragePolicy::Balanced ?
            EWarehouseStoragePolicy::Dedicated :
            EWarehouseStoragePolicy::Balanced;
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            ResolveMainWorldRuntimeRefreshAccess(World);

        if (RefreshAccess)
            RefreshAccess->RefreshRuntimeBuildingState();
    }

    OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
        L": " +
        GetWarehouseStoragePolicyDisplayName();
    return true;
}

bool CPlacementAreaObject::CycleWarehousePriority(std::wstring& OutMessage)
{
    OutMessage.clear();

    if (!IsWarehouse())
        return false;

    if (mRuntime.PreferredWarehouseResourceType == EResourceType::None)
    {
        mRuntime.PreferredWarehouseResourceType = EResourceType::Coconuts;
    }
    else
    {
        const int NextValue =
            static_cast<int>(mRuntime.PreferredWarehouseResourceType) + 1;
        mRuntime.PreferredWarehouseResourceType =
            NextValue < static_cast<int>(EResourceType::Count) ?
                static_cast<EResourceType>(NextValue) :
                EResourceType::None;
    }

    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            ResolveMainWorldRuntimeRefreshAccess(World);

        if (RefreshAccess)
            RefreshAccess->RefreshRuntimeBuildingState();
    }

    OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
        L": " +
        GetWarehousePriorityDisplayName();
    return true;
}

void CPlacementAreaObject::Destroy()
{
    Destroy(nullptr);
}

void CPlacementAreaObject::Destroy(FScopedTopologyBatch* TopologyBatch)
{
    const bool HadPlacedArea = HasPlacedArea();
    std::shared_ptr<CTileMapComponent> TileMap;

    if (AcquireTileMap(TileMap))
    {
        ApplyPlacementStateToTileMap(TileMap, mPrimaryPlacedIndices, false);

        for (size_t i = 0; i < mPreviewIndices.size(); ++i)
        {
            RestoreTileColor(TileMap, mPreviewIndices[i]);
        }
    }

    mPrimaryPlacedIndices.clear();
    mMarkerTileIndices.clear();
    mPreviewIndices.clear();
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mDemolitionHoverActive = false;
    mRuntime.AccessibilityScore = 0.f;

    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());

    if (HadPlacedArea)
        NotifyPlacementTopologyChanged(TopologyBatch);

    CGameObject::Destroy();
}

void CPlacementAreaObject::ApplyPlacementStateToTileMap(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices,
    bool Apply)
{
    if (!TileMap)
        return;

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        const int TileIndex = Indices[i];
        auto Tile = TileMap->GetTile(TileIndex).lock();

        if (!Tile)
            continue;

        if (IsRoad())
        {
            TileMap->SetRoadTile(TileIndex, Apply);
        }
        else
        {
            TileMap->SetRoadTile(TileIndex, false);
            Tile->SetTileType(
                Apply ? ETileType::UnableToMove : ETileType::Normal);
        }

        Tile->SetOutLineColor(FVector4::White);
    }
}

void CPlacementAreaObject::NotifyPlacementTopologyChanged(
    FScopedTopologyBatch* TopologyBatch)
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (TopologyBatch)
    {
        TopologyBatch->MarkDirty(World);
        return;
    }

    PlacementAreaObjectInternal::FlushPlacementTopologyUpdates(World);
}
