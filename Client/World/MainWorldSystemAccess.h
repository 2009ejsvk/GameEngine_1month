#pragma once

#include "KnowledgeSystem.h"
#include "../Politics/ConstitutionTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <memory>
#include <string>

class CMainWorld;
class CWorld;
class CMainWorldSystemAccess;

class IMainWorldCitizenPolicyAccess
{
public:
    virtual ~IMainWorldCitizenPolicyAccess() = default;

    virtual const FGovernmentProfile& GetGovernmentProfile() const = 0;
    virtual const FGovernmentEdictModifiers& GetEdictModifiers() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
    virtual const FWorldCrisisStatus& GetWorldCrisisStatus() const = 0;
};

class IMainWorldKnowledgeAccess
{
public:
    virtual ~IMainWorldKnowledgeAccess() = default;

    virtual const FKnowledgeState& GetKnowledgeState() const = 0;
    virtual int GetKnowledgePoints() const = 0;
    virtual int GetDailyKnowledgeGeneration() const = 0;
    virtual bool IsResearchUnlocked(const std::wstring& Key) const = 0;
    virtual bool TryUnlockResearch(
        const std::wstring& Key,
        int Cost) = 0;
};

class IMainWorldConstitutionAccess
{
public:
    virtual ~IMainWorldConstitutionAccess() = default;

    virtual const FConstitutionState& GetConstitutionState() const = 0;
    virtual bool TrySelectConstitutionOption(
        EConstitutionOptionId Id) = 0;
};

std::shared_ptr<CMainWorldSystemAccess> CreateMainWorldSystemAccessAdapter(
    CMainWorld* Owner);

std::shared_ptr<IMainWorldCitizenPolicyAccess>
    ResolveMainWorldCitizenPolicyAccess(
        const std::shared_ptr<CWorld>& World);
IMainWorldCitizenPolicyAccess* ResolveMainWorldCitizenPolicyAccess(
    CWorld* World);

std::shared_ptr<IMainWorldKnowledgeAccess> ResolveMainWorldKnowledgeAccess(
    const std::shared_ptr<CWorld>& World);
IMainWorldKnowledgeAccess* ResolveMainWorldKnowledgeAccess(CWorld* World);

std::shared_ptr<IMainWorldConstitutionAccess>
    ResolveMainWorldConstitutionAccess(
        const std::shared_ptr<CWorld>& World);
IMainWorldConstitutionAccess* ResolveMainWorldConstitutionAccess(
    CWorld* World);
