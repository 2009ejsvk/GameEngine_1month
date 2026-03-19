#include "CitizenSystem.h"
#include "World/World.h"
#include "../World/MainWorldSystemAccess.h"
#include "../GameConstants.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    struct FPendingHouseholdSeed
    {
        int HouseholdId = -1;
        int HouseholdSize = 0;
        int AssignedMembers = 0;
    };

    int GNextCitizenHouseholdId = 1;
    FPendingHouseholdSeed GPendingHouseholdSeeds[GCitizenWealthLevelCount] = {};

    float Clamp01(float Value)
    {
        return (std::max)(0.f, (std::min)(1.f, Value));
    }

    bool IsAssignablePlacedBuilding(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building &&
            Building->GetAlive() &&
            Building->GetEnable() &&
            Building->HasPlacedArea() &&
            !Building->IsRoad() &&
            !Building->IsBusStop();
    }

    float ResolveSatisfactionScore(int Cap)
    {
        return Clamp01(static_cast<float>(Cap) / 100.f);
    }

    float ResolveAccessibilityScore(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        return Building ? Clamp01(Building->GetAccessibilityScore()) : 0.5f;
    }

    int ResolveServiceAssignmentCapacity(
        const std::shared_ptr<CPlacementAreaObject>& Building,
        EBuildingServiceType Type)
    {
        if (!Building)
            return 0;

        const int ServiceCapacity = Building->GetServiceVisitCapacity(Type);

        if (ServiceCapacity > 0)
            return ServiceCapacity;

        return (std::max)(0, Building->GetCapacity());
    }

    unsigned int ResolveCitizenWealthMask(ECitizenWealthLevel WealthLevel)
    {
        switch (WealthLevel)
        {
        case ECitizenWealthLevel::Broke:
            return GBuildingWealthMaskBroke;
        case ECitizenWealthLevel::Poor:
            return GBuildingWealthMaskPoor;
        case ECitizenWealthLevel::WellOff:
            return GBuildingWealthMaskWellOff;
        case ECitizenWealthLevel::Rich:
            return GBuildingWealthMaskRich;
        case ECitizenWealthLevel::FilthyRich:
            return GBuildingWealthMaskFilthyRich;
        default:
            return GBuildingWealthMaskBroke;
        }
    }

    bool DoesBuildingAllowCitizenWealth(
        unsigned int AllowedWealthMask,
        ECitizenWealthLevel WealthLevel)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;
        return (EffectiveMask & ResolveCitizenWealthMask(WealthLevel)) != 0;
    }

    int ResolveHouseholdAssignmentCapacity(
        const std::shared_ptr<CPlacementAreaObject>& Building)
    {
        if (!Building)
            return 0;

        const int HouseholdCapacity =
            (std::max)(0, Building->GetHouseholdCapacity());

        if (HouseholdCapacity > 0)
            return HouseholdCapacity;

        return (std::max)(0, Building->GetCapacity());
    }

    bool HasValidCitizenHouseholdProfile(
        const FCitizenIdentityProfile& Profile)
    {
        return Profile.HouseholdId > 0 && Profile.HouseholdSize > 0;
    }

    int ResolveDesiredHouseholdSize(ECitizenWealthLevel WealthLevel)
    {
        const int Roll = rand() % 100;

        switch (WealthLevel)
        {
        case ECitizenWealthLevel::FilthyRich:
            if (Roll < 72)
                return 1;
            return 2;
        case ECitizenWealthLevel::Rich:
            if (Roll < 56)
                return 1;
            if (Roll < 91)
                return 2;
            return 3;
        case ECitizenWealthLevel::WellOff:
            if (Roll < 32)
                return 1;
            if (Roll < 71)
                return 2;
            if (Roll < 94)
                return 3;
            return 4;
        case ECitizenWealthLevel::Poor:
            if (Roll < 18)
                return 1;
            if (Roll < 53)
                return 2;
            if (Roll < 82)
                return 3;
            return 4;
        case ECitizenWealthLevel::Broke:
        default:
            if (Roll < 8)
                return 1;
            if (Roll < 30)
                return 2;
            if (Roll < 64)
                return 3;
            if (Roll < 89)
                return 4;
            return 5;
        }
    }

    void AssignCitizenHouseholdProfile(FCitizenIdentityProfile& Profile)
    {
        int WealthIndex = static_cast<int>(Profile.WealthLevel);

        if (WealthIndex < 0 || WealthIndex >= GCitizenWealthLevelCount)
            WealthIndex = 0;

        FPendingHouseholdSeed& Seed = GPendingHouseholdSeeds[WealthIndex];

        if (Seed.HouseholdId <= 0 ||
            Seed.HouseholdSize <= 0 ||
            Seed.AssignedMembers >= Seed.HouseholdSize)
        {
            Seed.HouseholdId = GNextCitizenHouseholdId++;
            Seed.HouseholdSize = ResolveDesiredHouseholdSize(Profile.WealthLevel);
            Seed.AssignedMembers = 0;
        }

        Profile.HouseholdId = Seed.HouseholdId;
        Profile.HouseholdSize = (std::max)(1, Seed.HouseholdSize);
        ++Seed.AssignedMembers;

        if (Seed.AssignedMembers >= Seed.HouseholdSize)
        {
            Seed.HouseholdId = -1;
            Seed.HouseholdSize = 0;
            Seed.AssignedMembers = 0;
        }
    }

    void AssignTouristHouseholdProfile(FCitizenIdentityProfile& Profile)
    {
        Profile.HouseholdId = GNextCitizenHouseholdId++;
        Profile.HouseholdSize = 1;
    }

    bool IsTourismVenueCategory(EBuildingCategory Category)
    {
        return Category == EBuildingCategory::Tourism ||
            Category == EBuildingCategory::LuxuryEntertainment;
    }

    ETouristPreference ResolveRandomTouristPreference()
    {
        const int Roll = rand() % 100;

        if (Roll < 24)
            return ETouristPreference::Relaxation;
        if (Roll < 44)
            return ETouristPreference::Cultural;
        if (Roll < 60)
            return ETouristPreference::Backpacker;
        if (Roll < 76)
            return ETouristPreference::Family;
        if (Roll < 90)
            return ETouristPreference::ThrillSeeker;
        return ETouristPreference::Celebrity;
    }

    FCitizenIdentityProfile BuildTouristIdentityProfile()
    {
        FCitizenIdentityProfile Profile;
        Profile.IsImmigrant = true;
        Profile.IsTourist = true;
        Profile.TouristProfile = ResolveRandomTouristPreference();

        const int EducationRoll = rand() % 100;
        const int WealthRoll = rand() % 100;

        switch (Profile.TouristProfile)
        {
        case ETouristPreference::Backpacker:
            Profile.EducationLevel =
                EducationRoll < 62 ?
                    ECitizenEducationLevel::HighSchool :
                    ECitizenEducationLevel::College;
            Profile.WealthLevel =
                WealthRoll < 28 ?
                    ECitizenWealthLevel::Broke :
                WealthRoll < 84 ?
                    ECitizenWealthLevel::Poor :
                    ECitizenWealthLevel::WellOff;
            break;
        case ETouristPreference::Celebrity:
            Profile.EducationLevel =
                EducationRoll < 68 ?
                    ECitizenEducationLevel::College :
                    ECitizenEducationLevel::HighSchool;
            Profile.WealthLevel =
                WealthRoll < 14 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
            break;
        case ETouristPreference::Cultural:
            Profile.EducationLevel =
                EducationRoll < 48 ?
                    ECitizenEducationLevel::College :
                    ECitizenEducationLevel::HighSchool;
            Profile.WealthLevel =
                WealthRoll < 8 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 62 ?
                    ECitizenWealthLevel::WellOff :
                WealthRoll < 88 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
            break;
        case ETouristPreference::Family:
            Profile.EducationLevel =
                EducationRoll < 18 ?
                    ECitizenEducationLevel::College :
                    ECitizenEducationLevel::HighSchool;
            Profile.WealthLevel =
                WealthRoll < 10 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 68 ?
                    ECitizenWealthLevel::WellOff :
                WealthRoll < 92 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
            break;
        case ETouristPreference::ThrillSeeker:
            Profile.EducationLevel =
                EducationRoll < 32 ?
                    ECitizenEducationLevel::College :
                    ECitizenEducationLevel::HighSchool;
            Profile.WealthLevel =
                WealthRoll < 12 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 66 ?
                    ECitizenWealthLevel::WellOff :
                WealthRoll < 90 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
            break;
        case ETouristPreference::Relaxation:
        default:
            Profile.EducationLevel =
                EducationRoll < 36 ?
                    ECitizenEducationLevel::College :
                    ECitizenEducationLevel::HighSchool;
            Profile.WealthLevel =
                WealthRoll < 6 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 58 ?
                    ECitizenWealthLevel::WellOff :
                WealthRoll < 86 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
            break;
        }

        AssignTouristHouseholdProfile(Profile);
        return Profile;
    }

    int ResolveTourismVenueScore(
        CWorld* World,
        const std::vector<std::string>& BuildingNames)
    {
        if (!World)
            return 0;

        int Score = 0;

        for (size_t Index = 0; Index < BuildingNames.size(); ++Index)
        {
            auto Building =
                World->FindObject<CPlacementAreaObject>(BuildingNames[Index]).
                    lock();

            if (!IsAssignablePlacedBuilding(Building))
                continue;

            if (IsTourismVenueCategory(Building->GetBuildingCategory()))
                Score += 2;
            else if (
                HasTouristPreference(Building->GetPrimaryTouristPreference()))
            {
                ++Score;
            }
        }

        return Score;
    }

    float ResolveTouristPreferenceMatchScore(
        const FCitizenIdentityProfile& Identity,
        EBuildingCategory Category,
        EBuildingLeisureClass LeisureClass,
        ETouristPreference Preference)
    {
        float Score = 0.f;

        if (!Identity.IsTourist)
        {
            if (IsTourismVenueCategory(Category))
                Score -= 0.04f;
            if (HasTouristPreference(Preference))
                Score -= 0.03f;
            return Score;
        }

        if (IsTourismVenueCategory(Category))
            Score += 0.12f;

        if (Preference == Identity.TouristProfile)
            Score += 0.26f;
        else if (HasTouristPreference(Preference))
            Score -= 0.08f;

        switch (Identity.TouristProfile)
        {
        case ETouristPreference::Cultural:
            if (LeisureClass == EBuildingLeisureClass::Cultural)
                Score += 0.08f;
            break;
        case ETouristPreference::Family:
            if (LeisureClass == EBuildingLeisureClass::General)
                Score += 0.06f;
            break;
        case ETouristPreference::Backpacker:
            if (Category == EBuildingCategory::Entertainment)
                Score += 0.05f;
            if (Category == EBuildingCategory::LuxuryEntertainment)
                Score -= 0.10f;
            break;
        case ETouristPreference::Relaxation:
            if (Category == EBuildingCategory::Tourism)
                Score += 0.08f;
            break;
        case ETouristPreference::ThrillSeeker:
            if (Category == EBuildingCategory::Entertainment ||
                Category == EBuildingCategory::Tourism)
            {
                Score += 0.06f;
            }
            break;
        case ETouristPreference::Celebrity:
            if (LeisureClass == EBuildingLeisureClass::Luxury ||
                Category == EBuildingCategory::LuxuryEntertainment)
            {
                Score += 0.12f;
            }
            break;
        default:
            break;
        }

        return Score;
    }

    float ResolveDistanceAffinity(
        const std::shared_ptr<CPlacementAreaObject>& FromBuilding,
        const std::shared_ptr<CPlacementAreaObject>& ToBuilding)
    {
        if (!FromBuilding || !ToBuilding)
            return -1.f;

        int FromX = 0;
        int FromY = 0;
        int ToX = 0;
        int ToY = 0;

        if (!FromBuilding->GetPlacedCenterGridCoords(FromX, FromY) ||
            !ToBuilding->GetPlacedCenterGridCoords(ToX, ToY))
        {
            return -1.f;
        }

        const float Dx = static_cast<float>(FromX - ToX);
        const float Dy = static_cast<float>(FromY - ToY);
        const float Distance = sqrtf(Dx * Dx + Dy * Dy);
        return 1.f / (1.f + Distance / 12.f);
    }

    float ResolveAverageDistanceAffinity(
        const std::shared_ptr<CPlacementAreaObject>& Candidate,
        const std::shared_ptr<CPlacementAreaObject>& RefA,
        const std::shared_ptr<CPlacementAreaObject>& RefB = nullptr,
        const std::shared_ptr<CPlacementAreaObject>& RefC = nullptr)
    {
        float TotalScore = 0.f;
        int ReferenceCount = 0;

        const float ScoreA = ResolveDistanceAffinity(Candidate, RefA);

        if (ScoreA >= 0.f)
        {
            TotalScore += ScoreA;
            ++ReferenceCount;
        }

        const float ScoreB = ResolveDistanceAffinity(Candidate, RefB);

        if (ScoreB >= 0.f)
        {
            TotalScore += ScoreB;
            ++ReferenceCount;
        }

        const float ScoreC = ResolveDistanceAffinity(Candidate, RefC);

        if (ScoreC >= 0.f)
        {
            TotalScore += ScoreC;
            ++ReferenceCount;
        }

        return ReferenceCount > 0 ?
            TotalScore / static_cast<float>(ReferenceCount) :
            0.5f;
    }

    float ResolveCongestionScore(
        int OccupancyAfterAssignment,
        int Capacity)
    {
        if (Capacity <= 0)
            return -1.f;

        const float Ratio =
            static_cast<float>(OccupancyAfterAssignment) /
            static_cast<float>(Capacity);

        if (Ratio <= 1.f)
            return 1.f - Ratio;

        return (std::max)(-1.f, 1.f - Ratio * 1.5f);
    }

    FCitizenIdentityProfile BuildCitizenIdentityProfile()
    {
        FCitizenIdentityProfile Profile;
        Profile.IsTourist = false;
        Profile.TouristProfile = ETouristPreference::None;
        const int EducationRoll = rand() % 100;

        if (EducationRoll < 62)
            Profile.EducationLevel = ECitizenEducationLevel::Uneducated;
        else if (EducationRoll < 88)
            Profile.EducationLevel = ECitizenEducationLevel::HighSchool;
        else
            Profile.EducationLevel = ECitizenEducationLevel::College;

        const int WealthRoll = rand() % 100;

        if (Profile.EducationLevel == ECitizenEducationLevel::College)
        {
            Profile.WealthLevel =
                WealthRoll < 10 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 52 ?
                    ECitizenWealthLevel::WellOff :
                WealthRoll < 90 ?
                    ECitizenWealthLevel::Rich :
                    ECitizenWealthLevel::FilthyRich;
        }
        else if (Profile.EducationLevel ==
            ECitizenEducationLevel::HighSchool)
        {
            Profile.WealthLevel =
                WealthRoll < 18 ?
                    ECitizenWealthLevel::Broke :
                WealthRoll < 64 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 94 ?
                    ECitizenWealthLevel::WellOff :
                    ECitizenWealthLevel::Rich;
        }
        else
        {
            Profile.WealthLevel =
                WealthRoll < 28 ?
                    ECitizenWealthLevel::Broke :
                WealthRoll < 82 ?
                    ECitizenWealthLevel::Poor :
                WealthRoll < 98 ?
                    ECitizenWealthLevel::WellOff :
                    ECitizenWealthLevel::Rich;
        }

        AssignCitizenHouseholdProfile(Profile);
        Profile.IsImmigrant = true;
        return Profile;
    }

    std::string PickRandomBuildingName(const std::vector<std::string>& Names)
    {
        if (Names.empty())
            return std::string();

        return Names[rand() % Names.size()];
    }

    void CollectCurrentBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            const std::string& Name = B->GetName();

            if (Name.empty())
                continue;

            if (std::find(Out.begin(), Out.end(), Name) == Out.end())
                Out.push_back(Name);
        }
    }

    void CollectHomeBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            if (B->IsResidential())
                Out.push_back(B->GetName());
        }
    }

    void CollectWorkBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            // FoodProvider 건물은 직장 겸 음식 생산지로 포함
            // EntertainmentProvider 전용 건물(주점 등)은 제외
            if (!B->IsResidential() &&
                B->GetCapacity() > 0 &&
                (!B->IsEntertainmentProvider() || B->IsFoodProvider()))
            {
                Out.push_back(B->GetName());
            }
        }
    }

    void CollectFoodBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            if (B->IsFoodProvider())
                Out.push_back(B->GetName());
        }
    }

    void CollectEntertainmentBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            if (B->IsEntertainmentProvider())
                Out.push_back(B->GetName());
        }
    }

    void CollectHealthBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            if (B->IsHealthProvider())
                Out.push_back(B->GetName());
        }
    }

    void CollectFaithBuildingNames(
        CWorld* World,
        std::vector<std::string>& Out)
    {
        Out.clear();

        std::vector<std::weak_ptr<CPlacementAreaObject>> List;

        if (!World->FindObjectListByType<CPlacementAreaObject>(List))
            return;

        for (size_t i = 0; i < List.size(); ++i)
        {
            auto B = List[i].lock();

            if (!B ||
                !B->GetAlive() ||
                !B->HasPlacedArea() ||
                B->IsRoad() ||
                B->IsBusStop())
            {
                continue;
            }

            if (B->IsFaithProvider())
                Out.push_back(B->GetName());
        }
    }
}

    void CitizenSystem::SpawnCitizenOrb(CWorld* World, int& SpawnedNpcCount)
{
    const int OrbIndex = SpawnedNpcCount;
    const std::string OrbName = (OrbIndex == 0) ?
        "BuildingMarkerOrb" :
        "BuildingMarkerOrb" + std::to_string(OrbIndex + 1);

    auto MarkerOrb = World->CreateGameObject<CBuildingMarkerOrb>(OrbName);
    auto MarkerOrbObj = MarkerOrb.lock();
    const IMainWorldCitizenPolicyAccess* MainWorldAccess =
        ResolveMainWorldCitizenPolicyAccess(World);

    if (!MarkerOrbObj)
        return;

    std::vector<std::string> AllNames;
    std::vector<std::string> HomeNames;
    std::vector<std::string> WorkNames;
    std::vector<std::string> FoodNames;
    std::vector<std::string> FunNames;
    std::vector<std::string> HealthNames;
    std::vector<std::string> FaithNames;
    CollectCurrentBuildingNames(World, AllNames);
    CollectHomeBuildingNames(World, HomeNames);
    CollectWorkBuildingNames(World, WorkNames);
    CollectFoodBuildingNames(World, FoodNames);
    CollectEntertainmentBuildingNames(World, FunNames);
    CollectHealthBuildingNames(World, HealthNames);
    CollectFaithBuildingNames(World, FaithNames);

    const int TourismVenueScore =
        ResolveTourismVenueScore(World, AllNames);
    const int TouristChancePercent =
        TourismVenueScore > 0 ?
            (std::min)(18, 3 + TourismVenueScore * 2) :
            0;
    const int TouristChanceBonusPercent = MainWorldAccess ?
        MainWorldAccess->GetEdictModifiers().TouristChanceBonusPercent :
        0;
    const int FinalTouristChancePercent = (std::max)(
        0,
        (std::min)(
            42,
            TouristChancePercent + TouristChanceBonusPercent));
    MarkerOrbObj->SetIdentityProfile(
        FinalTouristChancePercent > 0 &&
            (rand() % 100) < FinalTouristChancePercent ?
                BuildTouristIdentityProfile() :
                BuildCitizenIdentityProfile());

    MarkerOrbObj->SetRandomTargetNames(AllNames);

    if (!HomeNames.empty() &&
        !WorkNames.empty() &&
        !FoodNames.empty())
    {
        MarkerOrbObj->SetHomeBuilding(PickRandomBuildingName(HomeNames));
        MarkerOrbObj->SetWorkBuilding(PickRandomBuildingName(WorkNames));
        MarkerOrbObj->SetFoodBuilding(PickRandomBuildingName(FoodNames));
    }

    if (!FunNames.empty())
        MarkerOrbObj->SetFunBuilding(PickRandomBuildingName(FunNames));

    if (!HealthNames.empty())
        MarkerOrbObj->SetHealthBuilding(PickRandomBuildingName(HealthNames));

    if (!FaithNames.empty())
        MarkerOrbObj->SetFaithBuilding(PickRandomBuildingName(FaithNames));

    const float Speed = GameConstants::Citizen::NpcBaseMoveSpeed +
        ((float)(rand() % 1001) / 500.f - 1.f) *
            GameConstants::Citizen::NpcMoveSpeedVariance;
    MarkerOrbObj->SetMoveSpeed(Speed);
    ++SpawnedNpcCount;
}

void CitizenSystem::ReassignCitizenNeeds(CWorld* World)
{
    struct FHomeBuildingInfo
    {
        std::string Name;
        std::shared_ptr<CPlacementAreaObject> Building;
        int Capacity = 0;
        int HousingCap = 0;
        unsigned int AllowedWealthMask = GBuildingWealthMaskAll;
        int Assigned = 0;
        float Accessibility = 0.f;
    };

    struct FHouseholdInfo
    {
        int HouseholdId = -1;
        int HouseholdSize = 1;
        ECitizenWealthLevel WealthLevel = ECitizenWealthLevel::Poor;
        std::vector<int> MemberOrbIndices;
        int CurrentHomeIndex = -1;
    };

    struct FServiceBuildingInfo
    {
        std::string Name;
        std::shared_ptr<CPlacementAreaObject> Building;
        int SatisfactionCap = 0;
        int VisitCapacity = 0;
        int ActiveVisitors = 0;
        unsigned int AllowedWealthMask = GBuildingWealthMaskAll;
        EBuildingCategory Category = EBuildingCategory::Infrastructure;
        EBuildingLeisureClass LeisureClass = EBuildingLeisureClass::None;
        ETouristPreference PrimaryTouristPreference =
            ETouristPreference::None;
        int Assigned = 0;
        float Accessibility = 0.f;
    };

    struct FWorkBuildingInfo
    {
        std::string Name;
        std::shared_ptr<CPlacementAreaObject> Building;
        int Capacity = 0;
        int JobCap = 0;
        int Occupied = 0;
        int StandingNow = 0;
        int MinRequired = 0;
        ECitizenEducationLevel RequiredEducation =
            ECitizenEducationLevel::Uneducated;
        bool IsFoodProvider = false;
        float Accessibility = 0.f;
    };

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    std::vector<std::string> AllNames;
    std::vector<std::string> HomeNames;
    std::vector<std::string> WorkNames;
    std::vector<std::string> FoodNames;
    std::vector<std::string> FunNames;
    std::vector<std::string> HealthNames;
    std::vector<std::string> FaithNames;
    CollectCurrentBuildingNames(World, AllNames);
    CollectHomeBuildingNames(World, HomeNames);
    CollectWorkBuildingNames(World, WorkNames);
    CollectFoodBuildingNames(World, FoodNames);
    CollectEntertainmentBuildingNames(World, FunNames);
    CollectHealthBuildingNames(World, HealthNames);
    CollectFaithBuildingNames(World, FaithNames);

    std::vector<FHomeBuildingInfo> HomeInfos;
    HomeInfos.reserve(HomeNames.size());

    for (size_t i = 0; i < HomeNames.size(); ++i)
    {
        auto HomeBuilding =
            World->FindObject<CPlacementAreaObject>(HomeNames[i]).lock();
        const int HouseholdCapacity =
            ResolveHouseholdAssignmentCapacity(HomeBuilding);

        if (!IsAssignablePlacedBuilding(HomeBuilding) ||
            !HomeBuilding->IsResidential() ||
            HouseholdCapacity <= 0)
        {
            continue;
        }

        FHomeBuildingInfo Info;
        Info.Name = HomeNames[i];
        Info.Building = HomeBuilding;
        Info.Capacity = HouseholdCapacity;
        Info.HousingCap = HomeBuilding->GetHousingSatisfactionCap();
        Info.AllowedWealthMask = HomeBuilding->GetAllowedWealthMask();
        Info.Accessibility = ResolveAccessibilityScore(HomeBuilding);
        HomeInfos.push_back(std::move(Info));
    }

    std::vector<FServiceBuildingInfo> FoodInfos;
    FoodInfos.reserve(FoodNames.size());

    for (size_t i = 0; i < FoodNames.size(); ++i)
    {
        auto FoodBuilding =
            World->FindObject<CPlacementAreaObject>(FoodNames[i]).lock();

        if (!IsAssignablePlacedBuilding(FoodBuilding) ||
            !FoodBuilding->IsFoodProvider())
        {
            continue;
        }

        const int VisitCapacity =
            ResolveServiceAssignmentCapacity(
                FoodBuilding,
                EBuildingServiceType::Food);

        if (VisitCapacity <= 0)
            continue;

        FServiceBuildingInfo Info;
        Info.Name = FoodNames[i];
        Info.Building = FoodBuilding;
        Info.SatisfactionCap = FoodBuilding->GetFoodSatisfactionCap();
        Info.VisitCapacity = VisitCapacity;
        Info.ActiveVisitors =
            FoodBuilding->GetActiveServiceVisitorCount(
                EBuildingServiceType::Food);
        Info.AllowedWealthMask = FoodBuilding->GetAllowedWealthMask();
        Info.Category = FoodBuilding->GetBuildingCategory();
        Info.LeisureClass = FoodBuilding->GetLeisureClass();
        Info.PrimaryTouristPreference =
            FoodBuilding->GetPrimaryTouristPreference();
        Info.Accessibility = ResolveAccessibilityScore(FoodBuilding);
        FoodInfos.push_back(std::move(Info));
    }

    std::vector<FServiceBuildingInfo> FunInfos;
    FunInfos.reserve(FunNames.size());

    for (size_t i = 0; i < FunNames.size(); ++i)
    {
        auto FunBuilding =
            World->FindObject<CPlacementAreaObject>(FunNames[i]).lock();

        if (!IsAssignablePlacedBuilding(FunBuilding) ||
            !FunBuilding->IsEntertainmentProvider())
        {
            continue;
        }

        const int VisitCapacity =
            ResolveServiceAssignmentCapacity(
                FunBuilding,
                EBuildingServiceType::Fun);

        if (VisitCapacity <= 0)
            continue;

        FServiceBuildingInfo Info;
        Info.Name = FunNames[i];
        Info.Building = FunBuilding;
        Info.SatisfactionCap = FunBuilding->GetFunSatisfactionCap();
        Info.VisitCapacity = VisitCapacity;
        Info.ActiveVisitors =
            FunBuilding->GetActiveServiceVisitorCount(
                EBuildingServiceType::Fun);
        Info.AllowedWealthMask = FunBuilding->GetAllowedWealthMask();
        Info.Category = FunBuilding->GetBuildingCategory();
        Info.LeisureClass = FunBuilding->GetLeisureClass();
        Info.PrimaryTouristPreference =
            FunBuilding->GetPrimaryTouristPreference();
        Info.Accessibility = ResolveAccessibilityScore(FunBuilding);
        FunInfos.push_back(std::move(Info));
    }

    std::vector<FServiceBuildingInfo> HealthInfos;
    HealthInfos.reserve(HealthNames.size());

    for (size_t i = 0; i < HealthNames.size(); ++i)
    {
        auto HealthBuilding =
            World->FindObject<CPlacementAreaObject>(HealthNames[i]).lock();

        if (!IsAssignablePlacedBuilding(HealthBuilding) ||
            !HealthBuilding->IsHealthProvider())
        {
            continue;
        }

        const int VisitCapacity =
            ResolveServiceAssignmentCapacity(
                HealthBuilding,
                EBuildingServiceType::Health);

        if (VisitCapacity <= 0)
            continue;

        FServiceBuildingInfo Info;
        Info.Name = HealthNames[i];
        Info.Building = HealthBuilding;
        Info.SatisfactionCap = HealthBuilding->GetHealthSatisfactionCap();
        Info.VisitCapacity = VisitCapacity;
        Info.ActiveVisitors =
            HealthBuilding->GetActiveServiceVisitorCount(
                EBuildingServiceType::Health);
        Info.AllowedWealthMask = HealthBuilding->GetAllowedWealthMask();
        Info.Category = HealthBuilding->GetBuildingCategory();
        Info.LeisureClass = HealthBuilding->GetLeisureClass();
        Info.PrimaryTouristPreference =
            HealthBuilding->GetPrimaryTouristPreference();
        Info.Accessibility = ResolveAccessibilityScore(HealthBuilding);
        HealthInfos.push_back(std::move(Info));
    }

    std::vector<FServiceBuildingInfo> FaithInfos;
    FaithInfos.reserve(FaithNames.size());

    for (size_t i = 0; i < FaithNames.size(); ++i)
    {
        auto FaithBuilding =
            World->FindObject<CPlacementAreaObject>(FaithNames[i]).lock();

        if (!IsAssignablePlacedBuilding(FaithBuilding) ||
            !FaithBuilding->IsFaithProvider())
        {
            continue;
        }

        const int VisitCapacity =
            ResolveServiceAssignmentCapacity(
                FaithBuilding,
                EBuildingServiceType::Faith);

        if (VisitCapacity <= 0)
            continue;

        FServiceBuildingInfo Info;
        Info.Name = FaithNames[i];
        Info.Building = FaithBuilding;
        Info.SatisfactionCap = FaithBuilding->GetFaithSatisfactionCap();
        Info.VisitCapacity = VisitCapacity;
        Info.ActiveVisitors =
            FaithBuilding->GetActiveServiceVisitorCount(
                EBuildingServiceType::Faith);
        Info.AllowedWealthMask = FaithBuilding->GetAllowedWealthMask();
        Info.Category = FaithBuilding->GetBuildingCategory();
        Info.LeisureClass = FaithBuilding->GetLeisureClass();
        Info.PrimaryTouristPreference =
            FaithBuilding->GetPrimaryTouristPreference();
        Info.Accessibility = ResolveAccessibilityScore(FaithBuilding);
        FaithInfos.push_back(std::move(Info));
    }

    std::vector<FWorkBuildingInfo> WorkInfos;
    WorkInfos.reserve(WorkNames.size());

    for (size_t i = 0; i < WorkNames.size(); ++i)
    {
        auto WorkBuilding =
            World->FindObject<CPlacementAreaObject>(WorkNames[i]).lock();

        if (!IsAssignablePlacedBuilding(WorkBuilding) ||
            WorkBuilding->GetCapacity() <= 0)
        {
            continue;
        }

        FWorkBuildingInfo Info;
        Info.Name = WorkNames[i];
        Info.Building = WorkBuilding;
        Info.Capacity = (std::max)(0, WorkBuilding->GetCapacity());
        Info.JobCap = WorkBuilding->GetEffectiveJobSatisfactionCap();
        Info.RequiredEducation = WorkBuilding->GetRequiredEducationLevel();
        Info.IsFoodProvider = WorkBuilding->IsFoodProvider();
        Info.Accessibility = ResolveAccessibilityScore(WorkBuilding);
        WorkInfos.push_back(std::move(Info));
    }

    std::unordered_map<std::string, size_t> HomeIndexByName;
    HomeIndexByName.reserve(HomeInfos.size());

    for (size_t i = 0; i < HomeInfos.size(); ++i)
        HomeIndexByName.emplace(HomeInfos[i].Name, i);

    std::unordered_map<std::string, size_t> FoodIndexByName;
    FoodIndexByName.reserve(FoodInfos.size());

    for (size_t i = 0; i < FoodInfos.size(); ++i)
        FoodIndexByName.emplace(FoodInfos[i].Name, i);

    std::unordered_map<std::string, size_t> FunIndexByName;
    FunIndexByName.reserve(FunInfos.size());

    for (size_t i = 0; i < FunInfos.size(); ++i)
        FunIndexByName.emplace(FunInfos[i].Name, i);

    std::unordered_map<std::string, size_t> HealthIndexByName;
    HealthIndexByName.reserve(HealthInfos.size());

    for (size_t i = 0; i < HealthInfos.size(); ++i)
        HealthIndexByName.emplace(HealthInfos[i].Name, i);

    std::unordered_map<std::string, size_t> FaithIndexByName;
    FaithIndexByName.reserve(FaithInfos.size());

    for (size_t i = 0; i < FaithInfos.size(); ++i)
        FaithIndexByName.emplace(FaithInfos[i].Name, i);

    std::unordered_map<std::string, size_t> WorkIndexByName;
    WorkIndexByName.reserve(WorkInfos.size());

    for (size_t i = 0; i < WorkInfos.size(); ++i)
        WorkIndexByName.emplace(WorkInfos[i].Name, i);

    std::vector<std::shared_ptr<CBuildingMarkerOrb>> ActiveOrbs;
    ActiveOrbs.reserve(OrbList.size());

    for (size_t i = 0; i < OrbList.size(); ++i)
    {
        auto Orb = OrbList[i].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        ActiveOrbs.push_back(Orb);

        for (size_t NameIndex = 0; NameIndex < AllNames.size(); ++NameIndex)
            Orb->AddTargetBuildingName(AllNames[NameIndex]);

        if (Orb->GetHealthBuilding().empty() && !HealthNames.empty())
            Orb->SetHealthBuilding(PickRandomBuildingName(HealthNames));

        if (Orb->GetFaithBuilding().empty() && !FaithNames.empty())
            Orb->SetFaithBuilding(PickRandomBuildingName(FaithNames));
    }

    if (ActiveOrbs.empty())
    {
        const bool HasCitizenOrbRecords = !OrbList.empty();

        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            if (WorkInfos[i].Building)
            {
                if (HasCitizenOrbRecords)
                {
                    WorkInfos[i].Building->SetCurrentWorkerOccupancy(0);
                }

                WorkInfos[i].Building->SetWorkingNowOccupancy(0);
            }
        }
        return;
    }

    std::vector<int> OrbHomeIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbFoodIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbFunIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbHealthIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbFaithIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbWorkIndex(ActiveOrbs.size(), -1);

    auto SyncWorkBuildingOccupancy = [&]()
    {
        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            if (WorkInfos[i].Building)
            {
                WorkInfos[i].StandingNow = 0;
                WorkInfos[i].Building->SetCurrentWorkerOccupancy(
                    WorkInfos[i].Occupied);
            }
        }

        for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
        {
            const int WorkIdx = OrbWorkIndex[OrbIdx];

            if (WorkIdx < 0 ||
                WorkIdx >= static_cast<int>(WorkInfos.size()))
            {
                continue;
            }

            const auto& Orb = ActiveOrbs[OrbIdx];

            if (!Orb || Orb->GetCitizenState() != ECitizenState::AtWork)
                continue;

            ++WorkInfos[static_cast<size_t>(WorkIdx)].StandingNow;
        }

        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            if (WorkInfos[i].Building)
            {
                WorkInfos[i].Building->SetWorkingNowOccupancy(
                    WorkInfos[i].StandingNow);
            }
        }
    };

    auto IsFoodAssignmentLocked = [&](int OrbIdx) -> bool
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return false;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const ECitizenState State = Orb->GetCitizenState();
        return State == ECitizenState::GoingToFood ||
            State == ECitizenState::AtFood;
    };

    auto IsFunAssignmentLocked = [&](int OrbIdx) -> bool
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return false;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const ECitizenState State = Orb->GetCitizenState();
        return State == ECitizenState::GoingToFun ||
            State == ECitizenState::AtFun;
    };

    auto IsHealthAssignmentLocked = [&](int OrbIdx) -> bool
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return false;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const ECitizenState State = Orb->GetCitizenState();
        return State == ECitizenState::GoingToHealth ||
            State == ECitizenState::AtHealth;
    };

    auto IsFaithAssignmentLocked = [&](int OrbIdx) -> bool
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return false;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const ECitizenState State = Orb->GetCitizenState();
        return State == ECitizenState::GoingToFaith ||
            State == ECitizenState::AtFaith;
    };

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        auto Orb = ActiveOrbs[i];

        if (!Orb)
            continue;

        FCitizenIdentityProfile IdentityProfile = Orb->GetIdentityProfile();

        if (!HasValidCitizenHouseholdProfile(IdentityProfile))
        {
            AssignCitizenHouseholdProfile(IdentityProfile);
            Orb->SetIdentityProfile(IdentityProfile);
        }

        const std::string& CurrentHome = Orb->GetHomeBuilding();

        if (!CurrentHome.empty())
        {
            auto HomeIt = HomeIndexByName.find(CurrentHome);

            if (HomeIt != HomeIndexByName.end())
            {
                const int HomeIdx = static_cast<int>(HomeIt->second);
                OrbHomeIndex[i] = HomeIdx;
            }
            else
            {
                Orb->SetHomeBuilding("");
            }
        }

        const std::string& CurrentFood = Orb->GetFoodBuilding();

        if (!CurrentFood.empty())
        {
            auto FoodIt = FoodIndexByName.find(CurrentFood);

            if (FoodIt != FoodIndexByName.end())
            {
                const int FoodIdx = static_cast<int>(FoodIt->second);
                OrbFoodIndex[i] = FoodIdx;
                ++FoodInfos[FoodIdx].Assigned;
            }
            else
            {
                Orb->SetFoodBuilding("");
            }
        }

        const std::string& CurrentFun = Orb->GetFunBuilding();

        if (!CurrentFun.empty())
        {
            auto FunIt = FunIndexByName.find(CurrentFun);

            if (FunIt != FunIndexByName.end())
            {
                const int FunIdx = static_cast<int>(FunIt->second);
                OrbFunIndex[i] = FunIdx;
                ++FunInfos[FunIdx].Assigned;
            }
            else
            {
                Orb->SetFunBuilding("");
            }
        }

        const std::string& CurrentHealth = Orb->GetHealthBuilding();

        if (!CurrentHealth.empty())
        {
            auto HealthIt = HealthIndexByName.find(CurrentHealth);

            if (HealthIt != HealthIndexByName.end())
            {
                const int HealthIdx = static_cast<int>(HealthIt->second);
                OrbHealthIndex[i] = HealthIdx;
                ++HealthInfos[HealthIdx].Assigned;
            }
            else
            {
                Orb->SetHealthBuilding("");
            }
        }

        const std::string& CurrentFaith = Orb->GetFaithBuilding();

        if (!CurrentFaith.empty())
        {
            auto FaithIt = FaithIndexByName.find(CurrentFaith);

            if (FaithIt != FaithIndexByName.end())
            {
                const int FaithIdx = static_cast<int>(FaithIt->second);
                OrbFaithIndex[i] = FaithIdx;
                ++FaithInfos[FaithIdx].Assigned;
            }
            else
            {
                Orb->SetFaithBuilding("");
            }
        }

        const std::string& CurrentWork = Orb->GetWorkBuilding();

        if (CurrentWork.empty())
            continue;

        auto WorkIt = WorkIndexByName.find(CurrentWork);

        if (WorkIt == WorkIndexByName.end())
        {
            Orb->SetWorkBuilding("");
            continue;
        }

        const int WorkIdx = static_cast<int>(WorkIt->second);

        if (static_cast<int>(Orb->GetIdentityProfile().EducationLevel) <
            static_cast<int>(WorkInfos[WorkIdx].RequiredEducation))
        {
            Orb->SetWorkBuilding("");
            continue;
        }

        OrbWorkIndex[i] = WorkIdx;
        ++WorkInfos[WorkIdx].Occupied;
    }

    std::vector<FHouseholdInfo> Households;
    Households.reserve(ActiveOrbs.size());
    std::unordered_map<int, size_t> HouseholdIndexById;
    HouseholdIndexById.reserve(ActiveOrbs.size());
    std::vector<std::unordered_map<int, int>> HouseholdHomeVotes;
    HouseholdHomeVotes.reserve(ActiveOrbs.size());

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        auto Orb = ActiveOrbs[i];

        if (!Orb)
            continue;

        const FCitizenIdentityProfile& Identity = Orb->GetIdentityProfile();
        int HouseholdId = Identity.HouseholdId;

        if (HouseholdId <= 0)
            HouseholdId = -static_cast<int>(i) - 1;

        auto HouseholdIt = HouseholdIndexById.find(HouseholdId);
        size_t HouseholdIndex = 0;

        if (HouseholdIt == HouseholdIndexById.end())
        {
            HouseholdIndex = Households.size();
            HouseholdIndexById.emplace(HouseholdId, HouseholdIndex);
            Households.push_back(FHouseholdInfo());
            HouseholdHomeVotes.emplace_back();
            auto& Household = Households.back();
            Household.HouseholdId = HouseholdId;
            Household.HouseholdSize = (std::max)(1, Identity.HouseholdSize);
            Household.WealthLevel = Identity.WealthLevel;
        }
        else
        {
            HouseholdIndex = HouseholdIt->second;
        }

        auto& Household = Households[HouseholdIndex];
        Household.MemberOrbIndices.push_back(static_cast<int>(i));

        const int HomeIdx = OrbHomeIndex[i];

        if (HomeIdx >= 0)
            ++HouseholdHomeVotes[HouseholdIndex][HomeIdx];
    }

    for (size_t HouseholdIdx = 0; HouseholdIdx < Households.size(); ++HouseholdIdx)
    {
        auto& Household = Households[HouseholdIdx];
        int BestHomeIdx = -1;
        int BestVotes = 0;
        const auto& Votes = HouseholdHomeVotes[HouseholdIdx];

        for (auto VoteIt = Votes.begin(); VoteIt != Votes.end(); ++VoteIt)
        {
            if (VoteIt->second > BestVotes)
            {
                BestVotes = VoteIt->second;
                BestHomeIdx = VoteIt->first;
            }
        }

        Household.CurrentHomeIndex = BestHomeIdx;

        if (BestHomeIdx >= 0 &&
            BestHomeIdx < static_cast<int>(HomeInfos.size()))
        {
            ++HomeInfos[BestHomeIdx].Assigned;
            const std::string& HomeName = HomeInfos[BestHomeIdx].Name;

            for (size_t MemberIdx = 0;
                MemberIdx < Household.MemberOrbIndices.size();
                ++MemberIdx)
            {
                const int OrbIdx = Household.MemberOrbIndices[MemberIdx];

                if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
                    continue;

                auto HouseholdOrb = ActiveOrbs[OrbIdx];

                if (!HouseholdOrb)
                    continue;

                if (HouseholdOrb->GetHomeBuilding() != HomeName)
                    HouseholdOrb->SetHomeBuilding(HomeName);

                OrbHomeIndex[OrbIdx] = BestHomeIdx;
            }
        }
        else
        {
            Household.CurrentHomeIndex = -1;

            for (size_t MemberIdx = 0;
                MemberIdx < Household.MemberOrbIndices.size();
                ++MemberIdx)
            {
                const int OrbIdx = Household.MemberOrbIndices[MemberIdx];

                if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
                    continue;

                auto HouseholdOrb = ActiveOrbs[OrbIdx];

                if (!HouseholdOrb)
                    continue;

                if (!HouseholdOrb->GetHomeBuilding().empty())
                    HouseholdOrb->SetHomeBuilding("");

                OrbHomeIndex[OrbIdx] = -1;
            }
        }
    }

    auto ResolveHomeBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbHomeIndex.size()))
            return nullptr;

        const int HomeIdx = OrbHomeIndex[OrbIdx];

        if (HomeIdx < 0 || HomeIdx >= static_cast<int>(HomeInfos.size()))
            return nullptr;

        return HomeInfos[HomeIdx].Building;
    };

    auto ResolveFoodBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbFoodIndex.size()))
            return nullptr;

        const int FoodIdx = OrbFoodIndex[OrbIdx];

        if (FoodIdx < 0 || FoodIdx >= static_cast<int>(FoodInfos.size()))
            return nullptr;

        return FoodInfos[FoodIdx].Building;
    };

    auto ResolveFunBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbFunIndex.size()))
            return nullptr;

        const int FunIdx = OrbFunIndex[OrbIdx];

        if (FunIdx < 0 || FunIdx >= static_cast<int>(FunInfos.size()))
            return nullptr;

        return FunInfos[FunIdx].Building;
    };

    auto ResolveHealthBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbHealthIndex.size()))
            return nullptr;

        const int HealthIdx = OrbHealthIndex[OrbIdx];

        if (HealthIdx < 0 || HealthIdx >= static_cast<int>(HealthInfos.size()))
            return nullptr;

        return HealthInfos[HealthIdx].Building;
    };

    auto ResolveFaithBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbFaithIndex.size()))
            return nullptr;

        const int FaithIdx = OrbFaithIndex[OrbIdx];

        if (FaithIdx < 0 || FaithIdx >= static_cast<int>(FaithInfos.size()))
            return nullptr;

        return FaithInfos[FaithIdx].Building;
    };

    auto ResolveWorkBuildingForOrb = [&](int OrbIdx)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(OrbWorkIndex.size()))
            return nullptr;

        const int WorkIdx = OrbWorkIndex[OrbIdx];

        if (WorkIdx < 0 || WorkIdx >= static_cast<int>(WorkInfos.size()))
            return nullptr;

        return WorkInfos[WorkIdx].Building;
    };

    auto ResolveHouseholdDistanceScore =
        [&](int HouseholdIdx, int HomeIdx) -> float
    {
        if (HouseholdIdx < 0 ||
            HouseholdIdx >= static_cast<int>(Households.size()) ||
            HomeIdx < 0 ||
            HomeIdx >= static_cast<int>(HomeInfos.size()))
        {
            return 0.5f;
        }

        const auto& Household = Households[HouseholdIdx];
        float TotalScore = 0.f;
        int ScoreCount = 0;

        for (size_t MemberIdx = 0;
            MemberIdx < Household.MemberOrbIndices.size();
            ++MemberIdx)
        {
            const int OrbIdx = Household.MemberOrbIndices[MemberIdx];
            const auto HealthBuilding = ResolveHealthBuildingForOrb(OrbIdx);
            const auto FaithBuilding = ResolveFaithBuildingForOrb(OrbIdx);
            TotalScore += ResolveAverageDistanceAffinity(
                HomeInfos[HomeIdx].Building,
                ResolveWorkBuildingForOrb(OrbIdx),
                ResolveFoodBuildingForOrb(OrbIdx),
                ResolveFunBuildingForOrb(OrbIdx));
            ++ScoreCount;

            if (HealthBuilding || FaithBuilding)
            {
                TotalScore += ResolveAverageDistanceAffinity(
                    HomeInfos[HomeIdx].Building,
                    HealthBuilding,
                    FaithBuilding);
                ++ScoreCount;
            }
        }

        return ScoreCount > 0 ?
            TotalScore / static_cast<float>(ScoreCount) :
            0.5f;
    };

    auto AssignHouseholdToHome = [&](int HouseholdIdx, int HomeIdx) -> bool
    {
        if (HouseholdIdx < 0 || HomeIdx < 0)
            return false;

        if (HouseholdIdx >= static_cast<int>(Households.size()) ||
            HomeIdx >= static_cast<int>(HomeInfos.size()))
        {
            return false;
        }

        auto& Household = Households[HouseholdIdx];
        const int PrevHomeIdx = Household.CurrentHomeIndex;

        if (PrevHomeIdx == HomeIdx)
        {
            const std::string& TargetName = HomeInfos[HomeIdx].Name;

            for (size_t MemberIdx = 0;
                MemberIdx < Household.MemberOrbIndices.size();
                ++MemberIdx)
            {
                const int OrbIdx = Household.MemberOrbIndices[MemberIdx];

                if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
                    continue;

                auto Orb = ActiveOrbs[OrbIdx];

                if (!Orb)
                    continue;

                if (Orb->GetHomeBuilding() != TargetName)
                    Orb->SetHomeBuilding(TargetName);

                OrbHomeIndex[OrbIdx] = HomeIdx;
            }

            return true;
        }

        if (PrevHomeIdx >= 0 &&
            PrevHomeIdx < static_cast<int>(HomeInfos.size()) &&
            HomeInfos[PrevHomeIdx].Assigned > 0)
        {
            --HomeInfos[PrevHomeIdx].Assigned;
        }

        auto& TargetInfo = HomeInfos[HomeIdx];
        ++TargetInfo.Assigned;
        Household.CurrentHomeIndex = HomeIdx;

        for (size_t MemberIdx = 0;
            MemberIdx < Household.MemberOrbIndices.size();
            ++MemberIdx)
        {
            const int OrbIdx = Household.MemberOrbIndices[MemberIdx];

            if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
                continue;

            auto Orb = ActiveOrbs[OrbIdx];

            if (!Orb)
                continue;

            if (Orb->GetHomeBuilding() != TargetInfo.Name)
                Orb->SetHomeBuilding(TargetInfo.Name);

            OrbHomeIndex[OrbIdx] = HomeIdx;
        }

        return true;
    };

    auto AssignOrbToFood = [&](int OrbIdx, int FoodIdx) -> bool
    {
        if (OrbIdx < 0 || FoodIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            FoodIdx >= static_cast<int>(FoodInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const int PrevFoodIdx = OrbFoodIndex[OrbIdx];

        if (PrevFoodIdx == FoodIdx)
            return true;

        if (IsFoodAssignmentLocked(OrbIdx))
            return false;

        if (PrevFoodIdx >= 0 &&
            PrevFoodIdx < static_cast<int>(FoodInfos.size()) &&
            FoodInfos[PrevFoodIdx].Assigned > 0)
        {
            --FoodInfos[PrevFoodIdx].Assigned;
        }

        auto& TargetInfo = FoodInfos[FoodIdx];

        if (Orb->GetFoodBuilding() != TargetInfo.Name)
            Orb->SetFoodBuilding(TargetInfo.Name);

        ++TargetInfo.Assigned;
        OrbFoodIndex[OrbIdx] = FoodIdx;
        return true;
    };

    auto AssignOrbToFun = [&](int OrbIdx, int FunIdx) -> bool
    {
        if (OrbIdx < 0 || FunIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            FunIdx >= static_cast<int>(FunInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const int PrevFunIdx = OrbFunIndex[OrbIdx];

        if (PrevFunIdx == FunIdx)
            return true;

        if (IsFunAssignmentLocked(OrbIdx))
            return false;

        if (PrevFunIdx >= 0 &&
            PrevFunIdx < static_cast<int>(FunInfos.size()) &&
            FunInfos[PrevFunIdx].Assigned > 0)
        {
            --FunInfos[PrevFunIdx].Assigned;
        }

        auto& TargetInfo = FunInfos[FunIdx];

        if (Orb->GetFunBuilding() != TargetInfo.Name)
            Orb->SetFunBuilding(TargetInfo.Name);

        ++TargetInfo.Assigned;
        OrbFunIndex[OrbIdx] = FunIdx;
        return true;
    };

    auto AssignOrbToHealth = [&](int OrbIdx, int HealthIdx) -> bool
    {
        if (OrbIdx < 0 || HealthIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            HealthIdx >= static_cast<int>(HealthInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const int PrevHealthIdx = OrbHealthIndex[OrbIdx];

        if (PrevHealthIdx == HealthIdx)
            return true;

        if (IsHealthAssignmentLocked(OrbIdx))
            return false;

        if (PrevHealthIdx >= 0 &&
            PrevHealthIdx < static_cast<int>(HealthInfos.size()) &&
            HealthInfos[PrevHealthIdx].Assigned > 0)
        {
            --HealthInfos[PrevHealthIdx].Assigned;
        }

        auto& TargetInfo = HealthInfos[HealthIdx];

        if (Orb->GetHealthBuilding() != TargetInfo.Name)
            Orb->SetHealthBuilding(TargetInfo.Name);

        ++TargetInfo.Assigned;
        OrbHealthIndex[OrbIdx] = HealthIdx;
        return true;
    };

    auto AssignOrbToFaith = [&](int OrbIdx, int FaithIdx) -> bool
    {
        if (OrbIdx < 0 || FaithIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            FaithIdx >= static_cast<int>(FaithInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const int PrevFaithIdx = OrbFaithIndex[OrbIdx];

        if (PrevFaithIdx == FaithIdx)
            return true;

        if (IsFaithAssignmentLocked(OrbIdx))
            return false;

        if (PrevFaithIdx >= 0 &&
            PrevFaithIdx < static_cast<int>(FaithInfos.size()) &&
            FaithInfos[PrevFaithIdx].Assigned > 0)
        {
            --FaithInfos[PrevFaithIdx].Assigned;
        }

        auto& TargetInfo = FaithInfos[FaithIdx];

        if (Orb->GetFaithBuilding() != TargetInfo.Name)
            Orb->SetFaithBuilding(TargetInfo.Name);

        ++TargetInfo.Assigned;
        OrbFaithIndex[OrbIdx] = FaithIdx;
        return true;
    };

    struct FServiceScoringWeights
    {
        float Quality = 0.28f;
        float Accessibility = 0.20f;
        float Congestion = 0.18f;
        float Distance = 0.22f;
        float WealthFit = 0.12f;
    };

    auto ResolveServiceNeedUrgency =
        [](const FNpcSatisfaction& Satisfaction,
            EBuildingServiceType ServiceType) -> float
    {
        float CurrentValue = 70.f;

        switch (ServiceType)
        {
        case EBuildingServiceType::Food:
            CurrentValue = Satisfaction.Food;
            break;
        case EBuildingServiceType::Health:
            CurrentValue = Satisfaction.Health;
            break;
        case EBuildingServiceType::Faith:
            CurrentValue = Satisfaction.Faith;
            break;
        case EBuildingServiceType::Fun:
        default:
            CurrentValue = Satisfaction.Fun;
            break;
        }

        return 1.f - Clamp01(CurrentValue / 100.f);
    };

    auto ResolveAllowedWealthTierCount = [](unsigned int AllowedWealthMask)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;
        int TierCount = 0;

        if ((EffectiveMask & GBuildingWealthMaskBroke) != 0)
            ++TierCount;
        if ((EffectiveMask & GBuildingWealthMaskPoor) != 0)
            ++TierCount;
        if ((EffectiveMask & GBuildingWealthMaskWellOff) != 0)
            ++TierCount;
        if ((EffectiveMask & GBuildingWealthMaskRich) != 0)
            ++TierCount;
        if ((EffectiveMask & GBuildingWealthMaskFilthyRich) != 0)
            ++TierCount;

        return (std::max)(1, TierCount);
    };

    auto ResolveAllowedWealthFloorRank = [](unsigned int AllowedWealthMask)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;

        if ((EffectiveMask & GBuildingWealthMaskBroke) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::Broke);
        if ((EffectiveMask & GBuildingWealthMaskPoor) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::Poor);
        if ((EffectiveMask & GBuildingWealthMaskWellOff) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::WellOff);
        if ((EffectiveMask & GBuildingWealthMaskRich) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::Rich);
        return GetCitizenWealthRank(ECitizenWealthLevel::FilthyRich);
    };

    auto ResolveAllowedWealthCeilingRank = [](unsigned int AllowedWealthMask)
    {
        const unsigned int EffectiveMask =
            AllowedWealthMask == GBuildingWealthMaskNone ?
                GBuildingWealthMaskAll :
                AllowedWealthMask;

        if ((EffectiveMask & GBuildingWealthMaskFilthyRich) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::FilthyRich);
        if ((EffectiveMask & GBuildingWealthMaskRich) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::Rich);
        if ((EffectiveMask & GBuildingWealthMaskWellOff) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::WellOff);
        if ((EffectiveMask & GBuildingWealthMaskPoor) != 0)
            return GetCitizenWealthRank(ECitizenWealthLevel::Poor);
        return GetCitizenWealthRank(ECitizenWealthLevel::Broke);
    };

    auto ResolveServiceWealthFitScore =
        [&](unsigned int AllowedWealthMask,
            ECitizenWealthLevel WealthLevel) -> float
    {
        if (!DoesBuildingAllowCitizenWealth(
                AllowedWealthMask,
                WealthLevel))
        {
            return -1.f;
        }

        const int TierCount = ResolveAllowedWealthTierCount(AllowedWealthMask);
        const int CitizenRank = GetCitizenWealthRank(WealthLevel);
        const int FloorRank = ResolveAllowedWealthFloorRank(AllowedWealthMask);
        const int CeilingRank =
            ResolveAllowedWealthCeilingRank(AllowedWealthMask);

        if (TierCount <= 1 || FloorRank >= CeilingRank)
            return 1.f;

        const float Span =
            static_cast<float>((std::max)(1, CeilingRank - FloorRank));
        const float Offset =
            static_cast<float>((std::max)(0, CitizenRank - FloorRank));
        float Fit = 1.f - (Offset / Span) * 0.18f;
        Fit -= static_cast<float>((std::max)(0, TierCount - 2)) * 0.03f;

        if (CitizenRank == FloorRank)
            Fit += 0.03f;

        return (std::max)(0.66f, (std::min)(1.f, Fit));
    };

    auto ResolveCurrentVisitBuildingName =
        [](const std::shared_ptr<CBuildingMarkerOrb>& Orb,
            EBuildingServiceType ServiceType) -> std::string
    {
        if (!Orb)
            return std::string();

        switch (ServiceType)
        {
        case EBuildingServiceType::Food:
            return Orb->GetFoodVisitBuilding();
        case EBuildingServiceType::Health:
            return Orb->GetHealthVisitBuilding();
        case EBuildingServiceType::Faith:
            return Orb->GetFaithVisitBuilding();
        case EBuildingServiceType::Fun:
        default:
            return Orb->GetFunVisitBuilding();
        }
    };

    auto ResolveServiceScoringWeights =
        [&](const FCitizenIdentityProfile& Identity,
            const FNpcSatisfaction& Satisfaction,
            EBuildingServiceType ServiceType) -> FServiceScoringWeights
    {
        FServiceScoringWeights Weights;
        const float NeedUrgency =
            ResolveServiceNeedUrgency(Satisfaction, ServiceType);
        Weights.Quality += NeedUrgency * 0.10f;
        Weights.Accessibility += NeedUrgency * 0.05f;
        Weights.Congestion += NeedUrgency * 0.09f;
        Weights.Distance -= NeedUrgency * 0.06f;

        if (IsAffluentCitizen(Identity.WealthLevel) ||
            Identity.TouristProfile == ETouristPreference::Celebrity)
        {
            Weights.Quality += 0.07f;
            Weights.WealthFit += 0.05f;
            Weights.Distance -= 0.04f;
        }
        else if (IsLowWealthCitizen(Identity.WealthLevel) ||
            Identity.TouristProfile == ETouristPreference::Backpacker)
        {
            Weights.Accessibility += 0.06f;
            Weights.Distance += 0.05f;
            Weights.Quality -= 0.05f;
        }

        if (ServiceType == EBuildingServiceType::Health)
        {
            Weights.Quality += 0.04f;
            Weights.Congestion += 0.03f;
        }
        else if (ServiceType == EBuildingServiceType::Faith)
        {
            Weights.Distance += 0.04f;
            Weights.Accessibility += 0.02f;
        }
        else if (ServiceType == EBuildingServiceType::Fun)
        {
            Weights.Quality += 0.02f;
        }

        Weights.Quality = (std::max)(0.05f, Weights.Quality);
        Weights.Accessibility = (std::max)(0.05f, Weights.Accessibility);
        Weights.Congestion = (std::max)(0.05f, Weights.Congestion);
        Weights.Distance = (std::max)(0.05f, Weights.Distance);
        Weights.WealthFit = (std::max)(0.04f, Weights.WealthFit);

        const float WeightSum =
            Weights.Quality +
            Weights.Accessibility +
            Weights.Congestion +
            Weights.Distance +
            Weights.WealthFit;

        if (WeightSum > 0.f)
        {
            Weights.Quality /= WeightSum;
            Weights.Accessibility /= WeightSum;
            Weights.Congestion /= WeightSum;
            Weights.Distance /= WeightSum;
            Weights.WealthFit /= WeightSum;
        }

        return Weights;
    };

    auto ResolveServicePreferenceBias =
        [&](const FCitizenIdentityProfile& Identity,
            const FNpcPoliticalProfile& Politics,
            const FServiceBuildingInfo& Info,
            EBuildingServiceType ServiceType,
            float QualityScore,
            float AccessibilityScore,
            float CongestionScore,
            float DistanceScore,
            float WealthFitScore,
            float NeedUrgency) -> float
    {
        float Bias = 0.f;

        switch (ServiceType)
        {
        case EBuildingServiceType::Food:
            if (IsAffluentCitizen(Identity.WealthLevel) ||
                Identity.TouristProfile == ETouristPreference::Celebrity)
            {
                Bias += (QualityScore - AccessibilityScore) * 0.12f;
                Bias += (WealthFitScore - 0.5f) * 0.08f;
            }
            else if (IsLowWealthCitizen(Identity.WealthLevel) ||
                Identity.TouristProfile == ETouristPreference::Backpacker)
            {
                Bias +=
                    (AccessibilityScore + DistanceScore - QualityScore) *
                    0.08f;
            }
            else
            {
                Bias += (QualityScore - 0.5f) * 0.04f;
            }

            if (Identity.IsTourist &&
                Identity.TouristProfile == ETouristPreference::Family)
            {
                Bias += (CongestionScore - 0.5f) * 0.06f;
            }
            break;
        case EBuildingServiceType::Fun:
            Bias += ResolveTouristPreferenceMatchScore(
                Identity,
                Info.Category,
                Info.LeisureClass,
                Info.PrimaryTouristPreference);

            if (!Identity.IsTourist)
            {
                if (Identity.EducationLevel == ECitizenEducationLevel::College &&
                    Info.LeisureClass == EBuildingLeisureClass::Cultural)
                {
                    Bias += 0.08f;
                }

                if (IsAffluentCitizen(Identity.WealthLevel) &&
                    (Info.LeisureClass == EBuildingLeisureClass::Luxury ||
                        Info.Category ==
                            EBuildingCategory::LuxuryEntertainment))
                {
                    Bias +=
                        IsLuxuryWealthCitizen(Identity.WealthLevel) ?
                            0.16f :
                            0.12f;
                }

                if (IsLowWealthCitizen(Identity.WealthLevel) &&
                    Info.LeisureClass == EBuildingLeisureClass::General)
                {
                    Bias += 0.07f;
                }

                if (IsLowWealthCitizen(Identity.WealthLevel) &&
                    (Info.LeisureClass == EBuildingLeisureClass::Luxury ||
                        Info.Category ==
                            EBuildingCategory::LuxuryEntertainment))
                {
                    Bias -= 0.08f;
                }
            }
            break;
        case EBuildingServiceType::Health:
            Bias +=
                NeedUrgency *
                ((QualityScore - 0.5f) * 0.10f +
                    (CongestionScore - 0.5f) * 0.14f);

            if (IsAffluentCitizen(Identity.WealthLevel))
            {
                Bias += (WealthFitScore - 0.5f) * 0.10f;
                Bias += (QualityScore - DistanceScore) * 0.05f;
            }
            else if (IsLowWealthCitizen(Identity.WealthLevel))
            {
                Bias +=
                    (AccessibilityScore + DistanceScore - QualityScore) *
                    0.05f;
            }
            break;
        case EBuildingServiceType::Faith:
        {
            const FNpcPoliticalChoice& FaithChoice =
                Politics.Get(EPoliticalAxis::ReligionMilitarism);

            if (FaithChoice.Stance == EPoliticalStance::Right)
            {
                Bias += (QualityScore - 0.5f) * 0.12f;
                Bias += (CongestionScore - 0.5f) * 0.05f;
            }
            else if (FaithChoice.Stance == EPoliticalStance::Left)
            {
                Bias +=
                    (AccessibilityScore + DistanceScore - QualityScore) *
                    0.05f;
            }
            else
            {
                Bias += (DistanceScore - 0.5f) * 0.03f;
            }
            break;
        }
        default:
            break;
        }

        return (std::max)(-0.25f, (std::min)(0.30f, Bias));
    };

    auto ScoreHomeCandidate = [&](int HouseholdIdx, int HomeIdx) -> float
    {
        if (HouseholdIdx < 0 || HomeIdx < 0)
            return (std::numeric_limits<float>::lowest)();

        if (HouseholdIdx >= static_cast<int>(Households.size()) ||
            HomeIdx >= static_cast<int>(HomeInfos.size()))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        auto& Info = HomeInfos[HomeIdx];
        const auto& Household = Households[HouseholdIdx];

        if (!DoesBuildingAllowCitizenWealth(
                Info.AllowedWealthMask,
                Household.WealthLevel))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        const int EffectiveAssigned =
            Info.Assigned - (Household.CurrentHomeIndex == HomeIdx ? 1 : 0);
        const int OccupancyAfter = EffectiveAssigned + 1;
        const float QualityScore =
            ResolveSatisfactionScore(Info.HousingCap);
        const float AccessScore = Info.Accessibility;
        const float CongestionScore =
            ResolveCongestionScore(OccupancyAfter, Info.Capacity);
        const float DistanceScore =
            ResolveHouseholdDistanceScore(HouseholdIdx, HomeIdx);
        const float WealthFitScore =
            ResolveServiceWealthFitScore(
                Info.AllowedWealthMask,
                Household.WealthLevel);

        float Score =
            QualityScore * 0.34f +
            AccessScore * 0.18f +
            CongestionScore * 0.16f +
            DistanceScore * 0.20f +
            WealthFitScore * 0.12f;

        if (Household.CurrentHomeIndex == HomeIdx)
            Score += 0.05f;

        return Score;
    };

    auto ScoreServiceCandidate =
        [&](int OrbIdx,
            int ServiceIdx,
            const std::vector<FServiceBuildingInfo>& Infos,
            const std::vector<int>& OrbServiceIndex,
            EBuildingServiceType ServiceType) -> float
    {
        if (OrbIdx < 0 || ServiceIdx < 0)
            return (std::numeric_limits<float>::lowest)();

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            ServiceIdx >= static_cast<int>(Infos.size()))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        const FServiceBuildingInfo& Info = Infos[ServiceIdx];
        auto Orb = ActiveOrbs[OrbIdx];
        if (!Orb)
            return (std::numeric_limits<float>::lowest)();

        const FCitizenIdentityProfile& Identity = Orb->GetIdentityProfile();
        const FNpcSatisfaction& Satisfaction = Orb->GetSatisfaction();
        const FNpcPoliticalProfile& Politics = Orb->GetPoliticalProfile();
        if (
            !DoesBuildingAllowCitizenWealth(
                Info.AllowedWealthMask,
                Identity.WealthLevel))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        const int EffectiveAssigned =
            Info.Assigned - (OrbServiceIndex[OrbIdx] == ServiceIdx ? 1 : 0);
        const std::string CurrentVisitBuildingName =
            ResolveCurrentVisitBuildingName(Orb, ServiceType);
        const int RuntimeVisitors =
            (std::max)(
                0,
                Info.ActiveVisitors -
                    (CurrentVisitBuildingName == Info.Name ? 1 : 0));
        const int OccupancyAfter =
            (std::max)(EffectiveAssigned, RuntimeVisitors) + 1;
        const float QualityScore =
            ResolveSatisfactionScore(Info.SatisfactionCap);
        const float AccessScore = Info.Accessibility;
        const float CongestionScore =
            ResolveCongestionScore(OccupancyAfter, Info.VisitCapacity);
        const float DistanceScore =
            ResolveAverageDistanceAffinity(
                Info.Building,
                ResolveHomeBuildingForOrb(OrbIdx),
                ResolveWorkBuildingForOrb(OrbIdx));
        const float WealthFitScore =
            ResolveServiceWealthFitScore(
                Info.AllowedWealthMask,
                Identity.WealthLevel);
        const float NeedUrgency =
            ResolveServiceNeedUrgency(Satisfaction, ServiceType);
        const FServiceScoringWeights Weights =
            ResolveServiceScoringWeights(
                Identity,
                Satisfaction,
                ServiceType);
        const float PreferenceBias =
            ResolveServicePreferenceBias(
                Identity,
                Politics,
                Info,
                ServiceType,
                QualityScore,
                AccessScore,
                CongestionScore,
                DistanceScore,
                WealthFitScore,
                NeedUrgency);

        float Score =
            QualityScore * Weights.Quality +
            AccessScore * Weights.Accessibility +
            CongestionScore * Weights.Congestion +
            DistanceScore * Weights.Distance +
            WealthFitScore * Weights.WealthFit +
            PreferenceBias;

        if (NeedUrgency > 0.55f && CongestionScore < 0.15f)
            Score -= 0.04f;

        if (OrbServiceIndex[OrbIdx] == ServiceIdx)
            Score += 0.05f;

        return Score;
    };

    auto RunServiceReassignment =
        [&](std::vector<FServiceBuildingInfo>& Infos,
            std::vector<int>& OrbServiceIndex,
            const auto& IsAssignmentLocked,
            const auto& AssignOrbToService,
            EBuildingServiceType ServiceType,
            float ImprovementThreshold)
    {
        if (Infos.empty())
            return;

        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            if (IsAssignmentLocked(static_cast<int>(i)))
                continue;

            float CurrentScore =
                OrbServiceIndex[i] >= 0 ?
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        OrbServiceIndex[i],
                        Infos,
                        OrbServiceIndex,
                        ServiceType) :
                    (std::numeric_limits<float>::lowest)();
            float BestScore = CurrentScore;
            int BestIdx = OrbServiceIndex[i];

            for (size_t ServiceIdx = 0; ServiceIdx < Infos.size(); ++ServiceIdx)
            {
                const float CandidateScore =
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        static_cast<int>(ServiceIdx),
                        Infos,
                        OrbServiceIndex,
                        ServiceType);

                if (CandidateScore > BestScore + 0.0001f)
                {
                    BestScore = CandidateScore;
                    BestIdx = static_cast<int>(ServiceIdx);
                }
            }

            if (BestIdx >= 0 &&
                (OrbServiceIndex[i] < 0 ||
                    BestScore > CurrentScore + ImprovementThreshold))
            {
                AssignOrbToService(static_cast<int>(i), BestIdx);
            }
        }
    };

    if (!HomeInfos.empty())
    {
        for (size_t i = 0; i < Households.size(); ++i)
        {
            float CurrentScore =
                Households[i].CurrentHomeIndex >= 0 ?
                    ScoreHomeCandidate(
                        static_cast<int>(i),
                        Households[i].CurrentHomeIndex) :
                    (std::numeric_limits<float>::lowest)();
            float BestScore = CurrentScore;
            int BestIdx = Households[i].CurrentHomeIndex;

            for (size_t HomeIdx = 0; HomeIdx < HomeInfos.size(); ++HomeIdx)
            {
                const float CandidateScore =
                    ScoreHomeCandidate(
                        static_cast<int>(i),
                        static_cast<int>(HomeIdx));

                if (CandidateScore > BestScore + 0.0001f)
                {
                    BestScore = CandidateScore;
                    BestIdx = static_cast<int>(HomeIdx);
                }
            }

            if (BestIdx >= 0 &&
                (Households[i].CurrentHomeIndex < 0 ||
                    BestScore > CurrentScore + 0.06f))
            {
                AssignHouseholdToHome(static_cast<int>(i), BestIdx);
            }
        }
    }

    RunServiceReassignment(
        FoodInfos,
        OrbFoodIndex,
        IsFoodAssignmentLocked,
        AssignOrbToFood,
        EBuildingServiceType::Food,
        0.05f);

    std::unordered_map<std::string, int> FoodDemandByBuilding;
    FoodDemandByBuilding.reserve(FoodInfos.size());

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        const int FoodIdx = OrbFoodIndex[i];

        if (FoodIdx < 0 || FoodIdx >= static_cast<int>(FoodInfos.size()))
            continue;

        ++FoodDemandByBuilding[FoodInfos[FoodIdx].Name];
    }

    // ── 직장 배정 ──────────────────────────────────────────────────────────
    for (size_t i = 0; i < WorkInfos.size(); ++i)
    {
        auto& Info = WorkInfos[i];
        Info.MinRequired = 0;

        if (!Info.IsFoodProvider || Info.Capacity <= 0)
            continue;

        auto FoodDemandIt = FoodDemandByBuilding.find(Info.Name);

        if (FoodDemandIt != FoodDemandByBuilding.end() &&
            FoodDemandIt->second > 0)
        {
            const int RequiredForDemand =
                (FoodDemandIt->second + Info.Capacity - 1) / Info.Capacity;
            const int MaxLockedWorkers =
                (std::max)(1, Info.Capacity / 2);
            Info.MinRequired =
                (std::min)(RequiredForDemand, MaxLockedWorkers);
        }
    }

    auto CanOrbWorkAt = [&](int OrbIdx, int WorkIdx) -> bool
    {
        if (OrbIdx < 0 || WorkIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            WorkIdx >= static_cast<int>(WorkInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const auto& Identity = Orb->GetIdentityProfile();
        return static_cast<int>(Identity.EducationLevel) >=
            static_cast<int>(WorkInfos[WorkIdx].RequiredEducation);
    };

    auto HasWorkVacancyForOrb = [&](int OrbIdx, int WorkIdx) -> bool
    {
        if (OrbIdx < 0 || WorkIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(OrbWorkIndex.size()) ||
            WorkIdx >= static_cast<int>(WorkInfos.size()))
        {
            return false;
        }

        const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];
        const int EffectiveOccupied =
            WorkInfos[WorkIdx].Occupied -
            (CurrentWorkIdx == WorkIdx ? 1 : 0);
        return EffectiveOccupied < WorkInfos[WorkIdx].Capacity;
    };

    auto ScoreWorkCandidate = [&](int OrbIdx, int WorkIdx) -> float
    {
        if (OrbIdx < 0 || WorkIdx < 0)
            return (std::numeric_limits<float>::lowest)();

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            WorkIdx >= static_cast<int>(WorkInfos.size()))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        if (!CanOrbWorkAt(OrbIdx, WorkIdx))
            return (std::numeric_limits<float>::lowest)();

        auto& Info = WorkInfos[WorkIdx];
        const int EffectiveOccupied =
            Info.Occupied - (OrbWorkIndex[OrbIdx] == WorkIdx ? 1 : 0);
        const int OccupancyAfter = EffectiveOccupied + 1;
        const float QualityScore = ResolveSatisfactionScore(Info.JobCap);
        const float AccessScore = Info.Accessibility;
        const float CongestionScore =
            ResolveCongestionScore(OccupancyAfter, Info.Capacity);
        const float DistanceScore =
            ResolveAverageDistanceAffinity(
                Info.Building,
                ResolveHomeBuildingForOrb(OrbIdx));

        float Score =
            QualityScore * 0.36f +
            AccessScore * 0.18f +
            CongestionScore * 0.20f +
            DistanceScore * 0.26f;

        if (OrbWorkIndex[OrbIdx] == WorkIdx)
            Score += 0.05f;

        if (Info.IsFoodProvider && Info.MinRequired > 0)
            Score += 0.04f;

        return Score;
    };

    auto AssignOrbToWork = [&](int OrbIdx, int WorkIdx) -> bool
    {
        if (OrbIdx < 0 || WorkIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            WorkIdx >= static_cast<int>(WorkInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb || !CanOrbWorkAt(OrbIdx, WorkIdx))
            return false;

        const int PrevWorkIdx = OrbWorkIndex[OrbIdx];

        if (PrevWorkIdx == WorkIdx)
            return true;

        if (!HasWorkVacancyForOrb(OrbIdx, WorkIdx))
            return false;

        if (PrevWorkIdx >= 0 &&
            PrevWorkIdx < static_cast<int>(WorkInfos.size()))
        {
            auto& PrevInfo = WorkInfos[PrevWorkIdx];

            if (PrevInfo.Occupied <= PrevInfo.MinRequired)
                return false;

            if (PrevInfo.Occupied > 0)
                --PrevInfo.Occupied;
        }

        auto& TargetInfo = WorkInfos[WorkIdx];

        if (Orb->GetWorkBuilding() != TargetInfo.Name)
            Orb->SetWorkBuilding(TargetInfo.Name);

        ++TargetInfo.Occupied;
        OrbWorkIndex[OrbIdx] = WorkIdx;
        return true;
    };

    auto ClearOrbWorkAssignment = [&](int OrbIdx)
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return;

        const int PrevWorkIdx = OrbWorkIndex[OrbIdx];

        if (PrevWorkIdx >= 0 &&
            PrevWorkIdx < static_cast<int>(WorkInfos.size()) &&
            WorkInfos[PrevWorkIdx].Occupied > 0)
        {
            --WorkInfos[PrevWorkIdx].Occupied;
        }

        Orb->SetWorkBuilding("");
        OrbWorkIndex[OrbIdx] = -1;
    };

    for (size_t WorkIdx = 0; WorkIdx < WorkInfos.size(); ++WorkIdx)
    {
        while (WorkInfos[WorkIdx].Occupied > WorkInfos[WorkIdx].Capacity)
        {
            int WorstOrbIdx = -1;
            float WorstScore = (std::numeric_limits<float>::max)();

            for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
            {
                if (OrbWorkIndex[OrbIdx] != static_cast<int>(WorkIdx))
                    continue;

                const float CurrentScore =
                    ScoreWorkCandidate(
                        static_cast<int>(OrbIdx),
                        static_cast<int>(WorkIdx));

                if (WorstOrbIdx < 0 || CurrentScore < WorstScore)
                {
                    WorstScore = CurrentScore;
                    WorstOrbIdx = static_cast<int>(OrbIdx);
                }
            }

            if (WorstOrbIdx < 0)
                break;

            ClearOrbWorkAssignment(WorstOrbIdx);
        }
    }

    auto FindFoodDeficitWork = [&]() -> int
    {
        int BestIdx = -1;
        float BestPriority = (std::numeric_limits<float>::lowest)();

        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            const auto& Info = WorkInfos[i];

            if (!Info.IsFoodProvider ||
                Info.MinRequired <= 0 ||
                Info.Occupied >= Info.MinRequired ||
                Info.Occupied >= Info.Capacity)
            {
                continue;
            }

            const float Priority =
                ResolveSatisfactionScore(Info.JobCap) +
                Info.Accessibility * 0.25f;

            if (Priority > BestPriority)
            {
                BestPriority = Priority;
                BestIdx = static_cast<int>(i);
            }
        }

        return BestIdx;
    };

    auto FindBestOrbForDeficitWork = [&](int TargetWorkIdx) -> int
    {
        int BestOrbIdx = -1;
        float BestGain = (std::numeric_limits<float>::lowest)();

        for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
        {
            if (!CanOrbWorkAt(static_cast<int>(OrbIdx), TargetWorkIdx) ||
                !HasWorkVacancyForOrb(static_cast<int>(OrbIdx), TargetWorkIdx))
            {
                continue;
            }

            const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];

            if (CurrentWorkIdx == TargetWorkIdx)
                continue;

            if (CurrentWorkIdx >= 0 &&
                WorkInfos[CurrentWorkIdx].Occupied <=
                    WorkInfos[CurrentWorkIdx].MinRequired)
            {
                continue;
            }

            const float TargetScore =
                ScoreWorkCandidate(
                    static_cast<int>(OrbIdx),
                    TargetWorkIdx);
            const float CurrentScore =
                CurrentWorkIdx >= 0 ?
                    ScoreWorkCandidate(
                        static_cast<int>(OrbIdx),
                        CurrentWorkIdx) :
                    -0.10f;
            float Gain = TargetScore - CurrentScore;

            if (CurrentWorkIdx < 0)
                Gain += 0.15f;

            if (Gain > BestGain)
            {
                BestGain = Gain;
                BestOrbIdx = static_cast<int>(OrbIdx);
            }
        }

        return BestOrbIdx;
    };

    const int MaxFoodDeficitAssignments =
        (std::max)(1, static_cast<int>(ActiveOrbs.size()) + 1);

    for (int Iteration = 0;
        Iteration < MaxFoodDeficitAssignments;
        ++Iteration)
    {
        const int DeficitWorkIdx = FindFoodDeficitWork();

        if (DeficitWorkIdx < 0)
            break;

        const int BestOrbIdx = FindBestOrbForDeficitWork(DeficitWorkIdx);

        if (BestOrbIdx < 0)
            break;

        if (!AssignOrbToWork(BestOrbIdx, DeficitWorkIdx))
            break;
    }

    auto FindBestVacancyWorkForOrb = [&](int OrbIdx, float MinImprovement)
        -> int
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return -1;

        const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];
        const float CurrentScore =
            CurrentWorkIdx >= 0 ?
                ScoreWorkCandidate(OrbIdx, CurrentWorkIdx) :
                -0.10f;
        int BestIdx = -1;
        float BestScore = CurrentScore + MinImprovement;

        for (size_t WorkIdx = 0; WorkIdx < WorkInfos.size(); ++WorkIdx)
        {
            if (!CanOrbWorkAt(OrbIdx, static_cast<int>(WorkIdx)) ||
                !HasWorkVacancyForOrb(OrbIdx, static_cast<int>(WorkIdx)))
            {
                continue;
            }

            const float CandidateScore =
                ScoreWorkCandidate(OrbIdx, static_cast<int>(WorkIdx));

            if (CandidateScore > BestScore + 0.0001f)
            {
                BestScore = CandidateScore;
                BestIdx = static_cast<int>(WorkIdx);
            }
        }

        return BestIdx;
    };

    auto IsWorkOverstaffed = [&](int WorkIdx) -> bool
    {
        if (WorkIdx < 0 || WorkIdx >= static_cast<int>(WorkInfos.size()))
            return false;

        const auto& Info = WorkInfos[WorkIdx];

        if (Info.Capacity <= 0)
            return false;

        const int OverstaffedThreshold = (std::max)(
            Info.MinRequired,
            static_cast<int>(ceilf(static_cast<float>(Info.Capacity) * 0.80f)));
        return Info.Occupied > OverstaffedThreshold;
    };

    auto IsWorkUnderstaffed = [&](int WorkIdx) -> bool
    {
        if (WorkIdx < 0 || WorkIdx >= static_cast<int>(WorkInfos.size()))
            return false;

        const auto& Info = WorkInfos[WorkIdx];

        if (Info.Capacity <= 0 || Info.Occupied >= Info.Capacity)
            return false;

        const int UnderstaffedThreshold = (std::max)(
            (std::max)(1, Info.MinRequired),
            static_cast<int>(floorf(static_cast<float>(Info.Capacity) * 0.40f)));
        return Info.Occupied < UnderstaffedThreshold;
    };

    auto FindBestUnderstaffedWorkForOrb =
        [&](int OrbIdx, float MinImprovement) -> int
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return -1;

        const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];
        const float CurrentScore =
            CurrentWorkIdx >= 0 ?
                ScoreWorkCandidate(OrbIdx, CurrentWorkIdx) :
                -0.10f;
        int BestIdx = -1;
        float BestScore = CurrentScore + MinImprovement;

        for (size_t WorkIdx = 0; WorkIdx < WorkInfos.size(); ++WorkIdx)
        {
            if (!IsWorkUnderstaffed(static_cast<int>(WorkIdx)) ||
                !CanOrbWorkAt(OrbIdx, static_cast<int>(WorkIdx)) ||
                !HasWorkVacancyForOrb(OrbIdx, static_cast<int>(WorkIdx)))
            {
                continue;
            }

            const float CandidateScore =
                ScoreWorkCandidate(OrbIdx, static_cast<int>(WorkIdx));

            if (CandidateScore > BestScore + 0.0001f)
            {
                BestScore = CandidateScore;
                BestIdx = static_cast<int>(WorkIdx);
            }
        }

        return BestIdx;
    };

    for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
    {
        if (OrbWorkIndex[OrbIdx] >= 0)
            continue;

        const int BestWorkIdx =
            FindBestVacancyWorkForOrb(static_cast<int>(OrbIdx), 0.f);

        if (BestWorkIdx >= 0)
            AssignOrbToWork(static_cast<int>(OrbIdx), BestWorkIdx);
    }

    for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
    {
        const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];

        if (CurrentWorkIdx < 0 ||
            CurrentWorkIdx >= static_cast<int>(WorkInfos.size()))
        {
            continue;
        }

        if (WorkInfos[CurrentWorkIdx].Occupied <=
            WorkInfos[CurrentWorkIdx].MinRequired)
        {
            continue;
        }

        const int BetterWorkIdx =
            FindBestVacancyWorkForOrb(static_cast<int>(OrbIdx), 0.07f);

        if (BetterWorkIdx >= 0 && BetterWorkIdx != CurrentWorkIdx)
            AssignOrbToWork(static_cast<int>(OrbIdx), BetterWorkIdx);
    }

    for (size_t OrbIdx = 0; OrbIdx < ActiveOrbs.size(); ++OrbIdx)
    {
        const int CurrentWorkIdx = OrbWorkIndex[OrbIdx];

        if (CurrentWorkIdx < 0 ||
            CurrentWorkIdx >= static_cast<int>(WorkInfos.size()) ||
            !IsWorkOverstaffed(CurrentWorkIdx))
        {
            continue;
        }

        if (WorkInfos[CurrentWorkIdx].Occupied <=
            WorkInfos[CurrentWorkIdx].MinRequired)
        {
            continue;
        }

        const int RebalanceWorkIdx =
            FindBestUnderstaffedWorkForOrb(static_cast<int>(OrbIdx), 0.02f);

        if (RebalanceWorkIdx >= 0 && RebalanceWorkIdx != CurrentWorkIdx)
        {
            AssignOrbToWork(static_cast<int>(OrbIdx), RebalanceWorkIdx);
        }
    }

    SyncWorkBuildingOccupancy();

    RunServiceReassignment(
        FoodInfos,
        OrbFoodIndex,
        IsFoodAssignmentLocked,
        AssignOrbToFood,
        EBuildingServiceType::Food,
        0.03f);
    RunServiceReassignment(
        HealthInfos,
        OrbHealthIndex,
        IsHealthAssignmentLocked,
        AssignOrbToHealth,
        EBuildingServiceType::Health,
        0.04f);
    RunServiceReassignment(
        FaithInfos,
        OrbFaithIndex,
        IsFaithAssignmentLocked,
        AssignOrbToFaith,
        EBuildingServiceType::Faith,
        0.04f);
    RunServiceReassignment(
        FunInfos,
        OrbFunIndex,
        IsFunAssignmentLocked,
        AssignOrbToFun,
        EBuildingServiceType::Fun,
        0.05f);
}
