#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "../Citizen/CitizenCommuteCalc.h"
#include "../World/MainWorldInfrastructureAccess.h"
#include "../World/MainWorldSystemAccess.h"
#include "World/World.h"
#include <algorithm>

void CBuildingMarkerOrb::InitPoliticalProfile()
{
    mCitizenProfileState.InitPoliticalProfile();
}

void CBuildingMarkerOrb::UpdatePoliticalProfile(float DeltaTime)
{
    mCitizenProfileState.UpdatePoliticalProfile(DeltaTime);
}

void CBuildingMarkerOrb::ApplySatisfactionDelta(
    float FoodDelta,
    float HealthDelta,
    float FunDelta,
    float FaithDelta,
    float HousingDelta,
    float JobDelta,
    float FreedomDelta,
    float SecurityDelta)
{
    mCitizenProfileState.ApplySatisfactionDelta(
        FoodDelta,
        HealthDelta,
        FunDelta,
        FaithDelta,
        HousingDelta,
        JobDelta,
        FreedomDelta,
        SecurityDelta);
}

void CBuildingMarkerOrb::UpdateSatisfaction(float DeltaTime)
{
    auto& Satisfaction = mCitizenProfileState.Satisfaction;
    auto& FoodStockAvailableThisVisit =
        mCitizenProfileState.FoodStockAvailableThisVisit;
    auto& FunServiceAvailableThisVisit =
        mCitizenProfileState.FunServiceAvailableThisVisit;
    auto& HealthServiceAvailableThisVisit =
        mCitizenProfileState.HealthServiceAvailableThisVisit;
    auto& FaithServiceAvailableThisVisit =
        mCitizenProfileState.FaithServiceAvailableThisVisit;
    auto World = mWorld.lock();
    const IMainWorldCitizenPolicyAccess* MainWorld =
        ResolveMainWorldCitizenPolicyAccess(World.get());
    const IMainWorldRoadNetworkAccess* RoadNetworkAccess =
        ResolveMainWorldRoadNetworkAccess(World.get());
    const IMainWorldTransitAccess* TransitAccess =
        ResolveMainWorldTransitAccess(World.get());
    const FGovernmentEdictModifiers* EdictModifiers =
        MainWorld ? &MainWorld->GetEdictModifiers() : nullptr;
    const FTaxPolicy* TaxPolicy =
        MainWorld ? &MainWorld->GetTaxPolicy() : nullptr;
    const float FoodGainMultiplier =
        EdictModifiers ? EdictModifiers->FoodGainMultiplier : 1.f;
    const int FoodConsumptionPerVisit =
        EdictModifiers ?
        (std::max)(1, EdictModifiers->FoodConsumptionPerVisit) :
        1;

    auto ResolveBuilding = [&](const std::string& BuildingName)
        -> std::shared_ptr<CPlacementAreaObject>
    {
        if (!World || BuildingName.empty())
            return nullptr;

        auto Building = World->FindObject<CPlacementAreaObject>(
            BuildingName).lock();

        if (!Building || !Building->GetAlive())
            return nullptr;

        return Building;
    };

    auto ResolveBuildingCap = [&](
        const std::shared_ptr<CPlacementAreaObject>& Building,
        int (CPlacementAreaObject::*Getter)() const) -> float
    {
        if (!Building)
            return 100.f;

        return static_cast<float>((Building.get()->*Getter)());
    };

    auto ResolveServiceRecoveryMultiplier = [&](float SatisfactionCap) -> float
    {
        return Clamp<float>(
            0.75f + Clamp<float>(SatisfactionCap / 100.f, 0.f, 1.f) * 0.50f,
            0.55f,
            1.35f);
    };

    const auto HomeBuilding = ResolveBuilding(mHomeName);
    const auto WorkBuilding = ResolveBuilding(mWorkName);
    const float HomeHousingCap = ResolveBuildingCap(
        HomeBuilding, &CPlacementAreaObject::GetHousingSatisfactionCap);
    const std::string& FoodCapBuildingName = mFoodVisitBuildingName.empty() ?
        mFoodName :
        mFoodVisitBuildingName;
    const auto FoodCapBuilding = ResolveBuilding(FoodCapBuildingName);
    const float FoodCap = ResolveBuildingCap(
        FoodCapBuilding, &CPlacementAreaObject::GetFoodSatisfactionCap);
    const std::string& FunCapBuildingName = mFunVisitBuildingName.empty() ?
        mFunName :
        mFunVisitBuildingName;
    const auto FunCapBuilding = ResolveBuilding(FunCapBuildingName);
    const float FunCap = ResolveBuildingCap(
        FunCapBuilding, &CPlacementAreaObject::GetFunSatisfactionCap);
    const std::string& HealthCapBuildingName =
        mHealthVisitBuildingName.empty() ?
            mHealthName :
            mHealthVisitBuildingName;
    const auto HealthCapBuilding = ResolveBuilding(HealthCapBuildingName);
    const float HealthCap = ResolveBuildingCap(
        HealthCapBuilding, &CPlacementAreaObject::GetHealthSatisfactionCap);
    const std::string& FaithCapBuildingName =
        mFaithVisitBuildingName.empty() ?
            mFaithName :
            mFaithVisitBuildingName;
    const auto FaithCapBuilding = ResolveBuilding(FaithCapBuildingName);
    const float FaithCap = ResolveBuildingCap(
        FaithCapBuilding, &CPlacementAreaObject::GetFaithSatisfactionCap);
    const float WorkJobCap = ResolveBuildingCap(
        WorkBuilding, &CPlacementAreaObject::GetEffectiveJobSatisfactionCap);
    const float HomePollution =
        HomeBuilding ?
            HomeBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    const float WorkPollution =
        WorkBuilding ?
            WorkBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    const float FoodPollution =
        FoodCapBuilding ?
            FoodCapBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    const float FunPollution =
        FunCapBuilding ?
            FunCapBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    const float HealthPollution =
        HealthCapBuilding ?
            HealthCapBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    const float FaithPollution =
        FaithCapBuilding ?
            FaithCapBuilding->GetLocalPollutionExposureNormalized() :
            0.f;
    auto ResolveLocalFreedomBias =
        [&](const std::shared_ptr<CPlacementAreaObject>& Building) -> float
    {
        return Building ?
            (Building->GetLocalFreedomSupportNormalized() - 0.5f) * 2.f :
            0.f;
    };
    auto ResolveLocalSecurityBias =
        [&](const std::shared_ptr<CPlacementAreaObject>& Building) -> float
    {
        return Building ?
            (Building->GetLocalSecuritySupportNormalized() - 0.5f) * 2.f :
            0.f;
    };
    const float HomeFreedomBias = ResolveLocalFreedomBias(HomeBuilding);
    const float WorkFreedomBias = ResolveLocalFreedomBias(WorkBuilding);
    const float FoodFreedomBias = ResolveLocalFreedomBias(FoodCapBuilding);
    const float FunFreedomBias = ResolveLocalFreedomBias(FunCapBuilding);
    const float HealthFreedomBias = ResolveLocalFreedomBias(HealthCapBuilding);
    const float FaithFreedomBias = ResolveLocalFreedomBias(FaithCapBuilding);
    const float HomeSecurityBias = ResolveLocalSecurityBias(HomeBuilding);
    const float WorkSecurityBias = ResolveLocalSecurityBias(WorkBuilding);
    const float FoodSecurityBias = ResolveLocalSecurityBias(FoodCapBuilding);
    const float FunSecurityBias = ResolveLocalSecurityBias(FunCapBuilding);
    const float HealthSecurityBias =
        ResolveLocalSecurityBias(HealthCapBuilding);
    const float FaithSecurityBias =
        ResolveLocalSecurityBias(FaithCapBuilding);
    const float FoodRecoveryMultiplier =
        ResolveServiceRecoveryMultiplier(FoodCap);
    const float FunRecoveryMultiplier =
        ResolveServiceRecoveryMultiplier(FunCap);
    const float HealthRecoveryMultiplier =
        ResolveServiceRecoveryMultiplier(HealthCap);
    const float FaithRecoveryMultiplier =
        ResolveServiceRecoveryMultiplier(FaithCap);
    const FCommuteProfile CommuteProfile =
        CitizenCommuteCalc::ResolveCommuteProfile(
            HomeBuilding,
            WorkBuilding,
            mCitizenProfileState.IdentityProfile,
            RoadNetworkAccess ? RoadNetworkAccess->GetRoadNetwork() : nullptr,
            TransitAccess ? TransitAccess->GetBusRouteSystem() : nullptr);
    const float CommuteTimeSeconds = CommuteProfile.TravelSeconds;
    Satisfaction.CommuteTimePenalty =
        CitizenCommuteCalc::EstimateCommutePenalty(CommuteTimeSeconds);
    const float CommutePenaltyNormalized =
        Clamp<float>(Satisfaction.CommuteTimePenalty / 100.f, 0.f, 1.f);
    const float EffectiveWorkJobCap = (std::max)(
        0.f,
        WorkJobCap * CommuteProfile.JobRecoveryMultiplier);
    const float ConsumptionTaxDeviation =
        TaxPolicy ?
        GetTaxPolicyDeviationNormalized(
            *TaxPolicy,
            ETaxPolicyType::Consumption) :
        0.f;
    const float IncomeTaxDeviation =
        TaxPolicy ?
        GetTaxPolicyDeviationNormalized(
            *TaxPolicy,
            ETaxPolicyType::Income) :
        0.f;
    const float PropertyTaxDeviation =
        TaxPolicy ?
        GetTaxPolicyDeviationNormalized(
            *TaxPolicy,
            ETaxPolicyType::Property) :
        0.f;

    auto RecoverUnderCap = [&](float& Value, float GainPerSec, float Cap)
    {
        if (Value >= Cap)
            return;

        Value = (std::min)(Cap, Value + GainPerSec * DeltaTime);
    };
    auto ApplyClampedNeedDrift = [&](float& Value, float DeltaPerSecond)
    {
        Value = (std::max)(
            0.f,
            (std::min)(100.f, Value + DeltaPerSecond * DeltaTime));
    };

    // 욕구 자연 감소
    Satisfaction.Food = (std::max)(
        0.f, Satisfaction.Food - 1.2f * DeltaTime);
    Satisfaction.Job = (std::max)(
        0.f, Satisfaction.Job - 1.0f * DeltaTime);
    Satisfaction.Housing = (std::max)(
        0.f, Satisfaction.Housing - 0.5f * DeltaTime);
    Satisfaction.Fun = (std::max)(
        0.f, Satisfaction.Fun - 0.9f * DeltaTime);
    Satisfaction.Health = (std::max)(
        0.f, Satisfaction.Health - 0.2f * DeltaTime);
    Satisfaction.Faith = (std::max)(
        0.f, Satisfaction.Faith - 0.15f * DeltaTime);
    Satisfaction.Job = (std::max)(
        0.f,
        Satisfaction.Job - 0.75f * CommutePenaltyNormalized * DeltaTime);
    Satisfaction.Fun = (std::max)(
        0.f,
        Satisfaction.Fun - 0.30f * CommutePenaltyNormalized * DeltaTime);

    if (CommuteProfile.Mode == ECommuteMode::Transit)
    {
        Satisfaction.Freedom = (std::max)(
            0.f,
            Satisfaction.Freedom -
                0.18f *
                (CommutePenaltyNormalized + CommuteProfile.CrowdingPenalty) *
                DeltaTime);
    }
    else if (CommuteProfile.Mode == ECommuteMode::Vehicle)
    {
        Satisfaction.Freedom = (std::min)(
            100.f,
            Satisfaction.Freedom +
                0.08f * CommuteProfile.ServiceQuality * DeltaTime);
    }
    else if (CommuteProfile.Mode == ECommuteMode::Walk &&
        CommuteTimeSeconds > 0.f &&
        CommutePenaltyNormalized < 0.15f)
    {
        Satisfaction.Health = (std::min)(
            100.f,
            Satisfaction.Health + 0.10f * DeltaTime);
    }

    const float LocalFreedomDrift =
        HomeFreedomBias * 0.60f +
        WorkFreedomBias * 0.18f +
        FoodFreedomBias * 0.06f +
        FunFreedomBias * 0.12f +
        HealthFreedomBias * 0.02f +
        FaithFreedomBias * 0.02f;
    ApplyClampedNeedDrift(Satisfaction.Freedom, LocalFreedomDrift * 0.32f);

    const float LocalSecurityDrift =
        HomeSecurityBias * 0.58f +
        WorkSecurityBias * 0.22f +
        FoodSecurityBias * 0.05f +
        FunSecurityBias * 0.03f +
        HealthSecurityBias * 0.05f +
        FaithSecurityBias * 0.07f;
    ApplyClampedNeedDrift(
        Satisfaction.Security,
        LocalSecurityDrift * 0.34f -
            HomePollution * 0.05f -
            WorkPollution * 0.03f);
    ApplyClampedNeedDrift(
        Satisfaction.Housing,
        HomeSecurityBias * 0.08f - HomePollution * 0.10f);
    ApplyClampedNeedDrift(
        Satisfaction.Job,
        WorkSecurityBias * 0.06f - WorkPollution * 0.05f);

    // FSM 상태별 회복
    switch (mCitizenState)
    {
    case ECitizenState::AtWork:
        Satisfaction.Health = (std::max)(
            0.f,
            Satisfaction.Health - WorkPollution * 0.45f * DeltaTime);
        RecoverUnderCap(
            Satisfaction.Job,
            10.f * CommuteProfile.JobRecoveryMultiplier,
            EffectiveWorkJobCap);
        break;
    case ECitizenState::AtHome:
        RecoverUnderCap(Satisfaction.Housing, 8.f, HomeHousingCap);
        Satisfaction.Health = (std::min)(
            100.f,
            Satisfaction.Health +
                (1.f - HomePollution * 0.30f) * DeltaTime);
        break;
    case ECitizenState::AtFood:
        if (mFoodVisitReserved &&
            !FoodStockAvailableThisVisit &&
            World && !mFoodVisitBuildingName.empty())
        {
            auto FoodBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mFoodVisitBuildingName).lock();

            if (FoodBuilding)
            {
                FoodStockAvailableThisVisit =
                    FoodBuilding->TryConsumeVisitConsumptionResource(
                        FoodConsumptionPerVisit);
            }
        }

        // 재고가 있었을 때만 음식 만족도 회복
        if (mFoodVisitReserved && FoodStockAvailableThisVisit)
        {
            RecoverUnderCap(
                Satisfaction.Food,
                30.f * FoodGainMultiplier * FoodRecoveryMultiplier,
                FoodCap);
        }
        Satisfaction.Health = (std::min)(
            100.f,
            Satisfaction.Health +
                (3.f * HealthRecoveryMultiplier - FoodPollution * 0.18f) *
                DeltaTime);
        break;
    case ECitizenState::AtFun:
        if (mFunVisitReserved &&
            !FunServiceAvailableThisVisit &&
            World && !mFunVisitBuildingName.empty())
        {
            auto FunVisitBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mFunVisitBuildingName).lock();

            if (FunVisitBuilding)
            {
                FunServiceAvailableThisVisit =
                    FunVisitBuilding->TryConsumeServiceStock(
                        EBuildingServiceType::Fun,
                        1);
            }
        }

        if (mFunVisitReserved && FunServiceAvailableThisVisit)
        {
            RecoverUnderCap(
                Satisfaction.Fun,
                26.f * FunRecoveryMultiplier,
                FunCap);
        }

        Satisfaction.Health = (std::max)(
            0.f,
            Satisfaction.Health - FunPollution * 0.08f * DeltaTime);
        break;
    case ECitizenState::AtHealth:
        if (mHealthVisitReserved &&
            !HealthServiceAvailableThisVisit &&
            World && !mHealthVisitBuildingName.empty())
        {
            auto HealthVisitBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mHealthVisitBuildingName).lock();

            if (HealthVisitBuilding)
            {
                HealthServiceAvailableThisVisit =
                    HealthVisitBuilding->TryConsumeServiceStock(
                        EBuildingServiceType::Health,
                        1);
            }
        }

        if (mHealthVisitReserved && HealthServiceAvailableThisVisit)
        {
            RecoverUnderCap(
                Satisfaction.Health,
                24.f * HealthRecoveryMultiplier,
                HealthCap);
        }

        if (HealthPollution > 0.f)
        {
            Satisfaction.Health = (std::max)(
                0.f,
                Satisfaction.Health - HealthPollution * 0.05f * DeltaTime);
        }
        break;
    case ECitizenState::AtFaith:
        if (mFaithVisitReserved &&
            !FaithServiceAvailableThisVisit &&
            World && !mFaithVisitBuildingName.empty())
        {
            auto FaithVisitBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mFaithVisitBuildingName).lock();

            if (FaithVisitBuilding)
            {
                FaithServiceAvailableThisVisit =
                    FaithVisitBuilding->TryConsumeServiceStock(
                        EBuildingServiceType::Faith,
                        1);
            }
        }

        if (mFaithVisitReserved && FaithServiceAvailableThisVisit)
        {
            RecoverUnderCap(
                Satisfaction.Faith,
                22.f * FaithRecoveryMultiplier,
                FaithCap);
        }

        Satisfaction.Health = (std::max)(
            0.f,
            Satisfaction.Health - FaithPollution * 0.05f * DeltaTime);
        break;
    default:
        break;
    }

    if (TaxPolicy)
    {
        const float ConsumptionTaxStress =
            (std::max)(0.f, ConsumptionTaxDeviation);
        const float ConsumptionTaxRelief =
            (std::max)(0.f, -ConsumptionTaxDeviation);
        const float IncomeTaxStress =
            (std::max)(0.f, IncomeTaxDeviation);
        const float IncomeTaxRelief =
            (std::max)(0.f, -IncomeTaxDeviation);
        const float PropertyTaxStress =
            (std::max)(0.f, PropertyTaxDeviation);
        const float PropertyTaxRelief =
            (std::max)(0.f, -PropertyTaxDeviation);
        ApplyClampedNeedDrift(
            Satisfaction.Food,
            -0.10f * ConsumptionTaxStress +
            0.04f * ConsumptionTaxRelief);
        ApplyClampedNeedDrift(
            Satisfaction.Fun,
            -0.14f * ConsumptionTaxStress +
            0.06f * ConsumptionTaxRelief);
        ApplyClampedNeedDrift(
            Satisfaction.Job,
            -0.13f * IncomeTaxStress +
            0.05f * IncomeTaxRelief);
        ApplyClampedNeedDrift(
            Satisfaction.Housing,
            -0.15f * PropertyTaxStress +
            0.06f * PropertyTaxRelief);
        ApplyClampedNeedDrift(
            Satisfaction.Freedom,
            -(0.06f * ConsumptionTaxStress +
                0.08f * IncomeTaxStress +
                0.07f * PropertyTaxStress) +
            (0.03f * ConsumptionTaxRelief +
                0.04f * IncomeTaxRelief +
                0.03f * PropertyTaxRelief));
    }

    // 1초 틱으로 Overall 재계산 (매 프레임 불필요)
    mCitizenProfileState.TickOverallRecalculation(DeltaTime);
}

void CBuildingMarkerOrb::RecalculateOverallSatisfaction()
{
    mCitizenProfileState.RecalculateOverallSatisfaction();
}

