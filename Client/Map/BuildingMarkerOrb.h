#pragma once

#include "Object/GameObject.h"
#include <string>

class CBuildingMarkerOrb :
    public CGameObject
{
    friend class CWorld;
    friend class CObject;

protected:
    CBuildingMarkerOrb();
    CBuildingMarkerOrb(const CBuildingMarkerOrb& ref);
    CBuildingMarkerOrb(CBuildingMarkerOrb&& ref) noexcept;

public:
    virtual ~CBuildingMarkerOrb();

private:
    std::weak_ptr<class CMeshComponent> mMeshComponent;
    std::weak_ptr<class CObjectMovementComponent> mMovement;
    std::weak_ptr<class CPlacementAreaObject> mBuildingAObject;
    std::weak_ptr<class CPlacementAreaObject> mBuildingBObject;
    std::string mBuildingAName = "BuildingA";
    std::string mBuildingBName = "BuildingB";
    std::string mCurrentTargetName;
    float mMoveSpeed = 200.f;
    float mArrivalDistance = 16.f;
    bool mScaleInitialized = false;
    bool mHasStartPos = false;
    bool mHasMarkerCache = false;
    FVector3 mLastMarkerA = FVector3::Zero;
    FVector3 mLastMarkerB = FVector3::Zero;
#ifdef _DEBUG
    bool mDebugMissingDependencyLogged = false;
    bool mDebugMissingMarkerLogged = false;
    float mDebugStatusLogAccum = 0.f;
#endif

public:
    void SetBuildingNames(
        const std::string& BuildingAName,
        const std::string& BuildingBName)
    {
        mBuildingAName = BuildingAName;
        mBuildingBName = BuildingBName;
        mBuildingAObject.reset();
        mBuildingBObject.reset();
        mCurrentTargetName.clear();
        mHasStartPos = false;
        mHasMarkerCache = false;
    }

    void SetMoveSpeed(float Speed)
    {
        mMoveSpeed = Speed;
    }

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void RefreshBuildings();
    void UpdateScaleFromTileSize();
    void RequestMoveTo(const std::string& TargetBuildingName);
};
