#include "EraSubsystem.h"
#include "MainWorld.h"
#include "RuntimeConfigRegistry.h"
#include "WorldStatsSnapshot.h"
#include "../Politics/ConstitutionSystem.h"
#include "../GlobalSetting.h"
#include "ScenarioSubsystem.h"
#include <Windows.h>
#include <array>
#include <cstdio>
#include <string>

namespace
{
    constexpr const wchar_t* GEraConfigId = L"MainWorld.EraUnlockRequirements";
    constexpr int GEraTransitionNotificationDays = 6;
    std::array<
        FEraUnlockRequirement,
        static_cast<size_t>(EBuildingEra::Modern) + 1> GEraUnlockRequirements;

    size_t GetEraRequirementIndex(EBuildingEra Era)
    {
        return static_cast<size_t>(Era);
    }

    void ResetEraUnlockRequirementsToDefaults()
    {
        GEraUnlockRequirements.fill(FEraUnlockRequirement());

        FEraUnlockRequirement& WorldWars =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::WorldWars)];
        WorldWars.MinPopulation = 12;
        WorldWars.MinTotalBuildings = 4;
        WorldWars.MinFoodProviders = 1;

        FEraUnlockRequirement& ColdWar =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::ColdWar)];
        ColdWar.MinPopulation = 24;
        ColdWar.MinTotalBuildings = 8;
        ColdWar.MinIndustryBuildings = 1;
        ColdWar.MinPowerMW = 5;

        FEraUnlockRequirement& Modern =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::Modern)];
        Modern.MinPopulation = 50;
        Modern.MinTotalBuildings = 14;
        Modern.MinPublicServiceBuildings = 1;
        Modern.MinEntertainmentBuildings = 1;
        Modern.MinPowerMW = 10;
    }

    bool LoadEraUnlockRequirementsFromFile(const std::wstring& Path)
    {
        auto LoadRequirement =
            [&](EBuildingEra Era, const wchar_t* Section)
            {
                FEraUnlockRequirement& Requirement =
                    GEraUnlockRequirements[GetEraRequirementIndex(Era)];
                Requirement.MinPopulation = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPopulation",
                        Requirement.MinPopulation,
                        Path.c_str()));
                Requirement.MinTotalBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinTotalBuildings",
                        Requirement.MinTotalBuildings,
                        Path.c_str()));
                Requirement.MinFoodProviders = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinFoodProviders",
                        Requirement.MinFoodProviders,
                        Path.c_str()));
                Requirement.MinIndustryBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinIndustryBuildings",
                        Requirement.MinIndustryBuildings,
                        Path.c_str()));
                Requirement.MinPublicServiceBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPublicServiceBuildings",
                        Requirement.MinPublicServiceBuildings,
                        Path.c_str()));
                Requirement.MinEntertainmentBuildings = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinEntertainmentBuildings",
                        Requirement.MinEntertainmentBuildings,
                        Path.c_str()));
                Requirement.MinPowerMW = static_cast<int>(
                    GetPrivateProfileIntW(
                        Section,
                        L"MinPowerMW",
                        Requirement.MinPowerMW,
                        Path.c_str()));
            };

        LoadRequirement(EBuildingEra::WorldWars, L"WorldWars");
        LoadRequirement(EBuildingEra::ColdWar, L"ColdWar");
        LoadRequirement(EBuildingEra::Modern, L"Modern");
        return true;
    }

    void WriteEraUnlockRequirementsToFile(const std::wstring& Path)
    {
        const FEraUnlockRequirement& WorldWars =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::WorldWars)];
        const FEraUnlockRequirement& ColdWar =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::ColdWar)];
        const FEraUnlockRequirement& Modern =
            GEraUnlockRequirements[GetEraRequirementIndex(
                EBuildingEra::Modern)];

        const std::string Buffer =
            "; Runtime-tunable era unlock requirements.\r\n"
            "; Adjust values and save to hot-reload in-game.\r\n\r\n"
            "[WorldWars]\r\n"
            "MinPopulation=" + std::to_string(WorldWars.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(WorldWars.MinTotalBuildings) + "\r\n"
            "MinFoodProviders=" +
            std::to_string(WorldWars.MinFoodProviders) + "\r\n\r\n"
            "[ColdWar]\r\n"
            "MinPopulation=" + std::to_string(ColdWar.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(ColdWar.MinTotalBuildings) + "\r\n"
            "MinIndustryBuildings=" +
            std::to_string(ColdWar.MinIndustryBuildings) + "\r\n"
            "MinPowerMW=" + std::to_string(ColdWar.MinPowerMW) + "\r\n\r\n"
            "[Modern]\r\n"
            "MinPopulation=" + std::to_string(Modern.MinPopulation) +
            "\r\n"
            "MinTotalBuildings=" +
            std::to_string(Modern.MinTotalBuildings) + "\r\n"
            "MinPublicServiceBuildings=" +
            std::to_string(Modern.MinPublicServiceBuildings) + "\r\n"
            "MinEntertainmentBuildings=" +
            std::to_string(Modern.MinEntertainmentBuildings) + "\r\n"
            "MinPowerMW=" + std::to_string(Modern.MinPowerMW) + "\r\n";

        FILE* File = nullptr;

        if (_wfopen_s(&File, Path.c_str(), L"wb") != 0 || !File)
            return;

        static const unsigned char Bom[] = { 0xEF, 0xBB, 0xBF };
        fwrite(Bom, 1, sizeof(Bom), File);
        fwrite(Buffer.data(), 1, Buffer.size(), File);
        fclose(File);
    }

    void EnsureDefaultEraConfigFileExists(const std::wstring& Path)
    {
        const DWORD Attributes = GetFileAttributesW(Path.c_str());

        if (Attributes != INVALID_FILE_ATTRIBUTES &&
            (Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            return;
        }

        ResetEraUnlockRequirementsToDefaults();
        WriteEraUnlockRequirementsToFile(Path);
    }

    void EnsureEraRuntimeConfigRegistered()
    {
        if (RuntimeConfigRegistry::GetSourceGeneration(GEraConfigId) != 0)
            return;

        const std::wstring ConfigPath =
            RuntimeConfigRegistry::BuildExeRelativePath(L"EraConfig.ini");
        EnsureDefaultEraConfigFileExists(ConfigPath);
        RuntimeConfigRegistry::RegisterSource(
            {
                GEraConfigId,
                ConfigPath,
                0.5f,
                &ResetEraUnlockRequirementsToDefaults,
                &LoadEraUnlockRequirementsFromFile,
                nullptr
            });
    }

    FEraUnlockRequirement ResolveEraUnlockRequirement(EBuildingEra TargetEra)
    {
        EnsureEraRuntimeConfigRegistered();

        if (TargetEra < EBuildingEra::Colonial ||
            TargetEra > EBuildingEra::Modern)
        {
            return FEraUnlockRequirement();
        }

        return GEraUnlockRequirements[GetEraRequirementIndex(TargetEra)];
    }

    int ResolveIndustryBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::Industry)];
    }

    int ResolvePublicServiceBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::PublicService)];
    }

    int ResolveEntertainmentBuildingCount(
        const WorldStats::FWorldStatsSnapshot& Snapshot)
    {
        return Snapshot.BuildingCategoryCount[
            static_cast<int>(EBuildingCategory::Entertainment)];
    }

    bool MeetsEraUnlockRequirement(
        const WorldStats::FWorldStatsSnapshot& Snapshot,
        const FEraUnlockRequirement& Requirement)
    {
        if (Snapshot.ActiveCitizenCount < Requirement.MinPopulation)
            return false;
        if (Snapshot.TotalBuildingCount < Requirement.MinTotalBuildings)
            return false;
        if (Snapshot.FoodProviderCount < Requirement.MinFoodProviders)
            return false;
        if (ResolveIndustryBuildingCount(Snapshot) <
            Requirement.MinIndustryBuildings)
        {
            return false;
        }
        if (ResolvePublicServiceBuildingCount(Snapshot) <
            Requirement.MinPublicServiceBuildings)
        {
            return false;
        }
        if (ResolveEntertainmentBuildingCount(Snapshot) <
            Requirement.MinEntertainmentBuildings)
        {
            return false;
        }
        if (Snapshot.TotalProducedPowerMW < Requirement.MinPowerMW)
            return false;

        return true;
    }

    std::wstring BuildEraTransitionTitle(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return L"독립 선언";
        case EBuildingEra::ColdWar:
            return L"전후 질서 편입";
        case EBuildingEra::Modern:
            return L"현대화 선언";
        case EBuildingEra::Colonial:
        default:
            return L"시대 전환";
        }
    }

    std::wstring BuildEraTransitionConfirmText(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return L"독립 승인";
        case EBuildingEra::ColdWar:
            return L"냉전 진입";
        case EBuildingEra::Modern:
            return L"현대화 승인";
        case EBuildingEra::Colonial:
        default:
            return L"전환 승인";
        }
    }

    std::wstring BuildEraTransitionAvailableSummary(EBuildingEra TargetEra)
    {
        switch (TargetEra)
        {
        case EBuildingEra::WorldWars:
            return
                L"독립을 선포할 준비가 끝났습니다.\n"
                L"승인하면 세계대전 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::ColdWar:
            return
                L"전후 체제로 편입할 준비가 끝났습니다.\n"
                L"승인하면 냉전 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::Modern:
            return
                L"현대화 전환 준비가 끝났습니다.\n"
                L"승인하면 현대 시대 건물과 칙령이 즉시 열립니다.";
        case EBuildingEra::Colonial:
        default:
            return L"다음 시대로 전환할 수 있습니다.";
        }
    }

    std::wstring BuildEraTransitionCompletionSummary(
        EBuildingEra PreviousEra,
        EBuildingEra CurrentEra)
    {
        return
            std::wstring(GetBuildingEraDisplayName(PreviousEra)) +
            L"에서 " +
            GetBuildingEraDisplayName(CurrentEra) +
            L"(으)로 전환되었습니다. 새 시대 건물과 칙령을 확인하세요.";
    }

    void PopulateEraTransitionAvailableState(
        FEraTransitionState& OutState,
        const FEraProgressState& EraProgress,
        int Year,
        int Month,
        int Day)
    {
        const bool KeepAvailableDate =
            OutState.Stage == EEraTransitionStage::Available &&
            OutState.CurrentEra == EraProgress.CurrentEra &&
            OutState.TargetEra == EraProgress.NextEra;

        const int AvailableYear = KeepAvailableDate ?
            OutState.AvailableSinceYear :
            Year;
        const int AvailableMonth = KeepAvailableDate ?
            OutState.AvailableSinceMonth :
            Month;
        const int AvailableDay = KeepAvailableDate ?
            OutState.AvailableSinceDay :
            Day;

        OutState = FEraTransitionState();
        OutState.Stage = EEraTransitionStage::Available;
        OutState.Choice = EEraTransitionChoice::Confirm;
        OutState.CurrentEra = EraProgress.CurrentEra;
        OutState.TargetEra = EraProgress.NextEra;
        OutState.CanStart = true;
        OutState.AvailableSinceYear = AvailableYear;
        OutState.AvailableSinceMonth = AvailableMonth;
        OutState.AvailableSinceDay = AvailableDay;
        OutState.Title = BuildEraTransitionTitle(EraProgress.NextEra);
        OutState.Summary =
            BuildEraTransitionAvailableSummary(EraProgress.NextEra);
        OutState.ConfirmText =
            BuildEraTransitionConfirmText(EraProgress.NextEra);
    }
}

void CEraSubsystem::Reset()
{
    EraProgress = FEraProgressState();
    EraTransition = FEraTransitionState();
}

void CEraSubsystem::TickEraTransitionState()
{
    if (EraTransition.Stage != EEraTransitionStage::Cooldown ||
        EraTransition.NotificationDays <= 0)
    {
        return;
    }

    --EraTransition.NotificationDays;

    if (EraTransition.NotificationDays <= 0)
        EraTransition = FEraTransitionState();
}

void CEraSubsystem::RefreshEraProgress()
{
    if (!mOwner)
        return;

    const std::shared_ptr<CWorld> World = mOwner->mSelf.lock();

    if (!World)
        return;

    const WorldStats::FWorldStatsSnapshot Snapshot =
        WorldStats::BuildSnapshot(World);

    const EBuildingEra CurrentEra = EraProgress.CurrentEra;

    EraProgress = FEraProgressState();
    EraProgress.CurrentEra = CurrentEra;
    EraProgress.Population = Snapshot.ActiveCitizenCount;
    EraProgress.TotalBuildings = Snapshot.TotalBuildingCount;
    EraProgress.FoodProviders = Snapshot.FoodProviderCount;
    EraProgress.IndustryBuildings =
        ResolveIndustryBuildingCount(Snapshot);
    EraProgress.PublicServiceBuildings =
        ResolvePublicServiceBuildingCount(Snapshot);
    EraProgress.EntertainmentBuildings =
        ResolveEntertainmentBuildingCount(Snapshot);
    EraProgress.PowerMW = Snapshot.TotalProducedPowerMW;
    EraProgress.HasNextEra = HasNextBuildingEra(CurrentEra);

    if (EraProgress.HasNextEra)
    {
        EraProgress.NextEra = GetNextBuildingEra(CurrentEra);
        EraProgress.NextRequirement =
            ResolveEraUnlockRequirement(EraProgress.NextEra);
        EraProgress.NextEraReady =
            MeetsEraUnlockRequirement(
                Snapshot,
                EraProgress.NextRequirement);
    }
    else
    {
        EraProgress.NextEra = EBuildingEra::Modern;
        EraProgress.NextRequirement = FEraUnlockRequirement();
        EraProgress.NextEraReady = false;
    }

    RefreshEraTransitionState();
}

void CEraSubsystem::RefreshEraTransitionState()
{
    if (!mOwner)
        return;

    if (EraTransition.Stage == EEraTransitionStage::Cooldown &&
        EraTransition.NotificationDays > 0)
    {
        return;
    }

    // 시나리오 모드: EraTransitionUnlocked 시 NextEraReady 조건 없이 전환 허용
    const bool ScenarioUnlocked =
        GameSession::CurrentMode() == EGameMode::Scenario &&
        mOwner->mScenario->EraTransitionUnlocked;

    if (!EraProgress.HasNextEra || (!EraProgress.NextEraReady && !ScenarioUnlocked))
    {
        if (EraTransition.Stage == EEraTransitionStage::Available)
            EraTransition = FEraTransitionState();

        return;
    }

    // 시나리오 모드: Phase가 EraTransitionReady에 도달할 때까지 전환 차단
    if (GameSession::CurrentMode() == EGameMode::Scenario && !ScenarioUnlocked)
    {
        if (EraTransition.Stage == EEraTransitionStage::Available)
            EraTransition = FEraTransitionState();

        return;
    }

    PopulateEraTransitionAvailableState(
        EraTransition,
        EraProgress,
        mOwner->mSimulation->Year,
        mOwner->mSimulation->Month,
        mOwner->mSimulation->Day);
}

bool CEraSubsystem::TryExecuteEraTransition(EEraTransitionChoice Choice)
{
    if (!mOwner ||
        Choice != EEraTransitionChoice::Confirm ||
        EraTransition.Stage != EEraTransitionStage::Available ||
        !EraTransition.CanStart)
    {
        return false;
    }

    const EBuildingEra PreviousEra = EraProgress.CurrentEra;
    const EBuildingEra TargetEra = EraTransition.TargetEra;

    EraProgress.CurrentEra = TargetEra;
    mOwner->mTrade->CancelTradeRoutesForInactivePowers(TargetEra);
    mOwner->mTrade->State.ForeignPowerStandingStates = {};
    RefreshEraProgress();
    mOwner->mPolitics->InitializeElectionSchedule(
        mOwner->mEraState->EraProgress.CurrentEra,
        mOwner->mSimulation->Year);

    EraTransition = FEraTransitionState();
    EraTransition.Stage = EEraTransitionStage::Cooldown;
    EraTransition.Choice = Choice;
    EraTransition.CurrentEra = EraProgress.CurrentEra;
    EraTransition.TargetEra = TargetEra;
    EraTransition.NotificationDays = GEraTransitionNotificationDays;
    EraTransition.Title = BuildEraTransitionTitle(TargetEra);
    EraTransition.Summary =
        BuildEraTransitionCompletionSummary(
            PreviousEra,
            EraProgress.CurrentEra);
    EraTransition.ConfirmText =
        BuildEraTransitionConfirmText(TargetEra);

    ConstitutionSystem::OnEraTransitioned(
        mOwner->mKnowledgeState->ConstitutionState,
        TargetEra);
    mOwner->mPolitics->RefreshSnapshot(
        {
            &mOwner->mEconomy->TaxEventStatus,
            &mOwner->mKnowledgeState->ConstitutionState.ActiveEffects
        });
    mOwner->mTrade->OnDayAdvanced(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEconomy->TaxEventStatus,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mEraState->EraProgress.CurrentEra,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day,
            false
        });
    mOwner->mEconomy->RefreshWorldMarketPrices(
        {
            mOwner->mPolitics->GovernmentProfile,
            mOwner->mEdictState->GovernmentEdicts,
            mOwner->mCrisis->WorldCrisisService->GetStatus(),
            mOwner->mTrade->State.ForeignPowerStates,
            mOwner->mSimulation->Year,
            mOwner->mSimulation->Month,
            mOwner->mSimulation->Day
        });
    return true;
}
