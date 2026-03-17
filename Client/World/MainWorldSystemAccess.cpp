#include "MainWorld.h"
#include "MainWorldSystemAccess.h"

class CMainWorldSystemAccess final :
    public IMainWorldCitizenPolicyAccess,
    public IMainWorldKnowledgeAccess,
    public IMainWorldConstitutionAccess
{
public:
    explicit CMainWorldSystemAccess(CMainWorld* Owner)
        : mOwner(Owner)
    {
    }

    const FGovernmentProfile& GetGovernmentProfile() const override
    {
        return mOwner->GetGovernmentProfile();
    }

    const FGovernmentEdictModifiers& GetEdictModifiers() const override
    {
        return mOwner->GetEdictModifiers();
    }

    const FTaxPolicy& GetTaxPolicy() const override
    {
        return mOwner->GetTaxPolicy();
    }

    const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const override
    {
        return mOwner->GetTaxPolicyEventStatus();
    }

    const FWorldCrisisStatus& GetWorldCrisisStatus() const override
    {
        return mOwner->GetWorldCrisisStatus();
    }

    const FKnowledgeState& GetKnowledgeState() const override
    {
        return mOwner->GetKnowledgeState();
    }

    int GetKnowledgePoints() const override
    {
        return mOwner->GetKnowledgePoints();
    }

    int GetDailyKnowledgeGeneration() const override
    {
        return mOwner->GetDailyKnowledgeGeneration();
    }

    bool IsResearchUnlocked(const std::wstring& Key) const override
    {
        return mOwner->IsResearchUnlocked(Key);
    }

    bool TryUnlockResearch(
        const std::wstring& Key,
        int Cost) override
    {
        return mOwner->TryUnlockResearch(Key, Cost);
    }

    const FConstitutionState& GetConstitutionState() const override
    {
        return mOwner->GetConstitutionState();
    }

    bool TrySelectConstitutionOption(EConstitutionOptionId Id) override
    {
        return mOwner->TrySelectConstitutionOption(Id);
    }

private:
    CMainWorld* mOwner = nullptr;
};

std::shared_ptr<CMainWorldSystemAccess> CreateMainWorldSystemAccessAdapter(
    CMainWorld* Owner)
{
    return std::shared_ptr<CMainWorldSystemAccess>(
        new CMainWorldSystemAccess(Owner));
}

std::shared_ptr<IMainWorldCitizenPolicyAccess>
    CMainWorld::GetCitizenPolicyAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldCitizenPolicyAccess>(
        mAccess.SystemRead);
}

IMainWorldCitizenPolicyAccess* CMainWorld::GetCitizenPolicyAccessRaw() const
{
    return static_cast<IMainWorldCitizenPolicyAccess*>(mAccess.SystemRead.get());
}

std::shared_ptr<IMainWorldKnowledgeAccess>
    CMainWorld::GetKnowledgeAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldKnowledgeAccess>(
        mAccess.SystemRead);
}

IMainWorldKnowledgeAccess* CMainWorld::GetKnowledgeAccessRaw() const
{
    return static_cast<IMainWorldKnowledgeAccess*>(mAccess.SystemRead.get());
}

std::shared_ptr<IMainWorldConstitutionAccess>
    CMainWorld::GetConstitutionAccessHandle() const
{
    return std::static_pointer_cast<IMainWorldConstitutionAccess>(
        mAccess.SystemRead);
}

IMainWorldConstitutionAccess* CMainWorld::GetConstitutionAccessRaw() const
{
    return static_cast<IMainWorldConstitutionAccess*>(mAccess.SystemRead.get());
}
