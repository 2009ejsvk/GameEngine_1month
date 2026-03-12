#pragma once

#include "../Building/BuildingTypes.h"
#include "../Citizen/CitizenPolitics.h"
#include "../Citizen/CitizenSatisfaction.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace CitizenOrbAnimation
{
    constexpr int GDirectionCount = 8;

    constexpr int GAnimationDirByMoveDir[GDirectionCount] =
    {
        2, 1, 0, 7, 6, 5, 4, 3
    };

    constexpr const char* GWalkAnimationNamesBlue[GDirectionCount] =
    {
        "CitizenBlueWalk_Dir0",
        "CitizenBlueWalk_Dir1",
        "CitizenBlueWalk_Dir2",
        "CitizenBlueWalk_Dir3",
        "CitizenBlueWalk_Dir4",
        "CitizenBlueWalk_Dir5",
        "CitizenBlueWalk_Dir6",
        "CitizenBlueWalk_Dir7"
    };

    constexpr const char* GIdleAnimationNamesBlue[GDirectionCount] =
    {
        "CitizenBlueIdle_Dir0",
        "CitizenBlueIdle_Dir1",
        "CitizenBlueIdle_Dir2",
        "CitizenBlueIdle_Dir3",
        "CitizenBlueIdle_Dir4",
        "CitizenBlueIdle_Dir5",
        "CitizenBlueIdle_Dir6",
        "CitizenBlueIdle_Dir7"
    };

    constexpr const char* GWalkAnimationNamesRed[GDirectionCount] =
    {
        "CitizenRedWalk_Dir0",
        "CitizenRedWalk_Dir1",
        "CitizenRedWalk_Dir2",
        "CitizenRedWalk_Dir3",
        "CitizenRedWalk_Dir4",
        "CitizenRedWalk_Dir5",
        "CitizenRedWalk_Dir6",
        "CitizenRedWalk_Dir7"
    };

    constexpr const char* GIdleAnimationNamesRed[GDirectionCount] =
    {
        "CitizenRedIdle_Dir0",
        "CitizenRedIdle_Dir1",
        "CitizenRedIdle_Dir2",
        "CitizenRedIdle_Dir3",
        "CitizenRedIdle_Dir4",
        "CitizenRedIdle_Dir5",
        "CitizenRedIdle_Dir6",
        "CitizenRedIdle_Dir7"
    };
}

struct FCitizenWellbeingState
{
    FCitizenIdentityProfile IdentityProfile;
    FNpcSatisfaction Satisfaction;
    FNpcPoliticalProfile PoliticalProfile;
    float SatisfactionTickAccum = 0.f;
    float PoliticalTickAccum = 0.f;
    bool FoodStockAvailableThisVisit = false;
    bool FunServiceAvailableThisVisit = false;
    bool HealthServiceAvailableThisVisit = false;
    bool FaithServiceAvailableThisVisit = false;

    void InitializeDefaults()
    {
        Satisfaction.Food = 70.f;
        Satisfaction.Health = 70.f;
        Satisfaction.Fun = 70.f;
        Satisfaction.Faith = 70.f;
        Satisfaction.Housing = 70.f;
        Satisfaction.Job = 70.f;
        Satisfaction.CommuteTimePenalty = 0.f;
        Satisfaction.Freedom = 70.f;
        Satisfaction.Security = 70.f;
        RecalculateOverallSatisfaction();
        InitPoliticalProfile();
    }

    void InitPoliticalProfile()
    {
        CitizenPolitics::Init(PoliticalProfile, PoliticalTickAccum);
    }

    void UpdatePoliticalProfile(float DeltaTime)
    {
        CitizenPolitics::Update(
            PoliticalProfile,
            PoliticalTickAccum,
            DeltaTime,
            Satisfaction.Overall);
    }

    void ApplySatisfactionDelta(
        float FoodDelta,
        float HealthDelta,
        float FunDelta,
        float FaithDelta,
        float HousingDelta,
        float JobDelta,
        float FreedomDelta,
        float SecurityDelta)
    {
        auto ApplyDelta = [](float& Value, float Delta)
        {
            Value = (std::max)(0.f, (std::min)(100.f, Value + Delta));
        };

        ApplyDelta(Satisfaction.Food, FoodDelta);
        ApplyDelta(Satisfaction.Health, HealthDelta);
        ApplyDelta(Satisfaction.Fun, FunDelta);
        ApplyDelta(Satisfaction.Faith, FaithDelta);
        ApplyDelta(Satisfaction.Housing, HousingDelta);
        ApplyDelta(Satisfaction.Job, JobDelta);
        ApplyDelta(Satisfaction.Freedom, FreedomDelta);
        ApplyDelta(Satisfaction.Security, SecurityDelta);
        RecalculateOverallSatisfaction();
    }

    void RecalculateOverallSatisfaction()
    {
        CitizenSatisfaction::RecalculateOverall(Satisfaction);
    }

    void TickOverallRecalculation(float DeltaTime)
    {
        SatisfactionTickAccum += DeltaTime;

        if (SatisfactionTickAccum < 1.f)
            return;

        SatisfactionTickAccum = 0.f;
        RecalculateOverallSatisfaction();
    }
};

struct FTeamsterDeliveryState
{
    enum class ERouteMode
    {
        None = 0,
        Export,
        ConsumerDelivery
    };

    enum class ESourceReservationKind
    {
        None = 0,
        Exportable,
        Typed
    };

    ERouteMode Mode = ERouteMode::None;
    ESourceReservationKind SourceReservationKind =
        ESourceReservationKind::None;
    std::string SourceName;
    std::string DestinationName;
    int RequestedAmount = 0;
    EResourceType RequestedType = EResourceType::None;
    int CarryAmount = 0;
    EResourceType CargoType = EResourceType::None;
    bool DestinationReservationActive = false;
    bool SpeedBoostActive = false;

    void ClearCargo()
    {
        RequestedAmount = 0;
        RequestedType = EResourceType::None;
        CarryAmount = 0;
        CargoType = EResourceType::None;
    }

    void ClearRoute()
    {
        Mode = ERouteMode::None;
        SourceReservationKind = ESourceReservationKind::None;
        SourceName.clear();
        DestinationName.clear();
        DestinationReservationActive = false;
        ClearCargo();
    }
};

struct FOrbAnimationState
{
    int CurrentDirection = 0;
    bool WasMovingLastFrame = false;
    bool UseRedVariant = false;

    void ChooseRandomVariant()
    {
        UseRedVariant = (rand() % 2) == 1;
    }

    int ResolveDirectionIndexFromVelocity(const FVector3& Velocity) const
    {
        if (Velocity.IsZero())
            return CurrentDirection;

        float Angle = atan2f(Velocity.y, Velocity.x);
        const float Sector = 3.1415926535f / 4.f;
        int Direction = static_cast<int>(roundf(Angle / Sector));
        Direction = (Direction % CitizenOrbAnimation::GDirectionCount +
            CitizenOrbAnimation::GDirectionCount) %
            CitizenOrbAnimation::GDirectionCount;

        return Direction;
    }

    const char* GetIdleAnimationNameByDir(int Direction) const
    {
        if (Direction < 0 ||
            Direction >= CitizenOrbAnimation::GDirectionCount)
        {
            Direction = 0;
        }

        if (UseRedVariant)
            return CitizenOrbAnimation::GIdleAnimationNamesRed[Direction];

        return CitizenOrbAnimation::GIdleAnimationNamesBlue[Direction];
    }

    const char* GetWalkAnimationNameByDir(int Direction) const
    {
        if (Direction < 0 ||
            Direction >= CitizenOrbAnimation::GDirectionCount)
        {
            Direction = 0;
        }

        if (UseRedVariant)
            return CitizenOrbAnimation::GWalkAnimationNamesRed[Direction];

        return CitizenOrbAnimation::GWalkAnimationNamesBlue[Direction];
    }

    bool ResolveAnimationForVelocity(
        const FVector3& Velocity,
        const char*& OutAnimationName)
    {
        if (Velocity.IsZero())
        {
            if (!WasMovingLastFrame)
                return false;

            OutAnimationName = GetIdleAnimationNameByDir(CurrentDirection);
            WasMovingLastFrame = false;
            return true;
        }

        const int MoveDirection =
            ResolveDirectionIndexFromVelocity(Velocity);
        int AnimationDirection =
            CitizenOrbAnimation::GAnimationDirByMoveDir[MoveDirection];

        if (AnimationDirection < 0 ||
            AnimationDirection >= CitizenOrbAnimation::GDirectionCount)
        {
            AnimationDirection = MoveDirection;
        }

        if (!WasMovingLastFrame ||
            AnimationDirection != CurrentDirection)
        {
            CurrentDirection = AnimationDirection;
            OutAnimationName =
                GetWalkAnimationNameByDir(CurrentDirection);
            WasMovingLastFrame = true;
            return true;
        }

        WasMovingLastFrame = true;
        return false;
    }
};
