#include "PlacementAreaObject.h"
#include "Object/TileMapObject.h"

void CPlacementAreaObject::SetPlacementTemplateType(
    EPlacementTemplateType Type)
{
    SetPlacementTemplate(CreateTemplateByType(Type));
}

void CPlacementAreaObject::SetPlacementTemplate(
    const FPlacementTemplate& Template)
{
    mTemplate = Template;
    EnsureTemplateValidity();
    ResetPlacementState();
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

void CPlacementAreaObject::RotatePreviewCW(
    const FVector2& MouseWorldPos)
{
    mPreviewDirection = (mPreviewDirection + 1) % 4;

    if (mMovePreviewActive)
        UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::RotatePreviewCCW(
    const FVector2& MouseWorldPos)
{
    mPreviewDirection = (mPreviewDirection + 3) % 4;

    if (mMovePreviewActive)
        UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::ConfirmPlacement()
{
    EnsurePlacementObject();

    if (!mTileMapPrepared ||
        !mPreviewCanPlace ||
        mPreviewIndices.empty())
    {
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    std::vector<int> NextPrimaryIndices;

    if (!BuildDiamondAreaIndices(
            TileMap, mPreviewCenterIndex, NextPrimaryIndices) ||
        (int)NextPrimaryIndices.size() != mTemplate.GetExpectedTileCount())
    {
        return;
    }

    ApplyPlacementStateToTileMap(TileMap, mPrimaryPlacedIndices, false);
    mMarkerTileIndices.clear();
    ApplyPlacementStateToTileMap(TileMap, NextPrimaryIndices, true);
    mPrimaryPlacedIndices = NextPrimaryIndices;
    mPlacedCenterIndex = mPreviewCenterIndex;
    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mMovePreviewActive = false;

    ApplyPlacedAreaColor(TileMap);
    SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
    NotifyPlacementTopologyChanged();
}

void CPlacementAreaObject::CancelMovePreview()
{
    ClearPreview();
    mMovePreviewActive = false;
}

FPlacementTemplate CPlacementAreaObject::CreateTemplateByType(
    EPlacementTemplateType Type)
{
    FPlacementTemplate Template;
    Template.Type = Type;
    Template.AreaColor = FVector4::Blue;
    Template.HasDirectionalGap = false;

    switch (Type)
    {
    case EPlacementTemplateType::SingleTileMarker:
        Template.DiamondRadius = 0;
        Template.MarkerAnchors.push_back({ 0.f, 0.f });
        Template.HasDirectionalGap = false;
        break;

    case EPlacementTemplateType::Diamond5x5TwoMarker:
        Template.DiamondRadius = 2;
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        break;

    case EPlacementTemplateType::Diamond5x5FourMarker:
        Template.DiamondRadius = 2;
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        Template.MarkerAnchors.push_back({ 0.5f, -1.f });
        Template.MarkerAnchors.push_back({ -0.5f, 1.f });
        break;

    case EPlacementTemplateType::Diamond7x7ThreeMarker:
        Template.DiamondRadius = 3;
        Template.MarkerAnchors.push_back({ 1.5f, 1.f });
        Template.MarkerAnchors.push_back({ -1.5f, -1.f });
        Template.MarkerAnchors.push_back({ 0.f, 0.f });
        break;

    case EPlacementTemplateType::Diamond3x3SingleMarker:
    default:
        Template.DiamondRadius = 1;
        Template.MarkerAnchors.push_back({ 0.5f, 0.5f });
        Template.HasDirectionalGap = true;
        break;
    }

    return Template;
}

void CPlacementAreaObject::EnsureTemplateValidity()
{
    const int MinRadius =
        mTemplate.Type == EPlacementTemplateType::SingleTileMarker ? 0 : 1;

    if (mTemplate.DiamondRadius < MinRadius)
        mTemplate.DiamondRadius = MinRadius;

    if (mTemplate.MarkerAnchors.empty())
    {
        mTemplate.MarkerAnchors.push_back({ 0.5f, 0.5f });
    }
}

void CPlacementAreaObject::ResetPlacementState()
{
    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    mPrimaryPlacedIndices.clear();
    mAppliedPrimaryOverlayIndices.clear();
    mAppliedMarkerOverlayIndices.clear();
    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mTileMapPrepared = false;
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewDirection = 0;
    mMarkerTileIndices.clear();
    mAccessibilityScore = 0.f;
}

void CPlacementAreaObject::EnsurePlacementObject()
{
    if (mTileMapPrepared)
        return;

    EnsureTemplateValidity();

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();

    if (CountX <= 0 || CountY <= 0)
        return;

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

    if (!mAutoPlaceOnPrepare)
    {
        mPrimaryPlacedIndices.clear();
        mMarkerTileIndices.clear();
        mPreviewIndices.clear();
        mPlacedCenterIndex = -1;
        mPreviewCenterIndex = -1;
        mPreviewCanPlace = false;
        mTileMapPrepared = true;
        return;
    }

    std::vector<int> StartPrimaryIndices;
    int StartCenterIndex = -1;
    const int CenterX = Clamp<int>(
        CountX / 2 + mInitialCenterOffsetX, 0, CountX - 1);
    const int CenterY = Clamp<int>(
        CountY / 2 + mInitialCenterOffsetY, 0, CountY - 1);
    const int CenterIndex = CenterY * CountX + CenterX;

    bool Found = BuildDiamondAreaIndices(TileMap,
        CenterIndex, StartPrimaryIndices) &&
        (int)StartPrimaryIndices.size() == mTemplate.GetExpectedTileCount() &&
        IsAreaPlaceable(TileMap, StartPrimaryIndices);

    if (Found)
        StartCenterIndex = CenterIndex;

    if (!Found)
    {
        for (int y = 0; y < CountY && !Found; ++y)
        {
            for (int x = 0; x < CountX; ++x)
            {
                const int Index = y * CountX + x;

                if (!BuildDiamondAreaIndices(
                        TileMap, Index, StartPrimaryIndices) ||
                    (int)StartPrimaryIndices.size() !=
                    mTemplate.GetExpectedTileCount())
                {
                    continue;
                }

                if (IsAreaPlaceable(TileMap, StartPrimaryIndices))
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
        ApplyPlacementStateToTileMap(TileMap, StartPrimaryIndices, true);
        mPrimaryPlacedIndices = StartPrimaryIndices;
        mPlacedCenterIndex = StartCenterIndex;
        ApplyPlacedAreaColor(TileMap);
        SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
        mTileMapPrepared = true;
        NotifyPlacementTopologyChanged();
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

    if (!BuildDiamondAreaIndices(TileMap, CenterIndex, mPreviewIndices) ||
        (int)mPreviewIndices.size() != mTemplate.GetExpectedTileCount())
    {
        mPreviewIndices.clear();
        return;
    }

    mPreviewCenterIndex = CenterIndex;
    mPreviewCanPlace = IsAreaPlaceable(TileMap, mPreviewIndices);

    const FVector4 PreviewColor = mPreviewCanPlace ?
        FVector4::Green : FVector4::Red;

    SetAreaColor(TileMap, mPreviewIndices, PreviewColor);

    if (std::find(mPreviewIndices.begin(), mPreviewIndices.end(),
        mPreviewCenterIndex) != mPreviewIndices.end())
    {
        auto CenterTile = TileMap->GetTile(mPreviewCenterIndex).lock();

        if (CenterTile)
            CenterTile->SetOutLineColor(1.f, 1.f, 0.f, 1.f);
    }
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
