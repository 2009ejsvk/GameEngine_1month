#include "PlacementAreaObject.h"
#include "../Building/BuildingCatalog.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/Input.h"
#include "World/World.h"
#include <Windows.h>

namespace
{
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
    std::shared_ptr<CTileMapComponent> TileMap;

    if (AcquireTileMap(TileMap))
    {
        for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(mPrimaryPlacedIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::Normal);
            Tile->SetOutLineColor(FVector4::White);
        }

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

    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    CGameObject::Destroy();
}
