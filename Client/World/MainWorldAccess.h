#pragma once

#include "../Politics/PoliticalTypes.h"
#include <string>
#include <vector>

class IMainWorldAccess
{
public:
    virtual ~IMainWorldAccess() = default;

    virtual long long GetNationalBudget() const = 0;
    virtual int GetSimulationYear() const = 0;
    virtual int GetSimulationMonth() const = 0;
    virtual int GetSimulationDay() const = 0;
    virtual int GetSimulationMonthDayCount() const = 0;
    virtual float GetSimulationDayProgress() const = 0;
    virtual float GetSimulationMonthProgress() const = 0;
    virtual bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage) = 0;
    virtual bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage) = 0;
    virtual const FGovernmentProfile& GetGovernmentProfile() const = 0;
    virtual const FTaxPolicy& GetTaxPolicy() const = 0;
    virtual const FPoliticalWorldSnapshot& GetPoliticalSnapshot() const = 0;
    virtual const std::vector<FGovernmentEdictState>&
        GetGovernmentEdictStates() const = 0;
    virtual const FGovernmentEdictState* GetGovernmentEdictState(
        EGovernmentEdictType Type) const = 0;
    virtual const FGovernmentEdictModifiers& GetEdictModifiers() const = 0;
    virtual long long GetLastDailyEdictCost() const = 0;
    virtual long long GetLastDailyExportIncome() const = 0;
    virtual long long GetLastDailyTaxIncome() const = 0;
    virtual long long GetLastDailyConsumptionTaxIncome() const = 0;
    virtual long long GetLastDailyIncomeTaxIncome() const = 0;
    virtual long long GetLastDailyPropertyTaxIncome() const = 0;
    virtual double GetLastDailyTaxCollectionEfficiency() const = 0;
    virtual long long GetLastDailyNetChange() const = 0;
    virtual const FElectionStatus& GetElectionStatus() const = 0;
    virtual int GetDaysUntilNextElection() const = 0;
    virtual double GetElectionWarningScore() const = 0;
    virtual const FTaxPolicyEventStatus& GetTaxPolicyEventStatus() const = 0;
};
