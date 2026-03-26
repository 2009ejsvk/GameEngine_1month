#pragma once

#include "Object/GameObject.h"
#include "../GameConstants.h"
#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenTypes.h"
#include "CitizenOrbComponents.h"
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

class CBuildingMarkerOrb :
    public CGameObject
{
    friend class CObject;

public:
    CBuildingMarkerOrb();
    CBuildingMarkerOrb(const CBuildingMarkerOrb& ref);
    CBuildingMarkerOrb(CBuildingMarkerOrb&& ref) noexcept;

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
    FVector3 mLockedTargetPos = FVector3::Zero;
    FVector3 mLastProgressPos = FVector3::Zero;
    FCitizenWellbeingState mCitizenProfileState;
    std::string mHomeName;
    std::string mWorkName;
    std::string mFoodName;
    std::string mFoodVisitBuildingName;
    std::string mFunName;
    std::string mFunVisitBuildingName;
    std::string mHealthName;
    std::string mHealthVisitBuildingName;
    std::string mFaithName;
    std::string mFaithVisitBuildingName;
    struct FCachedBuildingRef
    {
        std::string Name;
        std::weak_ptr<class CPlacementAreaObject> Ptr;
    };
    FCachedBuildingRef mHomeBuildingCached;
    FCachedBuildingRef mWorkBuildingCached;
    FCachedBuildingRef mFoodCapBuildingCached;
    FCachedBuildingRef mFunCapBuildingCached;
    FCachedBuildingRef mHealthCapBuildingCached;
    FCachedBuildingRef mFaithCapBuildingCached;
    bool mFoodVisitReserved = false;
    bool mFunVisitReserved = false;
    bool mHealthVisitReserved = false;
    bool mFaithVisitReserved = false;
    FTeamsterDeliveryState mTeamsterDeliveryState;
    ECitizenState mCitizenState = ECitizenState::Wander;
    ECitizenState mResumeStateAfterService = ECitizenState::GoingToWork;
    float mDwellTimer = 0.f;
    float mDefaultMoveSpeed = 200.f;
    float mMoveSpeedVarianceAlpha = 0.f;
    bool mHasConfiguredMoveSpeed = false;
    bool mHasLockedTarget = false;
    bool mWaitingForPath = false;
    bool mScaleInitialized = false;
    bool mHasStartPos = false;
    float mRuntimeTraceAccum = 0.f;
    FOrbAnimationState mAnimationState;
    float mSmoothedApproval = 50.f;
    bool mSmoothedApprovalInitialized = false;
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

    float GetSmoothedApproval() const
    {
        return mSmoothedApproval;
    }

    void UpdateSmoothedApproval(float TargetApproval)
    {
        if (!mSmoothedApprovalInitialized)
        {
            mSmoothedApproval = TargetApproval;
            mSmoothedApprovalInitialized = true;
            return;
        }

        constexpr float LerpRate = 0.08f;
        constexpr float MaxDecreasePerDay = 4.0f;
        constexpr float MaxIncreasePerDay = 3.0f;
        const float Delta = TargetApproval - mSmoothedApproval;
        const float MaxDelta = (Delta < 0.f) ? -MaxDecreasePerDay : MaxIncreasePerDay;
        const float ClampedDelta =
            (std::abs(Delta) > std::abs(MaxDelta)) ? MaxDelta : Delta;
        mSmoothedApproval += ClampedDelta * LerpRate;
        mSmoothedApproval =
            (std::max)(0.f, (std::min)(100.f, mSmoothedApproval));
    }

    float GetArrivalDistance() const
    {
        return mArrivalDistance;
    }

    const FNpcSatisfaction& GetSatisfaction() const
    {
        return mCitizenProfileState.Satisfaction;
    }

    const FNpcPoliticalProfile& GetPoliticalProfile() const
    {
        return mCitizenProfileState.PoliticalProfile;
    }

    const FCitizenIdentityProfile& GetIdentityProfile() const
    {
        return mCitizenProfileState.IdentityProfile;
    }

    void SetIdentityProfile(const FCitizenIdentityProfile& Profile)
    {
        mCitizenProfileState.IdentityProfile = Profile;
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
        const float Variance =
            (std::max)(0.f, GameConstants::Citizen::NpcMoveSpeedVariance);

        if (Variance > 0.001f)
        {
            mMoveSpeedVarianceAlpha =
                (Speed - GameConstants::Citizen::NpcBaseMoveSpeed) / Variance;
        }
        else
            mMoveSpeedVarianceAlpha = 0.f;

        mHasConfiguredMoveSpeed = true;
        RefreshMoveSpeedFromGameConstants();
    }

    void RefreshMoveSpeedFromGameConstants()
    {
        if (!mHasConfiguredMoveSpeed)
            return;

        const float Speed =
            GameConstants::Citizen::NpcBaseMoveSpeed +
            mMoveSpeedVarianceAlpha *
                (std::max)(0.f, GameConstants::Citizen::NpcMoveSpeedVariance);

        mDefaultMoveSpeed = Speed;
        mMoveSpeed = Speed;

        if (mTeamsterDeliveryState.SpeedBoostActive)
        {
            mMoveSpeed =
                mDefaultMoveSpeed *
                GameConstants::Orb::TeamsterSpeedMultiplier;
        }
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
    void SetHealthBuilding(const std::string& Name);
    void SetFaithBuilding(const std::string& Name);
    ECitizenState GetCitizenState() const { return mCitizenState; }
    bool IsConsumingFoodThisVisit() const
    {
        return mCitizenProfileState.FoodStockAvailableThisVisit;
    }
    const std::string& GetFoodVisitBuilding() const
    {
        return mFoodVisitBuildingName;
    }
    const std::string& GetFunVisitBuilding() const
    {
        return mFunVisitBuildingName;
    }
    const std::string& GetHealthVisitBuilding() const
    {
        return mHealthVisitBuildingName;
    }
    const std::string& GetFaithVisitBuilding() const
    {
        return mFaithVisitBuildingName;
    }
    const std::string& GetHomeBuilding() const { return mHomeName; }
    const std::string& GetWorkBuilding() const { return mWorkName; }
    const std::string& GetFoodBuilding() const { return mFoodName; }
    const std::string& GetFunBuilding() const { return mFunName; }
    const std::string& GetHealthBuilding() const { return mHealthName; }
    const std::string& GetFaithBuilding() const { return mFaithName; }
    bool HasCoreAssignments() const
    {
        return !mHomeName.empty() &&
            !mWorkName.empty() &&
            !mFoodName.empty();
    }

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Destroy();

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
    void ReleaseTeamsterReservations();
    void ReleaseServiceVisitReservations();
    bool TryStartTeamsterDelivery();
    bool TryRerouteTeamsterAfterDelivery();
    bool TryPlanTeamsterWarehouseBufferDelivery(
        const std::shared_ptr<class CPlacementAreaObject>& OfficeBuilding,
        FTeamsterDeliveryState& OutDelivery) const;
    bool TryPlanTeamsterConsumerDelivery(
        const std::shared_ptr<class CPlacementAreaObject>& OfficeBuilding,
        FTeamsterDeliveryState& OutDelivery,
        bool CriticalOnly,
        const std::string& ExcludeDestName = {}) const;
    bool TryPlanTeamsterExportDelivery(
        const std::shared_ptr<class CPlacementAreaObject>& OfficeBuilding,
        FTeamsterDeliveryState& OutDelivery,
        bool UrgentOnly = false) const;
    std::string FindTeamsterSourceName() const;
    std::string FindHarborName() const;
    std::string FindTeamsterExportDropoffName(
        const std::string& SourceName,
        EResourceType CargoType,
        int CargoAmount) const;
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
