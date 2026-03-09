#include "PoliticsSystem.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorld.h"
#include "World/World.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
    struct FPlacedPoliticalSignal
    {
        std::string        BuildingName;
        FPoliticalSignalDef Signal;
    };

    float ClampSupportScore(float Value)
    {
        return (std::max)(0.f, (std::min)(100.f, Value));
    }

    float GetSupportLevelWeight(EPoliticalSupportLevel SupportLevel)
    {
        switch (SupportLevel)
        {
        case EPoliticalSupportLevel::Weak:
            return 0.70f;
        case EPoliticalSupportLevel::Strong:
            return 1.35f;
        default:
            return 1.00f;
        }
    }

    float GetStanceAlignment(
        EPoliticalStance CitizenStance,
        EPoliticalStance TargetStance)
    {
        if (CitizenStance == EPoliticalStance::Neutral ||
            TargetStance == EPoliticalStance::Neutral)
        {
            return 0.f;
        }

        return CitizenStance == TargetStance ? 1.f : -1.f;
    }

    bool DoesCitizenMatchScope(
        const CBuildingMarkerOrb& Citizen,
        const FPlacedPoliticalSignal& PlacedSignal)
    {
        switch (PlacedSignal.Signal.Scope)
        {
        case EPoliticalScope::Global:
            return true;
        case EPoliticalScope::Worker:
            return Citizen.GetWorkBuilding() == PlacedSignal.BuildingName;
        case EPoliticalScope::Resident:
            return Citizen.GetHomeBuilding() == PlacedSignal.BuildingName;
        case EPoliticalScope::Visitor:
            return Citizen.GetFoodBuilding() == PlacedSignal.BuildingName ||
                Citizen.GetFunBuilding() == PlacedSignal.BuildingName;
        default:
            return false;
        }
    }

    float CalculateAxisSignalContribution(
        const FNpcPoliticalChoice& CitizenChoice,
        const FPoliticalSignalDef& Signal)
    {
        const float Alignment = GetStanceAlignment(
            CitizenChoice.Stance,
            Signal.FavoredStance);

        if (Alignment == 0.f)
            return 0.f;

        return Alignment *
            GetSupportLevelWeight(CitizenChoice.Support) *
            Signal.Strength;
    }

    std::vector<FPlacedPoliticalSignal> CollectPlacedSignals(CWorld* World)
    {
        std::vector<FPlacedPoliticalSignal> Result;

        if (!World)
            return Result;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return Result;

        for (size_t i = 0; i < BuildingList.size(); ++i)
        {
            auto Building = BuildingList[i].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea())
            {
                continue;
            }

            const FBuildingCatalogEntry* Entry =
                FindBuildingCatalogEntry(Building->GetBuildingId());

            if (!Entry || Entry->PoliticalSignals.empty())
                continue;

            const float BuildingScale = Building->GetBudgetSatisfactionScale();

            for (size_t SignalIndex = 0;
                SignalIndex < Entry->PoliticalSignals.size();
                ++SignalIndex)
            {
                FPlacedPoliticalSignal PlacedSignal;
                PlacedSignal.BuildingName = Building->GetName();
                PlacedSignal.Signal = Entry->PoliticalSignals[SignalIndex];
                PlacedSignal.Signal.Strength *= BuildingScale;
                Result.push_back(PlacedSignal);
            }
        }

        return Result;
    }

    float EvaluateTaxPolicyActionScore(
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile)
    {
        const bool IsWorker = !Citizen.GetWorkBuilding().empty();
        const bool IsResident = !Citizen.GetHomeBuilding().empty();
        const float TaxBurden = GetCitizenTaxBurdenNormalized(
            GovernmentProfile.TaxPolicy,
            IsWorker,
            IsResident);

        if (TaxBurden == 0.f)
            return 0.f;

        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();
        const FNpcPoliticalChoice& EconomyChoice =
            PoliticalProfile.Get(EPoliticalAxis::Economy);
        const float SupportWeight =
            GetSupportLevelWeight(EconomyChoice.Support);
        const float TaxStress = (std::max)(0.f, TaxBurden);
        const float TaxRelief = (std::max)(0.f, -TaxBurden);
        float Score =
            -TaxStress * (IsWorker ? 1.8f : 1.2f) +
            TaxRelief * 0.6f;

        switch (EconomyChoice.Stance)
        {
        case EPoliticalStance::Left:
            Score += -TaxBurden * SupportWeight * 8.5f;
            break;
        case EPoliticalStance::Right:
            Score += TaxBurden * SupportWeight * 4.8f;
            break;
        default:
            Score += -TaxStress * 1.4f + TaxRelief * 0.5f;
            break;
        }

        return Score;
    }

    float EvaluateTaxEventActionScore(
        const CBuildingMarkerOrb& Citizen,
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        if (!TaxEventStatus ||
            !TaxEventStatus->Active ||
            TaxEventStatus->Type == ETaxPolicyEventType::None)
        {
            return 0.f;
        }

        const bool IsWorker = !Citizen.GetWorkBuilding().empty();
        const bool IsResident = !Citizen.GetHomeBuilding().empty();
        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();
        const float Severity =
            0.55f +
            (std::min)(
                1.10f,
                static_cast<float>(TaxEventStatus->DaysActive) / 4.5f);
        float Score = -2.5f * Severity;
        const auto ApplyDemandPressure =
            [&](EPoliticalAxis Axis,
                EPoliticalStance DemandingStance,
                float Strength)
        {
            const FNpcPoliticalChoice& Choice = PoliticalProfile.Get(Axis);
            const float Alignment = GetStanceAlignment(
                Choice.Stance,
                DemandingStance);
            const float Weight = GetSupportLevelWeight(Choice.Support);

            if (Alignment > 0.f)
                return -Weight * Strength;

            if (Alignment < 0.f)
                return Weight * Strength * 0.35f;

            return 0.f;
        };

        switch (TaxEventStatus->Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            Score += (IsWorker ? -11.5f : -4.0f) * Severity;

            if (IsResident)
                Score += -1.8f * Severity;

            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Left,
                4.6f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Left,
                2.8f * Severity);
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            Score += (IsResident ? -11.0f : -3.5f) * Severity;

            if (IsWorker)
                Score += -1.5f * Severity;

            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right,
                4.4f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Left,
                3.2f * Severity);
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            Score += (IsWorker || IsResident ? -9.0f : -6.5f) * Severity;
            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right,
                4.2f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Right,
                3.6f * Severity);
            break;
        default:
            break;
        }

        return Score;
    }

    FCitizenPoliticalEvaluation EvaluateCitizenInternal(
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile,
        const std::vector<FPlacedPoliticalSignal>& PlacedSignals,
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        FCitizenPoliticalEvaluation Evaluation;

        const FNpcSatisfaction& Satisfaction = Citizen.GetSatisfaction();
        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();

        Evaluation.LifeScore =
            (Satisfaction.Overall - 50.f) * 0.55f;

        if (Citizen.GetHomeBuilding().empty())
            Evaluation.LifeScore -= 12.f;

        if (Citizen.GetWorkBuilding().empty())
            Evaluation.LifeScore -= 10.f;

        if (Citizen.GetFoodBuilding().empty())
            Evaluation.LifeScore -= 8.f;

        const std::array<EPoliticalAxis, 4> Axes =
        {
            EPoliticalAxis::Economy,
            EPoliticalAxis::ReligionMilitarism,
            EPoliticalAxis::EnvironmentIndustry,
            EPoliticalAxis::IntellectualConservative
        };

        for (size_t AxisIndex = 0; AxisIndex < Axes.size(); ++AxisIndex)
        {
            const EPoliticalAxis Axis = Axes[AxisIndex];
            const FNpcPoliticalChoice& CitizenChoice =
                PoliticalProfile.Get(Axis);
            const FNpcPoliticalChoice& GovernmentChoice =
                GovernmentProfile.Ideology.Get(Axis);

            const float Alignment = GetStanceAlignment(
                CitizenChoice.Stance,
                GovernmentChoice.Stance);

            if (Alignment != 0.f)
            {
                Evaluation.GovernmentIdeologyScore +=
                    Alignment *
                    GetSupportLevelWeight(CitizenChoice.Support) *
                    7.5f;
            }
            else if (CitizenChoice.Stance == EPoliticalStance::Neutral ||
                GovernmentChoice.Stance == EPoliticalStance::Neutral)
            {
                Evaluation.GovernmentIdeologyScore += 1.f;
            }
        }

        for (size_t SignalIndex = 0;
            SignalIndex < PlacedSignals.size();
            ++SignalIndex)
        {
            const FPlacedPoliticalSignal& PlacedSignal =
                PlacedSignals[SignalIndex];

            if (!DoesCitizenMatchScope(Citizen, PlacedSignal))
                continue;

            const float ScopeMultiplier =
                PlacedSignal.Signal.Scope == EPoliticalScope::Global ?
                0.35f :
                1.00f;

            Evaluation.BuildingScore +=
                CalculateAxisSignalContribution(
                    PoliticalProfile.Get(PlacedSignal.Signal.Axis),
                    PlacedSignal.Signal) * ScopeMultiplier;
        }

        for (size_t ActionIndex = 0;
            ActionIndex < GovernmentProfile.ActiveActions.size();
            ++ActionIndex)
        {
            const FGovernmentActionRecord& Action =
                GovernmentProfile.ActiveActions[ActionIndex];

            for (size_t SignalIndex = 0;
                SignalIndex < Action.Signals.size();
                ++SignalIndex)
            {
                Evaluation.ActionScore +=
                    CalculateAxisSignalContribution(
                        PoliticalProfile.Get(Action.Signals[SignalIndex].Axis),
                        Action.Signals[SignalIndex]) *
                    Action.Strength;
            }
        }

        Evaluation.ActionScore += EvaluateTaxPolicyActionScore(
            Citizen,
            GovernmentProfile);
        Evaluation.ActionScore += EvaluateTaxEventActionScore(
            Citizen,
            TaxEventStatus);

        const FNpcPoliticalChoice& ReligionMilitarismChoice =
            PoliticalProfile.Get(EPoliticalAxis::ReligionMilitarism);

        const float OrderAlignment = GetStanceAlignment(
            ReligionMilitarismChoice.Stance,
            EPoliticalStance::Right);

        if (OrderAlignment != 0.f)
        {
            Evaluation.FearScore +=
                OrderAlignment *
                GetSupportLevelWeight(ReligionMilitarismChoice.Support) *
                GovernmentProfile.Militarization * 4.f;
        }

        const FNpcPoliticalChoice& SocialChoice =
            PoliticalProfile.Get(EPoliticalAxis::IntellectualConservative);

        const float LibertyAlignment = GetStanceAlignment(
            SocialChoice.Stance,
            EPoliticalStance::Left);

        if (LibertyAlignment != 0.f)
        {
            Evaluation.FearScore +=
                LibertyAlignment *
                GetSupportLevelWeight(SocialChoice.Support) *
                GovernmentProfile.LibertyBias * 4.f;
        }

        Evaluation.TotalSupportScore = ClampSupportScore(
            50.f +
            Evaluation.LifeScore +
            Evaluation.GovernmentIdeologyScore +
            Evaluation.BuildingScore +
            Evaluation.ActionScore +
            Evaluation.FearScore);

        if (Evaluation.TotalSupportScore >= 60.f)
            Evaluation.VoteIntent = EVoteIntent::Incumbent;
        else if (Evaluation.TotalSupportScore < 40.f)
            Evaluation.VoteIntent = EVoteIntent::Opposition;
        else
            Evaluation.VoteIntent = EVoteIntent::Abstain;

        return Evaluation;
    }
}

namespace PoliticsSystem
{
    void SetDefaultGovernmentProfile(FGovernmentProfile& OutProfile)
    {
        OutProfile = FGovernmentProfile();

        OutProfile.Ideology.Economy.Stance = EPoliticalStance::Left;
        OutProfile.Ideology.Economy.Support = EPoliticalSupportLevel::Normal;
        OutProfile.Ideology.ReligionMilitarism.Stance = EPoliticalStance::Right;
        OutProfile.Ideology.ReligionMilitarism.Support = EPoliticalSupportLevel::Normal;
        OutProfile.Ideology.EnvironmentIndustry.Stance = EPoliticalStance::Right;
        OutProfile.Ideology.EnvironmentIndustry.Support = EPoliticalSupportLevel::Strong;
        OutProfile.Ideology.IntellectualConservative.Stance = EPoliticalStance::Right;
        OutProfile.Ideology.IntellectualConservative.Support = EPoliticalSupportLevel::Normal;
        OutProfile.WelfareBias = -0.15f;
        OutProfile.LibertyBias = -0.20f;
        OutProfile.Militarization = 0.35f;
    }

    void TickGovernmentActions(
        FGovernmentProfile& InOutProfile,
        int DaysElapsed)
    {
        const int SafeDays = (std::max)(1, DaysElapsed);

        for (size_t i = 0; i < InOutProfile.ActiveActions.size(); ++i)
        {
            if (InOutProfile.ActiveActions[i].RemainingDays >= 0)
                InOutProfile.ActiveActions[i].RemainingDays -= SafeDays;
        }

        InOutProfile.ActiveActions.erase(
            std::remove_if(
                InOutProfile.ActiveActions.begin(),
                InOutProfile.ActiveActions.end(),
                [](const FGovernmentActionRecord& Action)
                {
                    return Action.RemainingDays == 0;
                }),
            InOutProfile.ActiveActions.end());
    }

    FCitizenPoliticalEvaluation EvaluateCitizen(
        CWorld* World,
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile)
    {
        const std::vector<FPlacedPoliticalSignal> PlacedSignals =
            CollectPlacedSignals(World);
        const CMainWorld* MainWorld =
            World ? dynamic_cast<CMainWorld*>(World) : nullptr;
        const FTaxPolicyEventStatus* TaxEventStatus =
            MainWorld ? &MainWorld->GetTaxPolicyEventStatus() : nullptr;

        return EvaluateCitizenInternal(
            Citizen,
            GovernmentProfile,
            PlacedSignals,
            TaxEventStatus);
    }

    FPoliticalWorldSnapshot EvaluateWorld(
        CWorld* World,
        const FGovernmentProfile& GovernmentProfile)
    {
        FPoliticalWorldSnapshot Snapshot;

        if (!World)
            return Snapshot;

        const std::vector<FPlacedPoliticalSignal> PlacedSignals =
            CollectPlacedSignals(World);
        const CMainWorld* MainWorld =
            World ? dynamic_cast<CMainWorld*>(World) : nullptr;
        const FTaxPolicyEventStatus* TaxEventStatus =
            MainWorld ? &MainWorld->GetTaxPolicyEventStatus() : nullptr;

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return Snapshot;

        double LifeScoreSum = 0.0;
        double GovernmentScoreSum = 0.0;
        double BuildingScoreSum = 0.0;
        double ActionScoreSum = 0.0;
        double SupportScoreSum = 0.0;

        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            const FCitizenPoliticalEvaluation Evaluation =
                EvaluateCitizenInternal(
                    *Orb,
                    GovernmentProfile,
                    PlacedSignals,
                    TaxEventStatus);

            ++Snapshot.ActiveCitizenCount;
            LifeScoreSum += Evaluation.LifeScore;
            GovernmentScoreSum += Evaluation.GovernmentIdeologyScore;
            BuildingScoreSum += Evaluation.BuildingScore;
            ActionScoreSum += Evaluation.ActionScore;
            SupportScoreSum += Evaluation.TotalSupportScore;

            switch (Evaluation.VoteIntent)
            {
            case EVoteIntent::Incumbent:
                ++Snapshot.IncumbentCount;
                break;
            case EVoteIntent::Opposition:
                ++Snapshot.OppositionCount;
                break;
            default:
                ++Snapshot.AbstainCount;
                break;
            }
        }

        if (Snapshot.ActiveCitizenCount <= 0)
            return Snapshot;

        const double Denominator =
            static_cast<double>(Snapshot.ActiveCitizenCount);

        Snapshot.AverageLifeScore = LifeScoreSum / Denominator;
        Snapshot.AverageGovernmentIdeologyScore =
            GovernmentScoreSum / Denominator;
        Snapshot.AverageBuildingScore = BuildingScoreSum / Denominator;
        Snapshot.AverageActionScore = ActionScoreSum / Denominator;
        Snapshot.AverageSupportScore = SupportScoreSum / Denominator;
        return Snapshot;
    }

    const wchar_t* GetVoteIntentDisplayName(EVoteIntent VoteIntent)
    {
        switch (VoteIntent)
        {
        case EVoteIntent::Incumbent:
            return L"현 정권 지지";
        case EVoteIntent::Opposition:
            return L"야권 지지";
        default:
            return L"기권/부동층";
        }
    }
}
