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
    int HousingSatisfactionCap,
    int JobSatisfactionCap,
    int FoodSatisfactionCap,
    int FunSatisfactionCap,
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
        HousingSatisfactionCap,
        JobSatisfactionCap,
        FoodSatisfactionCap,
        FunSatisfactionCap);
    mOperations.ConfigureEconomy(
        mServiceProfile,
        IsTransportOffice(),
        IsHarbor(),
        BaseMonthlyWage,
        BaseMonthlyUpkeep);
}

void CPlacementAreaObject::ApplyCatalogEntry(
    const FBuildingCatalogEntry& Entry)
{
    SetBuildingId(Entry.Id);
    SetBuildingCategory(Entry.Category);
    SetBuildingKind(Entry.BuildingKind);
    mOperations.ConfigureStorageBehavior(IsWarehouse());
    SetRequiredEducationLevel(Entry.RequiredEducationLevel);
    SetBuildingDisplayInfo(
        WideToUtf8(Entry.DisplayName),
        WideToUtf8(Entry.CategoryName),
        Entry.Residential,
        Entry.Capacity,
        Entry.FoodProvider,
        Entry.EntertainmentProvider,
        Entry.HousingSatisfactionCap,
        Entry.JobSatisfactionCap,
        Entry.FoodSatisfactionCap,
        Entry.FunSatisfactionCap);
    SetPlacementTemplateType(Entry.TemplateType);
    SetResourceBehavior(
        Entry.ProducedResourceType,
        Entry.VisitConsumptionResourceType,
        Entry.SupportsTeamsterPickup,
        Entry.CanExportStoredResources);
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
}
