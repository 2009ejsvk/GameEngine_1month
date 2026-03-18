#include "EdictSubsystem.h"
#include "MainWorld.h"
#include "../Economy/EconomySystem.h"
#include "../Politics/EdictSystem.h"
#include "../Politics/PoliticsSystem.h"
#include <algorithm>

namespace
{
    EBuildingEra ConvertEdictEra(EEdictEra Era)
    {
        switch (Era)
        {
        case EEdictEra::WorldWars:
            return EBuildingEra::WorldWars;
        case EEdictEra::ColdWar:
            return EBuildingEra::ColdWar;
        case EEdictEra::Modern:
            return EBuildingEra::Modern;
        case EEdictEra::Colonial:
        default:
            return EBuildingEra::Colonial;
        }
    }
}

void CEdictSubsystem::Reset()
{
    EdictSystem::InitializeGovernmentEdictStates(GovernmentEdicts);
    EdictModifiers = FGovernmentEdictModifiers();
}

void CEdictSubsystem::TickGovernmentEdicts()
{
    if (!mOwner)
        return;

    bool ModifiersChanged = false;

    for (size_t i = 0; i < GovernmentEdicts.size(); ++i)
    {
        FGovernmentEdictState& State = GovernmentEdicts[i];
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
                    mOwner->mPolitics->GovernmentProfile,
                    State.Type,
                    false);
                ModifiersChanged = true;
            }
        }

        if (!State.Active && State.CooldownDays > 0)
            --State.CooldownDays;
    }

    if (ModifiersChanged)
    {
        RefreshEdictModifiers();
        mOwner->mPolitics->RefreshSnapshot(
            {
                &mOwner->mEconomy->TaxEventStatus,
                &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
            });
        mOwner->mTrade->OnDayAdvanced(
            {
                mOwner->mPolitics->GovernmentProfile,
                mOwner->mEconomy->TaxEventStatus,
                GovernmentEdicts,
                mOwner->mEraState->EraProgress.CurrentEra,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day,
                false
            });
    }
}

void CEdictSubsystem::RefreshEdictModifiers()
{
    if (!mOwner)
        return;

    EdictModifiers = EdictSystem::CalculateEdictModifiers(
        GovernmentEdicts,
        mOwner->mPolitics->PoliticalSnapshot.ActiveCitizenCount);
    mOwner->mPolitics->GovernmentProfile.EdictFactionApprovalModifiers =
        EdictModifiers.FactionApprovalModifiers;
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
}

void CEdictSubsystem::ApplyDailyEdictCitizenEffects()
{
    if (!mOwner)
        return;

    PoliticsSystem::ApplyDailyEdictCitizenEffects(
        mOwner,
        EdictModifiers);
}

bool CEdictSubsystem::TryApplyEdict(
    EGovernmentEdictType Type,
    std::wstring& OutMessage)
{
    if (!mOwner)
    {
        OutMessage = L"월드 상태를 확인할 수 없습니다.";
        return false;
    }

    const FGovernmentEdictDefinition* Definition =
        EdictSystem::FindGovernmentEdictDefinition(Type);

    if (!Definition)
    {
        OutMessage = L"정의되지 않은 칙령입니다.";
        return false;
    }

    if (!IsBuildingEraUnlocked(
            mOwner->mEraState->EraProgress.CurrentEra,
            ConvertEdictEra(Definition->Era)))
    {
        OutMessage =
            std::wstring(GetBuildingEraDisplayName(
                ConvertEdictEra(Definition->Era))) +
            L" 이후에 시행할 수 있습니다.";
        return false;
    }

    if (!Definition->Implemented)
    {
        OutMessage = L"아직 구현되지 않은 칙령입니다.";
        return false;
    }

    FGovernmentEdictState* TargetState = nullptr;

    for (size_t i = 0; i < GovernmentEdicts.size(); ++i)
    {
        if (GovernmentEdicts[i].Type == Type)
        {
            TargetState = &GovernmentEdicts[i];
            break;
        }
    }

    if (!TargetState)
    {
        OutMessage = L"칙령 상태를 찾을 수 없습니다.";
        return false;
    }

    const int ActiveCitizenCount =
        (std::max)(0, mOwner->mPolitics->PoliticalSnapshot.ActiveCitizenCount);
    const ETaxPolicyEventType RequiredTaxEvent =
        EconomySystem::GetRequiredTaxPolicyEventForEdict(Type);

    if (RequiredTaxEvent != ETaxPolicyEventType::None)
    {
        if (!mOwner->mEconomy->TaxEventStatus.Active ||
            mOwner->mEconomy->TaxEventStatus.Type != RequiredTaxEvent)
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
            mOwner->mPolitics->GovernmentProfile,
            Type,
            false);
        RefreshEdictModifiers();
        mOwner->mPolitics->RefreshSnapshot(
            {
                &mOwner->mEconomy->TaxEventStatus,
                &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
            });
        mOwner->mTrade->OnDayAdvanced(
            {
                mOwner->mPolitics->GovernmentProfile,
                mOwner->mEconomy->TaxEventStatus,
                GovernmentEdicts,
                mOwner->mEraState->EraProgress.CurrentEra,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day,
                false
            });
        mOwner->mEconomy->RefreshWorldMarketPrices(
            {
                mOwner->mPolitics->GovernmentProfile,
                GovernmentEdicts,
                mOwner->mCrisis->WorldCrisisService->GetStatus(),
                mOwner->mTrade->State.ForeignPowerStates,
                mOwner->mSimulation->Year,
                mOwner->mSimulation->Month,
                mOwner->mSimulation->Day
            });
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

    // Per-edict availability gates
    if (Type == EGovernmentEdictType::LaborNegotiation)
    {
        const FWorldCrisisStatus& CrisisStatus =
            mOwner->mCrisis->WorldCrisisService->GetStatus();
        if (!CrisisStatus.Active ||
            CrisisStatus.Type != EWorldCrisisType::LaborStrike)
        {
            OutMessage = L"총파업 발생 중에만 시행할 수 있습니다.";
            return false;
        }
    }

    if (Type == EGovernmentEdictType::EmergencyWelfareSupport)
    {
        if (mOwner->mEraState->EraProgress.CurrentEra > EBuildingEra::WorldWars)
        {
            OutMessage = L"세계대전 시대에만 시행할 수 있습니다.";
            return false;
        }
    }

    const long long ActivationCost =
        EdictSystem::ResolveEdictActivationCost(
            *Definition,
            ActiveCitizenCount);

    if (ActivationCost > mOwner->mEconomy->NationalBudget)
    {
        OutMessage = L"예산이 부족합니다.";
        return false;
    }

    mOwner->mEconomy->ApplyEdictActivationExpense(ActivationCost);
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
            mOwner->mPolitics->GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Income,
            -4);
        EconomySystem::ResolveTaxPolicyEvent(mOwner->mEconomy->TaxEventStatus, true);
        ResponseMessage =
            L"소득세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::PropertyTaxRelief:
    {
        const int RateDelta = EconomySystem::ApplyTaxPolicyRateDelta(
            mOwner->mPolitics->GovernmentProfile.TaxPolicy,
            ETaxPolicyType::Property,
            -10);
        EconomySystem::ResolveTaxPolicyEvent(mOwner->mEconomy->TaxEventStatus, true);
        ResponseMessage =
            L"재산세 " +
            std::to_wstring((std::max)(0, -RateDelta)) +
            L"%p 인하";
        break;
    }
    case EGovernmentEdictType::EmergencyAusterity:
    {
        const long long EmergencyFunds = 12000;
        mOwner->mEconomy->ApplySpecialEventBudgetDelta(EmergencyFunds);
        EconomySystem::ResolveTaxPolicyEvent(mOwner->mEconomy->TaxEventStatus, true);
        ResponseMessage = L"긴급 자금 $12,000 투입";
        break;
    }
    case EGovernmentEdictType::LaborNegotiation:
    {
        if (mOwner->mCrisis->WorldCrisisService)
        {
            const bool Ended = mOwner->mCrisis->WorldCrisisService->ForceEndActiveCrisis(
                true,
                mOwner->mEraState->EraProgress.CurrentEra);
            ResponseMessage = Ended ? L"총파업 즉시 종료" : L"종료할 파업이 없음";
        }
        // Instant edict: mark inactive immediately so it goes straight to cooldown
        TargetState->Active = false;
        TargetState->RemainingDays = 0;
        break;
    }
    case EGovernmentEdictType::EmergencyWelfareSupport:
    {
        ResponseMessage = L"30일간 민생 지원 시행";
        break;
    }
    default:
        break;
    }

    PoliticsSystem::SyncGovernmentActionFromEdict(
        mOwner->mPolitics->GovernmentProfile,
        Type,
        TargetState->Active);
    RefreshEdictModifiers();
    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    mOwner->mTrade->OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEconomy->TaxEventStatus,
            GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });

    OutMessage = Definition->DisplayName + L" 시행";

    if (!ResponseMessage.empty())
    {
        OutMessage += L" / ";
        OutMessage += ResponseMessage;
    }

    return true;
}
