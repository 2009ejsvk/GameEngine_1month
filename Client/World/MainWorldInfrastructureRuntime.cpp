#include "MainWorldInfrastructureRuntime.h"
#include "World/World.h"
#include "../Map/PlacementAreaObject.h"
#include "../Building/BuildingCatalog.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
    constexpr float GBuildingPollutionRadiusTiles = 18.f;
    constexpr float GBuildingFreedomRadiusTiles = 18.f;
    constexpr float GBuildingSecurityRadiusTiles = 16.f;
    constexpr float GSecurityPollutionPenaltyWeight = 0.25f;
    constexpr int GPowerPriorityBandCount = 3;

    enum class EPowerPriorityBand
    {
        Industry = 0,
        Service,
        Housing
    };

    bool ContainsText(const std::wstring& Text, const wchar_t* Pattern)
    {
        return Pattern && Text.find(Pattern) != std::wstring::npos;
    }

    int ResolvePowerPriorityBandIndex(EBuildingCategory Category)
    {
        switch (Category)
        {
        case EBuildingCategory::FoodResource:
        case EBuildingCategory::Industry:
            return static_cast<int>(EPowerPriorityBand::Industry);
        case EBuildingCategory::Housing:
            return static_cast<int>(EPowerPriorityBand::Housing);
        case EBuildingCategory::Infrastructure:
        case EBuildingCategory::Entertainment:
        case EBuildingCategory::MediaEducation:
        case EBuildingCategory::Tourism:
        case EBuildingCategory::PublicService:
        default:
            return static_cast<int>(EPowerPriorityBand::Service);
        }
    }

    float ResolveOverlayOperationalScale(const CPlacementAreaObject& Building)
    {
        const float BudgetScale = Building.GetBudgetSatisfactionScale();
        const float PowerScale = (std::max)(
            0.35f,
            (std::min)(1.f, Building.GetPowerSupplyRatio()));
        return (std::max)(
            0.35f,
            (std::min)(1.15f, BudgetScale * (0.60f + 0.40f * PowerScale)));
    }

    float ResolveFreedomInfluenceScore(const FBuildingCatalogEntry& Entry)
    {
        float Score = 0.f;

        for (size_t Index = 0; Index < Entry.PoliticalSignals.size(); ++Index)
        {
            const FPoliticalSignalDef& Signal = Entry.PoliticalSignals[Index];

            if (Signal.Axis != EPoliticalAxis::IntellectualConservative)
                continue;

            const float Direction =
                Signal.FavoredStance == EPoliticalStance::Left ? 1.f :
                Signal.FavoredStance == EPoliticalStance::Right ? -1.f :
                0.f;

            if (Direction == 0.f)
                continue;

            const float ScopeWeight =
                Signal.Scope == EPoliticalScope::Resident ? 1.00f :
                Signal.Scope == EPoliticalScope::Visitor ? 0.85f :
                Signal.Scope == EPoliticalScope::Worker ? 0.72f :
                0.58f;
            Score += Direction * Signal.Strength * ScopeWeight;
        }

        if (ContainsText(Entry.DetailText, L"자유 상승") ||
            ContainsText(Entry.DetailText, L"자유 증가"))
        {
            Score += 4.f;
        }

        if (ContainsText(Entry.DetailText, L"자유 하락") ||
            ContainsText(Entry.DetailText, L"자유 감소"))
        {
            Score -= 4.f;
        }

        return Score;
    }

    float ResolveSecurityInfluenceScore(const FBuildingCatalogEntry& Entry)
    {
        float Score = 0.f;

        for (size_t Index = 0; Index < Entry.PoliticalSignals.size(); ++Index)
        {
            const FPoliticalSignalDef& Signal = Entry.PoliticalSignals[Index];

            if (Signal.Axis != EPoliticalAxis::ReligionMilitarism)
                continue;

            const float Direction =
                Signal.FavoredStance == EPoliticalStance::Right ? 1.f :
                Signal.FavoredStance == EPoliticalStance::Left ? 0.35f :
                0.f;
            const float ScopeWeight =
                Signal.Scope == EPoliticalScope::Resident ? 1.00f :
                Signal.Scope == EPoliticalScope::Worker ? 0.82f :
                Signal.Scope == EPoliticalScope::Visitor ? 0.72f :
                0.55f;
            Score += Direction * Signal.Strength * ScopeWeight;
        }

        if (Entry.Category == EBuildingCategory::Military)
            Score += 8.f;

        if (ContainsText(Entry.DisplayName, L"경찰") ||
            ContainsText(Entry.DisplayName, L"감시") ||
            ContainsText(Entry.DisplayName, L"소방") ||
            ContainsText(Entry.DisplayName, L"교도소"))
        {
            Score += 6.f;
        }

        if (ContainsText(Entry.DetailText, L"치안") ||
            ContainsText(Entry.DetailText, L"범죄"))
        {
            Score += 5.f;
        }

        if (ContainsText(Entry.DetailText, L"범죄 증가"))
            Score -= 4.f;

        return Score;
    }
}

namespace MainWorldInfrastructureRuntime
{
    void RefreshPowerGridCoverage(CWorld* World)
    {
        if (!World)
            return;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return;

        int TotalProducedPowerMW = 0;

        struct FPowerConsumerNode
        {
            std::shared_ptr<CPlacementAreaObject> Building;
            int RequiredPowerMW = 0;
            int PriorityBandIndex = 0;
        };

        std::vector<FPowerConsumerNode> PowerConsumers;
        PowerConsumers.reserve(BuildingList.size());
        std::array<int, GPowerPriorityBandCount> RequiredPowerByBand = {};

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea())
            {
                continue;
            }

            TotalProducedPowerMW +=
                (std::max)(0, Building->GetProducedPowerMW());
            const int RequiredPowerMW = Building->GetRequiredPowerMW();

            if (RequiredPowerMW <= 0)
            {
                Building->SetPowerSupplyRatio(1.f);
                continue;
            }

            FPowerConsumerNode Node;
            Node.Building = std::move(Building);
            Node.RequiredPowerMW = RequiredPowerMW;
            Node.PriorityBandIndex =
                ResolvePowerPriorityBandIndex(
                    Node.Building->GetBuildingCategory());
            PowerConsumers.push_back(std::move(Node));
            RequiredPowerByBand[static_cast<size_t>(
                PowerConsumers.back().PriorityBandIndex)] += RequiredPowerMW;
        }

        int RemainingProducedPowerMW = (std::max)(0, TotalProducedPowerMW);
        std::array<float, GPowerPriorityBandCount> CoverageByBand =
        {
            1.f,
            1.f,
            1.f
        };

        for (int BandIndex = 0;
            BandIndex < GPowerPriorityBandCount;
            ++BandIndex)
        {
            const int BandDemandMW =
                RequiredPowerByBand[static_cast<size_t>(BandIndex)];

            if (BandDemandMW <= 0)
                continue;

            CoverageByBand[static_cast<size_t>(BandIndex)] =
                Clamp<float>(
                    static_cast<float>(RemainingProducedPowerMW) /
                        static_cast<float>(BandDemandMW),
                    0.f,
                    1.f);
            RemainingProducedPowerMW = (std::max)(
                0,
                RemainingProducedPowerMW -
                    (std::min)(RemainingProducedPowerMW, BandDemandMW));
        }

        for (size_t Index = 0; Index < PowerConsumers.size(); ++Index)
        {
            const FPowerConsumerNode& Node = PowerConsumers[Index];
            Node.Building->SetPowerSupplyRatio(
                CoverageByBand[static_cast<size_t>(Node.PriorityBandIndex)]);
        }
    }

    void RefreshBuildingPollutionExposure(CWorld* World)
    {
        if (!World)
            return;

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (!World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
            return;

        struct FPollutionNode
        {
            std::shared_ptr<CPlacementAreaObject> Building;
            int GridX = 0;
            int GridY = 0;
            float PollutionInfluence = 0.f;
            float FreedomInfluence = 0.f;
            float SecurityInfluence = 0.f;
        };

        std::vector<FPollutionNode> PollutionNodes;
        PollutionNodes.reserve(BuildingList.size());

        for (size_t Index = 0; Index < BuildingList.size(); ++Index)
        {
            auto Building = BuildingList[Index].lock();

            if (!Building ||
                !Building->GetAlive() ||
                !Building->GetEnable() ||
                !Building->HasPlacedArea())
            {
                continue;
            }

            int GridX = 0;
            int GridY = 0;

            if (!Building->GetPlacedCenterGridCoords(GridX, GridY))
                continue;

            FPollutionNode Node;
            Node.Building = Building;
            Node.GridX = GridX;
            Node.GridY = GridY;
            const float OperationalScale =
                ResolveOverlayOperationalScale(*Building);
            Node.PollutionInfluence =
                static_cast<float>(
                    Building->GetPollutionOutput() -
                    Building->GetPollutionMitigation()) *
                OperationalScale;

            const FBuildingCatalogEntry* const Entry =
                FindBuildingCatalogEntry(Building->GetBuildingId());
            if (Entry)
            {
                Node.FreedomInfluence =
                    ResolveFreedomInfluenceScore(*Entry) * OperationalScale;
                Node.SecurityInfluence =
                    ResolveSecurityInfluenceScore(*Entry) * OperationalScale;
            }
            PollutionNodes.push_back(std::move(Node));
        }

        const float PollutionRadiusSq =
            GBuildingPollutionRadiusTiles * GBuildingPollutionRadiusTiles;
        const float FreedomRadiusSq =
            GBuildingFreedomRadiusTiles * GBuildingFreedomRadiusTiles;
        const float SecurityRadiusSq =
            GBuildingSecurityRadiusTiles * GBuildingSecurityRadiusTiles;

        auto ComputeWeight = [](
            const FPollutionNode& TargetNode,
            const FPollutionNode& SourceNode,
            float RadiusTiles,
            float RadiusSq) -> float
        {
            if (RadiusTiles <= 0.f)
                return 0.f;

            const float dx =
                static_cast<float>(TargetNode.GridX - SourceNode.GridX);
            const float dy =
                static_cast<float>(TargetNode.GridY - SourceNode.GridY);
            const float DistanceSq = dx * dx + dy * dy;

            if (DistanceSq > RadiusSq)
                return 0.f;

            const float Distance = DistanceSq > 0.f ? std::sqrt(DistanceSq) : 0.f;
            return (std::max)(0.f, 1.f - Distance / RadiusTiles);
        };

        for (size_t TargetIndex = 0;
            TargetIndex < PollutionNodes.size();
            ++TargetIndex)
        {
            FPollutionNode& TargetNode = PollutionNodes[TargetIndex];
            float TotalExposure = 0.f;
            float TotalFreedom = 0.f;
            float TotalSecurity = 0.f;

            for (size_t SourceIndex = 0;
                SourceIndex < PollutionNodes.size();
                ++SourceIndex)
            {
                const FPollutionNode& SourceNode = PollutionNodes[SourceIndex];
                const float PollutionWeight = ComputeWeight(
                    TargetNode,
                    SourceNode,
                    GBuildingPollutionRadiusTiles,
                    PollutionRadiusSq);
                if (PollutionWeight > 0.f && SourceNode.PollutionInfluence != 0.f)
                {
                    TotalExposure +=
                        SourceNode.PollutionInfluence * PollutionWeight;
                }

                const float FreedomWeight = ComputeWeight(
                    TargetNode,
                    SourceNode,
                    GBuildingFreedomRadiusTiles,
                    FreedomRadiusSq);
                if (FreedomWeight > 0.f && SourceNode.FreedomInfluence != 0.f)
                {
                    TotalFreedom += SourceNode.FreedomInfluence * FreedomWeight;
                }

                const float SecurityWeight = ComputeWeight(
                    TargetNode,
                    SourceNode,
                    GBuildingSecurityRadiusTiles,
                    SecurityRadiusSq);
                if (SecurityWeight > 0.f &&
                    (SourceNode.SecurityInfluence != 0.f ||
                        SourceNode.PollutionInfluence > 0.f))
                {
                    TotalSecurity +=
                        (SourceNode.SecurityInfluence -
                            (std::max)(0.f, SourceNode.PollutionInfluence) *
                                GSecurityPollutionPenaltyWeight) *
                        SecurityWeight;
                }
            }

            TargetNode.Building->SetLocalPollutionExposure(
                (std::max)(
                    0,
                    (std::min)(100, static_cast<int>(roundf(TotalExposure)))));
            TargetNode.Building->SetLocalFreedomSupport(
                (std::max)(
                    -100,
                    (std::min)(
                        100,
                        static_cast<int>(roundf(TotalFreedom * 2.2f)))));
            TargetNode.Building->SetLocalSecuritySupport(
                (std::max)(
                    -100,
                    (std::min)(
                        100,
                        static_cast<int>(roundf(TotalSecurity * 2.0f)))));
        }
    }
}
