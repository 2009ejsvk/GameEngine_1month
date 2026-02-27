#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "World/World.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
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

    auto BuildingA = mBuildingAObject.lock();
    auto BuildingB = mBuildingBObject.lock();
    auto Movement = mMovement.lock();

    if (!BuildingA || !BuildingB || !Movement)
    {
#ifdef _DEBUG
        if (!mDebugMissingDependencyLogged)
        {
            DebugOrbLog(
                "[Orb] Missing dependency A=%d B=%d Move=%d\n",
                BuildingA ? 1 : 0,
                BuildingB ? 1 : 0,
                Movement ? 1 : 0);
            mDebugMissingDependencyLogged = true;
        }
#endif
        return;
    }

#ifdef _DEBUG
    mDebugMissingDependencyLogged = false;
#endif

    FVector3 MarkerA;
    FVector3 MarkerB;

    if (!BuildingA->GetMarkerWorldPos(MarkerA) ||
        !BuildingB->GetMarkerWorldPos(MarkerB))
    {
#ifdef _DEBUG
        if (!mDebugMissingMarkerLogged)
        {
            DebugOrbLog(
                "[Orb] Marker unavailable A=%s B=%s\n",
                BuildingA->GetName().c_str(),
                BuildingB->GetName().c_str());
            mDebugMissingMarkerLogged = true;
        }
#endif
        return;
    }

#ifdef _DEBUG
    mDebugMissingMarkerLogged = false;
#endif

    const float CurrentZ = GetWorldPos().z;
    MarkerA.z = CurrentZ;
    MarkerB.z = CurrentZ;

    Movement->SetSpeed(mMoveSpeed);

    bool MarkerMoved = false;

    if (mHasMarkerCache)
    {
        const float MarkerMoveEpsilonSq = 1.f;
        const FVector3 DeltaA = MarkerA - mLastMarkerA;
        const FVector3 DeltaB = MarkerB - mLastMarkerB;
        const float DistSqA = DeltaA.x * DeltaA.x +
            DeltaA.y * DeltaA.y + DeltaA.z * DeltaA.z;
        const float DistSqB = DeltaB.x * DeltaB.x +
            DeltaB.y * DeltaB.y + DeltaB.z * DeltaB.z;

        MarkerMoved =
            DistSqA > MarkerMoveEpsilonSq ||
            DistSqB > MarkerMoveEpsilonSq;
    }

    mLastMarkerA = MarkerA;
    mLastMarkerB = MarkerB;
    mHasMarkerCache = true;

    if (!mHasStartPos)
    {
        SetWorldPos(MarkerA);
        mCurrentTargetName = mBuildingBName;
        RequestMoveTo(mCurrentTargetName);

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Start at A marker=(%.1f, %.1f) target=%s\n",
            MarkerA.x, MarkerA.y, mCurrentTargetName.c_str());
#endif

        mHasStartPos = true;
        return;
    }

    if (mCurrentTargetName.empty())
    {
        mCurrentTargetName = mBuildingBName;
        RequestMoveTo(mCurrentTargetName);
        return;
    }

    if (MarkerMoved)
    {
        RequestMoveTo(mCurrentTargetName);
    }

    FVector3 TargetMarker =
        mCurrentTargetName == mBuildingAName ? MarkerA : MarkerB;
    TargetMarker.z = CurrentZ;

    FVector3 Current = GetWorldPos();
    Current.z = CurrentZ;

    const float Dist = Current.Distance(TargetMarker);

    if (Dist <= mArrivalDistance)
    {
        if (mCurrentTargetName == mBuildingAName)
            mCurrentTargetName = mBuildingBName;
        else
            mCurrentTargetName = mBuildingAName;

#ifdef _DEBUG
        DebugOrbLog(
            "[Orb] Arrived. switch target=%s dist=%.2f arrival=%.2f\n",
            mCurrentTargetName.c_str(), Dist, mArrivalDistance);
#endif

        RequestMoveTo(mCurrentTargetName);
        return;
    }

    if (Movement->GetPathTargetObjectName().empty())
    {
        RequestMoveTo(mCurrentTargetName);
    }

#ifdef _DEBUG
    mDebugStatusLogAccum += DeltaTime;

    if (mDebugStatusLogAccum >= 1.f)
    {
        mDebugStatusLogAccum = 0.f;

        const FVector3 Pos = GetWorldPos();
        const std::string& PathTarget = Movement->GetPathTargetObjectName();

        DebugOrbLog(
            "[Orb] Pos=(%.1f, %.1f) target=%s pathTarget=%s dist=%.1f markerA=(%.1f, %.1f) markerB=(%.1f, %.1f)\n",
            Pos.x, Pos.y,
            mCurrentTargetName.c_str(),
            PathTarget.empty() ? "<none>" : PathTarget.c_str(),
            Dist,
            MarkerA.x, MarkerA.y,
            MarkerB.x, MarkerB.y);
    }
#endif
}

void CBuildingMarkerOrb::RefreshBuildings()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    if (mBuildingAObject.expired())
    {
        mBuildingAObject =
            World->FindObject<CPlacementAreaObject>(mBuildingAName);
    }

    if (mBuildingBObject.expired())
    {
        mBuildingBObject =
            World->FindObject<CPlacementAreaObject>(mBuildingBName);
    }
}

void CBuildingMarkerOrb::UpdateScaleFromTileSize()
{
    if (mScaleInitialized)
        return;

    auto BuildingA = mBuildingAObject.lock();
    auto BuildingB = mBuildingBObject.lock();

    FVector2 TileSize;
    bool SizeFound = false;

    if (BuildingA)
    {
        SizeFound = BuildingA->GetTileSize(TileSize);
    }

    if (!SizeFound && BuildingB)
    {
        SizeFound = BuildingB->GetTileSize(TileSize);
    }

    if (!SizeFound)
        return;

    float Diameter = (std::min)(TileSize.x, TileSize.y) * 0.25f;

    if (Diameter < 1.f)
        Diameter = 1.f;

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

void CBuildingMarkerOrb::RequestMoveTo(
    const std::string& TargetBuildingName)
{
    auto Movement = mMovement.lock();

    if (!Movement || TargetBuildingName.empty())
        return;

#ifdef _DEBUG
    DebugOrbLog("[Orb] RequestMoveTo target=%s\n",
        TargetBuildingName.c_str());
#endif

    Movement->MovePathToObject(TargetBuildingName);
}
