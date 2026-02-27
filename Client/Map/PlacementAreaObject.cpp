#include "PlacementAreaObject.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include "World/Input.h"
#include <cmath>
#include <cfloat>
#include <algorithm>

CPlacementAreaObject::CPlacementAreaObject()
{
    SetClassType<CPlacementAreaObject>();
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

bool CPlacementAreaObject::IsNavigationObstacle() const
{
    return true;
}

void CPlacementAreaObject::GetNavigationGoalTiles(
    std::vector<int>& OutIndices)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    if (mMarkerTileIndex < 0)
    {
        ApplyPlacedAreaColor(TileMap);
    }

    if (mMarkerTileIndex < 0)
        return;

    if (!IsPlacedIndex(mMarkerTileIndex))
        return;

    OutIndices.push_back(mMarkerTileIndex);
}

void CPlacementAreaObject::GetNavigationBlockedTiles(
    std::vector<int>& OutIndices)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared || mPlacedIndices.empty())
        return;

    std::vector<int> GoalTiles;
    GetNavigationGoalTiles(GoalTiles);

    for (size_t i = 0; i < mPlacedIndices.size(); ++i)
    {
        const int Index = mPlacedIndices[i];

        if (std::find(GoalTiles.begin(), GoalTiles.end(), Index) !=
            GoalTiles.end())
        {
            continue;
        }

        OutIndices.push_back(Index);
    }
}

void CPlacementAreaObject::StartMovePreview(
    const FVector2& MouseWorldPos)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    mMovePreviewActive = true;
    UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::ConfirmPlacement()
{
    EnsurePlacementObject();

    if (!mTileMapPrepared ||
        !mPreviewCanPlace ||
        mPreviewIndices.size() != 9)
    {
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    mMarkerTileIndex = -1;

    for (size_t i = 0; i < mPlacedIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(mPlacedIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::Normal);
        Tile->SetOutLineColor(FVector4::White);
    }

    for (size_t i = 0; i < mPreviewIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(mPreviewIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::UnableToMove);
    }

    mPlacedIndices = mPreviewIndices;
    mPlacedCenterIndex = mPreviewCenterIndex;
    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mMovePreviewActive = false;

    ApplyPlacedAreaColor(TileMap);
    SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
}

void CPlacementAreaObject::CancelMovePreview()
{
    ClearPreview();
    mMovePreviewActive = false;
}

bool CPlacementAreaObject::ContainsPlacedTile(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    const int TileIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (TileIndex < 0)
        return false;

    return IsPlacedIndex(TileIndex);
}

float CPlacementAreaObject::GetCenterDistanceSq(
    const FVector2& MouseWorldPos) const
{
    const FVector3 Pos = GetWorldPos();
    const float dx = Pos.x - MouseWorldPos.x;
    const float dy = Pos.y - MouseWorldPos.y;

    return dx * dx + dy * dy;
}

bool CPlacementAreaObject::GetMarkerWorldPos(FVector3& OutWorldPos)
{
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return false;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    if (mMarkerTileIndex < 0)
    {
        ApplyPlacedAreaColor(TileMap);
    }

    if (mMarkerTileIndex < 0)
        return false;

    auto MarkerTile = TileMap->GetTile(mMarkerTileIndex).lock();

    if (!MarkerTile)
        return false;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    const FVector2 MarkerCenter = MarkerTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    OutWorldPos = FVector3(
        MarkerCenter.x + TileMapWorldPos.x,
        MarkerCenter.y + TileMapWorldPos.y,
        0.f);

    return true;
}

bool CPlacementAreaObject::GetTileSize(FVector2& OutTileSize)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    OutTileSize = TileMap->GetTileSize();

    return true;
}

void CPlacementAreaObject::EnsurePlacementObject()
{
    if (mTileMapPrepared)
        return;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();

    if (CountX <= 0 || CountY <= 0)
        return;

    // 여러 배치 오브젝트가 있어도 동일 타일맵에 대해서만 1회 초기화한다.
    static CTileMapComponent* sInitializedTileMap = nullptr;

    if (sInitializedTileMap != TileMap.get())
    {
        const int TileCount = CountX * CountY;

        for (int i = 0; i < TileCount; ++i)
        {
            auto Tile = TileMap->GetTile(i).lock();

            if (!Tile)
                continue;

            if (Tile->GetType() == ETileType::UnableToMove)
                Tile->SetOutLineColor(FVector4::Blue);

            else
                Tile->SetOutLineColor(FVector4::White);
        }

        sInitializedTileMap = TileMap.get();
    }

    std::vector<int> StartIndices;
    int StartCenterIndex = -1;

    const int CenterX = Clamp<int>(
        CountX / 2 + mInitialCenterOffsetX, 0, CountX - 1);
    const int CenterY = Clamp<int>(
        CountY / 2 + mInitialCenterOffsetY, 0, CountY - 1);
    const int CenterIndex = CenterY * CountX + CenterX;

    bool Found = BuildDiamondAreaIndices(TileMap,
        CenterIndex, StartIndices) &&
        IsAreaPlaceable(TileMap, StartIndices);

    if (Found)
    {
        StartCenterIndex = CenterIndex;
    }

    if (!Found)
    {
        for (int y = 0; y < CountY && !Found; ++y)
        {
            for (int x = 0; x < CountX; ++x)
            {
                const int Index = y * CountX + x;

                if (!BuildDiamondAreaIndices(TileMap,
                    Index, StartIndices))
                {
                    continue;
                }

                if (IsAreaPlaceable(TileMap, StartIndices))
                {
                    Found = true;
                    StartCenterIndex = Index;
                    break;
                }
            }
        }
    }

    if (Found)
    {
        for (size_t i = 0; i < StartIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(StartIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::UnableToMove);
        }

        mPlacedIndices = StartIndices;
        mPlacedCenterIndex = StartCenterIndex;
        ApplyPlacedAreaColor(TileMap);
        SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
    }

    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mTileMapPrepared = true;
}

void CPlacementAreaObject::UpdatePlacementPreviewFromMouse(
    const FVector2& MouseWorldPos)
{
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    ClearPreview();

    const int CenterIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (CenterIndex < 0)
        return;

    if (!BuildDiamondAreaIndices(TileMap,
        CenterIndex, mPreviewIndices))
    {
        return;
    }

    mPreviewCenterIndex = CenterIndex;
    mPreviewCanPlace = IsAreaPlaceable(TileMap, mPreviewIndices);

    const FVector4 PreviewColor = mPreviewCanPlace ?
        FVector4::Green : FVector4::Red;

    SetAreaColor(TileMap, mPreviewIndices, PreviewColor);
}

void CPlacementAreaObject::ClearPreview()
{
    if (mPreviewIndices.empty())
    {
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
    {
        mPreviewIndices.clear();
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    for (size_t i = 0; i < mPreviewIndices.size(); ++i)
    {
        RestoreTileColor(TileMap, mPreviewIndices[i]);
    }

    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mPreviewCenterIndex = -1;
}

bool CPlacementAreaObject::AcquireTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mTileMapObject.expired())
    {
        mTileMapObject = World->FindObject<CTileMapObject>("TileMap");
    }

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    OutTileMap = TileMapObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::BuildDiamondAreaIndices(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, std::vector<int>& OutIndices) const
{
    OutIndices.clear();

    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int CenterX = CenterTile->GetIndexX();
    const int CenterY = CenterTile->GetIndexY();

    const float CenterLogicalX = CenterX +
        (CenterY % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterY * 0.5f;
    const float DiamondRadius = 1.f;

    for (int y = CenterY - 2; y <= CenterY + 2; ++y)
    {
        if (y < 0 || y >= CountY)
            continue;

        for (int x = CenterX - 2; x <= CenterX + 2; ++x)
        {
            if (x < 0 || x >= CountX)
                continue;

            const float LogicalX = x + (y % 2 == 0 ? 0.f : 0.5f);
            const float LogicalY = y * 0.5f;
            const float DistX = fabs(LogicalX - CenterLogicalX);
            const float DistY = fabs(LogicalY - CenterLogicalY);

            if (DistX + DistY > DiamondRadius)
                continue;

            OutIndices.push_back(y * CountX + x);
        }
    }

    return !OutIndices.empty();
}

bool CPlacementAreaObject::IsAreaPlaceable(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices) const
{
    if (Indices.size() != 9)
        return false;

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            return false;

        if (Tile->GetType() == ETileType::UnableToMove &&
            !IsPlacedIndex(Indices[i]))
        {
            return false;
        }
    }

    return true;
}

void CPlacementAreaObject::SetAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices, const FVector4& Color)
{
    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetOutLineColor(Color);
    }
}

bool CPlacementAreaObject::IsPlacedIndex(int Index) const
{
    for (size_t i = 0; i < mPlacedIndices.size(); ++i)
    {
        if (mPlacedIndices[i] == Index)
            return true;
    }

    return false;
}

int CPlacementAreaObject::FindLowerRightMiddleTileIndex(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, const std::vector<int>& Indices) const
{
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return -1;

    const float CenterLogicalX = CenterTile->GetIndexX() +
        (CenterTile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterTile->GetIndexY() * 0.5f;

    int BestIndex = -1;
    float BestScore = FLT_MAX;

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        const float LogicalX = Tile->GetIndexX() +
            (Tile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
        const float LogicalY = Tile->GetIndexY() * 0.5f;
        const float DiffX = LogicalX - CenterLogicalX;
        const float DiffY = LogicalY - CenterLogicalY;

        // 오른쪽 하단 방향의 내부 타일(약 +0.5, +0.5)을 찾는다.
        if (DiffX <= 0.f || DiffY <= 0.f)
            continue;

        const float Score = fabs(DiffX - 0.5f) + fabs(DiffY - 0.5f);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = Indices[i];
        }
    }

    return BestIndex;
}

void CPlacementAreaObject::ApplyPlacedAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap)
{
    SetAreaColor(TileMap, mPlacedIndices, FVector4::Blue);

    mMarkerTileIndex = -1;

    if (mPlacedCenterIndex < 0 ||
        mPlacedIndices.empty())
    {
        return;
    }

    mMarkerTileIndex = FindLowerRightMiddleTileIndex(
        TileMap, mPlacedCenterIndex, mPlacedIndices);

    if (mMarkerTileIndex < 0)
        return;

    auto MarkerTile = TileMap->GetTile(mMarkerTileIndex).lock();

    if (!MarkerTile)
        return;

    MarkerTile->SetOutLineColor(1.f, 1.f, 0.f, 1.f);
}

void CPlacementAreaObject::RestoreTileColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap, int Index)
{
    auto Tile = TileMap->GetTile(Index).lock();

    if (!Tile)
        return;

    if (Tile->GetType() == ETileType::UnableToMove)
    {
        if (Index == mMarkerTileIndex)
        {
            Tile->SetOutLineColor(1.f, 1.f, 0.f, 1.f);
        }

        else
            Tile->SetOutLineColor(FVector4::Blue);
    }

    else
        Tile->SetOutLineColor(FVector4::White);
}

void CPlacementAreaObject::SyncWorldPosFromCenter(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex)
{
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return;

    const FVector2 Center = CenterTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    SetWorldPos(Center.x + TileMapWorldPos.x,
        Center.y + TileMapWorldPos.y, GetWorldPos().z);
}

bool CPlacementAreaObject::IsPlacedEdgeTile(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex) const
{
    if (!IsPlacedIndex(TileIndex))
        return false;

    for (int i = 0; i < 8; ++i)
    {
        const int Neighbor = GetIsoNeighborIndexByDir(
            TileMap, TileIndex, i);

        if (Neighbor < 0 || !IsPlacedIndex(Neighbor))
            return true;
    }

    return false;
}

int CPlacementAreaObject::GetIsoNeighborIndexByDir(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex, int DirIndex) const
{
    if (!TileMap || DirIndex < 0 || DirIndex >= 8)
        return -1;

    auto Tile = TileMap->GetTile(TileIndex).lock();

    if (!Tile)
        return -1;

    const int x = Tile->GetIndexX();
    const int y = Tile->GetIndexY();
    const int GridX = x + ((y + (y & 1)) / 2);
    const int GridY = x - (y / 2);
    const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int NextGridX = GridX + DirX[DirIndex];
    const int NextGridY = GridY + DirY[DirIndex];
    const int NextY = NextGridX - NextGridY;

    if (NextY < 0 || NextY >= TileMap->GetTileCountY())
        return -1;

    const int NextX = NextGridY + (NextY / 2);

    if (NextX < 0 || NextX >= TileMap->GetTileCountX())
        return -1;

    return NextY * TileMap->GetTileCountX() + NextX;
}
