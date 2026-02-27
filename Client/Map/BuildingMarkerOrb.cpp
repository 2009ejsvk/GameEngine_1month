#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <functional>
#include <utility>

namespace
{
#ifdef _DEBUG
    void DebugOrbLog(const char* Format, ...)
    {
        char Text[512] = {};

        va_list Args;
        va_start(Args, Format);
        vsprintf_s(Text, Format, Args);
        va_end(Args);

        OutputDebugStringA(Text);
    }
#endif
}

CBuildingMarkerOrb::CBuildingMarkerOrb()
{
    SetClassType<CBuildingMarkerOrb>();
}

CBuildingMarkerOrb::CBuildingMarkerOrb(
    const CBuildingMarkerOrb& ref) :
    CGameObject(ref)
{
}

CBuildingMarkerOrb::CBuildingMarkerOrb(
    CBuildingMarkerOrb&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CBuildingMarkerOrb::~CBuildingMarkerOrb()
{
}

bool CBuildingMarkerOrb::Init()
{
    CGameObject::Init();

    mMeshComponent = CreateComponent<CMeshComponent>("MarkerOrbMesh");
    mMovement = CreateComponent<CObjectMovementComponent>(
        "MarkerOrbMovement");

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        Mesh->SetShader("MaterialColor2D");
        Mesh->SetMesh("FrameSphere2DColor");
        Mesh->SetRelativeScale(20.f, 20.f);
        Mesh->SetMaterialBaseColor(0, 1.f, 0.f, 0.f, 1.f);
    }

    auto Movement = mMovement.lock();

    if (Movement)
    {
        Movement->SetUpdateComponent(mMeshComponent);
        Movement->SetSpeed(mMoveSpeed);
        Movement->SetAcceptDistance(4.f);
    }

    SetWorldPos(0.f, 0.f, 10.f);

#ifdef _DEBUG
    DebugOrbLog("[Orb] Init name=%s speed=%.1f\n",
        GetName().c_str(), mMoveSpeed);
#endif

    return true;
}

void CBuildingMarkerOrb::Update(float DeltaTime)
{
    CGameObject::Update(DeltaTime);

    RefreshBuildings();
    UpdateScaleFromTileSize();

    auto World = mWorld.lock();

    if (World)
    {
        ApplySoftSeparation(DeltaTime);
    }

    auto Movement = mMovement.lock();

    if (!World || !Movement)
    {
#ifdef _DEBUG
        if (!mDebugMissingDependencyLogged)
        {
            DebugOrbLog(
                "[Orb] Missing dependency World=%d Move=%d\n",
                World ? 1 : 0,
                Movement ? 1 : 0);
            mDebugMissingDependencyLogged = true;
        }
#endif
        return;
    }

#ifdef _DEBUG
    mDebugMissingDependencyLogged = false;
#endif

    std::vector<std::pair<std::string, FVector3>> MarkerList;

    if (!CollectTargetMarkers(MarkerList))
    {
#ifdef _DEBUG
        if (!mDebugMissingMarkerLogged)
        {
            DebugOrbLog(
                "[Orb] Marker unavailable. no valid target marker\n");
            mDebugMissingMarkerLogged = true;
        }
#endif
        return;
    }

#ifdef _DEBUG
    mDebugMissingMarkerLogged = false;
#endif

    const float CurrentZ = GetWorldPos().z;

    for (size_t i = 0; i < MarkerList.size(); ++i)
    {
        MarkerList[i].second.z = CurrentZ;
    }

    Movement->SetSpeed(mMoveSpeed);
    mPathRetryAccum += DeltaTime;

    if (!mHasStartPos)
    {
        const int StartIndex = rand() % (int)MarkerList.size();
        const std::string& StartName = MarkerList[StartIndex].first;
        SetWorldPos(MarkerList[StartIndex].second);
        mCurrentTargetName = PickRandomTargetName(StartName);

        if (mCurrentTargetName.empty())
            mCurrentTargetName = StartName;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Start at %s marker=(%.1f, %.1f) target=%s\n",
            StartName.c_str(),
            MarkerList[StartIndex].second.x,
            MarkerList[StartIndex].second.y,
            mCurrentTargetName.c_str());
#endif

        mHasStartPos = true;
        return;
    }

    if (mCurrentTargetName.empty())
    {
        mCurrentTargetName = PickRandomTargetName(std::string());

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    FVector3 TargetMarker = FVector3::Zero;
    bool TargetFound = false;

    for (size_t i = 0; i < MarkerList.size(); ++i)
    {
        if (MarkerList[i].first == mCurrentTargetName)
        {
            TargetMarker = MarkerList[i].second;
            TargetFound = true;
            break;
        }
    }

    if (!TargetFound)
    {
        mCurrentTargetName = PickRandomTargetName(std::string());

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    FVector3 Current = GetWorldPos();
    Current.z = CurrentZ;

    const float Dist = Current.Distance(TargetMarker);
    bool ArrivedByTile = false;

    if (Dist > mArrivalDistance)
    {
        auto TileMapObj = mTileMapObject.lock();

        if (TileMapObj)
        {
            auto TileMap = TileMapObj->GetTileMap().lock();

            if (TileMap)
            {
                const int CurrentTileIndex = TileMap->GetTileIndex(Current);
                const int TargetTileIndex = TileMap->GetTileIndex(TargetMarker);

                ArrivedByTile =
                    CurrentTileIndex >= 0 &&
                    CurrentTileIndex == TargetTileIndex;
            }
        }
    }

    if (Dist <= mArrivalDistance || ArrivedByTile)
    {
        mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

        if (mCurrentTargetName.empty())
            return;

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Arrived. switch target=%s dist=%.2f arrival=%.2f byTile=%d\n",
            mCurrentTargetName.c_str(),
            Dist, mArrivalDistance,
            ArrivedByTile ? 1 : 0);
#endif

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    if (mPathRetryAccum >= mPathRetryInterval &&
        Movement->GetVelocity().IsZero())
    {
        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
    }

#ifdef _DEBUG
    mDebugStatusLogAccum += DeltaTime;

    if (mDebugStatusLogAccum >= 1.f)
    {
        mDebugStatusLogAccum = 0.f;

        const FVector3 Pos = GetWorldPos();
        const std::string& PathTarget = Movement->GetPathTargetObjectName();

        DebugOrbLog(
            "[Orb] Pos=(%.1f, %.1f) target=%s pathTarget=%s dist=%.1f markerCount=%d\n",
            Pos.x, Pos.y,
            mCurrentTargetName.c_str(),
            PathTarget.empty() ? "<none>" : PathTarget.c_str(),
            Dist,
            (int)MarkerList.size());
    }
#endif
}

void CBuildingMarkerOrb::ApplySoftSeparation(float DeltaTime)
{
    auto World = mWorld.lock();

    if (!World || DeltaTime <= 0.f)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    if (OrbList.size() <= 1)
        return;

    const float SafeSelfDiameter = mOrbDiameter > 0.f ?
        mOrbDiameter : 1.f;
    const float SelfRadius = SafeSelfDiameter * 0.5f;

    // 30% 겹침 허용 -> 최소 분리 거리 비율은 70%
    const float MinDistanceScale = Clamp<float>(
        1.f - mAllowedOverlapRatio, 0.1f, 1.f);
    const float Epsilon = 0.0001f;

    const FVector3 SelfPos = GetWorldPos();
    FVector3 AccumulatedPush = FVector3::Zero;
    int OverlapCount = 0;

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Other = OrbList[i].lock();

        if (!Other || Other.get() == this ||
            !Other->GetAlive() || !Other->GetEnable())
        {
            continue;
        }

        FVector3 Delta = SelfPos - Other->GetWorldPos();
        Delta.z = 0.f;

        float Dist = Delta.Length();
        const float SafeOtherDiameter = Other->mOrbDiameter > 0.f ?
            Other->mOrbDiameter : 1.f;
        const float OtherRadius = SafeOtherDiameter * 0.5f;
        const float MinDist = (SelfRadius + OtherRadius) *
            MinDistanceScale;

        if (Dist >= MinDist)
            continue;

        FVector3 PushDir = FVector3::Zero;

        if (Dist > Epsilon)
        {
            PushDir = Delta / Dist;
        }

        else
        {
            const size_t HashA =
                std::hash<std::string>{}(GetName());
            const size_t HashB =
                std::hash<std::string>{}(Other->GetName());
            const float Angle =
                (float)((HashA ^ (HashB << 1)) % 6283) * 0.001f;
            PushDir.x = cosf(Angle);
            PushDir.y = sinf(Angle);
            PushDir.z = 0.f;
        }

        const float Penetration = MinDist - Dist;
        AccumulatedPush += PushDir * Penetration;
        ++OverlapCount;
    }

    if (OverlapCount <= 0 || AccumulatedPush.IsZero())
        return;

    FVector3 PushDelta = AccumulatedPush *
        (mSeparationStrength * DeltaTime / (float)OverlapCount);
    PushDelta.z = 0.f;

    const float MaxPushDist = mSeparationMaxSpeed * DeltaTime;
    const float PushLen = PushDelta.Length();

    if (PushLen > MaxPushDist && PushLen > Epsilon)
    {
        PushDelta *= (MaxPushDist / PushLen);
    }

    AddWorldPos(PushDelta);
}

void CBuildingMarkerOrb::RefreshBuildings()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (mTileMapObject.expired())
    {
        mTileMapObject = World->FindObject<CTileMapObject>("TileMap");
    }
}

void CBuildingMarkerOrb::UpdateScaleFromTileSize()
{
    if (mScaleInitialized)
        return;

    auto World = mWorld.lock();
    FVector2 TileSize;
    bool SizeFound = false;

    if (!World)
        return;

    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        auto Building = World->FindObject<CPlacementAreaObject>(
            UniqueNames[i]).lock();

        if (!Building)
            continue;

        if (Building->GetTileSize(TileSize))
        {
            SizeFound = true;
            break;
        }
    }

    if (!SizeFound)
        return;

    float Diameter = (std::min)(TileSize.x, TileSize.y) * 0.25f;

    if (Diameter < 1.f)
        Diameter = 1.f;

    mOrbDiameter = Diameter;

    mArrivalDistance = Diameter * 0.75f;

    if (mArrivalDistance < 4.f)
        mArrivalDistance = 4.f;

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        Mesh->SetRelativeScale(Diameter, Diameter);
    }

    mScaleInitialized = true;
}

bool CBuildingMarkerOrb::CollectTargetMarkers(
    std::vector<std::pair<std::string, FVector3>>& OutMarkers)
{
    OutMarkers.clear();

    auto World = mWorld.lock();

    if (!World)
        return false;

    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        auto Building = World->FindObject<CPlacementAreaObject>(
            UniqueNames[i]).lock();

        if (!Building)
            continue;

        FVector3 MarkerPos;

        if (!Building->GetClosestMarkerWorldPos(
            GetWorldPos(), MarkerPos))
            continue;

        OutMarkers.emplace_back(UniqueNames[i], MarkerPos);
    }

    return !OutMarkers.empty();
}

std::string CBuildingMarkerOrb::PickRandomTargetName(
    const std::string& ExcludeName) const
{
    std::vector<std::string> TargetNames;

    if (!mRandomTargetNames.empty())
    {
        TargetNames = mRandomTargetNames;
    }

    else
    {
        if (!mBuildingAName.empty())
            TargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            TargetNames.push_back(mBuildingBName);
        }
    }

    std::vector<std::string> UniqueNames;

    for (size_t i = 0; i < TargetNames.size(); ++i)
    {
        const std::string& Name = TargetNames[i];

        if (Name.empty())
            continue;

        if (std::find(UniqueNames.begin(), UniqueNames.end(), Name) ==
            UniqueNames.end())
        {
            UniqueNames.push_back(Name);
        }
    }

    if (UniqueNames.empty())
        return std::string();

    std::vector<std::string> CandidateNames;

    for (size_t i = 0; i < UniqueNames.size(); ++i)
    {
        if (UniqueNames[i] != ExcludeName)
            CandidateNames.push_back(UniqueNames[i]);
    }

    if (CandidateNames.empty())
        CandidateNames = UniqueNames;

    const int PickIndex = rand() % (int)CandidateNames.size();

    return CandidateNames[PickIndex];
}

void CBuildingMarkerOrb::RequestMoveTo(
    const std::string& TargetBuildingName)
{
    auto Movement = mMovement.lock();
    auto World = mWorld.lock();

    if (!Movement || !World || TargetBuildingName.empty())
        return;

#ifdef _DEBUG
    DebugOrbLog("[Orb] RequestMoveTo target=%s\n",
        TargetBuildingName.c_str());
#endif

    auto TargetBuilding =
        World->FindObject<CPlacementAreaObject>(TargetBuildingName).lock();

    if (TargetBuilding)
    {
        FVector3 MarkerPos;

        if (TargetBuilding->GetClosestMarkerWorldPos(
            GetWorldPos(), MarkerPos))
        {
            Movement->MovePath(MarkerPos);
            return;
        }
    }

    // Fallback: marker 좌표 확보에 실패했을 때만 기존 오브젝트 타겟 경로를 사용한다.
    Movement->MovePathToObject(TargetBuildingName);
}
