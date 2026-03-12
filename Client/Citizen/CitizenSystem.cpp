#include "CitizenSystem.h"
#include "World/World.h"
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
                WealthRoll < 35 ?
                ECitizenWealthLevel::WellOff :
                ECitizenWealthLevel::Rich;
        }
        else if (Profile.EducationLevel ==
            ECitizenEducationLevel::HighSchool)
        {
            Profile.WealthLevel =
                WealthRoll < 70 ?
                ECitizenWealthLevel::Poor :
                ECitizenWealthLevel::WellOff;
        }
        else
        {
            Profile.WealthLevel =
                WealthRoll < 85 ?
                ECitizenWealthLevel::Poor :
                ECitizenWealthLevel::WellOff;
        }

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
                !B->IsHarbor() &&
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

    if (!MarkerOrbObj)
        return;

    MarkerOrbObj->SetIdentityProfile(BuildCitizenIdentityProfile());

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
        int Assigned = 0;
        float Accessibility = 0.f;
    };

    struct FServiceBuildingInfo
    {
        std::string Name;
        std::shared_ptr<CPlacementAreaObject> Building;
        int SatisfactionCap = 0;
        int VisitCapacity = 0;
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

        if (!IsAssignablePlacedBuilding(HomeBuilding) ||
            !HomeBuilding->IsResidential() ||
            HomeBuilding->GetCapacity() <= 0)
        {
            continue;
        }

        FHomeBuildingInfo Info;
        Info.Name = HomeNames[i];
        Info.Building = HomeBuilding;
        Info.Capacity = (std::max)(1, HomeBuilding->GetCapacity());
        Info.HousingCap = HomeBuilding->GetHousingSatisfactionCap();
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
        Info.Accessibility = ResolveAccessibilityScore(FunBuilding);
        FunInfos.push_back(std::move(Info));
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
        return;

    std::vector<int> OrbHomeIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbFoodIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbFunIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbWorkIndex(ActiveOrbs.size(), -1);

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

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        auto Orb = ActiveOrbs[i];

        if (!Orb)
            continue;

        const std::string& CurrentHome = Orb->GetHomeBuilding();

        if (!CurrentHome.empty())
        {
            auto HomeIt = HomeIndexByName.find(CurrentHome);

            if (HomeIt != HomeIndexByName.end())
            {
                const int HomeIdx = static_cast<int>(HomeIt->second);
                OrbHomeIndex[i] = HomeIdx;
                ++HomeInfos[HomeIdx].Assigned;
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

    auto AssignOrbToHome = [&](int OrbIdx, int HomeIdx) -> bool
    {
        if (OrbIdx < 0 || HomeIdx < 0)
            return false;

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            HomeIdx >= static_cast<int>(HomeInfos.size()))
        {
            return false;
        }

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return false;

        const int PrevHomeIdx = OrbHomeIndex[OrbIdx];

        if (PrevHomeIdx == HomeIdx)
            return true;

        if (PrevHomeIdx >= 0 &&
            PrevHomeIdx < static_cast<int>(HomeInfos.size()) &&
            HomeInfos[PrevHomeIdx].Assigned > 0)
        {
            --HomeInfos[PrevHomeIdx].Assigned;
        }

        auto& TargetInfo = HomeInfos[HomeIdx];

        if (Orb->GetHomeBuilding() != TargetInfo.Name)
            Orb->SetHomeBuilding(TargetInfo.Name);

        ++TargetInfo.Assigned;
        OrbHomeIndex[OrbIdx] = HomeIdx;
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

    auto ScoreHomeCandidate = [&](int OrbIdx, int HomeIdx) -> float
    {
        if (OrbIdx < 0 || HomeIdx < 0)
            return (std::numeric_limits<float>::lowest)();

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            HomeIdx >= static_cast<int>(HomeInfos.size()))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        auto& Info = HomeInfos[HomeIdx];
        const int EffectiveAssigned =
            Info.Assigned - (OrbHomeIndex[OrbIdx] == HomeIdx ? 1 : 0);
        const int OccupancyAfter = EffectiveAssigned + 1;
        const float QualityScore =
            ResolveSatisfactionScore(Info.HousingCap);
        const float AccessScore = Info.Accessibility;
        const float CongestionScore =
            ResolveCongestionScore(OccupancyAfter, Info.Capacity);
        const float DistanceScore =
            ResolveAverageDistanceAffinity(
                Info.Building,
                ResolveWorkBuildingForOrb(OrbIdx),
                ResolveFoodBuildingForOrb(OrbIdx),
                ResolveFunBuildingForOrb(OrbIdx));

        float Score =
            QualityScore * 0.38f +
            AccessScore * 0.20f +
            CongestionScore * 0.18f +
            DistanceScore * 0.24f;

        if (OrbHomeIndex[OrbIdx] == HomeIdx)
            Score += 0.05f;

        return Score;
    };

    auto ScoreServiceCandidate =
        [&](int OrbIdx,
            int ServiceIdx,
            const std::vector<FServiceBuildingInfo>& Infos,
            const std::vector<int>& OrbServiceIndex) -> float
    {
        if (OrbIdx < 0 || ServiceIdx < 0)
            return (std::numeric_limits<float>::lowest)();

        if (OrbIdx >= static_cast<int>(ActiveOrbs.size()) ||
            ServiceIdx >= static_cast<int>(Infos.size()))
        {
            return (std::numeric_limits<float>::lowest)();
        }

        const FServiceBuildingInfo& Info = Infos[ServiceIdx];
        const int EffectiveAssigned =
            Info.Assigned - (OrbServiceIndex[OrbIdx] == ServiceIdx ? 1 : 0);
        const int OccupancyAfter = EffectiveAssigned + 1;
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

        float Score =
            QualityScore * 0.34f +
            AccessScore * 0.18f +
            CongestionScore * 0.22f +
            DistanceScore * 0.26f;

        if (OrbServiceIndex[OrbIdx] == ServiceIdx)
            Score += 0.05f;

        return Score;
    };

    if (!HomeInfos.empty())
    {
        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            float CurrentScore =
                OrbHomeIndex[i] >= 0 ?
                    ScoreHomeCandidate(static_cast<int>(i), OrbHomeIndex[i]) :
                    (std::numeric_limits<float>::lowest)();
            float BestScore = CurrentScore;
            int BestIdx = OrbHomeIndex[i];

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
                (OrbHomeIndex[i] < 0 || BestScore > CurrentScore + 0.06f))
            {
                AssignOrbToHome(static_cast<int>(i), BestIdx);
            }
        }
    }

    if (!FoodInfos.empty())
    {
        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            if (IsFoodAssignmentLocked(static_cast<int>(i)))
                continue;

            float CurrentScore =
                OrbFoodIndex[i] >= 0 ?
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        OrbFoodIndex[i],
                        FoodInfos,
                        OrbFoodIndex) :
                    (std::numeric_limits<float>::lowest)();
            float BestScore = CurrentScore;
            int BestIdx = OrbFoodIndex[i];

            for (size_t FoodIdx = 0; FoodIdx < FoodInfos.size(); ++FoodIdx)
            {
                const float CandidateScore =
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        static_cast<int>(FoodIdx),
                        FoodInfos,
                        OrbFoodIndex);

                if (CandidateScore > BestScore + 0.0001f)
                {
                    BestScore = CandidateScore;
                    BestIdx = static_cast<int>(FoodIdx);
                }
            }

            if (BestIdx >= 0 &&
                (OrbFoodIndex[i] < 0 || BestScore > CurrentScore + 0.05f))
            {
                AssignOrbToFood(static_cast<int>(i), BestIdx);
            }
        }
    }

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
            Info.MinRequired = 1;
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

    while (true)
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

    if (!FunInfos.empty())
    {
        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            if (IsFunAssignmentLocked(static_cast<int>(i)))
                continue;

            float CurrentScore =
                OrbFunIndex[i] >= 0 ?
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        OrbFunIndex[i],
                        FunInfos,
                        OrbFunIndex) :
                    (std::numeric_limits<float>::lowest)();
            float BestScore = CurrentScore;
            int BestIdx = OrbFunIndex[i];

            for (size_t FunIdx = 0; FunIdx < FunInfos.size(); ++FunIdx)
            {
                const float CandidateScore =
                    ScoreServiceCandidate(
                        static_cast<int>(i),
                        static_cast<int>(FunIdx),
                        FunInfos,
                        OrbFunIndex);

                if (CandidateScore > BestScore + 0.0001f)
                {
                    BestScore = CandidateScore;
                    BestIdx = static_cast<int>(FunIdx);
                }
            }

            if (BestIdx >= 0 &&
                (OrbFunIndex[i] < 0 || BestScore > CurrentScore + 0.05f))
            {
                AssignOrbToFun(static_cast<int>(i), BestIdx);
            }
        }
    }
}
