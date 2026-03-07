#pragma once

#include "Object/GameObject.h"
#include "../Citizen/CitizenTypes.h"
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
    std::weak_ptr<class CAnimation2DComponent> mAnimation2DComponent;
    std::weak_ptr<class CObjectMovementComponent> mMovement;
    std::weak_ptr<class CTileMapObject> mTileMapObject;
    std::string mBuildingAName;
    std::string mBuildingBName;
    std::vector<std::string> mRandomTargetNames;
    std::string mCurrentTargetName;
    float mMoveSpeed = 200.f;
    float mArrivalDistance = 16.f;
    float mOrbDiameter = 20.f;
    float mAllowedOverlapRatio = 0.1f;
    float mSeparationStrength = 12.f;
    float mSeparationMaxSpeed = 180.f;
    float mPathRetryInterval = 1.f;
    float mPathRetryAccum = 0.f;
    float mWaitingPathAccum = 0.f;
    float mStallAccum = 0.f;
    float mSatisfactionTickAccum = 0.f;
    float mPoliticalTickAccum = 0.f;
    FVector3 mLockedTargetPos = FVector3::Zero;
    FVector3 mLastProgressPos = FVector3::Zero;
    FNpcSatisfaction mSatisfaction;
    FNpcPoliticalProfile mPoliticalProfile;
    std::string mHomeName;
    std::string mWorkName;
    std::string mFoodName;
    std::string mFoodVisitBuildingName;
    std::string mFunName;
    std::string mTeamsterSourceName;
    std::string mTeamsterHarborName;
    int mTeamsterCarryAmount = 0;
    ECitizenState mCitizenState = ECitizenState::Wander;
    ECitizenState mResumeStateAfterService = ECitizenState::GoingToWork;
    float mDwellTimer = 0.f;
    float mDefaultMoveSpeed = 200.f;
    bool mTeamsterSpeedBoostActive = false;
    static constexpr float GAtWorkDuration = 15.f;
    static constexpr float GAtHomeDuration = 10.f;
    static constexpr float GAtFoodDuration = 4.f;
    static constexpr float GAtFunDuration = 6.f;
    static constexpr float GFoodInterruptThreshold = 25.f;
    static constexpr float GFunInterruptThreshold = 30.f;
    static constexpr float GHealthRemoveThreshold = 5.f;
    static constexpr float GTeamsterSpeedMultiplier = 5.f;
    static constexpr int GTeamsterTransferUnit = 1000;
    static constexpr float GPoliticalShiftInterval = 12.f;
    bool mHasLockedTarget = false;
    bool mWaitingForPath = false;
    bool mScaleInitialized = false;
    bool mHasStartPos = false;
    bool mFoodStockAvailableThisVisit = false;
    float mRuntimeTraceAccum = 0.f;
    int mCurrentAnimationDirection = 0;
    bool mWasMovingLastFrame = false;
    bool mUseRedVariant = false;
#ifdef _DEBUG
    bool mDebugMissingDependencyLogged = false;
    bool mDebugMissingMarkerLogged = false;
    float mDebugStatusLogAccum = 0.f;
#endif

public:
    float GetOrbDiameter() const
    {
        return mOrbDiameter;
    }

    float GetArrivalDistance() const
    {
        return mArrivalDistance;
    }

    const FNpcSatisfaction& GetSatisfaction() const
    {
        return mSatisfaction;
    }

    const FNpcPoliticalProfile& GetPoliticalProfile() const
    {
        return mPoliticalProfile;
    }

    void ApplySatisfactionDelta(
        float FoodDelta,
        float HealthDelta,
        float FunDelta,
        float FaithDelta,
        float HousingDelta,
        float JobDelta,
        float FreedomDelta,
        float SecurityDelta);

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
        mHasLockedTarget = false;
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
        mStallAccum = 0.f;
    }

    void SetRandomTargetNames(
        const std::vector<std::string>& BuildingNames)
    {
        mRandomTargetNames.clear();
        mBuildingAName.clear();
        mBuildingBName.clear();

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
        mHasLockedTarget = false;
        mWaitingForPath = false;
        mWaitingPathAccum = 0.f;
        mStallAccum = 0.f;
    }

    void SetMoveSpeed(float Speed)
    {
        mDefaultMoveSpeed = Speed;
        mMoveSpeed = Speed;

        if (mTeamsterSpeedBoostActive)
            mMoveSpeed = mDefaultMoveSpeed * GTeamsterSpeedMultiplier;
    }

    void AddTargetBuildingName(const std::string& BuildingName)
    {
        if (BuildingName.empty())
            return;

        if (std::find(mRandomTargetNames.begin(),
            mRandomTargetNames.end(), BuildingName) ==
            mRandomTargetNames.end())
        {
            mRandomTargetNames.push_back(BuildingName);
        }

        if (mBuildingAName.empty())
        {
            mBuildingAName = BuildingName;
        }
        else if (mBuildingBName.empty() &&
            mBuildingAName != BuildingName)
        {
            mBuildingBName = BuildingName;
        }
    }

    void RemoveTargetBuildingName(const std::string& BuildingName);

    void SetHomeBuilding(const std::string& Name);
    void SetWorkBuilding(const std::string& Name);
    void SetFoodBuilding(const std::string& Name);
    void SetFunBuilding(const std::string& Name);
    ECitizenState GetCitizenState() const { return mCitizenState; }
    bool IsConsumingFoodThisVisit() const
    {
        return mFoodStockAvailableThisVisit;
    }
    const std::string& GetFoodVisitBuilding() const
    {
        return mFoodVisitBuildingName;
    }
    const std::string& GetHomeBuilding() const { return mHomeName; }
    const std::string& GetWorkBuilding() const { return mWorkName; }
    const std::string& GetFoodBuilding() const { return mFoodName; }
    const std::string& GetFunBuilding() const { return mFunName; }
    bool HasCoreAssignments() const
    {
        return !mHomeName.empty() &&
            !mWorkName.empty() &&
            !mFoodName.empty();
    }

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void UpdateSatisfaction(float DeltaTime);
    void InitPoliticalProfile();
    void UpdatePoliticalProfile(float DeltaTime);
    void TransitionFsm(ECitizenState NewState);
    void RecalculateOverallSatisfaction();
    void ApplySoftSeparation(float DeltaTime);
    void RefreshBuildings();
    void UpdateScaleFromTileSize();
    bool TryGetFallbackStartPos(FVector3& OutPos);
    bool CollectTargetMarkers(
        std::vector<std::pair<std::string, FVector3>>& OutMarkers);
    std::string PickRandomTargetName(
        const std::string& ExcludeName) const;
    void RequestMoveTo(const std::string& TargetBuildingName);
    void TryStartCoreLoop();
    bool IsTeamsterState(ECitizenState State) const;
    void StartTeamsterSpeedBoost();
    void ResetTeamsterSpeed();
    bool TryStartTeamsterDelivery();
    std::string FindTeamsterSourceName() const;
    std::string FindHarborName() const;
    void UpdateSpriteAnimationFromVelocity(const FVector3& Velocity);
    int ResolveDirectionIndexFromVelocity(const FVector3& Velocity) const;
    const char* GetIdleAnimationNameByDir(int Direction) const;
    const char* GetWalkAnimationNameByDir(int Direction) const;

    // Update() 분해 — FSM 헬퍼
    void CancelCurrentPath();
    ECitizenState NormalizeResumeState(ECitizenState State) const;
    ECitizenState ResolveStateAfterService() const;
    bool TryInterruptByNeed();
    std::string ResolveTargetByState() const;

    // Update() 분해 — 대형 블록
    bool TickDwellState(float DeltaTime);
    bool TickInitialPosition(float CurrentZ);
    void HandleMissingTarget();
    void HandleArrival(float Dist);
};
