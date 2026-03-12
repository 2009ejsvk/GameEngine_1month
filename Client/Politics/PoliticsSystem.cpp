#include "PoliticsSystem.h"
#include "EdictSystem.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
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

    float ResolveWealthTaxSensitivity(
        const FCitizenIdentityProfile& Identity)
    {
        switch (Identity.WealthLevel)
        {
        case ECitizenWealthLevel::Rich:
            return 0.92f;
        case ECitizenWealthLevel::WellOff:
            return 1.00f;
        default:
            return 1.12f;
        }
    }

    float ResolveExpectedLivingStandard(
        const FCitizenIdentityProfile& Identity)
    {
        switch (Identity.WealthLevel)
        {
        case ECitizenWealthLevel::Rich:
            return 76.f;
        case ECitizenWealthLevel::WellOff:
            return 67.f;
        default:
            return 58.f;
        }
    }

    float EvaluateFactionMembershipScore(
        const CBuildingMarkerOrb& Citizen,
        EPoliticalFaction Faction)
    {
        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();
        const FNpcSatisfaction& Satisfaction = Citizen.GetSatisfaction();
        const FCitizenIdentityProfile& Identity = Citizen.GetIdentityProfile();
        const bool HasHome = !Citizen.GetHomeBuilding().empty();
        const bool HasWork = !Citizen.GetWorkBuilding().empty();
        const bool HasFaithAccess = !Citizen.GetFaithBuilding().empty();
        const EPoliticalAxis Axis = GetPoliticalFactionAxis(Faction);
        const EPoliticalStance Stance = GetPoliticalFactionStance(Faction);
        const FNpcPoliticalChoice& Choice = PoliticalProfile.Get(Axis);
        const float SupportWeight = GetSupportLevelWeight(Choice.Support);
        float Score = 0.f;

        if (Choice.Stance == Stance)
            Score = 24.f + SupportWeight * 12.f;
        else if (Choice.Stance == EPoliticalStance::Neutral)
            Score = 10.f + SupportWeight * 3.f;
        else
            Score = 2.5f;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            Score +=
                Identity.WealthLevel == ECitizenWealthLevel::Poor ? 8.f :
                Identity.WealthLevel == ECitizenWealthLevel::WellOff ? 3.f :
                -4.f;
            Score += (std::max)(0.f, 60.f - Satisfaction.Housing) * 0.14f;
            Score += (std::max)(0.f, 58.f - Satisfaction.Job) * 0.12f;
            if (!HasHome)
                Score += 4.f;
            if (!HasWork)
                Score += 5.f;
            break;
        case EPoliticalFaction::Capitalists:
            Score +=
                Identity.WealthLevel == ECitizenWealthLevel::Rich ? 9.f :
                Identity.WealthLevel == ECitizenWealthLevel::WellOff ? 4.f :
                -3.f;
            Score += (std::max)(0.f, Satisfaction.Freedom - 52.f) * 0.10f;
            if (HasWork)
                Score += 3.f;
            break;
        case EPoliticalFaction::Religious:
            Score += (std::max)(0.f, Satisfaction.Faith - 50.f) * 0.12f;
            if (HasFaithAccess)
                Score += 4.f;
            break;
        case EPoliticalFaction::Militarists:
            Score += (std::max)(0.f, Satisfaction.Security - 48.f) * 0.10f;
            break;
        case EPoliticalFaction::Environmentalists:
            Score +=
                Identity.EducationLevel == ECitizenEducationLevel::College ? 4.f :
                Identity.EducationLevel == ECitizenEducationLevel::HighSchool ?
                    1.5f :
                    0.f;
            Score += (std::max)(0.f, 60.f - Satisfaction.Health) * 0.10f;
            break;
        case EPoliticalFaction::Industrialists:
            if (HasWork)
                Score += 4.f;
            Score += (std::max)(0.f, Satisfaction.Job - 52.f) * 0.10f;
            break;
        case EPoliticalFaction::Intellectuals:
            Score +=
                Identity.EducationLevel == ECitizenEducationLevel::College ? 9.f :
                Identity.EducationLevel == ECitizenEducationLevel::HighSchool ?
                    4.f :
                    -2.f;
            Score += (std::max)(0.f, Satisfaction.Freedom - 52.f) * 0.12f;
            break;
        case EPoliticalFaction::Conservatives:
            Score += (std::max)(0.f, Satisfaction.Security - 50.f) * 0.08f;
            Score += (std::max)(0.f, Satisfaction.Faith - 52.f) * 0.06f;
            if (HasHome)
                Score += 2.f;
            break;
        default:
            break;
        }

        return Score;
    }

    EPoliticalFaction ResolvePrimaryFaction(
        const CBuildingMarkerOrb& Citizen)
    {
        EPoliticalFaction BestFaction = EPoliticalFaction::Communists;
        float BestScore = -1.f;

        for (int FactionIndex = 0;
            FactionIndex < GPoliticalFactionCount;
            ++FactionIndex)
        {
            const EPoliticalFaction Candidate =
                static_cast<EPoliticalFaction>(FactionIndex);
            const float CandidateScore =
                EvaluateFactionMembershipScore(Citizen, Candidate);

            if (FactionIndex == 0 || CandidateScore > BestScore)
            {
                BestFaction = Candidate;
                BestScore = CandidateScore;
            }
        }

        return BestFaction;
    }

    float EvaluateFactionAlignmentScore(
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile,
        EPoliticalFaction Faction)
    {
        const EPoliticalAxis Axis = GetPoliticalFactionAxis(Faction);
        const EPoliticalStance FactionStance = GetPoliticalFactionStance(Faction);
        const FNpcPoliticalChoice& CitizenChoice =
            Citizen.GetPoliticalProfile().Get(Axis);
        const FNpcPoliticalChoice& GovernmentChoice =
            GovernmentProfile.Ideology.Get(Axis);
        const float SupportWeight = GetSupportLevelWeight(CitizenChoice.Support);
        const float MembershipWeight =
            CitizenChoice.Stance == FactionStance ? 1.0f :
            CitizenChoice.Stance == EPoliticalStance::Neutral ? 0.45f :
            0.18f;

        if (GovernmentChoice.Stance == FactionStance)
            return (6.f + SupportWeight * 6.f) * MembershipWeight;

        if (GovernmentChoice.Stance == EPoliticalStance::Neutral)
            return 1.5f * MembershipWeight;

        return -(5.f + SupportWeight * 5.f) * MembershipWeight;
    }

    float EvaluateFactionIssueScore(
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile,
        EPoliticalFaction Faction)
    {
        const FNpcSatisfaction& Satisfaction = Citizen.GetSatisfaction();
        const FCitizenIdentityProfile& Identity = Citizen.GetIdentityProfile();
        const bool IsWorker = !Citizen.GetWorkBuilding().empty();
        const bool IsResident = !Citizen.GetHomeBuilding().empty();
        const bool HasFaithAccess = !Citizen.GetFaithBuilding().empty();
        const float TaxBurden = GetCitizenTaxBurdenNormalized(
            GovernmentProfile.TaxPolicy,
            IsWorker,
            IsResident);
        float Score = 0.f;

        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            Score += (Satisfaction.Housing - 55.f) * 0.12f;
            Score += (Satisfaction.Job - 55.f) * 0.11f;
            Score += (Satisfaction.Food - 55.f) * 0.08f;
            Score += GovernmentProfile.WelfareBias * 9.f;
            Score -= (std::max)(0.f, TaxBurden) * 6.f;
            break;
        case EPoliticalFaction::Capitalists:
            Score += (Satisfaction.Job - 55.f) * 0.10f;
            Score += (Satisfaction.Freedom - 55.f) * 0.11f;
            Score -= (std::max)(0.f, TaxBurden) * 13.f;
            Score += (std::max)(0.f, -GovernmentProfile.WelfareBias) * 6.f;
            Score +=
                Identity.WealthLevel == ECitizenWealthLevel::Rich ? 4.f :
                Identity.WealthLevel == ECitizenWealthLevel::Poor ? -2.f :
                0.f;
            break;
        case EPoliticalFaction::Religious:
            Score += (Satisfaction.Faith - 55.f) * 0.17f;
            Score += (Satisfaction.Security - 55.f) * 0.04f;
            if (!HasFaithAccess)
                Score -= 4.f;
            break;
        case EPoliticalFaction::Militarists:
            Score += (Satisfaction.Security - 55.f) * 0.16f;
            Score += GovernmentProfile.Militarization * 10.f;
            break;
        case EPoliticalFaction::Environmentalists:
            Score += (Satisfaction.Health - 55.f) * 0.18f;
            Score += (Satisfaction.Freedom - 55.f) * 0.05f;
            break;
        case EPoliticalFaction::Industrialists:
            Score += (Satisfaction.Job - 55.f) * 0.15f;
            Score += (Satisfaction.Security - 55.f) * 0.05f;
            if (!IsWorker)
                Score -= 4.f;
            break;
        case EPoliticalFaction::Intellectuals:
            Score += (Satisfaction.Freedom - 55.f) * 0.15f;
            Score += (Satisfaction.Health - 55.f) * 0.05f;
            Score +=
                Identity.EducationLevel == ECitizenEducationLevel::College ? 4.f :
                Identity.EducationLevel == ECitizenEducationLevel::HighSchool ?
                    1.5f :
                    -2.f;
            break;
        case EPoliticalFaction::Conservatives:
            Score += (Satisfaction.Security - 55.f) * 0.11f;
            Score += (Satisfaction.Faith - 55.f) * 0.08f;
            if (!IsResident)
                Score -= 2.f;
            break;
        default:
            break;
        }

        return Score;
    }

    float CalculateNeedDistressPenalty(
        float Value,
        float WarningThreshold,
        float CrisisThreshold,
        float WarningScale,
        float CrisisScale)
    {
        if (Value >= WarningThreshold)
            return 0.f;

        float Penalty = (WarningThreshold - Value) * WarningScale;

        if (Value < CrisisThreshold)
            Penalty += (CrisisThreshold - Value) * CrisisScale;

        return Penalty;
    }

    float CalculateLivingStandardAdjustment(
        const CBuildingMarkerOrb& Citizen)
    {
        const FNpcSatisfaction& Satisfaction = Citizen.GetSatisfaction();
        const float LivingAverage =
            Satisfaction.Food * 0.24f +
            Satisfaction.Housing * 0.24f +
            Satisfaction.Job * 0.18f +
            Satisfaction.Health * 0.14f +
            Satisfaction.Fun * 0.10f +
            Satisfaction.Faith * 0.10f;
        const float Expected =
            ResolveExpectedLivingStandard(Citizen.GetIdentityProfile());
        return (LivingAverage - Expected) * 0.24f;
    }

    float CalculateServiceScarcityPenalty(
        const FNpcSatisfaction& Satisfaction)
    {
        return
            CalculateNeedDistressPenalty(
                Satisfaction.Food, 58.f, 34.f, 0.12f, 0.24f) +
            CalculateNeedDistressPenalty(
                Satisfaction.Health, 56.f, 34.f, 0.10f, 0.18f) +
            CalculateNeedDistressPenalty(
                Satisfaction.Fun, 52.f, 28.f, 0.08f, 0.15f) +
            CalculateNeedDistressPenalty(
                Satisfaction.Faith, 50.f, 28.f, 0.06f, 0.12f);
    }

    float CalculateSecurityFreedomPressure(
        const CBuildingMarkerOrb& Citizen)
    {
        const FNpcSatisfaction& Satisfaction = Citizen.GetSatisfaction();
        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();
        const FNpcPoliticalChoice& OrderChoice =
            PoliticalProfile.Get(EPoliticalAxis::ReligionMilitarism);
        const FNpcPoliticalChoice& LibertyChoice =
            PoliticalProfile.Get(EPoliticalAxis::IntellectualConservative);
        const float OrderSensitivity =
            1.f +
            (std::max)(
                0.f,
                GetStanceAlignment(
                    OrderChoice.Stance,
                    EPoliticalStance::Right)) *
                0.55f *
                GetSupportLevelWeight(OrderChoice.Support);
        const float LibertySensitivity =
            0.85f +
            (std::max)(
                0.f,
                GetStanceAlignment(
                    LibertyChoice.Stance,
                    EPoliticalStance::Left)) *
                0.65f *
                GetSupportLevelWeight(LibertyChoice.Support);
        const float SecurityPenalty =
            CalculateNeedDistressPenalty(
                Satisfaction.Security,
                58.f,
                34.f,
                0.10f,
                0.20f) *
            OrderSensitivity;
        const float FreedomPenalty =
            CalculateNeedDistressPenalty(
                Satisfaction.Freedom,
                58.f,
                36.f,
                0.09f,
                0.18f) *
            LibertySensitivity;
        return -(SecurityPenalty + FreedomPenalty);
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
        const FCitizenIdentityProfile& Identity = Citizen.GetIdentityProfile();
        const float TaxBurden = GetCitizenTaxBurdenNormalized(
            GovernmentProfile.TaxPolicy,
            IsWorker,
            IsResident);
        const float ConsumptionTaxStress = (std::max)(
            0.f,
            GetTaxPolicyDeviationNormalized(
                GovernmentProfile.TaxPolicy,
                ETaxPolicyType::Consumption));
        const float IncomeTaxStress = (std::max)(
            0.f,
            GetTaxPolicyDeviationNormalized(
                GovernmentProfile.TaxPolicy,
                ETaxPolicyType::Income));
        const float PropertyTaxStress = (std::max)(
            0.f,
            GetTaxPolicyDeviationNormalized(
                GovernmentProfile.TaxPolicy,
                ETaxPolicyType::Property));

        if (TaxBurden == 0.f)
            return 0.f;

        const FNpcPoliticalProfile& PoliticalProfile =
            Citizen.GetPoliticalProfile();
        const FNpcPoliticalChoice& EconomyChoice =
            PoliticalProfile.Get(EPoliticalAxis::Economy);
        const float SupportWeight =
            GetSupportLevelWeight(EconomyChoice.Support);
        const float WealthSensitivity =
            ResolveWealthTaxSensitivity(Identity);
        const float TaxStress = (std::max)(0.f, TaxBurden);
        const float TaxRelief = (std::max)(0.f, -TaxBurden);
        float Score =
            -TaxStress * (IsWorker ? 3.6f : 2.8f) * WealthSensitivity +
            TaxRelief * (IsWorker ? 1.1f : 0.8f);

        Score -=
            ConsumptionTaxStress * 4.4f *
            (0.85f + WealthSensitivity * 0.40f);
        Score -=
            IncomeTaxStress *
            (IsWorker ? 5.1f : 1.4f) *
            WealthSensitivity;
        Score -=
            PropertyTaxStress *
            (IsResident ?
                (Identity.WealthLevel == ECitizenWealthLevel::Rich ?
                    5.6f :
                    3.5f) :
                0.8f);

        if (TaxStress > 0.30f)
            Score -= (TaxStress - 0.30f) * 10.5f;

        switch (EconomyChoice.Stance)
        {
        case EPoliticalStance::Left:
            Score += -TaxBurden * SupportWeight * 10.5f;
            break;
        case EPoliticalStance::Right:
            Score += TaxBurden * SupportWeight * 6.2f;
            break;
        default:
            Score += -TaxStress * 2.1f + TaxRelief * 0.7f;
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
        float Score = -3.5f * Severity;
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
            Score += (IsWorker ? -14.0f : -5.2f) * Severity;

            if (IsResident)
                Score += -2.6f * Severity;

            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Left,
                5.6f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Left,
                3.4f * Severity);
            break;
        case ETaxPolicyEventType::PropertyTaxBacklash:
            Score += (IsResident ? -13.0f : -4.6f) * Severity;

            if (IsWorker)
                Score += -2.2f * Severity;

            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right,
                5.0f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Left,
                3.8f * Severity);
            break;
        case ETaxPolicyEventType::BudgetCrisis:
            Score += (IsWorker || IsResident ? -11.5f : -8.0f) * Severity;
            Score += ApplyDemandPressure(
                EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right,
                4.8f * Severity);
            Score += ApplyDemandPressure(
                EPoliticalAxis::Economy,
                EPoliticalStance::Right,
                4.2f * Severity);
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
        const bool HasHome = !Citizen.GetHomeBuilding().empty();
        const bool HasWork = !Citizen.GetWorkBuilding().empty();
        const bool HasFoodAccess = !Citizen.GetFoodBuilding().empty();
        const bool HasFunAccess = !Citizen.GetFunBuilding().empty();
        const bool HasHealthAccess = !Citizen.GetHealthBuilding().empty();
        const bool HasFaithAccess = !Citizen.GetFaithBuilding().empty();
        const float CoreNeedPenalty =
            CalculateNeedDistressPenalty(
                Satisfaction.Housing, 60.f, 36.f, 0.11f, 0.20f) +
            CalculateNeedDistressPenalty(
                Satisfaction.Job, 58.f, 34.f, 0.12f, 0.22f);
        const float ServiceScarcityPenalty =
            CalculateServiceScarcityPenalty(Satisfaction);
        const float CommutePenalty =
            Clamp<float>(
                Satisfaction.CommuteTimePenalty,
                0.f,
                100.f) * 0.08f;

        Evaluation.LifeScore =
            (Satisfaction.Overall - 50.f) * 0.68f;

        Evaluation.LifeScore +=
            CalculateLivingStandardAdjustment(Citizen);
        Evaluation.LifeScore +=
            CalculateSecurityFreedomPressure(Citizen);
        Evaluation.LifeScore -=
            CoreNeedPenalty + ServiceScarcityPenalty + CommutePenalty;

        if (!HasHome)
            Evaluation.LifeScore -= 18.f;

        if (!HasWork)
            Evaluation.LifeScore -= 20.f;

        if (!HasFoodAccess)
            Evaluation.LifeScore -= 12.f;

        if (!HasFunAccess)
            Evaluation.LifeScore -= 3.5f;

        if (!HasHealthAccess)
            Evaluation.LifeScore -= 3.0f;

        if (!HasFaithAccess)
            Evaluation.LifeScore -= 2.0f;

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

        if (!HasWork)
            Evaluation.ActionScore -= 6.5f;

        if (!HasHome)
            Evaluation.ActionScore -= 5.5f;

        if (Satisfaction.Food < 42.f)
            Evaluation.ActionScore -= (42.f - Satisfaction.Food) * 0.12f;

        if (Satisfaction.Security < 45.f)
            Evaluation.ActionScore -=
                (45.f - Satisfaction.Security) * 0.10f;

        if (Satisfaction.Overall < 45.f)
            Evaluation.ActionScore -=
                (45.f - Satisfaction.Overall) * 0.16f;

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

        Evaluation.PrimaryFaction = ResolvePrimaryFaction(Citizen);
        Evaluation.FactionAlignmentScore = EvaluateFactionAlignmentScore(
            Citizen,
            GovernmentProfile,
            Evaluation.PrimaryFaction);
        Evaluation.FactionApprovalScore = ClampSupportScore(
            Evaluation.TotalSupportScore * 0.72f +
            14.f +
            Evaluation.FactionAlignmentScore +
            EvaluateFactionIssueScore(
                Citizen,
                GovernmentProfile,
                Evaluation.PrimaryFaction) +
            static_cast<float>(
                GovernmentProfile.FactionApprovalModifiers[
                    static_cast<size_t>(Evaluation.PrimaryFaction)] +
                GovernmentProfile.EdictFactionApprovalModifiers[
                    static_cast<size_t>(Evaluation.PrimaryFaction)]));

        if (Evaluation.TotalSupportScore >= 58.f)
            Evaluation.VoteIntent = EVoteIntent::Incumbent;
        else if (Evaluation.TotalSupportScore < 46.f)
            Evaluation.VoteIntent = EVoteIntent::Opposition;
        else
            Evaluation.VoteIntent = EVoteIntent::Abstain;

        return Evaluation;
    }

    int GetDaysInMonth(int Year, int Month)
    {
        switch (Month)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
        {
            const bool IsLeapYear =
                (Year % 400 == 0) || (Year % 4 == 0 && Year % 100 != 0);
            return IsLeapYear ? 29 : 28;
        }
        default:
            return 30;
        }
    }

    void ScheduleNextElection(
        FElectionStatus& InOutStatus,
        int CurrentYear,
        int YearsUntilElection,
        int ElectionMonth,
        int ElectionDay)
    {
        const int SafeYearsUntilElection = (std::max)(1, YearsUntilElection);
        InOutStatus.NextElectionYear = CurrentYear + SafeYearsUntilElection;
        InOutStatus.NextElectionMonth = ElectionMonth;
        InOutStatus.NextElectionDay = ElectionDay;
    }

    int CountActiveElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (ElectionStatus.ActivePromises[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int CountMetElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (IsElectionPromiseMet(
                    ElectionStatus.ActivePromises[static_cast<size_t>(Index)]))
            {
                ++Count;
            }
        }

        return Count;
    }

    double ComputeElectionPromiseRisk(const FElectionStatus& ElectionStatus)
    {
        int ActiveCount = 0;
        double RiskSum = 0.0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            const FElectionPromiseState& Promise =
                ElectionStatus.ActivePromises[static_cast<size_t>(Index)];

            if (!Promise.Active)
                continue;

            ++ActiveCount;

            if (Promise.TargetValue <= Promise.BaselineValue)
            {
                RiskSum += IsElectionPromiseMet(Promise) ? 0.0 : 1.0;
                continue;
            }

            const double Progress =
                Clamp<double>(
                    static_cast<double>(Promise.CurrentValue - Promise.BaselineValue) /
                        static_cast<double>(
                            Promise.TargetValue - Promise.BaselineValue),
                    0.0,
                    1.0);
            RiskSum += 1.0 - Progress;
        }

        if (ActiveCount <= 0)
            return 0.0;

        return Clamp<double>(
            RiskSum / static_cast<double>(ActiveCount),
            0.0,
            1.0);
    }

    int ComputeElectionPromiseVoteModifierPercent(
        const FElectionStatus& ElectionStatus,
        int& OutMetCount,
        int& OutFailedCount)
    {
        int VoteModifierPercent = 0;
        OutMetCount = 0;
        OutFailedCount = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            const FElectionPromiseState& Promise =
                ElectionStatus.ActivePromises[static_cast<size_t>(Index)];

            if (!Promise.Active)
                continue;

            if (IsElectionPromiseMet(Promise))
            {
                ++OutMetCount;
                VoteModifierPercent += Promise.SuccessVoteModifierPercent;
            }
            else
            {
                ++OutFailedCount;
                VoteModifierPercent -= Promise.FailureVoteModifierPercent;
            }
        }

        return VoteModifierPercent;
    }

    void ApplyElectionPromiseVoteModifier(
        int VoteModifierPercent,
        int ActiveCitizenCount,
        int& InOutIncumbentVotes,
        int& InOutOppositionVotes,
        int& InOutAbstainVotes)
    {
        if (VoteModifierPercent == 0 || ActiveCitizenCount <= 0)
            return;

        int VoteShift = static_cast<int>(std::lround(
            static_cast<double>(ActiveCitizenCount) *
            static_cast<double>(VoteModifierPercent) / 100.0));

        if (VoteShift > 0)
        {
            int ShiftFromOpposition =
                (std::min)(InOutOppositionVotes, VoteShift / 2);
            int ShiftFromAbstain =
                (std::min)(InOutAbstainVotes, VoteShift - ShiftFromOpposition);
            int RemainingShift =
                VoteShift - ShiftFromOpposition - ShiftFromAbstain;

            if (RemainingShift > 0)
            {
                const int AdditionalOppositionShift =
                    (std::min)(
                        InOutOppositionVotes - ShiftFromOpposition,
                        RemainingShift);
                ShiftFromOpposition += AdditionalOppositionShift;
                RemainingShift -= AdditionalOppositionShift;
            }

            if (RemainingShift > 0)
            {
                ShiftFromAbstain +=
                    (std::min)(
                        InOutAbstainVotes - ShiftFromAbstain,
                        RemainingShift);
            }

            InOutOppositionVotes -= ShiftFromOpposition;
            InOutAbstainVotes -= ShiftFromAbstain;
            InOutIncumbentVotes += ShiftFromOpposition + ShiftFromAbstain;
            return;
        }

        VoteShift = -VoteShift;
        int ShiftToOpposition =
            (std::min)(InOutIncumbentVotes, VoteShift * 2 / 3);
        int ShiftToAbstain =
            (std::min)(
                InOutIncumbentVotes - ShiftToOpposition,
                VoteShift - ShiftToOpposition);

        if (ShiftToOpposition + ShiftToAbstain < VoteShift)
        {
            ShiftToOpposition +=
                (std::min)(
                    InOutIncumbentVotes - ShiftToOpposition - ShiftToAbstain,
                    VoteShift - ShiftToOpposition - ShiftToAbstain);
        }

        InOutIncumbentVotes -= ShiftToOpposition + ShiftToAbstain;
        InOutOppositionVotes += ShiftToOpposition;
        InOutAbstainVotes += ShiftToAbstain;
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

    void SyncGovernmentActionFromEdict(
        FGovernmentProfile& InOutProfile,
        EGovernmentEdictType Type,
        bool Active)
    {
        const FGovernmentEdictDefinition* Definition =
            EdictSystem::FindGovernmentEdictDefinition(Type);

        if (!Definition || Definition->ActionType == EPoliticalActionType::None)
            return;

        InOutProfile.ActiveActions.erase(
            std::remove_if(
                InOutProfile.ActiveActions.begin(),
                InOutProfile.ActiveActions.end(),
                [&](const FGovernmentActionRecord& Action)
                {
                    return Action.Type == Definition->ActionType;
                }),
            InOutProfile.ActiveActions.end());

        if (!Active)
            return;

        InOutProfile.ActiveActions.push_back(
            EdictSystem::MakeGovernmentActionFromEdict(*Definition));
    }

    void ApplyDailyEdictCitizenEffects(
        CWorld* World,
        const FGovernmentEdictModifiers& Modifiers)
    {
        if (!World)
            return;

        if (Modifiers.DailyFoodDelta == 0.f &&
            Modifiers.DailyHousingDelta == 0.f &&
            Modifiers.DailyJobDelta == 0.f &&
            Modifiers.DailyFreedomDelta == 0.f &&
            Modifiers.DailySecurityDelta == 0.f)
        {
            return;
        }

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return;

        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            Orb->ApplySatisfactionDelta(
                Modifiers.DailyFoodDelta,
                0.f,
                0.f,
                0.f,
                Modifiers.DailyHousingDelta,
                Modifiers.DailyJobDelta,
                Modifiers.DailyFreedomDelta,
                Modifiers.DailySecurityDelta);
        }
    }

    FCitizenPoliticalEvaluation EvaluateCitizen(
        CWorld* World,
        const CBuildingMarkerOrb& Citizen,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        const std::vector<FPlacedPoliticalSignal> PlacedSignals =
            CollectPlacedSignals(World);

        return EvaluateCitizenInternal(
            Citizen,
            GovernmentProfile,
            PlacedSignals,
            TaxEventStatus);
    }

    FPoliticalWorldSnapshot EvaluateWorld(
        CWorld* World,
        const FGovernmentProfile& GovernmentProfile,
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        FPoliticalWorldSnapshot Snapshot;

        if (!World)
            return Snapshot;

        const std::vector<FPlacedPoliticalSignal> PlacedSignals =
            CollectPlacedSignals(World);

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return Snapshot;

        double LifeScoreSum = 0.0;
        double GovernmentScoreSum = 0.0;
        double BuildingScoreSum = 0.0;
        double ActionScoreSum = 0.0;
        double SupportScoreSum = 0.0;
        std::array<int, GPoliticalFactionCount> FactionCounts = {};
        std::array<double, GPoliticalFactionCount> FactionApprovalSums = {};
        std::array<double, GPoliticalFactionCount> FactionLifeSums = {};
        std::array<double, GPoliticalFactionCount> FactionBuildingSums = {};
        std::array<double, GPoliticalFactionCount> FactionActionSums = {};
        std::array<double, GPoliticalFactionCount> FactionAlignmentSums = {};

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
            const int FactionIndex =
                static_cast<int>(Evaluation.PrimaryFaction);

            if (FactionIndex >= 0 && FactionIndex < GPoliticalFactionCount)
            {
                ++FactionCounts[static_cast<size_t>(FactionIndex)];
                FactionApprovalSums[static_cast<size_t>(FactionIndex)] +=
                    Evaluation.FactionApprovalScore;
                FactionLifeSums[static_cast<size_t>(FactionIndex)] +=
                    Evaluation.LifeScore;
                FactionBuildingSums[static_cast<size_t>(FactionIndex)] +=
                    Evaluation.BuildingScore;
                FactionActionSums[static_cast<size_t>(FactionIndex)] +=
                    Evaluation.ActionScore;
                FactionAlignmentSums[static_cast<size_t>(FactionIndex)] +=
                    Evaluation.FactionAlignmentScore;
            }

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

        for (int FactionIndex = 0;
            FactionIndex < GPoliticalFactionCount;
            ++FactionIndex)
        {
            auto& FactionSnapshot =
                Snapshot.Factions[static_cast<size_t>(FactionIndex)];
            const int MemberCount =
                FactionCounts[static_cast<size_t>(FactionIndex)];
            FactionSnapshot.MemberCount = MemberCount;
            FactionSnapshot.ApprovalModifier =
                static_cast<double>(
                    GovernmentProfile.FactionApprovalModifiers[
                        static_cast<size_t>(FactionIndex)] +
                    GovernmentProfile.EdictFactionApprovalModifiers[
                        static_cast<size_t>(FactionIndex)]);

            if (MemberCount <= 0)
            {
                FactionSnapshot.AverageApproval = 50.0;
                FactionSnapshot.AverageLifeScore = 0.0;
                FactionSnapshot.AverageBuildingScore = 0.0;
                FactionSnapshot.AverageActionScore = 0.0;
                FactionSnapshot.AverageAlignmentScore = 0.0;
                continue;
            }

            const double FactionDenominator =
                static_cast<double>(MemberCount);
            FactionSnapshot.AverageApproval =
                FactionApprovalSums[static_cast<size_t>(FactionIndex)] /
                FactionDenominator;
            FactionSnapshot.AverageLifeScore =
                FactionLifeSums[static_cast<size_t>(FactionIndex)] /
                FactionDenominator;
            FactionSnapshot.AverageBuildingScore =
                FactionBuildingSums[static_cast<size_t>(FactionIndex)] /
                FactionDenominator;
            FactionSnapshot.AverageActionScore =
                FactionActionSums[static_cast<size_t>(FactionIndex)] /
                FactionDenominator;
            FactionSnapshot.AverageAlignmentScore =
                FactionAlignmentSums[static_cast<size_t>(FactionIndex)] /
                FactionDenominator;
        }

        return Snapshot;
    }

    void InitializeElectionSchedule(
        FElectionStatus& OutStatus,
        int CurrentYear,
        int InitialLeadYears,
        int ElectionMonth,
        int ElectionDay)
    {
        OutStatus = FElectionStatus();
        OutStatus.ActivePromises = {};
        OutStatus.CampaignPromisesIssued = false;
        OutStatus.PromiseIssueYear = 0;
        OutStatus.PromiseIssueMonth = 0;
        OutStatus.PromiseIssueDay = 0;
        ScheduleNextElection(
            OutStatus,
            CurrentYear,
            InitialLeadYears,
            ElectionMonth,
            ElectionDay);
    }

    void ResolveScheduledElection(
        FElectionStatus& InOutStatus,
        const FPoliticalWorldSnapshot& Snapshot,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay,
        int ElectionIntervalYears,
        int ElectionMonth,
        int ElectionDay)
    {
        if (InOutStatus.GameLost)
            return;

        if (CurrentYear != InOutStatus.NextElectionYear ||
            CurrentMonth != InOutStatus.NextElectionMonth ||
            CurrentDay != InOutStatus.NextElectionDay)
        {
            return;
        }

        const int ActiveCitizenCount =
            (std::max)(0, Snapshot.ActiveCitizenCount);
        int IncumbentVotes =
            (std::max)(0, Snapshot.IncumbentCount);
        int OppositionVotes =
            (std::max)(0, Snapshot.OppositionCount);
        int AbstainVotes =
            (std::max)(0, Snapshot.AbstainCount);
        int PromiseMetCount = 0;
        int PromiseFailedCount = 0;
        const int PromiseVoteModifierPercent =
            ComputeElectionPromiseVoteModifierPercent(
                InOutStatus,
                PromiseMetCount,
                PromiseFailedCount);
        ApplyElectionPromiseVoteModifier(
            PromiseVoteModifierPercent,
            ActiveCitizenCount,
            IncumbentVotes,
            OppositionVotes,
            AbstainVotes);
        const int CastVotes = IncumbentVotes + OppositionVotes;
        const bool IncumbentWon =
            CastVotes > 0 && IncumbentVotes >= OppositionVotes;

        InOutStatus.HasRecordedElection = true;
        InOutStatus.IncumbentWonLastElection = IncumbentWon;
        InOutStatus.LastElectionYear = CurrentYear;
        InOutStatus.LastElectionMonth = CurrentMonth;
        InOutStatus.LastElectionDay = CurrentDay;
        InOutStatus.LastIncumbentVotes = IncumbentVotes;
        InOutStatus.LastOppositionVotes = OppositionVotes;
        InOutStatus.LastAbstainVotes = AbstainVotes;
        InOutStatus.LastVoteShare =
            CastVotes > 0 ?
            static_cast<double>(IncumbentVotes) /
                static_cast<double>(CastVotes) * 100.0 :
            0.0;
        InOutStatus.LastTurnoutPercent =
            ActiveCitizenCount > 0 ?
            static_cast<double>(CastVotes) /
                static_cast<double>(ActiveCitizenCount) * 100.0 :
            0.0;
        InOutStatus.HasPromiseEvaluation =
            CountActiveElectionPromises(InOutStatus) > 0;
        InOutStatus.LastPromiseMetCount = PromiseMetCount;
        InOutStatus.LastPromiseFailedCount = PromiseFailedCount;
        InOutStatus.LastPromiseVoteModifierPercent =
            PromiseVoteModifierPercent;
        InOutStatus.ActivePromises = {};
        InOutStatus.CampaignPromisesIssued = false;
        InOutStatus.PromiseIssueYear = 0;
        InOutStatus.PromiseIssueMonth = 0;
        InOutStatus.PromiseIssueDay = 0;

        if (IncumbentWon)
        {
            ++InOutStatus.ElectionsWon;
            ScheduleNextElection(
                InOutStatus,
                CurrentYear,
                ElectionIntervalYears,
                ElectionMonth,
                ElectionDay);
            return;
        }

        InOutStatus.GameLost = true;
        InOutStatus.NextElectionYear = 0;
        InOutStatus.NextElectionMonth = 0;
        InOutStatus.NextElectionDay = 0;
    }

    int GetDaysUntilNextElection(
        const FElectionStatus& ElectionStatus,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay)
    {
        if (ElectionStatus.GameLost ||
            ElectionStatus.NextElectionYear <= 0 ||
            ElectionStatus.NextElectionMonth <= 0 ||
            ElectionStatus.NextElectionDay <= 0)
        {
            return -1;
        }

        if (CurrentYear > ElectionStatus.NextElectionYear ||
            (CurrentYear == ElectionStatus.NextElectionYear &&
                CurrentMonth > ElectionStatus.NextElectionMonth) ||
            (CurrentYear == ElectionStatus.NextElectionYear &&
                CurrentMonth == ElectionStatus.NextElectionMonth &&
                CurrentDay >= ElectionStatus.NextElectionDay))
        {
            return 0;
        }

        int DaysUntilElection = 0;
        int Year = CurrentYear;
        int Month = CurrentMonth;
        int Day = CurrentDay;

        while (Year < ElectionStatus.NextElectionYear ||
            (Year == ElectionStatus.NextElectionYear &&
                Month < ElectionStatus.NextElectionMonth) ||
            (Year == ElectionStatus.NextElectionYear &&
                Month == ElectionStatus.NextElectionMonth &&
                Day < ElectionStatus.NextElectionDay))
        {
            ++DaysUntilElection;
            ++Day;

            if (Day > GetDaysInMonth(Year, Month))
            {
                Day = 1;
                ++Month;

                if (Month > 12)
                {
                    Month = 1;
                    ++Year;
                }
            }

            if (DaysUntilElection > 3660)
                break;
        }

        return DaysUntilElection;
    }

    double GetElectionWarningScore(
        const FElectionStatus& ElectionStatus,
        const FPoliticalWorldSnapshot& Snapshot,
        const FTaxPolicyEventStatus& TaxEventStatus,
        int CurrentYear,
        int CurrentMonth,
        int CurrentDay)
    {
        if (ElectionStatus.GameLost)
            return 1.0;

        const int DaysUntilElection = GetDaysUntilNextElection(
            ElectionStatus,
            CurrentYear,
            CurrentMonth,
            CurrentDay);

        if (DaysUntilElection < 0)
            return 0.0;

        double ProximityPressure = 0.0;

        if (DaysUntilElection <= 30)
        {
            ProximityPressure = 1.0;
        }
        else if (DaysUntilElection <= 90)
        {
            ProximityPressure =
                0.75 +
                static_cast<double>(90 - DaysUntilElection) / 60.0 * 0.25;
        }
        else if (DaysUntilElection <= 180)
        {
            ProximityPressure =
                0.45 +
                static_cast<double>(180 - DaysUntilElection) / 90.0 * 0.30;
        }
        else if (DaysUntilElection <= 365)
        {
            ProximityPressure =
                0.15 +
                static_cast<double>(365 - DaysUntilElection) / 185.0 * 0.30;
        }

        const int ActiveCitizenCount = (std::max)(1, Snapshot.ActiveCitizenCount);
        const double SupportPercent =
            static_cast<double>(Snapshot.IncumbentCount) /
            static_cast<double>(ActiveCitizenCount) * 100.0;
        const double OppositionPercent =
            static_cast<double>(Snapshot.OppositionCount) /
            static_cast<double>(ActiveCitizenCount) * 100.0;
        const double AbstainPercent =
            static_cast<double>(Snapshot.AbstainCount) /
            static_cast<double>(ActiveCitizenCount) * 100.0;
        const double SupportRisk =
            Clamp<double>((52.0 - SupportPercent) / 22.0, 0.0, 1.0);
        const double OppositionRisk =
            Clamp<double>((OppositionPercent - 34.0) / 24.0, 0.0, 1.0);
        const double AbstainRisk =
            Clamp<double>((AbstainPercent - 18.0) / 25.0, 0.0, 1.0);
        const double LifeRisk =
            Clamp<double>((6.0 - Snapshot.AverageLifeScore) / 18.0, 0.0, 1.0);
        const double ActionRisk =
            Clamp<double>((0.0 - Snapshot.AverageActionScore) / 18.0, 0.0, 1.0);
        const double PromiseRisk =
            DaysUntilElection <= 365 ?
                ComputeElectionPromiseRisk(ElectionStatus) :
                0.0;

        double EventPressure = 0.0;

        if (TaxEventStatus.Active)
        {
            EventPressure =
                0.40 +
                Clamp<double>(
                    static_cast<double>(TaxEventStatus.DaysActive + 1) / 6.0,
                    0.0,
                    1.0) * 0.35;

            if (TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis)
                EventPressure += 0.10;
        }
        else if (TaxEventStatus.NotificationDays > 0 &&
            !TaxEventStatus.Summary.empty())
        {
            EventPressure = 0.18;
        }

        const double Score =
            0.34 * ProximityPressure +
            0.19 * SupportRisk +
            0.10 * OppositionRisk +
            0.06 * AbstainRisk +
            0.14 * LifeRisk +
            0.12 * ActionRisk +
            0.09 * PromiseRisk +
            0.08 * (ProximityPressure * PromiseRisk) +
            0.22 * (ProximityPressure * Clamp<double>(EventPressure, 0.0, 1.0));

        return Clamp<double>(Score, 0.0, 1.0);
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
