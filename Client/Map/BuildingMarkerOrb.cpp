#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Object/TileMapObject.h"
#include "Render/RenderManager.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <functional>
#include <unordered_map>
#include <unordered_set>
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

    struct FOrbSeparationEntry
    {
        CBuildingMarkerOrb* Orb = nullptr;
        FVector3 Pos = FVector3::Zero;
        float Radius = 0.f;
        size_t NameHash = 0;
        int CellX = 0;
        int CellY = 0;
    };

    struct FOrbSpatialHashCache
    {
        CWorld* World = nullptr;
        float CellSize = 32.f;
        int ActiveCount = 0;
        std::vector<FOrbSeparationEntry> Entries;
        std::unordered_map<long long, std::vector<int>> Cells;
        std::unordered_map<const CBuildingMarkerOrb*, int> IndexByOrb;
        std::unordered_set<const CBuildingMarkerOrb*> SeenOrbs;
    };

    long long MakeCellKey(int CellX, int CellY)
    {
        return (static_cast<long long>(CellX) << 32) ^
            static_cast<unsigned int>(CellY);
    }

    int ToCellCoord(float Value, float CellSize)
    {
        return static_cast<int>(floorf(Value / CellSize));
    }

    void RebuildOrbSpatialHash(
        CWorld* World,
        const std::vector<std::weak_ptr<CBuildingMarkerOrb>>& OrbList,
        FOrbSpatialHashCache& Cache)
    {
        Cache.World = World;
        Cache.Entries.clear();
        Cache.Cells.clear();
        Cache.IndexByOrb.clear();
        Cache.SeenOrbs.clear();
        Cache.ActiveCount = 0;

        float MaxDiameter = 1.f;

        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            FOrbSeparationEntry Entry;
            Entry.Orb = Orb.get();
            Entry.Pos = Orb->GetWorldPos();
            Entry.Pos.z = 0.f;
            const float SafeDiameter = Orb->GetOrbDiameter() > 0.f ?
                Orb->GetOrbDiameter() : 1.f;
            Entry.Radius = SafeDiameter * 0.5f;
            Entry.NameHash = std::hash<std::string>{}(Orb->GetName());

            const int Index = static_cast<int>(Cache.Entries.size());
            Cache.Entries.push_back(Entry);
            Cache.IndexByOrb.emplace(Entry.Orb, Index);

            MaxDiameter = (std::max)(MaxDiameter, SafeDiameter);
        }

        Cache.ActiveCount = static_cast<int>(Cache.Entries.size());

        if (Cache.ActiveCount <= 0)
            return;

        Cache.CellSize = (std::max)(1.f, MaxDiameter);

        for (int i = 0; i < Cache.ActiveCount; ++i)
        {
            auto& Entry = Cache.Entries[i];
            Entry.CellX = ToCellCoord(Entry.Pos.x, Cache.CellSize);
            Entry.CellY = ToCellCoord(Entry.Pos.y, Cache.CellSize);
            Cache.Cells[MakeCellKey(Entry.CellX, Entry.CellY)].
                push_back(i);
        }
    }
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
        auto RenderMgr = CRenderManager::GetInst();
        int MarkerOrbLayer = RenderMgr->GetLayerOrder("MarkerOrb");

        if (MarkerOrbLayer < 0)
        {
            for (int Order = 5; Order <= 100; ++Order)
            {
                RenderMgr->CreateLayer("MarkerOrb", Order, ERenderListSort::Y);
                MarkerOrbLayer = RenderMgr->GetLayerOrder("MarkerOrb");

                if (MarkerOrbLayer >= 0)
                    break;
            }
        }

        Mesh->SetShader("MaterialColor2D");
        Mesh->SetMesh("FrameSphere2DColor");
        Mesh->SetBlendState(0, "AlphaBlend");
        Mesh->SetRelativeScale(20.f, 20.f);
        Mesh->SetMaterialBaseColor(0, 1.f, 0.f, 0.f, 1.f);
        Mesh->SetEnable(true);
        Mesh->SetMaterialOpacity(0, 1.f);

        if (MarkerOrbLayer >= 0)
            Mesh->SetRenderLayer("MarkerOrb");
    }

    auto Movement = mMovement.lock();

    if (Movement)
    {
        Movement->SetUpdateComponent(mMeshComponent);
        Movement->SetSpeed(mMoveSpeed);
        Movement->SetAcceptDistance(4.f);
    }

    SetWorldPos(0.f, 0.f, 10.f);
    mLastProgressPos = GetWorldPos();

    // 개별 orb마다 retry 위상을 조금씩 다르게 해 요청 버스트를 완화한다.
    mPathRetryInterval = 0.9f + ((float)(rand() % 61) / 100.f);

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

    const float CurrentZ = GetWorldPos().z;

    Movement->SetSpeed(mMoveSpeed);
    mPathRetryAccum += DeltaTime;

    // 즉시 경로 취소 시에는 request id도 함께 갱신해야
    // 큐에 남아 있던 구 FindPath 완료 패킷이 적용되지 않는다.
    auto CancelCurrentPath = [&]()
    {
        Movement->AdvancePathRequestId();
        Movement->StartPathPoint();
        Movement->SetPathTargetObjectName("");
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
    };

    if (!mHasStartPos)
    {
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

        for (size_t i = 0; i < MarkerList.size(); ++i)
        {
            MarkerList[i].second.z = CurrentZ;
        }

        const int StartIndex = rand() % (int)MarkerList.size();
        const std::string& StartName = MarkerList[StartIndex].first;
        SetWorldPos(MarkerList[StartIndex].second);
        mCurrentTargetName = PickRandomTargetName(StartName);

        if (mCurrentTargetName.empty())
            mCurrentTargetName = StartName;

        // 즉시 경로를 요청하지 않는다.
        // mPathRetryAccum을 음수로 설정해 orb마다 다른 시점에 첫 요청이 발생하게 한다.
        // retry 로직(+mPathRetryInterval)이 더해지므로 실제 출발은 약 1초 내외로 분산된다.
        mPathRetryAccum = -((float)(rand() % 20) / 100.f);

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Start at %s marker=(%.1f, %.1f) target=%s delay=%.2f\n",
            StartName.c_str(),
            MarkerList[StartIndex].second.x,
            MarkerList[StartIndex].second.y,
            mCurrentTargetName.c_str(),
            -mPathRetryAccum);
#endif

        mHasStartPos = true;
        return;
    }

    if (mCurrentTargetName.empty())
    {
        CancelCurrentPath();
        mHasLockedTarget = false;
        mCurrentTargetName = PickRandomTargetName(std::string());

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    // TargetBuilding 포인터를 한 번 취득해 이동 감지 / 도착 판정 양쪽에 재사용한다.
    auto TargetBuilding =
        World->FindObject<CPlacementAreaObject>(mCurrentTargetName).lock();

    if (!TargetBuilding)
    {
        CancelCurrentPath();
        mHasLockedTarget = false;
        mCurrentTargetName = PickRandomTargetName(std::string());

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

    FVector3 TargetMarker = FVector3::Zero;

    if (!TargetBuilding->GetClosestMarkerWorldPos(
        GetWorldPos(), TargetMarker))
    {
#ifdef _DEBUG
        if (!mDebugMissingMarkerLogged)
        {
            DebugOrbLog(
                "[Orb] Marker unavailable. target=%s\n",
                mCurrentTargetName.c_str());
            mDebugMissingMarkerLogged = true;
        }
#endif
        CancelCurrentPath();
        mHasLockedTarget = false;
        mCurrentTargetName = PickRandomTargetName(std::string());

        if (mCurrentTargetName.empty())
            return;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        return;
    }

#ifdef _DEBUG
    mDebugMissingMarkerLogged = false;
#endif

    TargetMarker.z = CurrentZ;

    // 건물 이동 감지: mLockedTargetPos와 건물의 현재 closest 마커가 멀어졌으면
    // 경로를 즉시 취소하고 retry를 분산 스케줄해 nav 큐 폭주를 방지한다.
    if (mHasLockedTarget)
    {
        if (TargetBuilding)
        {
            FVector3 ClosestToLocked;

            if (TargetBuilding->GetClosestMarkerWorldPos(
                mLockedTargetPos, ClosestToLocked))
            {
                ClosestToLocked.z = mLockedTargetPos.z;

                if (mLockedTargetPos.Distance(ClosestToLocked) > 1.f)
                {
                    mHasLockedTarget = false;
                    // 기존 경로(구 건물 위치로 향하는)를 즉시 취소한다.
                    CancelCurrentPath();
                    // 재시도 시점을 0.75~0.95 × interval 범위에 분산한다.
                    // (실제 재요청 대기: 약 0.05~0.25 × interval)
                    const float Jitter =
                        (float)(rand() % 100) / 100.f *
                        mPathRetryInterval * 0.2f;
                    mPathRetryAccum =
                        mPathRetryInterval * 0.75f + Jitter;
                }
            }
            else
            {
                mHasLockedTarget = false;
                CancelCurrentPath();
                mPathRetryAccum = mPathRetryInterval;
            }
        }
        else
        {
            mHasLockedTarget = false;
            CancelCurrentPath();
        }
    }

    FVector3 ArrivalRef = mHasLockedTarget ? mLockedTargetPos : TargetMarker;
    ArrivalRef.z = CurrentZ;

    FVector3 Current = GetWorldPos();
    Current.z = CurrentZ;

    const float Dist = Current.Distance(ArrivalRef);

    bool ArrivedAtBuilding = (Dist <= mArrivalDistance);

    // 잠금 좌표와 다른 마커(goal tile)에 도착하는 경우를 허용한다.
    // 현재 위치 기준의 최근접 마커(TargetMarker)에 충분히 가까우면
    // 동일 건물 도착으로 처리한다.
    if (!ArrivedAtBuilding && mHasLockedTarget)
    {
        ArrivedAtBuilding =
            Current.Distance(TargetMarker) <= mArrivalDistance;
    }

    if (ArrivedAtBuilding)
    {
        // 구 경로를 즉시 클리어해 velocity가 다음 프레임에 0이 되도록 보장한다.
        CancelCurrentPath();
        mHasLockedTarget = false;
        mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

        if (mCurrentTargetName.empty())
            return;

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Arrived. switch target=%s dist=%.2f arrival=%.2f\n",
            mCurrentTargetName.c_str(),
            Dist, mArrivalDistance);
#endif

        // 즉시 경로를 요청하지 않는다.
        // 도착 후 대기는 짧게 유지해 정지 체감을 줄인다.
        mPathRetryAccum = -((float)(rand() % 10) / 100.f);
        return;
    }

    // 경로 대기 상태 관리:
    // 경로 요청 후 velocity가 한 번이라도 non-zero가 되면 경로가 수신됐다고 판단한다.
    if (mWaitingForPath && !Movement->GetVelocity().IsZero())
    {
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
    }

    if (mWaitingForPath)
    {
        mWaitingPathAccum += DeltaTime;

        // 응답이 너무 늦으면 대기를 강제로 해제해 즉시 재요청한다.
        if (mWaitingPathAccum >= 0.35f)
        {
            mWaitingForPath = false;
            mWaitingPathAccum = 0.f;
            mPathRetryAccum = mPathRetryInterval;
        }
    }
    else
    {
        mWaitingPathAccum = 0.f;
    }

    const FVector3 Velocity = Movement->GetVelocity();

    if (!Velocity.IsZero())
    {
        mStallAccum = 0.f;
        mLastProgressPos = Current;
    }
    else
    {
        if (Current.Distance(mLastProgressPos) > 2.f)
        {
            mStallAccum = 0.f;
            mLastProgressPos = Current;
        }
        else
        {
            mStallAccum += DeltaTime;
        }
    }

    // 일정 시간 정체되면 타겟을 바꿔 고착 상태를 해소한다.
    if (mStallAccum >= 1.5f)
    {
        const std::string PrevTarget = mCurrentTargetName;

        CancelCurrentPath();
        mHasLockedTarget = false;
        mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

        if (mCurrentTargetName.empty())
            mCurrentTargetName = PrevTarget;

        RequestMoveTo(mCurrentTargetName);
        mPathRetryAccum = 0.f;
        mStallAccum = 0.f;
        mLastProgressPos = Current;
        return;
    }

    // 대기 중이면 retry 간격을 약간만 늘려 큐 폭주를 방지한다.
    // 대기 중이 아니면 표준 간격으로 retry한다.
    const float EffectiveRetryInterval = mWaitingForPath
        ? mPathRetryInterval * 1.2f
        : mPathRetryInterval;

    if (mPathRetryAccum >= EffectiveRetryInterval &&
        Movement->GetVelocity().IsZero())
    {
        mWaitingForPath = false;
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
            "[Orb] Pos=(%.1f, %.1f) target=%s pathTarget=%s dist=%.1f\n",
            Pos.x, Pos.y,
            mCurrentTargetName.c_str(),
            PathTarget.empty() ? "<none>" : PathTarget.c_str(),
            Dist);
    }
#endif
}

void CBuildingMarkerOrb::ApplySoftSeparation(float DeltaTime)
{
    auto World = mWorld.lock();

    if (!World || DeltaTime <= 0.f)
        return;

    static FOrbSpatialHashCache SpatialCache;

    if (SpatialCache.World != World.get() ||
        SpatialCache.SeenOrbs.empty())
    {
        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return;

        RebuildOrbSpatialHash(World.get(), OrbList, SpatialCache);
    }

    auto FinalizeVisit = [&]()
    {
        SpatialCache.SeenOrbs.insert(this);

        if (SpatialCache.ActiveCount <= 0 ||
            static_cast<int>(SpatialCache.SeenOrbs.size()) >=
            SpatialCache.ActiveCount)
        {
            SpatialCache.SeenOrbs.clear();
        }
    };

    if (SpatialCache.ActiveCount <= 1)
    {
        FinalizeVisit();
        return;
    }

    auto SelfIter = SpatialCache.IndexByOrb.find(this);

    if (SelfIter == SpatialCache.IndexByOrb.end())
    {
        FinalizeVisit();
        return;
    }

    const int SelfIndex = SelfIter->second;
    auto& SelfEntry = SpatialCache.Entries[SelfIndex];
    const float SelfRadius = SelfEntry.Radius;

    // 30% 겹침 허용 -> 최소 분리 거리 비율은 70%
    const float MinDistanceScale = Clamp<float>(
        1.f - mAllowedOverlapRatio, 0.1f, 1.f);
    const float Epsilon = 0.0001f;

    const FVector3 SelfPos = SelfEntry.Pos;
    FVector3 AccumulatedPush = FVector3::Zero;
    int OverlapCount = 0;

    for (int CellY = SelfEntry.CellY - 1;
        CellY <= SelfEntry.CellY + 1; ++CellY)
    {
        for (int CellX = SelfEntry.CellX - 1;
            CellX <= SelfEntry.CellX + 1; ++CellX)
        {
            auto CellIter = SpatialCache.Cells.find(
                MakeCellKey(CellX, CellY));

            if (CellIter == SpatialCache.Cells.end())
                continue;

            const auto& CellEntries = CellIter->second;

            for (size_t i = 0; i < CellEntries.size(); ++i)
            {
                const int OtherIndex = CellEntries[i];

                if (OtherIndex == SelfIndex)
                    continue;

                const auto& OtherEntry =
                    SpatialCache.Entries[OtherIndex];

                FVector3 Delta = SelfPos - OtherEntry.Pos;
                Delta.z = 0.f;

                const float Dist = Delta.Length();
                const float OtherRadius = OtherEntry.Radius;
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
                    const float Angle =
                        (float)((SelfEntry.NameHash ^
                            (OtherEntry.NameHash << 1)) % 6283) *
                        0.001f;
                    PushDir.x = cosf(Angle);
                    PushDir.y = sinf(Angle);
                    PushDir.z = 0.f;
                }

                const float Penetration = MinDist - Dist;
                AccumulatedPush += PushDir * Penetration;
                ++OverlapCount;
            }
        }
    }

    if (OverlapCount <= 0 || AccumulatedPush.IsZero())
    {
        FinalizeVisit();
        return;
    }

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

    // 같은 프레임의 후속 오브 계산이 최신 위치를 보게 한다.
    SelfEntry.Pos += PushDelta;
    SelfEntry.Pos.z = 0.f;
    const int NewCellX = ToCellCoord(SelfEntry.Pos.x, SpatialCache.CellSize);
    const int NewCellY = ToCellCoord(SelfEntry.Pos.y, SpatialCache.CellSize);

    if (NewCellX != SelfEntry.CellX || NewCellY != SelfEntry.CellY)
    {
        auto OldCellIter = SpatialCache.Cells.find(
            MakeCellKey(SelfEntry.CellX, SelfEntry.CellY));

        if (OldCellIter != SpatialCache.Cells.end())
        {
            auto& OldCellEntries = OldCellIter->second;
            OldCellEntries.erase(
                std::remove(OldCellEntries.begin(),
                    OldCellEntries.end(), SelfIndex),
                OldCellEntries.end());
        }

        SelfEntry.CellX = NewCellX;
        SelfEntry.CellY = NewCellY;
        SpatialCache.Cells[MakeCellKey(NewCellX, NewCellY)].
            push_back(SelfIndex);
    }

    FinalizeVisit();
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

    auto TargetBuilding =
        World->FindObject<CPlacementAreaObject>(TargetBuildingName).lock();

    if (TargetBuilding)
    {
        FVector3 MarkerPos;

        if (TargetBuilding->GetClosestMarkerWorldPos(
            GetWorldPos(), MarkerPos))
        {
            // 첫 요청에만 목표 위치를 잠근다.
            // 잠금이 살아있으면 기존 좌표를 재사용해 매 retry마다
            // 가장 가까운 마커가 바뀌는 것(circling 원인)을 방지한다.
            if (!mHasLockedTarget)
            {
                mLockedTargetPos = MarkerPos;
                mHasLockedTarget = true;
            }

            const FVector3& NavTarget = mLockedTargetPos;

#ifdef _DEBUG
            DebugOrbLog("[Orb] RequestMoveTo target=%s pos=(%.1f,%.1f) locked=%d\n",
                TargetBuildingName.c_str(), NavTarget.x, NavTarget.y,
                mHasLockedTarget ? 1 : 0);
#endif
            const bool Requested = Movement->MovePath(NavTarget);
            mWaitingForPath = Requested;
            mWaitingPathAccum = 0.f;

            if (!Requested)
            {
                // 큐 포화로 요청이 반려되면 빠르게 재시도한다.
                mPathRetryAccum = mPathRetryInterval * 0.9f;
#ifdef _DEBUG
                DebugOrbLog(
                    "[Orb] RequestMoveTo enqueue failed target=%s\n",
                    TargetBuildingName.c_str());
#endif
            }
            return;
        }
    }

    // Fallback: marker 좌표 확보에 실패했을 때만 기존 오브젝트 타겟 경로를 사용한다.
#ifdef _DEBUG
    DebugOrbLog("[Orb] RequestMoveTo fallback target=%s\n",
        TargetBuildingName.c_str());
#endif
    const bool Requested =
        Movement->MovePathToObject(TargetBuildingName);
    mWaitingForPath = Requested;
    mWaitingPathAccum = 0.f;

    if (!Requested)
    {
        mPathRetryAccum = mPathRetryInterval * 0.9f;
#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] RequestMoveTo fallback enqueue failed target=%s\n",
            TargetBuildingName.c_str());
#endif
    }
}
