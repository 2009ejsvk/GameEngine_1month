#include "PlacementAreaObject.h"
#include "../Building/BuildingCatalog.h"
#include "../World/MainWorldAccess.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/Input.h"
#include "World/World.h"
#include <Windows.h>

namespace
{
    int GTopologyBatchDepth = 0;
    bool GTopologyBatchPending = false;
    std::weak_ptr<CWorld> GTopologyBatchWorld;

    std::string WideToUtf8(const std::wstring& Text)
    {
        if (Text.empty())
            return std::string();

        const int RequiredBytes = WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0, nullptr, nullptr);

        if (RequiredBytes <= 0)
        {
            std::string Fallback;
            Fallback.reserve(Text.size());

            for (wchar_t Ch : Text)
            {
                Fallback.push_back(Ch >= 0 && Ch <= 0x7f ?
                    static_cast<char>(Ch) :
                    '?');
            }

            return Fallback;
        }

        std::string Utf8;
        Utf8.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Utf8[0], RequiredBytes, nullptr, nullptr);
        return Utf8;
    }

    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0);

        if (RequiredCount <= 0)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &WideText[0], RequiredCount);
        return WideText;
    }

}

void CPlacementAreaObject::BeginTopologyBatchUpdate()
{
    ++GTopologyBatchDepth;
}

void CPlacementAreaObject::EndTopologyBatchUpdate()
{
    if (GTopologyBatchDepth <= 0)
        return;

    --GTopologyBatchDepth;

    if (GTopologyBatchDepth > 0 || !GTopologyBatchPending)
        return;

    GTopologyBatchPending = false;
    auto World = GTopologyBatchWorld.lock();
    GTopologyBatchWorld.reset();

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
            dynamic_cast<IMainWorldRoadNetworkAccess*>(World.get()))
    {
        RoadNetworkAccess->RebuildRoadNetwork();
    }

    if (auto RefreshAccess =
            dynamic_cast<IMainWorldRuntimeRefreshAccess*>(World.get()))
    {
        RefreshAccess->RefreshRuntimeBuildingState();
    }
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
    SetBuildingKind(Entry.BuildingKind);
    mActiveOperationModeIndex = 0;
    mActiveRuntimeUpgradeIndex = -1;
    mWarehouseStoragePolicy = EWarehouseStoragePolicy::Balanced;
    mPreferredWarehouseResourceType = EResourceType::None;
    mPowerSupplyRatio = 1.f;
    mLocalPollutionExposure = 0;
    mOperations.ConfigureStorageBehavior(IsWarehouse());
    RefreshWarehouseStorageRuntime();
    SetRequiredEducationLevel(Entry.RequiredEducationLevel);
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
    SetResourceBehavior(
        Entry.ProducedResourceType,
        Entry.VisitConsumptionResourceType,
        Entry.SupportsTeamsterPickup,
        Entry.CanExportStoredResources,
        Entry.ProductionInputTypes,
        Entry.ProductionInputAmounts);
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
                ResolvePowerOperationalMultiplier(),
            ResolveEffectiveServiceCapacityDelta());
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
    mActiveOperationModeIndex =
        (ResolveActiveOperationModeIndex(Entry) + 1) % ModeCount;
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            std::dynamic_pointer_cast<IMainWorldRuntimeRefreshAccess>(World);

        if (RefreshAccess)
            RefreshAccess->RefreshRuntimeBuildingState();
    }

    std::wstring ModeLabel = GetActiveOperationModeDisplayName();

    if (ModeLabel.empty())
        ModeLabel = L"-";

    OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
        L": " +
        ModeLabel;

    const std::wstring EffectSummary = GetActiveOperationModeEffectSummary();

    if (!EffectSummary.empty())
    {
        OutMessage += L" (";
        OutMessage += EffectSummary;
        OutMessage += L")";
    }

    return true;
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
        OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
            L": " +
            GetActiveOperationModeDisplayName();
        return true;
    }

    mActiveOperationModeIndex = ModeIndex;
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            std::dynamic_pointer_cast<IMainWorldRuntimeRefreshAccess>(World);

        if (RefreshAccess)
            RefreshAccess->RefreshRuntimeBuildingState();
    }

    std::wstring ModeLabel = GetActiveOperationModeDisplayName();

    if (ModeLabel.empty())
        ModeLabel = L"-";

    OutMessage = Utf8ToWide(GetBuildingDisplayName()) +
        L": " +
        ModeLabel;

    const std::wstring EffectSummary = GetActiveOperationModeEffectSummary();

    if (!EffectSummary.empty())
    {
        OutMessage += L" (";
        OutMessage += EffectSummary;
        OutMessage += L")";
    }

    return true;
}

bool CPlacementAreaObject::CycleRuntimeUpgrade(std::wstring& OutMessage)
{
    OutMessage.clear();

    const FBuildingCatalogEntry* const Entry = ResolveCatalogEntry();

    if (!Entry || Entry->RuntimeUpgradeDefs.empty())
        return false;

    const int UpgradeCount = static_cast<int>(Entry->RuntimeUpgradeDefs.size());
    const int CurrentIndex = ResolveActiveRuntimeUpgradeIndex(Entry);
    mActiveRuntimeUpgradeIndex =
        CurrentIndex < 0 ? 0 :
        (CurrentIndex + 1 < UpgradeCount ? CurrentIndex + 1 : -1);
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            std::dynamic_pointer_cast<IMainWorldRuntimeRefreshAccess>(World);

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

    mWarehouseStoragePolicy =
        mWarehouseStoragePolicy == EWarehouseStoragePolicy::Balanced ?
            EWarehouseStoragePolicy::Dedicated :
            EWarehouseStoragePolicy::Balanced;
    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            std::dynamic_pointer_cast<IMainWorldRuntimeRefreshAccess>(World);

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

    if (mPreferredWarehouseResourceType == EResourceType::None)
    {
        mPreferredWarehouseResourceType = EResourceType::Coconuts;
    }
    else
    {
        const int NextValue =
            static_cast<int>(mPreferredWarehouseResourceType) + 1;
        mPreferredWarehouseResourceType =
            NextValue < static_cast<int>(EResourceType::Count) ?
                static_cast<EResourceType>(NextValue) :
                EResourceType::None;
    }

    RefreshWarehouseStorageRuntime();

    if (auto World = mWorld.lock())
    {
        auto RefreshAccess =
            std::dynamic_pointer_cast<IMainWorldRuntimeRefreshAccess>(World);

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
    mAccessibilityScore = 0.f;

    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());

    if (HadPlacedArea)
        NotifyPlacementTopologyChanged();

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

void CPlacementAreaObject::NotifyPlacementTopologyChanged()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (GTopologyBatchDepth > 0)
    {
        GTopologyBatchPending = true;
        GTopologyBatchWorld = World;
        return;
    }

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
            dynamic_cast<IMainWorldRoadNetworkAccess*>(World.get()))
    {
        RoadNetworkAccess->RebuildRoadNetwork();
    }

    if (auto RefreshAccess =
            dynamic_cast<IMainWorldRuntimeRefreshAccess*>(World.get()))
    {
        RefreshAccess->RefreshRuntimeBuildingState();
    }
}
