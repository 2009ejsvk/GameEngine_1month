#pragma once

#include "Object/GameObject.h"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

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
    std::weak_ptr<class CTileMapObject> mTileMapObject;
    std::string mBuildingAName = "BuildingA";
    std::string mBuildingBName = "BuildingB";
    std::vector<std::string> mRandomTargetNames;
    std::string mCurrentTargetName;
    float mMoveSpeed = 200.f;
    float mArrivalDistance = 16.f;
    bool mScaleInitialized = false;
    bool mHasStartPos = false;
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
        mRandomTargetNames.clear();

        if (!mBuildingAName.empty())
            mRandomTargetNames.push_back(mBuildingAName);

        if (!mBuildingBName.empty() &&
            mBuildingBName != mBuildingAName)
        {
            mRandomTargetNames.push_back(mBuildingBName);
        }

        mCurrentTargetName.clear();
        mHasStartPos = false;
    }

    void SetRandomTargetNames(
        const std::vector<std::string>& BuildingNames)
    {
        mRandomTargetNames.clear();

        for (size_t i = 0; i < BuildingNames.size(); ++i)
        {
            const std::string& Name = BuildingNames[i];

            if (Name.empty())
                continue;

            if (std::find(mRandomTargetNames.begin(),
                mRandomTargetNames.end(), Name) ==
                mRandomTargetNames.end())
            {
                mRandomTargetNames.push_back(Name);
            }
        }

        if (!mRandomTargetNames.empty())
        {
            mBuildingAName = mRandomTargetNames[0];

            if (mRandomTargetNames.size() >= 2)
                mBuildingBName = mRandomTargetNames[1];

            else
                mBuildingBName = mRandomTargetNames[0];
        }

        mCurrentTargetName.clear();
        mHasStartPos = false;
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
    bool CollectTargetMarkers(
        std::vector<std::pair<std::string, FVector3>>& OutMarkers);
    std::string PickRandomTargetName(
        const std::string& ExcludeName) const;
    void RequestMoveTo(const std::string& TargetBuildingName);
};
