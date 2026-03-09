#include "CitizenSystem.h"
#include "World/World.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Citizen/CitizenTypes.h"
#include <algorithm>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    constexpr float GNpcSpeedBase     = 140.f;
    constexpr float GNpcSpeedVariance = 21.f;

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

            if (!B || !B->GetAlive() || !B->HasPlacedArea())
                continue;

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

            if (!B || !B->GetAlive() || !B->HasPlacedArea())
                continue;

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

            if (!B || !B->GetAlive() || !B->HasPlacedArea())
                continue;

            // FoodProvider 건물은 직장 겸 음식 생산지로 포함
            // EntertainmentProvider 전용 건물(주점 등)은 제외
            if (!B->IsResidential() &&
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

            if (!B || !B->GetAlive() || !B->HasPlacedArea())
                continue;

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

            if (!B || !B->GetAlive() || !B->HasPlacedArea())
                continue;

            if (B->IsEntertainmentProvider())
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
    CollectCurrentBuildingNames(World, AllNames);
    CollectHomeBuildingNames(World, HomeNames);
    CollectWorkBuildingNames(World, WorkNames);
    CollectFoodBuildingNames(World, FoodNames);
    CollectEntertainmentBuildingNames(World, FunNames);

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

    const float Speed = GNpcSpeedBase +
        ((float)(rand() % 1001) / 500.f - 1.f) * GNpcSpeedVariance;
    MarkerOrbObj->SetMoveSpeed(Speed);
    ++SpawnedNpcCount;
}

void CitizenSystem::ReassignCitizenNeeds(CWorld* World)
{
    struct FFoodBuildingInfo
    {
        std::string Name;
        int FoodCap  = 0;
        int Assigned = 0;
    };

    struct FWorkBuildingInfo
    {
        std::string Name;
        int  Capacity      = 0;
        int  JobCap        = 0;
        int  Occupied      = 0;
        int  MinRequired   = 0;
        ECitizenEducationLevel RequiredEducation =
            ECitizenEducationLevel::Uneducated;
        bool IsFoodProvider = false;
    };

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    std::vector<std::string> AllNames;
    std::vector<std::string> HomeNames;
    std::vector<std::string> WorkNames;
    std::vector<std::string> FoodNames;
    std::vector<std::string> FunNames;
    CollectCurrentBuildingNames(World, AllNames);
    CollectHomeBuildingNames(World, HomeNames);
    CollectWorkBuildingNames(World, WorkNames);
    CollectFoodBuildingNames(World, FoodNames);
    CollectEntertainmentBuildingNames(World, FunNames);

    std::vector<FWorkBuildingInfo> WorkInfos;
    WorkInfos.reserve(WorkNames.size());

    for (size_t i = 0; i < WorkNames.size(); ++i)
    {
        auto WorkBuilding =
            World->FindObject<CPlacementAreaObject>(WorkNames[i]).lock();

        if (!WorkBuilding ||
            !WorkBuilding->GetAlive() ||
            !WorkBuilding->GetEnable() ||
            !WorkBuilding->HasPlacedArea())
        {
            continue;
        }

        FWorkBuildingInfo Info;
        Info.Name         = WorkNames[i];
        Info.Capacity     = (std::max)(0, WorkBuilding->GetCapacity());
        Info.JobCap       = WorkBuilding->GetJobSatisfactionCap();
        Info.RequiredEducation =
            WorkBuilding->GetRequiredEducationLevel();
        Info.IsFoodProvider = WorkBuilding->IsFoodProvider();
        WorkInfos.push_back(std::move(Info));
    }

    std::sort(WorkInfos.begin(), WorkInfos.end(),
        [](const FWorkBuildingInfo& A, const FWorkBuildingInfo& B)
        {
            if (A.JobCap != B.JobCap)
                return A.JobCap > B.JobCap;

            if (A.Capacity != B.Capacity)
                return A.Capacity > B.Capacity;

            return A.Name < B.Name;
        });

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

        const ECitizenState OrbState  = Orb->GetCitizenState();
        const bool IsInFoodVisit =
            OrbState == ECitizenState::GoingToFood ||
            OrbState == ECitizenState::AtFood;

        if (Orb->GetHomeBuilding().empty() && !HomeNames.empty())
            Orb->SetHomeBuilding(PickRandomBuildingName(HomeNames));

        if (!IsInFoodVisit &&
            Orb->GetFoodBuilding().empty() &&
            !FoodNames.empty())
        {
            Orb->SetFoodBuilding(PickRandomBuildingName(FoodNames));
        }

        if (Orb->GetFunBuilding().empty() && !FunNames.empty())
            Orb->SetFunBuilding(PickRandomBuildingName(FunNames));
    }

    if (ActiveOrbs.empty())
        return;

    // ── 음식 배정 ──────────────────────────────────────────────────────────
    std::vector<FFoodBuildingInfo> FoodInfos;
    FoodInfos.reserve(FoodNames.size());

    for (size_t i = 0; i < FoodNames.size(); ++i)
    {
        auto FoodBuilding =
            World->FindObject<CPlacementAreaObject>(FoodNames[i]).lock();

        if (!FoodBuilding ||
            !FoodBuilding->GetAlive() ||
            !FoodBuilding->GetEnable() ||
            !FoodBuilding->HasPlacedArea())
        {
            continue;
        }

        FFoodBuildingInfo Info;
        Info.Name    = FoodNames[i];
        Info.FoodCap = FoodBuilding->GetFoodSatisfactionCap();
        FoodInfos.push_back(std::move(Info));
    }

    std::sort(FoodInfos.begin(), FoodInfos.end(),
        [](const FFoodBuildingInfo& A, const FFoodBuildingInfo& B)
        {
            if (A.FoodCap != B.FoodCap)
                return A.FoodCap > B.FoodCap;

            return A.Name < B.Name;
        });

    std::unordered_map<std::string, size_t> FoodIndexByName;
    FoodIndexByName.reserve(FoodInfos.size());

    for (size_t i = 0; i < FoodInfos.size(); ++i)
        FoodIndexByName.emplace(FoodInfos[i].Name, i);

    std::vector<int> OrbFoodIndex(ActiveOrbs.size(), -1);

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

    auto FindLeastLoadedFood = [&]() -> int
    {
        if (FoodInfos.empty())
            return -1;

        int BestIdx = -1;

        for (size_t i = 0; i < FoodInfos.size(); ++i)
        {
            if (BestIdx < 0 ||
                FoodInfos[i].Assigned < FoodInfos[BestIdx].Assigned ||
                (FoodInfos[i].Assigned == FoodInfos[BestIdx].Assigned &&
                 FoodInfos[i].FoodCap  > FoodInfos[BestIdx].FoodCap))
            {
                BestIdx = static_cast<int>(i);
            }
        }

        return BestIdx;
    };

    auto FindDonorFood = [&]() -> int
    {
        int DonorIdx = -1;

        for (size_t i = 0; i < FoodInfos.size(); ++i)
        {
            if (FoodInfos[i].Assigned <= 1)
                continue;

            if (DonorIdx < 0 ||
                FoodInfos[i].Assigned > FoodInfos[DonorIdx].Assigned)
            {
                DonorIdx = static_cast<int>(i);
            }
        }

        return DonorIdx;
    };

    auto FindOrbAssignedToFood = [&](int FoodIdx) -> int
    {
        for (size_t i = 0; i < OrbFoodIndex.size(); ++i)
        {
            if (OrbFoodIndex[i] == FoodIdx &&
                !IsFoodAssignmentLocked(static_cast<int>(i)))
            {
                return static_cast<int>(i);
            }
        }

        return -1;
    };

    if (!FoodInfos.empty())
    {
        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            auto Orb = ActiveOrbs[i];

            if (!Orb)
                continue;

            const std::string& CurrentFood = Orb->GetFoodBuilding();

            if (CurrentFood.empty())
                continue;

            auto FoodIt = FoodIndexByName.find(CurrentFood);

            if (FoodIt == FoodIndexByName.end())
            {
                Orb->SetFoodBuilding("");
                continue;
            }

            const int FoodIdx = static_cast<int>(FoodIt->second);
            OrbFoodIndex[i] = FoodIdx;
            ++FoodInfos[FoodIdx].Assigned;
        }

        for (size_t i = 0; i < ActiveOrbs.size(); ++i)
        {
            if (OrbFoodIndex[i] >= 0)
                continue;

            if (IsFoodAssignmentLocked(static_cast<int>(i)))
                continue;

            const int BestFoodIdx = FindLeastLoadedFood();

            if (BestFoodIdx < 0)
                break;

            AssignOrbToFood(static_cast<int>(i), BestFoodIdx);
        }

        const bool CanCoverAllFoodBuildings =
            ActiveOrbs.size() >= FoodInfos.size();

        if (CanCoverAllFoodBuildings)
        {
            for (size_t FoodIdx = 0;
                 FoodIdx < FoodInfos.size();
                 ++FoodIdx)
            {
                if (FoodInfos[FoodIdx].Assigned > 0)
                    continue;

                const int DonorFoodIdx = FindDonorFood();

                if (DonorFoodIdx < 0)
                    break;

                const int DonorOrbIdx =
                    FindOrbAssignedToFood(DonorFoodIdx);

                if (DonorOrbIdx < 0)
                    break;

                AssignOrbToFood(DonorOrbIdx, static_cast<int>(FoodIdx));
            }
        }
    }

    if (WorkInfos.empty())
        return;

    // ── 직장 배정 ──────────────────────────────────────────────────────────
    std::vector<int> OrbWorkIndex(ActiveOrbs.size(), -1);
    std::vector<int> OrbWorkCap(ActiveOrbs.size(), -1);
    std::vector<int> UnemployedOrbIndices;
    UnemployedOrbIndices.reserve(ActiveOrbs.size());

    std::unordered_map<std::string, int> FoodDemandByBuilding;
    FoodDemandByBuilding.reserve(FoodNames.size());

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        auto Orb = ActiveOrbs[i];

        if (!Orb)
            continue;

        const std::string& FoodName = Orb->GetFoodBuilding();

        if (!FoodName.empty())
            ++FoodDemandByBuilding[FoodName];
    }

    for (size_t i = 0; i < WorkInfos.size(); ++i)
    {
        auto& Info = WorkInfos[i];

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

        if (!Orb)
            return false;

        auto& TargetInfo = WorkInfos[WorkIdx];

        if (!CanOrbWorkAt(OrbIdx, WorkIdx))
            return false;

        const int PrevWorkIdx = OrbWorkIndex[OrbIdx];

        if (PrevWorkIdx == WorkIdx)
            return true;

        if (PrevWorkIdx >= 0 &&
            PrevWorkIdx < static_cast<int>(WorkInfos.size()))
        {
            auto& PrevInfo = WorkInfos[PrevWorkIdx];

            if (PrevInfo.Occupied <= PrevInfo.MinRequired)
                return false;

            if (PrevInfo.Occupied > 0)
                --PrevInfo.Occupied;
        }

        if (TargetInfo.Capacity <= 0 ||
            TargetInfo.Occupied >= TargetInfo.Capacity)
        {
            return false;
        }

        const std::string& TargetName = TargetInfo.Name;

        if (Orb->GetWorkBuilding() != TargetName)
            Orb->SetWorkBuilding(TargetName);

        ++TargetInfo.Occupied;
        OrbWorkIndex[OrbIdx] = WorkIdx;
        OrbWorkCap[OrbIdx]   = TargetInfo.JobCap;
        return true;
    };

    auto FindBestVacancyWork = [&](int OrbIdx, int MinJobCapExclusive) -> int
    {
        if (OrbIdx < 0 || OrbIdx >= static_cast<int>(ActiveOrbs.size()))
            return -1;

        auto Orb = ActiveOrbs[OrbIdx];

        if (!Orb)
            return -1;

        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            const FWorkBuildingInfo& Info = WorkInfos[i];

            if (Info.JobCap <= MinJobCapExclusive)
                break;

            if (Info.Capacity <= 0)
                continue;

            if (!CanOrbWorkAt(OrbIdx, static_cast<int>(i)))
                continue;

            if (Info.Occupied < Info.Capacity)
                return static_cast<int>(i);
        }

        return -1;
    };

    auto FindFoodDeficitWork = [&]() -> int
    {
        for (size_t i = 0; i < WorkInfos.size(); ++i)
        {
            const FWorkBuildingInfo& Info = WorkInfos[i];

            if (!Info.IsFoodProvider || Info.MinRequired <= 0)
                continue;

            if (Info.Capacity <= 0)
                continue;

            if (Info.Occupied < Info.MinRequired &&
                Info.Occupied < Info.Capacity)
            {
                return static_cast<int>(i);
            }
        }

        return -1;
    };

    auto FindDonorWork = [&](int ExcludeWorkIdx) -> int
    {
        for (int i = static_cast<int>(WorkInfos.size()) - 1; i >= 0; --i)
        {
            if (i == ExcludeWorkIdx)
                continue;

            const FWorkBuildingInfo& Info = WorkInfos[i];

            if (Info.Occupied > Info.MinRequired)
                return i;
        }

        return -1;
    };

    auto FindOrbAssignedToWork = [&](int WorkIdx, int TargetWorkIdx) -> int
    {
        for (size_t i = 0; i < OrbWorkIndex.size(); ++i)
        {
            if (OrbWorkIndex[i] != WorkIdx)
                continue;

            if (TargetWorkIdx >= 0 &&
                !CanOrbWorkAt(static_cast<int>(i), TargetWorkIdx))
                continue;

            if (OrbWorkIndex[i] == WorkIdx)
                return static_cast<int>(i);
        }

        return -1;
    };

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        auto Orb = ActiveOrbs[i];

        if (!Orb)
            continue;

        const std::string& CurrentWork = Orb->GetWorkBuilding();

        if (CurrentWork.empty())
        {
            UnemployedOrbIndices.push_back(static_cast<int>(i));
            continue;
        }

        auto WorkIt = WorkIndexByName.find(CurrentWork);

        if (WorkIt == WorkIndexByName.end())
        {
            Orb->SetWorkBuilding("");
            UnemployedOrbIndices.push_back(static_cast<int>(i));
            continue;
        }

        const int WorkIdx = static_cast<int>(WorkIt->second);

        if (static_cast<int>(Orb->GetIdentityProfile().EducationLevel) <
            static_cast<int>(WorkInfos[WorkIdx].RequiredEducation))
        {
            Orb->SetWorkBuilding("");
            UnemployedOrbIndices.push_back(static_cast<int>(i));
            continue;
        }

        OrbWorkIndex[i] = WorkIdx;
        OrbWorkCap[i]   = WorkInfos[WorkIdx].JobCap;
        ++WorkInfos[WorkIdx].Occupied;
    }

    for (size_t WorkIdx = 0; WorkIdx < WorkInfos.size(); ++WorkIdx)
    {
        FWorkBuildingInfo& Info = WorkInfos[WorkIdx];

        if (Info.Capacity < 0)
            Info.Capacity = 0;

        if (Info.Occupied <= Info.Capacity)
            continue;

        int Overflow = Info.Occupied - Info.Capacity;

        for (size_t OrbIdx = 0;
             OrbIdx < ActiveOrbs.size() && Overflow > 0;
             ++OrbIdx)
        {
            if (OrbWorkIndex[OrbIdx] != static_cast<int>(WorkIdx))
                continue;

            auto Orb = ActiveOrbs[OrbIdx];

            if (!Orb)
                continue;

            Orb->SetWorkBuilding("");
            OrbWorkIndex[OrbIdx] = -1;
            OrbWorkCap[OrbIdx]   = -1;

            if (Info.Occupied > 0)
                --Info.Occupied;

            --Overflow;
            UnemployedOrbIndices.push_back(static_cast<int>(OrbIdx));
        }
    }

    while (true)
    {
        const int DeficitWorkIdx = FindFoodDeficitWork();

        if (DeficitWorkIdx < 0)
            break;

        bool Assigned = false;

        for (size_t i = 0; i < UnemployedOrbIndices.size(); ++i)
        {
            const int OrbIdx = UnemployedOrbIndices[i];

            if (OrbIdx < 0 ||
                OrbIdx >= static_cast<int>(OrbWorkIndex.size()))
            {
                continue;
            }

            if (OrbWorkIndex[OrbIdx] >= 0)
                continue;

            if (AssignOrbToWork(OrbIdx, DeficitWorkIdx))
            {
                Assigned = true;
                break;
            }
        }

        if (Assigned)
            continue;

        const int DonorWorkIdx = FindDonorWork(DeficitWorkIdx);

        if (DonorWorkIdx < 0)
            break;

        const int DonorOrbIdx =
            FindOrbAssignedToWork(DonorWorkIdx, DeficitWorkIdx);

        if (DonorOrbIdx < 0)
            break;

        if (!AssignOrbToWork(DonorOrbIdx, DeficitWorkIdx))
            break;
    }

    for (size_t i = 0; i < UnemployedOrbIndices.size(); ++i)
    {
        const int OrbIdx = UnemployedOrbIndices[i];

        if (OrbIdx < 0 ||
            OrbIdx >= static_cast<int>(OrbWorkIndex.size()))
        {
            continue;
        }

        if (OrbWorkIndex[OrbIdx] >= 0)
            continue;

        const int BestWorkIdx = FindBestVacancyWork(OrbIdx, -1);

        if (BestWorkIdx < 0)
            break;

        AssignOrbToWork(OrbIdx, BestWorkIdx);
    }

    for (size_t i = 0; i < ActiveOrbs.size(); ++i)
    {
        if (OrbWorkIndex[i] < 0)
            continue;

        const int CurrentWorkIdx = OrbWorkIndex[i];

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
            FindBestVacancyWork(static_cast<int>(i), OrbWorkCap[i]);

        if (BetterWorkIdx < 0)
            continue;

        AssignOrbToWork(static_cast<int>(i), BetterWorkIdx);
    }
}
