#include "CitizenInfoWorldQuerySourceInternal.h"
#include "../StringUtils.h"
#include <algorithm>

using namespace CitizenInfoWorldQuerySourceInternal;

bool CWorldCitizenInfoQuerySource::TryGetCitizenRecord(
    const std::string& CitizenName,
    CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord) const
{
    return mCitizenQuery.TryGetCitizenRecord(CitizenName, OutRecord);
}

std::wstring CWorldCitizenInfoQuerySource::ResolveBuildingDisplayName(
    const std::string& BuildingName) const
{
    return mCitizenQuery.ResolveBuildingDisplayName(BuildingName);
}

bool CWorldCitizenInfoCitizenQuery::TryGetCitizenRecord(
    const std::string& CitizenName,
    CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord) const
{
    auto Citizen = FindValidCitizen(CitizenName);

    if (!Citizen)
        return false;

    OutRecord =
        CitizenInfoDataProvider::FCitizenInfoCitizenRecord();
    OutRecord.Valid = true;
    OutRecord.Name = Citizen->GetName();
    OutRecord.Satisfaction = Citizen->GetSatisfaction();
    OutRecord.IdentityProfile = Citizen->GetIdentityProfile();
    OutRecord.PoliticalProfile = Citizen->GetPoliticalProfile();
    OutRecord.State = Citizen->GetCitizenState();
    OutRecord.HomeBuildingName = Citizen->GetHomeBuilding();
    OutRecord.WorkBuildingName = Citizen->GetWorkBuilding();
    OutRecord.FoodBuildingName = Citizen->GetFoodBuilding();
    OutRecord.FoodVisitBuildingName = Citizen->GetFoodVisitBuilding();
    OutRecord.FunBuildingName = Citizen->GetFunBuilding();
    OutRecord.FunVisitBuildingName = Citizen->GetFunVisitBuilding();
    OutRecord.HealthBuildingName = Citizen->GetHealthBuilding();
    OutRecord.HealthVisitBuildingName =
        Citizen->GetHealthVisitBuilding();
    OutRecord.FaithBuildingName = Citizen->GetFaithBuilding();
    OutRecord.FaithVisitBuildingName =
        Citizen->GetFaithVisitBuilding();
    return true;
}

std::wstring CWorldCitizenInfoCitizenQuery::ResolveBuildingDisplayName(
    const std::string& BuildingName) const
{
    if (BuildingName.empty())
        return L"-";

    auto Building = FindValidBuilding(BuildingName);

    if (!Building)
        return StringUtils::Utf8ToWide(BuildingName);

    const std::wstring DisplayName =
        StringUtils::Utf8ToWide(Building->GetBuildingDisplayName());
    return DisplayName.empty() ?
        StringUtils::Utf8ToWide(BuildingName) :
        DisplayName;
}

std::shared_ptr<CPlacementAreaObject>
    CWorldCitizenInfoCitizenQuery::FindValidBuilding(
        const std::string& BuildingName) const
{
    if (!mOwner.mWorld || BuildingName.empty())
        return nullptr;

    auto Building =
        mOwner.mWorld->FindObject<CPlacementAreaObject>(BuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return nullptr;

    return Building;
}

std::shared_ptr<CBuildingMarkerOrb>
    CWorldCitizenInfoCitizenQuery::FindValidCitizen(
        const std::string& CitizenName) const
{
    if (!mOwner.mWorld || CitizenName.empty())
        return nullptr;

    auto Citizen =
        mOwner.mWorld->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

    if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
        return nullptr;

    return Citizen;
}

void CWorldCitizenInfoCitizenQuery::PopulateCitizenAssignments(
    const std::string& BuildingName,
    CPlacementAreaObject& Building,
    CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord) const
{
    if (!mOwner.mWorld)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

    if (!mOwner.mWorld->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        return;

    for (size_t Index = 0; Index < OrbList.size(); ++Index)
    {
        auto Orb = OrbList[Index].lock();

        if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
            continue;

        const std::string OrbName = Orb->GetName();

        if (Orb->GetHomeBuilding() == BuildingName)
            PushUnique(OutRecord.Residents, OrbName);

        if (OutRecord.WorkProvider &&
            Orb->GetWorkBuilding() == BuildingName)
        {
            PushUnique(OutRecord.AssignedEmployees, OrbName);

            if (Orb->GetCitizenState() == ECitizenState::AtWork)
            {
                PushUnique(
                    OutRecord.WorkingEmployees,
                    OrbName);
            }
        }

        if (OutRecord.EntertainmentProvider &&
            Orb->GetFunBuilding() == BuildingName)
        {
            PushUnique(OutRecord.AssignedVisitors, OrbName);

            if (Orb->GetCitizenState() == ECitizenState::AtFun)
                PushUnique(OutRecord.ArrivedVisitors, OrbName);
        }

        if (!OutRecord.FoodProvider)
            continue;

        const ECitizenState OrbState = Orb->GetCitizenState();

        if (OrbState == ECitizenState::AtFood)
        {
            const std::string& VisitFoodBuilding =
                Orb->GetFoodVisitBuilding();
            const bool IsVisitBuildingMatched =
                VisitFoodBuilding == BuildingName ||
                (VisitFoodBuilding.empty() &&
                    Orb->GetFoodBuilding() == BuildingName);

            if (!IsVisitBuildingMatched)
                continue;

            PushUnique(OutRecord.AssignedVisitors, OrbName);
            PushUnique(OutRecord.ArrivedVisitors, OrbName);
            continue;
        }

        if (OrbState != ECitizenState::GoingToFood ||
            Orb->GetFoodBuilding() != BuildingName)
        {
            continue;
        }

        FVector3 MarkerPos = FVector3::Zero;

        if (!Building.GetClosestMarkerWorldPos(
            Orb->GetWorldPos(),
            MarkerPos))
        {
            continue;
        }

        FVector3 OrbPos = Orb->GetWorldPos();
        OrbPos.z = MarkerPos.z;

        const float NearDistance =
            (std::max)(8.f, Orb->GetArrivalDistance() * 2.f);

        if (OrbPos.Distance(MarkerPos) > NearDistance)
            continue;

        PushUnique(OutRecord.AssignedVisitors, OrbName);
        PushUnique(OutRecord.IncomingVisitors, OrbName);
    }

    std::sort(OutRecord.Residents.begin(), OutRecord.Residents.end());
    std::sort(
        OutRecord.AssignedEmployees.begin(),
        OutRecord.AssignedEmployees.end());
    std::sort(
        OutRecord.WorkingEmployees.begin(),
        OutRecord.WorkingEmployees.end());
    std::sort(
        OutRecord.AssignedVisitors.begin(),
        OutRecord.AssignedVisitors.end());
    std::sort(
        OutRecord.ArrivedVisitors.begin(),
        OutRecord.ArrivedVisitors.end());
    std::sort(
        OutRecord.IncomingVisitors.begin(),
        OutRecord.IncomingVisitors.end());
}
