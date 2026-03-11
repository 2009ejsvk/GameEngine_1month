#include "AlmanacRenderer.h"
#include "AlmanacRendererCalc.h"
#include <cassert>
#include <cmath>

void FAlmanacRenderer::ValidateSnapshotForDebug(
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
#ifndef NDEBUG
    auto IsPercent = [](double Value)
    {
        return std::isfinite(Value) &&
            Value >= -0.1 &&
            Value <= 100.1;
    };

    assert(Snapshot.ActiveCitizenCount >= 0);
    assert(Snapshot.HomelessCount >= 0);
    assert(Snapshot.UnemployedCount >= 0);
    assert(Snapshot.ActiveEdictCount >= 0);
    assert(Snapshot.TotalBuildingCount >= 0);
    assert(Snapshot.ResidentialCapacity >= 0);
    assert(Snapshot.JobCapacity >= 0);
    assert(IsPercent(Snapshot.SupportPercent));
    assert(IsPercent(Snapshot.OppositionPercent));
    assert(IsPercent(Snapshot.AbstainPercent));
    assert(IsPercent(Snapshot.RebelRiskScore));
    assert(IsPercent(Snapshot.AverageFood));
    assert(IsPercent(Snapshot.AverageHealth));
    assert(IsPercent(Snapshot.AverageFun));
    assert(IsPercent(Snapshot.AverageFaith));
    assert(IsPercent(Snapshot.AverageHousing));
    assert(IsPercent(Snapshot.AverageJob));
    assert(IsPercent(Snapshot.AverageFreedom));
    assert(IsPercent(Snapshot.AverageSecurity));
    assert(IsPercent(Snapshot.AverageOverall));

    for (const auto& Resource : Snapshot.ResourceTypes)
    {
        assert(Resource.TotalStock >= 0);
        assert(Resource.AvailableStock >= 0);
        assert(Resource.ReservedIncoming >= 0);
        assert(Resource.Capacity >= 0);
        assert(Resource.ProducerBuildingCount >= 0);
        assert(Resource.ConsumerBuildingCount >= 0);
        assert(Resource.StorageBuildingCount >= 0);
        assert(Resource.HarborBuildingCount >= 0);
    }

    for (const auto& Category : Snapshot.BuildingCategories)
    {
        assert(Category.Count >= 0);
    }
#else
    (void)Snapshot;
#endif
}

void FAlmanacRenderer::ApplySnapshot(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    ValidateSnapshotForDebug(Snapshot);
    ApplyOverviewPage(Widget, Snapshot);
    ApplySatisfactionPage(Widget, Snapshot);
    ApplyPopulationPage(Widget, Snapshot);
    ApplyEconomyPage(Widget, Snapshot);
    ApplyResourcePage(Widget, Snapshot);
    ApplyPoliticsPage(Widget, Snapshot);
    ApplyForeignPage(Widget, Snapshot);
    ApplyBuildingPage(Widget, Snapshot);
    ApplyConflictPage(
        Widget,
        Snapshot,
        AlmanacRendererCalc::BuildConflictPageComputedData(Snapshot));
}
