#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include "../Economy/EconomySystem.h"
#include <algorithm>
#include <string>

void CMainWorld::InitializeElectionSchedule()
{
    PoliticsSystem::InitializeElectionSchedule(
        mElectionStatus,
        mSimulationYear,
        MainWorldConfig::GInitialElectionLeadYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

void CMainWorld::ResolveScheduledElection()
{
    RefreshPoliticalSnapshot();
    PoliticsSystem::ResolveScheduledElection(
        mElectionStatus,
        mPoliticalSnapshot,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        MainWorldConfig::GElectionIntervalYears,
        MainWorldConfig::GElectionMonth,
        MainWorldConfig::GElectionDay);
}

int CMainWorld::GetDaysUntilNextElection() const
{
    return PoliticsSystem::GetDaysUntilNextElection(
        mElectionStatus,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

double CMainWorld::GetElectionWarningScore() const
{
    return PoliticsSystem::GetElectionWarningScore(
        mElectionStatus,
        mPoliticalSnapshot,
        mTaxEventStatus,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay);
}

void CMainWorld::ApplyDailyEconomySettlement()
{
    const int DaysInMonth = GetDaysInMonth(mSimulationYear, mSimulationMonth);
    const auto Result = EconomySystem::ApplyDailyWorldSettlement(
        this,
        DaysInMonth,
        mGovernmentProfile,
        mTaxEventStatus,
        mGovernmentEdicts,
        mEdictModifiers);
    mLastDailyWageCost     = Result.BaseResult.WageCost;
    mLastDailyUpkeepCost   = Result.BaseResult.UpkeepCost;
    mLastDailyExportIncome = Result.BaseResult.ExportIncome;
    mLastDailyTaxIncome    = Result.AdjustedTaxIncome;
    mLastDailyConsumptionTaxIncome = Result.AdjustedConsumptionTaxIncome;
    mLastDailyIncomeTaxIncome = Result.AdjustedIncomeTaxIncome;
    mLastDailyPropertyTaxIncome = Result.AdjustedPropertyTaxIncome;
    mLastDailyEdictCost    = Result.DailyEdictCost;
    mLastDailyImportExpense = Result.BaseResult.ImportExpense;
    mLastDailyTaxCollectionEfficiency =
        Result.BaseResult.TaxCollectionEfficiency;
    mLastDailyNetChange = Result.NetBudgetChange;
    mNationalBudget += mLastDailyNetChange;
}

bool CMainWorld::TryApplyEdict(
    EGovernmentEdictType Type,
    std::wstring& OutMessage)
{
    const FGovernmentEdictDefinition* Definition =
        EdictSystem::FindGovernmentEdictDefinition(Type);

    if (!Definition)
    {
        OutMessage = L"정의되지 않은 칙령입니다.";
        return false;
    }

    if (!Definition->Implemented)
    {
        OutMessage = L"아직 구현되지 않은 칙령입니다.";
        return false;
    }

    FGovernmentEdictState* TargetState = nullptr;

    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        if (mGovernmentEdicts[i].Type == Type)
        {
            TargetState = &mGovernmentEdicts[i];
            break;
        }
    }

    if (!TargetState)
    {
        OutMessage = L"칙령 상태를 찾을 수 없습니다.";
        return false;
    }

    const int ActiveCitizenCount =
        (std::max)(0, mPoliticalSnapshot.ActiveCitizenCount);
    const ETaxPolicyEventType RequiredTaxEvent =
        EconomySystem::GetRequiredTaxPolicyEventForEdict(Type);

    if (RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        if (!mTaxEventStatus.Active || mTaxEventStatus.Type != RequiredTaxEvent)
        {
            OutMessage =
                Definition->DisplayName +
                L"은(는) " +
                EconomySystem::GetTaxPolicyEventTitle(RequiredTaxEvent) +
                L" 발생 중에만 시행할 수 있습니다.";
            return false;
        }
    }

    if (Definition->Mode == EGovernmentEdictMode::Passive &&
        TargetState->Active)
    {
        TargetState->Active = false;
        TargetState->RemainingDays = 0;
        PoliticsSystem::SyncGovernmentActionFromEdict(
            mGovernmentProfile,
            Type,
            false);
        RefreshEdictModifiers();
        RefreshPoliticalSnapshot();
        OutMessage = Definition->DisplayName + L" 해제";
        return true;
    }

    if (TargetState->Active)
    {
        OutMessage = Definition->DisplayName + L" 시행 중";
        return false;
    }

    if (TargetState->CooldownDays > 0)
    {
        OutMessage = Definition->DisplayName + L" 재사용 대기 중";
        return false;
    }

    const long long ActivationCost =
        EdictSystem::ResolveEdictActivationCost(
            *Definition,
            ActiveCitizenCount);

    if (ActivationCost > mNationalBudget)
    {
        OutMessage = L"예산이 부족합니다.";
        return false;
    }

    mNationalBudget -= ActivationCost;
    TargetState->Active = true;

    if (Definition->Mode == EGovernmentEdictMode::Active)
    {
        TargetState->RemainingDays = (std::max)(1, Definition->DurationDays);
        TargetState->CooldownDays = (std::max)(1, Definition->CooldownDays);
    }
    else
    {
        TargetState->RemainingDays = -1;
        TargetState->CooldownDays = 0;
    }

    std::wstring ResponseMessage;

    switch (Type)
    {
    case EGovernmentEdictType::LaborTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mGovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income,
            -4);
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage =
            L"소득세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::PropertyTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mGovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property,
            -10);
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage =
            L"재산세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::EmergencyAusterity:
    {
        const long long EmergencyFunds = 12000;
        mNationalBudget += EmergencyFunds;
        mLastDailyNetChange += EmergencyFunds;
        EconomySystem::ResolveTaxPolicyEvent(mTaxEventStatus, true);
        ResponseMessage = L"긴급 자금 $12,000 투입";
        break;
    }
    default:
        break;
    }

    PoliticsSystem::SyncGovernmentActionFromEdict(
        mGovernmentProfile,
        Type,
        true);
    RefreshEdictModifiers();
    RefreshPoliticalSnapshot();

    OutMessage = Definition->DisplayName + L" 시행";

    if (!ResponseMessage.empty())
    {
        OutMessage += L" / ";
        OutMessage += ResponseMessage;
    }

    return true;
}

bool CMainWorld::AdjustTaxPolicy(
    ETaxPolicyType Type,
    int DeltaPercent,
    std::wstring& OutMessage)
{
    return EconomySystem::AdjustTaxPolicy(
        mGovernmentProfile.TaxPolicy,
        Type,
        DeltaPercent,
        OutMessage);
}

const FGovernmentEdictState* CMainWorld::GetGovernmentEdictState(
    EGovernmentEdictType Type) const
{
    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        if (mGovernmentEdicts[i].Type == Type)
            return &mGovernmentEdicts[i];
    }

    return nullptr;
}

void CMainWorld::TickGovernmentEdicts()
{
    bool ModifiersChanged = false;

    for (size_t i = 0; i < mGovernmentEdicts.size(); ++i)
    {
        FGovernmentEdictState& State = mGovernmentEdicts[i];
        const FGovernmentEdictDefinition* Definition =
            EdictSystem::FindGovernmentEdictDefinition(State.Type);

        if (!Definition)
            continue;

        if (State.Active &&
            Definition->Mode == EGovernmentEdictMode::Active &&
            State.RemainingDays > 0)
        {
            --State.RemainingDays;

            if (State.RemainingDays <= 0)
            {
                State.Active = false;
                State.RemainingDays = 0;
                PoliticsSystem::SyncGovernmentActionFromEdict(
                    mGovernmentProfile,
                    State.Type,
                    false);
                ModifiersChanged = true;
            }
        }

        if (!State.Active && State.CooldownDays > 0)
            --State.CooldownDays;
    }

    if (ModifiersChanged)
        RefreshEdictModifiers();
}

void CMainWorld::RefreshEdictModifiers()
{
    mEdictModifiers = EdictSystem::CalculateEdictModifiers(
        mGovernmentEdicts,
        mPoliticalSnapshot.ActiveCitizenCount);
}

void CMainWorld::ApplyDailyEdictCitizenEffects()
{
    PoliticsSystem::ApplyDailyEdictCitizenEffects(
        this,
        mEdictModifiers);
}

void CMainWorld::ApplyDailyTaxPolicyEventEffects()
{
    EconomySystem::ApplyDailyTaxPolicyEventEffects(
        this,
        mTaxEventStatus);
}

void CMainWorld::TickTaxPolicyEvents()
{
    EconomySystem::TickTaxPolicyEvents(
        mPoliticalSnapshot,
        mGovernmentProfile,
        mSimulationYear,
        mSimulationMonth,
        mSimulationDay,
        mNationalBudget,
        mLastDailyNetChange,
        mWorkerTaxPressureDays,
        mPropertyTaxPressureDays,
        mBudgetCrisisPressureDays,
        mTaxEventStatus);
}

void CMainWorld::RefreshPoliticalSnapshot()
{
    mPoliticalSnapshot = PoliticsSystem::EvaluateWorld(
        this,
        mGovernmentProfile,
        &mTaxEventStatus);
}
