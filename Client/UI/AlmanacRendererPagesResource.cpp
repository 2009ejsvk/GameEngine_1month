#include "AlmanacRendererPagesCoreShared.h"

void FAlmanacRenderer::ApplyResourcePage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    struct FResourceUiEntry
    {
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot* Resource =
            nullptr;
        std::wstring Name;
    };
    std::vector<FResourceUiEntry> ResourceEntries;
    ResourceEntries.reserve(static_cast<size_t>(EResourceType::Count));

    for (int ResourceIndex = 1;
        ResourceIndex < static_cast<int>(EResourceType::Count);
        ++ResourceIndex)
    {
        const EResourceType ResourceType =
            static_cast<EResourceType>(ResourceIndex);

        if (!IsExportableResourceType(ResourceType))
            continue;

        FResourceUiEntry Entry;
        Entry.Resource =
            &Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];
        Entry.Name = GetResourceTypeDisplayName(ResourceType);
        ResourceEntries.push_back(std::move(Entry));
    }

    const int ResourceMaxIndex =
        (std::max)(0, static_cast<int>(ResourceEntries.size()) - 1);
    const int SelectedResourceIndex =
        (std::max)(0, (std::min)(Widget.mSelectedResourceIndex, ResourceMaxIndex));
    Widget.mSelectedResourceIndex = SelectedResourceIndex;
    const int VisibleRowCount =
        static_cast<int>(Widget.mResourceRows.size());
    const int ResourceWindowMaxStart = (std::max)(
        0,
        static_cast<int>(ResourceEntries.size()) - VisibleRowCount);
    const int VisibleStartIndex = (std::max)(
        0,
        (std::min)(
            SelectedResourceIndex - VisibleRowCount / 2,
            ResourceWindowMaxStart));
    Widget.mVisibleResourceRowOffset = VisibleStartIndex;
    const FResourceUiEntry& SelectedResourceEntry =
        ResourceEntries[static_cast<size_t>(SelectedResourceIndex)];
    const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& SelectedResource =
        *SelectedResourceEntry.Resource;
    const int ExportUnitPrice =
        ResourceTradePricing::GetExportPricePerStockUnit(
            SelectedResource.Type);
    const int ImportUnitPrice =
        ResourceTradePricing::GetImportPricePerStockUnit(
            SelectedResource.Type);
    const int StorageBiasPercent =
        ResourceTradePricing::GetStorageBiasPercent(
            SelectedResource.Type);
    const int BalanceBiasPercent =
        ResourceTradePricing::GetBalanceBiasPercent(
            SelectedResource.Type);
    const int TemporalBiasPercent =
        ResourceTradePricing::GetTemporalBiasPercent(
            SelectedResource.Type);
    const int EventBiasPercent =
        ResourceTradePricing::GetEventBiasPercent(
            SelectedResource.Type);
    const int DiplomacyExportBiasPercent =
        ResourceTradePricing::GetDiplomacyExportBiasPercent(
            SelectedResource.Type);
    const int DiplomacyImportBiasPercent =
        ResourceTradePricing::GetDiplomacyImportBiasPercent(
            SelectedResource.Type);
    const int EdictExportBiasPercent =
        ResourceTradePricing::GetEdictExportBiasPercent(
            SelectedResource.Type);
    const int EdictImportBiasPercent =
        ResourceTradePricing::GetEdictImportBiasPercent(
            SelectedResource.Type);

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceRows.size()); ++Index)
    {
        auto& Row = Widget.mResourceRows[static_cast<size_t>(Index)];
        auto Button = Row.Button.lock();
        auto Label = Row.Label.lock();
        auto Value = Row.Value.lock();
        const int EntryIndex = VisibleStartIndex + Index;

        if (EntryIndex < static_cast<int>(ResourceEntries.size()))
        {
            SetDetailRowData(
                Row,
                ResourceEntries[static_cast<size_t>(EntryIndex)].Name,
                FormatInteger(
                    ResourceEntries[static_cast<size_t>(EntryIndex)].
                        Resource->TotalStock),
                EntryIndex == SelectedResourceIndex);

            if (Button)
            {
                Button->SetEnable(true);
                Button->ButtonEnable(true);
            }
            if (Label)
                Label->SetEnable(true);
            if (Value)
                Value->SetEnable(true);
        }
        else
        {
            SetDetailRowData(Row, L"", L"", false);

            if (Button)
            {
                Button->SetEnable(false);
                Button->ButtonEnable(false);
            }
            if (Label)
            {
                Label->SetText(L"");
                Label->SetEnable(false);
            }
            if (Value)
            {
                Value->SetText(L"");
                Value->SetEnable(false);
            }
        }
    }

    if (auto Title = Widget.mResourceListTitle.lock())
        Title->SetText(L"자원 유형");
    if (auto Text = Widget.mResourceFilterText.lock())
        Text->SetText(L"실시간 집계");
    if (auto Title = Widget.mResourceProductionTitle.lock())
        Title->SetText(L"시장 가격 추세");
    if (auto Title = Widget.mResourceDistributionTitle.lock())
        Title->SetText(L"자원 흐름 단계");
    if (auto Text = Widget.mResourceDistributionFilterText.lock())
        Text->SetText(L"생산지 -> 창고 -> 소비지 -> 항구");
    if (auto Title = Widget.mResourceTrackingTitle.lock())
        Title->SetText(L"흐름 세부");
    if (auto Name = Widget.mResourceTrackingName.lock())
        Name->SetText(SelectedResourceEntry.Name.c_str());
    if (auto Value = Widget.mResourceTrackingValue.lock())
        Value->SetText(FormatInteger(SelectedResource.TotalStock).c_str());
    if (auto Text = Widget.mResourceProductionLegendPrimaryText.lock())
        Text->SetText(L"수출 단가");
    if (auto Text = Widget.mResourceProductionLegendSecondaryText.lock())
        Text->SetText(L"수입 단가");
    if (auto Swatch = Widget.mResourceProductionLegendPrimarySwatch.lock())
        Swatch->SetTint(0.22f, 0.58f, 0.82f, 0.92f);
    if (auto Swatch = Widget.mResourceProductionLegendSecondarySwatch.lock())
        Swatch->SetTint(0.84f, 0.62f, 0.18f, 0.92f);

    constexpr int PriceHistoryGroupCount = GResourceProductionBarCount / 2;
    std::array<int, PriceHistoryGroupCount> ExportHistory = {};
    std::array<int, PriceHistoryGroupCount> ImportHistory = {};
    int GraphMaxValue = 1;

    for (int Index = 0; Index < PriceHistoryGroupCount; ++Index)
    {
        ExportHistory[static_cast<size_t>(Index)] =
            ResourceTradePricing::GetExportPriceHistoryPoint(
                SelectedResource.Type,
                Index);
        ImportHistory[static_cast<size_t>(Index)] =
            ResourceTradePricing::GetImportPriceHistoryPoint(
                SelectedResource.Type,
                Index);
        GraphMaxValue = (std::max)(
            GraphMaxValue,
            (std::max)(
                ExportHistory[static_cast<size_t>(Index)],
                ImportHistory[static_cast<size_t>(Index)]));
    }

    const std::wstring ResourceXAxisLabels[GResourceProductionXAxisLabelCount] =
    {
        L"11일 전",
        L"7일 전",
        L"3일 전",
        L"오늘"
    };

    for (int Index = 0; Index < GResourceProductionXAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionXAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionXAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(true);
            Label->SetText(ResourceXAxisLabels[static_cast<size_t>(Index)].c_str());
        }
    }

    const std::wstring ResourceYAxisLabels[GResourceProductionYAxisLabelCount] =
    {
        FormatInteger(GraphMaxValue),
        FormatInteger(GraphMaxValue / 2),
        L"0"
    };
    for (int Index = 0; Index < GResourceProductionYAxisLabelCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mResourceProductionYAxisLabels.size()))
            break;

        if (auto Label = Widget.mResourceProductionYAxisLabels[static_cast<size_t>(Index)].lock())
        {
            Label->SetEnable(true);
            Label->SetText(ResourceYAxisLabels[Index].c_str());
        }
    }

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceProductionBars.size()); ++Index)
    {
        if (auto Bar = Widget.mResourceProductionBars[static_cast<size_t>(Index)].lock())
            Bar->SetEnable(false);
    }

    if (auto Frame = Widget.mResourceProductionFrame.lock())
    {
        const float GraphLeft = Frame->GetPos().x + 22.f;
        const float GraphTop = Frame->GetPos().y + 14.f;
        const float GraphWidth = Frame->GetSize().x - 40.f;
        const float GraphHeight = Frame->GetSize().y - 32.f;
        const float GroupWidth =
            GraphWidth / static_cast<float>((std::max)(1, PriceHistoryGroupCount));
        const float BarWidth =
            (std::max)(4.f, GroupWidth * 0.26f);
        const float GroupInset =
            (GroupWidth - BarWidth * 2.f) * 0.5f;

        for (int Index = 0; Index < PriceHistoryGroupCount; ++Index)
        {
            const float ExportHeight =
                GraphHeight *
                Clamp01(
                    static_cast<float>(ExportHistory[static_cast<size_t>(Index)]) /
                    static_cast<float>(GraphMaxValue));
            const float ImportHeight =
                GraphHeight *
                Clamp01(
                    static_cast<float>(ImportHistory[static_cast<size_t>(Index)]) /
                    static_cast<float>(GraphMaxValue));
            const float GroupX =
                GraphLeft + GroupWidth * static_cast<float>(Index);
            const int ExportBarIndex = Index * 2;
            const int ImportBarIndex = ExportBarIndex + 1;

            if (ExportBarIndex < static_cast<int>(Widget.mResourceProductionBars.size()))
            {
                if (auto Bar = Widget.mResourceProductionBars[
                        static_cast<size_t>(ExportBarIndex)].lock())
                {
                    if (ExportHeight <= 0.f)
                    {
                        Bar->SetEnable(false);
                    }
                    else
                    {
                        Bar->SetTint(FVector4(0.22f, 0.58f, 0.82f, 0.92f));
                        Bar->SetEnable(true);
                        Bar->SetPos(
                            GroupX + GroupInset,
                            GraphTop + GraphHeight - ExportHeight);
                        Bar->SetSize(BarWidth, (std::max)(2.f, ExportHeight));
                    }
                }
            }

            if (ImportBarIndex < static_cast<int>(Widget.mResourceProductionBars.size()))
            {
                if (auto Bar = Widget.mResourceProductionBars[
                        static_cast<size_t>(ImportBarIndex)].lock())
                {
                    if (ImportHeight <= 0.f)
                    {
                        Bar->SetEnable(false);
                    }
                    else
                    {
                        Bar->SetTint(FVector4(0.84f, 0.62f, 0.18f, 0.92f));
                        Bar->SetEnable(true);
                        Bar->SetPos(
                            GroupX + GroupInset + BarWidth,
                            GraphTop + GraphHeight - ImportHeight);
                        Bar->SetSize(BarWidth, (std::max)(2.f, ImportHeight));
                    }
                }
            }
        }
    }

    const int ProducerFlowValue =
        (std::max)(0, SelectedResource.ProducerAvailableStock);
    const int WarehouseFlowValue =
        (std::max)(0, SelectedResource.WarehouseBufferedStock);
    const int ConsumerFlowValue =
        (std::max)(
            0,
            SelectedResource.ShortagePressure > 0 ?
                SelectedResource.ShortagePressure :
                SelectedResource.ConsumerCoveredStock);
    const int HarborFlowValue =
        (std::max)(0, SelectedResource.HarborExportableStock);
    const int FlowStageMaxValue =
        (std::max)(
            1,
            (std::max)(
                (std::max)(ProducerFlowValue, WarehouseFlowValue),
                (std::max)(ConsumerFlowValue, HarborFlowValue)));

    const struct FResourceDistributionRow
    {
        const wchar_t* Label;
        std::wstring Value;
        float Percent;
        FVector4 Tint;
    } DistributionRows[GResourceDistributionRowCount] =
    {
        {
            L"생산지",
            L"대기 " +
                FormatInteger(SelectedResource.ProducerAvailableStock) +
                L" / 건물 " +
                FormatInteger(SelectedResource.ProducerBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopProducerBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(ProducerFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.22f, 0.58f, 0.82f, 0.95f)
        },
        {
            L"창고",
            L"보관 " +
                FormatInteger(SelectedResource.WarehouseBufferedStock) +
                L" / 창고 " +
                FormatInteger(SelectedResource.WarehouseBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopWarehouseBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(WarehouseFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.42f, 0.66f, 0.32f, 0.95f)
        },
        {
            L"소비지",
            (SelectedResource.ShortagePressure > 0 ?
                (L"부족 " +
                    FormatInteger(SelectedResource.ShortagePressure)) :
                (L"보급 " +
                    FormatInteger(SelectedResource.ConsumerCoveredStock))) +
                L" / 소비처 " +
                FormatInteger(SelectedResource.ConsumerBuildingCount) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.ShortagePressure > 0 ?
                        SelectedResource.TopShortageBuildings :
                        SelectedResource.TopConsumerBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(ConsumerFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            SelectedResource.ShortagePressure > 0 ?
                FVector4(0.82f, 0.38f, 0.28f, 0.95f) :
                FVector4(0.80f, 0.62f, 0.22f, 0.95f)
        },
        {
            L"항구",
            L"수출 가능 " +
                FormatInteger(SelectedResource.HarborExportableStock) +
                L" / 예약 " +
                FormatInteger(SelectedResource.HarborReservedPickup) +
                L" / 대표 " +
                BuildFlowStageHeadline(
                    SelectedResource.TopHarborBuildings,
                    L"-"),
            static_cast<float>(Clamp01(
                static_cast<double>(HarborFlowValue) /
                static_cast<double>(FlowStageMaxValue))),
            FVector4(0.66f, 0.48f, 0.84f, 0.95f)
        }
    };

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceDistributionRows.size()); ++Index)
    {
        if (Index >= GResourceDistributionRowCount)
            break;

        auto& Row = Widget.mResourceDistributionRows[static_cast<size_t>(Index)];
        SetMetricRowData(
            Row,
            DistributionRows[Index].Label,
            DistributionRows[Index].Value,
            DistributionRows[Index].Percent,
            DistributionRows[Index].Tint,
            false);

        if (auto Background = Row.Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_flat",
                GRowTexture);
            Background->SetTint(1.f, 1.f, 1.f, 0.22f);
        }
        if (auto Label = Row.Label.lock())
            Label->SetTextColor(106, 98, 84, 255);
        if (auto Value = Row.Value.lock())
            Value->SetTextColor(126, 118, 102, 255);
        if (auto Bar = Row.Bar.lock())
        {
            Bar->SetEnable(true);
            Bar->SetTint(
                EProgressBarImageType::Back,
                FVector4(0.88f, 0.84f, 0.74f, 0.20f));
        }
    }

    const std::wstring ResourceTrackingValues[GResourceDetailCount] =
    {
        FormatInteger(SelectedResource.AvailableStock) +
            L" / " +
            FormatInteger(SelectedResource.ReservedPickup),
        FormatInteger(SelectedResource.ReservedIncoming) +
            L" / " +
            FormatInteger(SelectedResource.AvailableIncomingCapacity),
        FormatCurrency(ExportUnitPrice) +
            L" / " +
            FormatCurrency(ImportUnitPrice) +
            L" | 가치 " +
            FormatCompactCurrency(
                ResourceTradePricing::ComputeExportValue(
                    SelectedResource.Type,
                    SelectedResource.AvailableStock)),
        BuildStoragePressureText(StorageBiasPercent) +
            L" | " +
            BuildBalancePressureText(BalanceBiasPercent) +
            L" | 외교 수출 " +
            FormatSignedPercentValue(DiplomacyExportBiasPercent) +
            L" / 수입 " +
            FormatSignedPercentValue(DiplomacyImportBiasPercent) +
            L" | 칙령 수출 " +
            FormatSignedPercentValue(EdictExportBiasPercent) +
            L" / 수입 " +
            FormatSignedPercentValue(EdictImportBiasPercent) +
            L" | " +
            BuildEventPressureText(Snapshot, EventBiasPercent)
    };
    const wchar_t* ResourceTrackingLabels[GResourceDetailCount] =
    {
        L"사용 가능 / 픽업 예약",
        L"예약 입고 / 여유 용량",
        L"수출 / 수입 단가",
        L"시장 요인"
    };

    for (int Index = 0; Index < static_cast<int>(Widget.mResourceDetails.size()); ++Index)
    {
        if (Index >= GResourceDetailCount)
            break;

        SetDetailRowData(
            Widget.mResourceDetails[static_cast<size_t>(Index)],
            ResourceTrackingLabels[Index],
            ResourceTrackingValues[Index],
            false,
            FVector4(0.31f, 0.27f, 0.21f, 1.f));

        if (auto Background = Widget.mResourceDetails[static_cast<size_t>(Index)].Background.lock())
        {
            Background->SetTexture(
                Background->GetName() + "_base",
                GRowTexture);
            Background->SetTint(
                Index == 2 ?
                    FVector4(0.86f, 0.86f, 0.84f, 0.76f) :
                    FVector4(1.f, 1.f, 1.f, 0.94f));
        }
        if (auto Label = Widget.mResourceDetails[static_cast<size_t>(Index)].Label.lock())
        {
            Label->SetTextColor(
                Index == 2 ? 116 : 92,
                Index == 2 ? 112 : 84,
                Index == 2 ? 104 : 66,
                255);
        }
        if (auto Value = Widget.mResourceDetails[static_cast<size_t>(Index)].Value.lock())
        {
            Value->SetTextColor(
                Index == 2 ? 116 : 92,
                Index == 2 ? 112 : 84,
                Index == 2 ? 104 : 66,
                255);
        }
    }

    if (auto Notice = Widget.mResourceNotice.lock())
    {
        const bool ShowNotice =
            SelectedResource.TotalStock <= 0 &&
            SelectedResource.ReservedIncoming <= 0 &&
            SelectedResource.ReservedPickup <= 0 &&
            SelectedResource.ProducerBuildingCount <= 0 &&
            SelectedResource.ConsumerBuildingCount <= 0;
        const std::wstring NoticeText =
            ShowNotice ?
                std::wstring(L"활성 자원 흐름이 없습니다.") :
                (BuildFlowStageNotice(SelectedResource) +
                    L"\n" +
                    L"경보: " +
                    BuildResourceLogisticsWarning(SelectedResource));
        Notice->SetEnable(true);
        Notice->SetText(NoticeText.c_str());
    }
}

