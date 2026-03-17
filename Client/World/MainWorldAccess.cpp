#include "MainWorld.h"
#include "MainWorldAccess.h"

namespace
{
    template <typename TAccess, typename TSharedGetter>
    std::shared_ptr<TAccess> ResolveMainWorldSharedAccess(
        const std::shared_ptr<CWorld>& World,
        TSharedGetter Getter)
    {
        if (!World)
            return nullptr;

        if (auto Access = std::dynamic_pointer_cast<TAccess>(World))
            return Access;

        auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);
        return MainWorld ? (MainWorld.get()->*Getter)() : nullptr;
    }

    template <typename TAccess, typename TRawGetter>
    TAccess* ResolveMainWorldRawAccess(
        CWorld* World,
        TRawGetter Getter)
    {
        if (!World)
            return nullptr;

        if (auto Access = dynamic_cast<TAccess*>(World))
            return Access;

        auto MainWorld = dynamic_cast<CMainWorld*>(World);
        return MainWorld ? (MainWorld->*Getter)() : nullptr;
    }
}

CMainWorld::CMainWorld()
{
    mServices.ElectionService.reset(new CMainWorldElectionService());
    mServices.PoliticalDemandService.reset(new CMainWorldPoliticalDemandService());
    mServices.WorldCrisisService.reset(new CMainWorldWorldCrisisService());
    mAccess.UiRead = CreateMainWorldUiReadAccessAdapter(this);
    mAccess.SystemRead = CreateMainWorldSystemAccessAdapter(this);
    mAccess.InfrastructureRead =
        CreateMainWorldInfrastructureReadAccessAdapter(this);
    mAccess.BuildingControl =
        CreateMainWorldBuildingControlAccessAdapter(this);
    mAccess.Trade = CreateMainWorldTradeAccessAdapter(this);
    mAccess.GovernmentCommand = CreateGovernmentCommandServiceAdapter(this);
}

CMainWorld::~CMainWorld()
{
}

std::shared_ptr<IMainWorldBuildMenuAccess> ResolveMainWorldBuildMenuAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldBuildMenuAccess>(
        World,
        &CMainWorld::GetBuildMenuAccessHandle);
}

IMainWorldBuildMenuAccess* ResolveMainWorldBuildMenuAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldBuildMenuAccess>(
        World,
        &CMainWorld::GetBuildMenuAccessRaw);
}

std::shared_ptr<IMainWorldHudAccess> ResolveMainWorldHudAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldHudAccess>(
        World,
        &CMainWorld::GetHudAccessHandle);
}

IMainWorldHudAccess* ResolveMainWorldHudAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldHudAccess>(
        World,
        &CMainWorld::GetHudAccessRaw);
}

std::shared_ptr<IMainWorldAlmanacAccess> ResolveMainWorldAlmanacAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldAlmanacAccess>(
        World,
        &CMainWorld::GetAlmanacAccessHandle);
}

IMainWorldAlmanacAccess* ResolveMainWorldAlmanacAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldAlmanacAccess>(
        World,
        &CMainWorld::GetAlmanacAccessRaw);
}

std::shared_ptr<IMainWorldEdictReadAccess> ResolveMainWorldEdictReadAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldEdictReadAccess>(
        World,
        &CMainWorld::GetEdictReadAccessHandle);
}

IMainWorldEdictReadAccess* ResolveMainWorldEdictReadAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldEdictReadAccess>(
        World,
        &CMainWorld::GetEdictReadAccessRaw);
}

std::shared_ptr<IMainWorldCitizenPolicyAccess>
    ResolveMainWorldCitizenPolicyAccess(
        const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldCitizenPolicyAccess>(
        World,
        &CMainWorld::GetCitizenPolicyAccessHandle);
}

IMainWorldCitizenPolicyAccess* ResolveMainWorldCitizenPolicyAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldCitizenPolicyAccess>(
        World,
        &CMainWorld::GetCitizenPolicyAccessRaw);
}

std::shared_ptr<IMainWorldRoadNetworkAccess> ResolveMainWorldRoadNetworkAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldRoadNetworkAccess>(
        World,
        &CMainWorld::GetRoadNetworkAccessHandle);
}

IMainWorldRoadNetworkAccess* ResolveMainWorldRoadNetworkAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldRoadNetworkAccess>(
        World,
        &CMainWorld::GetRoadNetworkAccessRaw);
}

std::shared_ptr<IMainWorldTransitAccess> ResolveMainWorldTransitAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldTransitAccess>(
        World,
        &CMainWorld::GetTransitAccessHandle);
}

IMainWorldTransitAccess* ResolveMainWorldTransitAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldTransitAccess>(
        World,
        &CMainWorld::GetTransitAccessRaw);
}

std::shared_ptr<IMainWorldRuntimeRefreshAccess>
    ResolveMainWorldRuntimeRefreshAccess(
        const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldRuntimeRefreshAccess>(
        World,
        &CMainWorld::GetRuntimeRefreshAccessHandle);
}

IMainWorldRuntimeRefreshAccess* ResolveMainWorldRuntimeRefreshAccess(
    CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldRuntimeRefreshAccess>(
        World,
        &CMainWorld::GetRuntimeRefreshAccessRaw);
}

std::shared_ptr<IMainWorldBuildingConditionAccess>
    ResolveMainWorldBuildingConditionAccess(
        const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldBuildingConditionAccess>(
        World,
        &CMainWorld::GetBuildingConditionAccessHandle);
}

IMainWorldBuildingConditionAccess* ResolveMainWorldBuildingConditionAccess(
    CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldBuildingConditionAccess>(
        World,
        &CMainWorld::GetBuildingConditionAccessRaw);
}

std::shared_ptr<IMainWorldTradeAccess> ResolveMainWorldTradeAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldTradeAccess>(
        World,
        &CMainWorld::GetTradeAccessHandle);
}

IMainWorldTradeAccess* ResolveMainWorldTradeAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldTradeAccess>(
        World,
        &CMainWorld::GetTradeAccessRaw);
}

std::shared_ptr<IMainWorldKnowledgeAccess> ResolveMainWorldKnowledgeAccess(
    const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldKnowledgeAccess>(
        World,
        &CMainWorld::GetKnowledgeAccessHandle);
}

IMainWorldKnowledgeAccess* ResolveMainWorldKnowledgeAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldKnowledgeAccess>(
        World,
        &CMainWorld::GetKnowledgeAccessRaw);
}

std::shared_ptr<IMainWorldConstitutionAccess>
    ResolveMainWorldConstitutionAccess(
        const std::shared_ptr<CWorld>& World)
{
    return ResolveMainWorldSharedAccess<IMainWorldConstitutionAccess>(
        World,
        &CMainWorld::GetConstitutionAccessHandle);
}

IMainWorldConstitutionAccess* ResolveMainWorldConstitutionAccess(CWorld* World)
{
    return ResolveMainWorldRawAccess<IMainWorldConstitutionAccess>(
        World,
        &CMainWorld::GetConstitutionAccessRaw);
}
