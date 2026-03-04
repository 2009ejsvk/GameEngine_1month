#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Object/TileMapObject.h"
#include "Render/RenderManager.h"
#include "World/World.h"
#include "World/CameraManager.h"
#include "Component/CameraComponent.h"
#include "Device.h"
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

    mSatisfaction.Food = 70.f;
    mSatisfaction.Health = 70.f;
    mSatisfaction.Fun = 70.f;
    mSatisfaction.Faith = 70.f;
    mSatisfaction.Housing = 70.f;
    mSatisfaction.Job = 70.f;
    mSatisfaction.Freedom = 70.f;
    mSatisfaction.Security = 70.f;
    RecalculateOverallSatisfaction();

    mMeshComponent = CreateComponent<CMeshComponent>("MarkerOrbMesh");
    mMovement = CreateComponent<CObjectMovementComponent>(
        "MarkerOrbMovement");

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        auto RenderMgr = CRenderManager::GetInst();
        int MarkerOrbLayer = RenderMgr->GetLayerOrder("BuildingVisual");

        if (MarkerOrbLayer < 0)
        {
            for (int Order = 3; Order <= 100; ++Order)
            {
                RenderMgr->CreateLayer("BuildingVisual", Order, ERenderListSort::Y);
                MarkerOrbLayer = RenderMgr->GetLayerOrder("BuildingVisual");

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
        Mesh->SetRenderSortYBias(-mOrbDiameter * 0.5f);
        Mesh->SetRenderSortPriority(1);

        if (MarkerOrbLayer >= 0)
            Mesh->SetRenderLayer("BuildingVisual");
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

    UpdateSatisfaction(DeltaTime);

    if (mSatisfaction.Health <= GHealthRemoveThreshold)
    {
        Destroy();
        return;
    }

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

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        auto CameraMgr = World->GetCameraManager().lock();

        if (!CameraMgr)
        {
            Mesh->SetEnable(false);
            return;
        }

        const FVector3 CamPos = CameraMgr->GetMainCameraWorldPos();
        float ViewWidth = (float)CDevice::GetInst()->GetResolution().Width;
        float ViewHeight = (float)CDevice::GetInst()->GetResolution().Height;

        auto MainCamera = CameraMgr->GetMainCamera().lock();

        if (MainCamera &&
            MainCamera->GetProjectionType() ==
            ECameraProjectionType::Ortho)
        {
            ViewWidth = MainCamera->GetViewWidth();
            ViewHeight = MainCamera->GetViewHeight();
        }

        const float Margin = (std::max)(mOrbDiameter * 2.f, 32.f);
        const FVector3 Pos = GetWorldPos();
        const bool Visible =
            fabsf(Pos.x - CamPos.x) <= (ViewWidth * 0.5f + Margin) &&
            fabsf(Pos.y - CamPos.y) <= (ViewHeight * 0.5f + Margin);

        Mesh->SetEnable(Visible);
    }

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
        mHasLockedTarget = false;
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
    };

    auto NormalizeResumeState = [&](ECitizenState State) -> ECitizenState
    {
        switch (State)
        {
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
            return ECitizenState::GoingHome;
        case ECitizenState::GoingToWork:
        case ECitizenState::AtWork:
            return ECitizenState::GoingToWork;
        default:
            return ECitizenState::GoingToWork;
        }
    };

    auto ResolveStateAfterService = [&]() -> ECitizenState
    {
        if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
            return ECitizenState::Wander;

        ECitizenState ResumeState = NormalizeResumeState(
            mResumeStateAfterService);

        if (ResumeState == ECitizenState::GoingHome &&
            !mHomeName.empty())
        {
            return ECitizenState::GoingHome;
        }

        if (!mWorkName.empty())
            return ECitizenState::GoingToWork;

        return ECitizenState::GoingHome;
    };

    auto TryInterruptByNeed = [&]() -> bool
    {
        if (mCitizenState == ECitizenState::Wander)
            return false;

        if (IsTeamsterState(mCitizenState))
            return false;

        const bool IsFoodState =
            mCitizenState == ECitizenState::GoingToFood ||
            mCitizenState == ECitizenState::AtFood;
        const bool IsFunState =
            mCitizenState == ECitizenState::GoingToFun ||
            mCitizenState == ECitizenState::AtFun;

        if (!IsFoodState &&
            !mFoodName.empty() &&
            mSatisfaction.Food <= GFoodInterruptThreshold)
        {
            mResumeStateAfterService =
                NormalizeResumeState(mCitizenState);
            CancelCurrentPath();
            TransitionFsm(ECitizenState::GoingToFood);
            mPathRetryAccum = 0.f;
            return true;
        }

        if (!IsFunState &&
            !IsFoodState &&
            !mFunName.empty() &&
            mSatisfaction.Fun <= GFunInterruptThreshold)
        {
            mResumeStateAfterService =
                NormalizeResumeState(mCitizenState);
            CancelCurrentPath();
            TransitionFsm(ECitizenState::GoingToFun);
            mPathRetryAccum = 0.f;
            return true;
        }

        return false;
    };

    auto ResolveTargetByState = [&]() -> std::string
    {
        switch (mCitizenState)
        {
        case ECitizenState::GoingToWork: return mWorkName;
        case ECitizenState::GoingHome: return mHomeName;
        case ECitizenState::GoingToFood: return mFoodName;
        case ECitizenState::GoingToFun: return mFunName;
        case ECitizenState::GoingToTeamsterSource:
            return mTeamsterSourceName;
        case ECitizenState::GoingToTeamsterHarbor:
            return mTeamsterHarborName;
        case ECitizenState::GoingToTeamsterOffice:
            return mWorkName;
        default: return PickRandomTargetName(std::string());
        }
    };

    // 체류 상태: 타이머 소모 후 다음 상태로 전환
    if (mCitizenState == ECitizenState::AtWork ||
        mCitizenState == ECitizenState::AtHome ||
        mCitizenState == ECitizenState::AtFood ||
        mCitizenState == ECitizenState::AtFun)
    {
        if (mCitizenState == ECitizenState::AtWork &&
            TryStartTeamsterDelivery())
        {
            return;
        }

        if ((mCitizenState == ECitizenState::AtWork ||
            mCitizenState == ECitizenState::AtHome) &&
            TryInterruptByNeed())
        {
            return;
        }

        mDwellTimer -= DeltaTime;

        if (mDwellTimer <= 0.f)
        {
            CancelCurrentPath();

            if (mCitizenState == ECitizenState::AtWork)
            {
                TransitionFsm(ECitizenState::GoingHome);
            }
            else if (mCitizenState == ECitizenState::AtHome)
            {
                TransitionFsm(ECitizenState::GoingToWork);
            }
            else
            {
                TransitionFsm(ResolveStateAfterService());
            }
        }

        return;
    }

    TryInterruptByNeed();

    if (!mHasStartPos)
    {
        std::vector<std::pair<std::string, FVector3>> MarkerList;

        if (!CollectTargetMarkers(MarkerList))
        {
            FVector3 FallbackStartPos = FVector3::Zero;

            if (TryGetFallbackStartPos(FallbackStartPos))
            {
                SetWorldPos(FallbackStartPos);
                mCurrentTargetName.clear();
                mHasStartPos = true;
#ifdef _DEBUG
                mDebugMissingMarkerLogged = false;
#endif
                return;
            }

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

        if (mCitizenState == ECitizenState::Wander)
            mCurrentTargetName = PickRandomTargetName(StartName);
        else
            mCurrentTargetName = ResolveTargetByState();

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
        mCurrentTargetName = ResolveTargetByState();

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
        switch (mCitizenState)
        {
        case ECitizenState::GoingToWork:
        case ECitizenState::AtWork:
            mWorkName.clear();
            mTeamsterSourceName.clear();
            mTeamsterHarborName.clear();
            mTeamsterCarryAmount = 0;
            ResetTeamsterSpeed();
            break;
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
            mHomeName.clear();
            break;
        case ECitizenState::GoingToFood:
        case ECitizenState::AtFood:
            mFoodName.clear();
            break;
        case ECitizenState::GoingToFun:
        case ECitizenState::AtFun:
            mFunName.clear();
            break;
        case ECitizenState::GoingToTeamsterSource:
            mTeamsterSourceName.clear();
            mTeamsterCarryAmount = 0;
            ResetTeamsterSpeed();
            break;
        case ECitizenState::GoingToTeamsterHarbor:
            mTeamsterHarborName.clear();
            mTeamsterCarryAmount = 0;
            ResetTeamsterSpeed();
            break;
        case ECitizenState::GoingToTeamsterOffice:
            mWorkName.clear();
            mTeamsterSourceName.clear();
            mTeamsterHarborName.clear();
            mTeamsterCarryAmount = 0;
            ResetTeamsterSpeed();
            break;
        default:
            break;
        }

        if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
        {
            TransitionFsm(ECitizenState::Wander);
        }
        else if (mCitizenState == ECitizenState::GoingToFood ||
            mCitizenState == ECitizenState::AtFood ||
            mCitizenState == ECitizenState::GoingToFun ||
            mCitizenState == ECitizenState::AtFun)
        {
            TransitionFsm(ResolveStateAfterService());
        }
        else if (IsTeamsterState(mCitizenState))
        {
            ResetTeamsterSpeed();
            TransitionFsm(ECitizenState::GoingToWork);
        }

        if (mCitizenState == ECitizenState::Wander)
            mCurrentTargetName = PickRandomTargetName(std::string());
        else
            mCurrentTargetName = ResolveTargetByState();

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

        if (mCitizenState == ECitizenState::Wander)
            mCurrentTargetName = PickRandomTargetName(std::string());
        else
            mCurrentTargetName = ResolveTargetByState();

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

        if (mCitizenState == ECitizenState::GoingToWork)
        {
            TransitionFsm(ECitizenState::AtWork);
        }
        else if (mCitizenState == ECitizenState::GoingHome)
        {
            TransitionFsm(ECitizenState::AtHome);
        }
        else if (mCitizenState == ECitizenState::GoingToFood)
        {
            // 실제 도착한 건물명을 우선 고정한다.
            // (주기 재배정으로 mFoodName이 바뀌어도 이번 방문은 유지)
            mFoodVisitBuildingName = mCurrentTargetName;
            TransitionFsm(ECitizenState::AtFood);
        }
        else if (mCitizenState == ECitizenState::GoingToFun)
        {
            TransitionFsm(ECitizenState::AtFun);
        }
        else if (mCitizenState == ECitizenState::GoingToTeamsterSource)
        {
            bool LoadedCargo = false;

            if (World && !mCurrentTargetName.empty())
            {
                auto SourceBuilding =
                    World->FindObject<CPlacementAreaObject>(
                        mCurrentTargetName).lock();

                const bool IsProductionOrFood =
                    SourceBuilding &&
                    !SourceBuilding->IsResidential() &&
                    (!SourceBuilding->IsEntertainmentProvider() ||
                        SourceBuilding->IsFoodProvider());

                if (SourceBuilding &&
                    IsProductionOrFood &&
                    !SourceBuilding->IsTransportOffice() &&
                    !SourceBuilding->IsHarbor() &&
                    SourceBuilding->TryConsumeResource(GTeamsterTransferUnit))
                {
                    mTeamsterCarryAmount = GTeamsterTransferUnit;
                    LoadedCargo = true;
                }
            }

            if (LoadedCargo)
            {
                if (mTeamsterHarborName.empty())
                    mTeamsterHarborName = FindHarborName();

                if (!mTeamsterHarborName.empty())
                    TransitionFsm(ECitizenState::GoingToTeamsterHarbor);
                else
                    TransitionFsm(ECitizenState::GoingToTeamsterOffice);
            }
            else
            {
                mTeamsterCarryAmount = 0;
                TransitionFsm(ECitizenState::GoingToTeamsterOffice);
            }
        }
        else if (mCitizenState == ECitizenState::GoingToTeamsterHarbor)
        {
            if (World &&
                mTeamsterCarryAmount > 0 &&
                !mCurrentTargetName.empty())
            {
                auto HarborBuilding =
                    World->FindObject<CPlacementAreaObject>(
                        mCurrentTargetName).lock();

                if (HarborBuilding && HarborBuilding->IsHarbor())
                {
                    HarborBuilding->AddResourceStock(mTeamsterCarryAmount);
                }
            }

            mTeamsterCarryAmount = 0;
            TransitionFsm(ECitizenState::GoingToTeamsterOffice);
        }
        else if (mCitizenState == ECitizenState::GoingToTeamsterOffice)
        {
            ResetTeamsterSpeed();
            mTeamsterCarryAmount = 0;
            TransitionFsm(ECitizenState::AtWork);
        }
        else
        {
            // Wander 모드: 기존 랜덤 타겟 선택
            mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

            if (mCurrentTargetName.empty())
                return;

#ifdef _DEBUG
            DebugOrbLog(
                "[Orb] Arrived. switch target=%s dist=%.2f arrival=%.2f\n",
                mCurrentTargetName.c_str(),
                Dist, mArrivalDistance);
#endif
        }

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
        CancelCurrentPath();

        if (mCitizenState != ECitizenState::Wander)
        {
            // FSM 모드: 같은 목표로 재시도 (경로 문제)
            RequestMoveTo(mCurrentTargetName);
        }
        else
        {
            const std::string PrevTarget = mCurrentTargetName;
            mCurrentTargetName = PickRandomTargetName(mCurrentTargetName);

            if (mCurrentTargetName.empty())
                mCurrentTargetName = PrevTarget;

            RequestMoveTo(mCurrentTargetName);
        }

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

void CBuildingMarkerOrb::UpdateSatisfaction(float DeltaTime)
{
    auto World = mWorld.lock();

    auto ResolveBuildingCap = [&](
        const std::string& BuildingName,
        int (CPlacementAreaObject::*Getter)() const) -> float
    {
        if (!World || BuildingName.empty())
            return 100.f;

        auto Building = World->FindObject<CPlacementAreaObject>(
            BuildingName).lock();

        if (!Building || !Building->GetAlive())
            return 100.f;

        return static_cast<float>((Building.get()->*Getter)());
    };

    const float HomeHousingCap = ResolveBuildingCap(
        mHomeName, &CPlacementAreaObject::GetHousingSatisfactionCap);
    const float WorkJobCap = ResolveBuildingCap(
        mWorkName, &CPlacementAreaObject::GetJobSatisfactionCap);
    const std::string& FoodCapBuildingName = mFoodVisitBuildingName.empty() ?
        mFoodName :
        mFoodVisitBuildingName;
    const float FoodCap = ResolveBuildingCap(
        FoodCapBuildingName, &CPlacementAreaObject::GetFoodSatisfactionCap);
    const float FunCap = ResolveBuildingCap(
        mFunName, &CPlacementAreaObject::GetFunSatisfactionCap);

    auto RecoverUnderCap = [&](float& Value, float GainPerSec, float Cap)
    {
        if (Value >= Cap)
            return;

        Value = (std::min)(Cap, Value + GainPerSec * DeltaTime);
    };

    // 욕구 자연 감소
    mSatisfaction.Food = (std::max)(
        0.f, mSatisfaction.Food - 1.2f * DeltaTime);
    mSatisfaction.Job = (std::max)(
        0.f, mSatisfaction.Job - 1.0f * DeltaTime);
    mSatisfaction.Housing = (std::max)(
        0.f, mSatisfaction.Housing - 0.5f * DeltaTime);
    mSatisfaction.Fun = (std::max)(
        0.f, mSatisfaction.Fun - 0.9f * DeltaTime);
    mSatisfaction.Health = (std::max)(
        0.f, mSatisfaction.Health - 0.2f * DeltaTime);
    mSatisfaction.Faith = (std::max)(
        0.f, mSatisfaction.Faith - 0.15f * DeltaTime);

    // FSM 상태별 회복
    switch (mCitizenState)
    {
    case ECitizenState::AtWork:
        RecoverUnderCap(mSatisfaction.Job, 10.f, WorkJobCap);
        // 생산/식량 시설만 재고를 생산한다.
        // 운송업자 사무소/항구는 재고를 직접 생산하지 않는다.
        if (World && !mWorkName.empty())
        {
            auto WorkBuilding =
                World->FindObject<CPlacementAreaObject>(mWorkName).lock();
            if (WorkBuilding)
            {
                float ProductionPerSec = 0.f;

                if (WorkBuilding->IsFoodProductionFacility())
                    ProductionPerSec = 40.f;
                else if (!WorkBuilding->IsTransportOffice() &&
                    !WorkBuilding->IsHarbor())
                {
                    ProductionPerSec = 2.f;
                }

                WorkBuilding->AddProduction(ProductionPerSec, DeltaTime);
            }
        }
        break;
    case ECitizenState::AtHome:
        RecoverUnderCap(mSatisfaction.Housing, 8.f, HomeHousingCap);
        mSatisfaction.Health = (std::min)(
            100.f, mSatisfaction.Health + 1.f * DeltaTime);
        break;
    case ECitizenState::AtFood:
        if (!mFoodStockAvailableThisVisit &&
            World && !mFoodVisitBuildingName.empty())
        {
            auto FoodBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mFoodVisitBuildingName).lock();

            if (FoodBuilding)
                mFoodStockAvailableThisVisit =
                    FoodBuilding->TryConsumeResource(1);
        }

        // 재고가 있었을 때만 음식 만족도 회복
        if (mFoodStockAvailableThisVisit)
            RecoverUnderCap(mSatisfaction.Food, 30.f, FoodCap);
        mSatisfaction.Health = (std::min)(
            100.f, mSatisfaction.Health + 3.f * DeltaTime);
        break;
    case ECitizenState::AtFun:
        RecoverUnderCap(mSatisfaction.Fun, 26.f, FunCap);
        break;
    default:
        break;
    }

    // 1초 틱으로 Overall 재계산 (매 프레임 불필요)
    mSatisfactionTickAccum += DeltaTime;
    if (mSatisfactionTickAccum >= 1.f)
    {
        mSatisfactionTickAccum = 0.f;
        RecalculateOverallSatisfaction();
    }
}

void CBuildingMarkerOrb::RecalculateOverallSatisfaction()
{
    const float WeightedAverage =
        mSatisfaction.Food * 0.18f +
        mSatisfaction.Health * 0.14f +
        mSatisfaction.Fun * 0.10f +
        mSatisfaction.Faith * 0.10f +
        mSatisfaction.Housing * 0.13f +
        mSatisfaction.Job * 0.13f +
        mSatisfaction.Freedom * 0.10f +
        mSatisfaction.Security * 0.12f;

    float MinimumNeed = mSatisfaction.Food;
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Health);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Fun);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Faith);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Housing);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Job);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Freedom);
    MinimumNeed = (std::min)(MinimumNeed, mSatisfaction.Security);

    const float LowNeedPenalty =
        (std::max)(0.f, 30.f - MinimumNeed) * 0.8f;

    mSatisfaction.Overall =
        Clamp<float>(WeightedAverage - LowNeedPenalty, 0.f, 100.f);
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

bool CBuildingMarkerOrb::TryGetFallbackStartPos(FVector3& OutPos)
{
    auto TileMapObject = mTileMapObject.lock();

    if (!TileMapObject)
        return false;

    auto TileMap = TileMapObject->GetTileMap().lock();

    if (!TileMap)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();

    if (CountX <= 0 || CountY <= 0)
        return false;

    const int TileCount = CountX * CountY;
    int ChosenIndex = -1;
    const int MaxAttempts = (std::min)(TileCount, 64);

    for (int Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        const int CandidateIndex = rand() % TileCount;
        auto CandidateTile = TileMap->GetTile(CandidateIndex).lock();

        if (!CandidateTile)
            continue;

        if (CandidateTile->GetType() == ETileType::UnableToMove)
            continue;

        ChosenIndex = CandidateIndex;
        break;
    }

    if (ChosenIndex < 0)
    {
        ChosenIndex = (CountY / 2) * CountX + (CountX / 2);
    }

    auto SpawnTile = TileMap->GetTile(ChosenIndex).lock();

    if (!SpawnTile)
        return false;

    const FVector2 Center = SpawnTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObject->GetWorldPos();
    const float CurrentZ = GetWorldPos().z;
    OutPos = FVector3(
        Center.x + TileMapWorldPos.x,
        Center.y + TileMapWorldPos.y,
        CurrentZ);

    return true;
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

    mArrivalDistance = Diameter * 2.5f;

    if (mArrivalDistance < 4.f)
        mArrivalDistance = 4.f;

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        Mesh->SetRelativeScale(Diameter, Diameter);
        Mesh->SetRenderSortYBias(-Diameter * 0.5f);
        Mesh->SetRenderSortPriority(1);
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

void CBuildingMarkerOrb::TransitionFsm(ECitizenState NewState)
{
    if (!IsTeamsterState(NewState))
        ResetTeamsterSpeed();

    mCitizenState = NewState;
    mDwellTimer = 0.f;

    switch (NewState)
    {
    case ECitizenState::GoingToWork:
        mFoodVisitBuildingName.clear();
        mTeamsterCarryAmount = 0;
        mCurrentTargetName = mWorkName;
        break;
    case ECitizenState::AtWork:
        mFoodVisitBuildingName.clear();
        mTeamsterSourceName.clear();
        mTeamsterHarborName.clear();
        mTeamsterCarryAmount = 0;
        mDwellTimer = GAtWorkDuration;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingHome:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mHomeName;
        break;
    case ECitizenState::AtHome:
        mFoodVisitBuildingName.clear();
        mDwellTimer = GAtHomeDuration;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingToFood:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFoodName;
        break;
    case ECitizenState::AtFood:
        mDwellTimer = GAtFoodDuration;
        mCurrentTargetName.clear();
        if (mFoodVisitBuildingName.empty())
            mFoodVisitBuildingName = mFoodName;
        mFoodStockAvailableThisVisit = false;
        {
            auto FoodWorld = mWorld.lock();
            if (FoodWorld && !mFoodVisitBuildingName.empty())
            {
                auto FoodBuilding =
                    FoodWorld->FindObject<CPlacementAreaObject>(
                        mFoodVisitBuildingName).lock();
                if (FoodBuilding)
                    mFoodStockAvailableThisVisit = FoodBuilding->TryConsumeResource(1);
            }
        }
        break;
    case ECitizenState::GoingToFun:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName = mFunName;
        break;
    case ECitizenState::AtFun:
        mFoodVisitBuildingName.clear();
        mDwellTimer = GAtFunDuration;
        mCurrentTargetName.clear();
        break;
    case ECitizenState::GoingToTeamsterSource:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = mTeamsterSourceName;
        break;
    case ECitizenState::GoingToTeamsterHarbor:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = mTeamsterHarborName;
        break;
    case ECitizenState::GoingToTeamsterOffice:
        mFoodVisitBuildingName.clear();
        StartTeamsterSpeedBoost();
        mCurrentTargetName = mWorkName;
        break;
    default:
        mFoodVisitBuildingName.clear();
        mCurrentTargetName.clear();
        break;
    }
}

void CBuildingMarkerOrb::SetHomeBuilding(const std::string& Name)
{
    mHomeName = Name;
    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetWorkBuilding(const std::string& Name)
{
    if (mWorkName != Name)
    {
        mTeamsterSourceName.clear();
        mTeamsterHarborName.clear();
        mTeamsterCarryAmount = 0;
        ResetTeamsterSpeed();
    }

    mWorkName = Name;

    if (IsTeamsterState(mCitizenState))
        TransitionFsm(ECitizenState::GoingToWork);

    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetFoodBuilding(const std::string& Name)
{
    mFoodName = Name;
    TryStartCoreLoop();
}

void CBuildingMarkerOrb::SetFunBuilding(const std::string& Name)
{
    mFunName = Name;
}

void CBuildingMarkerOrb::TryStartCoreLoop()
{
    if (mCitizenState != ECitizenState::Wander)
        return;

    if (mHomeName.empty() || mWorkName.empty() || mFoodName.empty())
        return;

    TransitionFsm(ECitizenState::GoingToWork);
    mHasLockedTarget = false;
    mPathRetryAccum = 0.f;
}

bool CBuildingMarkerOrb::IsTeamsterState(ECitizenState State) const
{
    return State == ECitizenState::GoingToTeamsterSource ||
        State == ECitizenState::GoingToTeamsterHarbor ||
        State == ECitizenState::GoingToTeamsterOffice;
}

void CBuildingMarkerOrb::StartTeamsterSpeedBoost()
{
    mTeamsterSpeedBoostActive = true;
    mMoveSpeed = mDefaultMoveSpeed * GTeamsterSpeedMultiplier;
}

void CBuildingMarkerOrb::ResetTeamsterSpeed()
{
    mTeamsterSpeedBoostActive = false;
    mMoveSpeed = mDefaultMoveSpeed;
}

bool CBuildingMarkerOrb::TryStartTeamsterDelivery()
{
    if (mCitizenState != ECitizenState::AtWork)
        return false;

    auto World = mWorld.lock();

    if (!World || mWorkName.empty())
        return false;

    auto WorkBuilding =
        World->FindObject<CPlacementAreaObject>(mWorkName).lock();

    if (!WorkBuilding ||
        !WorkBuilding->GetAlive() ||
        !WorkBuilding->GetEnable() ||
        !WorkBuilding->HasPlacedArea() ||
        !WorkBuilding->IsTransportOffice())
    {
        return false;
    }

    const std::string SourceName = FindTeamsterSourceName();

    if (SourceName.empty())
        return false;

    const std::string HarborName = FindHarborName();

    if (HarborName.empty())
        return false;

    mTeamsterSourceName = SourceName;
    mTeamsterHarborName = HarborName;
    mTeamsterCarryAmount = 0;

    TransitionFsm(ECitizenState::GoingToTeamsterSource);
    mPathRetryAccum = 0.f;
    return true;
}

std::string CBuildingMarkerOrb::FindTeamsterSourceName() const
{
    auto World = mWorld.lock();

    if (!World)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    std::string BestName;
    int BestStock = 0;

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        const bool IsProductionOrFood =
            Building &&
            !Building->IsResidential() &&
            (!Building->IsEntertainmentProvider() ||
                Building->IsFoodProvider());

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea() ||
            !IsProductionOrFood ||
            Building->IsTransportOffice() ||
            Building->IsHarbor())
        {
            continue;
        }

        const int Stock = Building->GetResourceStock();

        if (Stock < GTeamsterTransferUnit)
            continue;

        if (BestName.empty() || Stock > BestStock)
        {
            BestName = Building->GetName();
            BestStock = Stock;
        }
    }

    return BestName;
}

std::string CBuildingMarkerOrb::FindHarborName() const
{
    auto World = mWorld.lock();

    if (!World)
        return std::string();

    std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

    if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        return std::string();

    for (size_t i = 0; i < BuildingList.size(); ++i)
    {
        auto Building = BuildingList[i].lock();

        if (!Building ||
            !Building->GetAlive() ||
            !Building->GetEnable() ||
            !Building->HasPlacedArea() ||
            !Building->IsHarbor())
        {
            continue;
        }

        return Building->GetName();
    }

    return std::string();
}

void CBuildingMarkerOrb::RemoveTargetBuildingName(
    const std::string& BuildingName)
{
    if (BuildingName.empty())
        return;

    mRandomTargetNames.erase(
        std::remove(mRandomTargetNames.begin(),
            mRandomTargetNames.end(), BuildingName),
        mRandomTargetNames.end());

    if (mBuildingAName == BuildingName)
        mBuildingAName.clear();

    if (mBuildingBName == BuildingName)
        mBuildingBName.clear();

    if (mCurrentTargetName == BuildingName)
    {
        mCurrentTargetName.clear();
        mHasLockedTarget = false;
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
        mStallAccum = 0.f;

        auto Movement = mMovement.lock();

        if (Movement)
        {
            Movement->AdvancePathRequestId();
            Movement->StartPathPoint();
            Movement->SetPathTargetObjectName("");
        }
    }

    if (mBuildingAName.empty() && !mRandomTargetNames.empty())
        mBuildingAName = mRandomTargetNames[0];

    if (mBuildingBName.empty())
    {
        for (size_t i = 0; i < mRandomTargetNames.size(); ++i)
        {
            if (mRandomTargetNames[i] != mBuildingAName)
            {
                mBuildingBName = mRandomTargetNames[i];
                break;
            }
        }
    }

    // 핵심 건물(집/직장/음식)이 철거되면 Wander로 폴백
    if (mHomeName == BuildingName) { mHomeName.clear(); }
    if (mWorkName == BuildingName) { mWorkName.clear(); }
    if (mFoodName == BuildingName) { mFoodName.clear(); }
    if (mFoodVisitBuildingName == BuildingName)
    {
        mFoodVisitBuildingName.clear();
        mFoodStockAvailableThisVisit = false;
    }
    if (mFunName == BuildingName) { mFunName.clear(); }
    if (mTeamsterSourceName == BuildingName)
    {
        mTeamsterSourceName.clear();
        mTeamsterCarryAmount = 0;
    }
    if (mTeamsterHarborName == BuildingName)
    {
        mTeamsterHarborName.clear();
        mTeamsterCarryAmount = 0;
    }

    if ((mTeamsterSourceName.empty() || mTeamsterHarborName.empty()) &&
        IsTeamsterState(mCitizenState))
    {
        ResetTeamsterSpeed();
        TransitionFsm(ECitizenState::GoingToWork);
    }

    if ((mHomeName.empty() || mWorkName.empty() || mFoodName.empty()) &&
        mCitizenState != ECitizenState::Wander)
    {
        ResetTeamsterSpeed();
        mCitizenState = ECitizenState::Wander;
        mCurrentTargetName.clear();
        mHasLockedTarget = false;
    }
    else if ((mCitizenState == ECitizenState::GoingToFun ||
        mCitizenState == ECitizenState::AtFun) &&
        mFunName.empty())
    {
        TransitionFsm(ECitizenState::GoingToWork);
    }
}
