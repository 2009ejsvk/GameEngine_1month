#include "BuildingMarkerOrb.h"
#include "PlacementAreaObject.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <algorithm>

namespace
{
    float ResolveTaxEventProductionMultiplier(
        const FTaxPolicyEventStatus* TaxEventStatus)
    {
        if (!TaxEventStatus ||
            !TaxEventStatus->Active ||
            TaxEventStatus->Type == ETaxPolicyEventType::None)
        {
            return 1.f;
        }

        const float Severity = Clamp<float>(
            static_cast<float>(TaxEventStatus->DaysActive + 1) / 6.f,
            0.f,
            1.f);

        switch (TaxEventStatus->Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return 0.74f - 0.30f * Severity;
        case ETaxPolicyEventType::BudgetCrisis:
            return 0.92f - 0.18f * Severity;
        default:
            return 1.f;
        }
    }
}

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
    auto World = mWorld.lock();
    const IMainWorldAccess* MainWorld =
        World ? dynamic_cast<IMainWorldAccess*>(World.get()) : nullptr;
    const FGovernmentEdictModifiers* EdictModifiers =
        MainWorld ? &MainWorld->GetEdictModifiers() : nullptr;
    const FTaxPolicy* TaxPolicy =
        MainWorld ? &MainWorld->GetTaxPolicy() : nullptr;
    const FTaxPolicyEventStatus* TaxEventStatus =
        MainWorld ? &MainWorld->GetTaxPolicyEventStatus() : nullptr;
    const float FoodGainMultiplier =
        EdictModifiers ? EdictModifiers->FoodGainMultiplier : 1.f;
    const float ProductionMultiplier =
        EdictModifiers ? EdictModifiers->ProductionMultiplier : 1.f;
    const float TaxEventProductionMultiplier =
        ResolveTaxEventProductionMultiplier(TaxEventStatus);
    const int FoodConsumptionPerVisit =
        EdictModifiers ?
        (std::max)(1, EdictModifiers->FoodConsumptionPerVisit) :
        1;

    auto ResolveBuildingCap = [&](
        const std::string& BuildingName,
        int (CPlacementAreaObject::*Getter)() const) -> float
    {
        if (!World || BuildingName.empty())
            return 100.f;

        auto Building = World->FindObject<CPlacementAreaObject>(
            BuildingName).lock();

        if (!Building || !Building->GetAlive())
            return 100.f;

        return static_cast<float>((Building.get()->*Getter)());
    };

    const float HomeHousingCap = ResolveBuildingCap(
        mHomeName, &CPlacementAreaObject::GetHousingSatisfactionCap);
    const float WorkJobCap = ResolveBuildingCap(
        mWorkName, &CPlacementAreaObject::GetJobSatisfactionCap);
    const std::string& FoodCapBuildingName = mFoodVisitBuildingName.empty() ?
        mFoodName :
        mFoodVisitBuildingName;
    const float FoodCap = ResolveBuildingCap(
        FoodCapBuildingName, &CPlacementAreaObject::GetFoodSatisfactionCap);
    const float FunCap = ResolveBuildingCap(
        mFunName, &CPlacementAreaObject::GetFunSatisfactionCap);
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

    // FSM 상태별 회복
    switch (mCitizenState)
    {
    case ECitizenState::AtWork:
        RecoverUnderCap(Satisfaction.Job, 10.f, WorkJobCap);
        // 생산/식량 시설만 재고를 생산한다.
        // 운송업자 사무소/항구는 재고를 직접 생산하지 않는다.
        if (World && !mWorkName.empty())
        {
            auto WorkBuilding =
                World->FindObject<CPlacementAreaObject>(mWorkName).lock();
            if (WorkBuilding)
            {
                float ProductionPerSec = 0.f;
                const EResourceType ProducedType =
                    WorkBuilding->GetProducedResourceType();

                if (ProducedType == EResourceType::Food)
                {
                    ProductionPerSec =
                        (WorkBuilding->GetBuildingCategory() ==
                            EBuildingCategory::FoodResource ?
                            40.f :
                            8.f) *
                        ProductionMultiplier *
                        TaxEventProductionMultiplier;
                }
                else if (ProducedType == EResourceType::RawGoods)
                {
                    ProductionPerSec =
                        10.f *
                        ProductionMultiplier *
                        TaxEventProductionMultiplier;
                }
                else if (ProducedType == EResourceType::ManufacturedGoods)
                {
                    ProductionPerSec =
                        6.f *
                        ProductionMultiplier *
                        TaxEventProductionMultiplier;
                }
                else if (ProducedType == EResourceType::LuxuryGoods)
                {
                    ProductionPerSec =
                        4.f *
                        ProductionMultiplier *
                        TaxEventProductionMultiplier;
                }

                WorkBuilding->AddProduction(ProductionPerSec, DeltaTime);
            }
        }
        break;
    case ECitizenState::AtHome:
        RecoverUnderCap(Satisfaction.Housing, 8.f, HomeHousingCap);
        Satisfaction.Health = (std::min)(
            100.f, Satisfaction.Health + 1.f * DeltaTime);
        break;
    case ECitizenState::AtFood:
        if (!FoodStockAvailableThisVisit &&
            World && !mFoodVisitBuildingName.empty())
        {
            auto FoodBuilding =
                World->FindObject<CPlacementAreaObject>(
                    mFoodVisitBuildingName).lock();

            if (FoodBuilding)
                FoodStockAvailableThisVisit =
                    FoodBuilding->TryConsumeResource(
                        EResourceType::Food,
                        FoodConsumptionPerVisit);
        }

        // 재고가 있었을 때만 음식 만족도 회복
        if (FoodStockAvailableThisVisit)
        {
            RecoverUnderCap(
                Satisfaction.Food,
                30.f * FoodGainMultiplier,
                FoodCap);
        }
        Satisfaction.Health = (std::min)(
            100.f, Satisfaction.Health + 3.f * DeltaTime);
        break;
    case ECitizenState::AtFun:
        RecoverUnderCap(Satisfaction.Fun, 26.f, FunCap);
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

        auto ApplyNeedDrift = [&](float& Value, float DeltaPerSecond)
        {
            Value = (std::max)(
                0.f,
                (std::min)(100.f, Value + DeltaPerSecond * DeltaTime));
        };

        ApplyNeedDrift(
            Satisfaction.Food,
            -0.10f * ConsumptionTaxStress +
            0.04f * ConsumptionTaxRelief);
        ApplyNeedDrift(
            Satisfaction.Fun,
            -0.14f * ConsumptionTaxStress +
            0.06f * ConsumptionTaxRelief);
        ApplyNeedDrift(
            Satisfaction.Job,
            -0.13f * IncomeTaxStress +
            0.05f * IncomeTaxRelief);
        ApplyNeedDrift(
            Satisfaction.Housing,
            -0.15f * PropertyTaxStress +
            0.06f * PropertyTaxRelief);
        ApplyNeedDrift(
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

