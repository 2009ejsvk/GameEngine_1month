#include "CrisisSubsystem.h"
#include "MainWorld.h"

void CCrisisSubsystem::Reset()
{
    if (!WorldCrisisService)
        WorldCrisisService = std::make_unique<CMainWorldWorldCrisisService>();

    WorldCrisisService->Reset();
}

void CCrisisSubsystem::ApplyDailyWorldCrisisEffects()
{
    if (!mOwner || !WorldCrisisService)
        return;

    const CMainWorldWorldCrisisService::FDailyContext Context =
    {
        mOwner->mSelf.lock(),
        mOwner->mEconomy->NationalBudget,
        mOwner->mEconomy->LastDailyNetChange,
        mOwner->mEconomy->LastDailyTaxIncome,
        mOwner->mEconomy->LastDailyConsumptionTaxIncome,
        mOwner->mEconomy->LastDailyIncomeTaxIncome,
        mOwner->mEconomy->LastDailyPropertyTaxIncome
    };
    WorldCrisisService->ApplyDailyEffects(Context);
}

void CCrisisSubsystem::TickWorldCrises()
{
    if (!mOwner || !WorldCrisisService)
        return;

    const long long BudgetBefore = mOwner->mEconomy->NationalBudget;
    const CMainWorldWorldCrisisService::FTickContext Context =
    {
        mOwner->mSelf.lock(),
        mOwner->mPolitics->PoliticalSnapshot,
        mOwner->mEdictState->GovernmentEdicts,
        mOwner->mPolitics->GovernmentProfile.TaxPolicy,
        mOwner->mEconomy->TaxEventStatus,
        mOwner->mSimulation->Year,
        mOwner->mSimulation->Month,
        mOwner->mSimulation->Day,
        mOwner->mEconomy->NationalBudget,
        mOwner->mEconomy->LastDailyNetChange,
        mOwner->mEconomy->LastDailyTaxCollectionEfficiency,
        mOwner->mEraState->EraProgress.CurrentEra
    };
    WorldCrisisService->Tick(Context);
    mOwner->mEconomy->RecordSpecialEventBudgetDelta(
        mOwner->mEconomy->NationalBudget - BudgetBefore);
}
