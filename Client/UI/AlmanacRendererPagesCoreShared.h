#pragma once

#include "AlmanacRenderer.h"
#include "AlmanacCalc.h"
#include "AlmanacRendererInternal.h"
#include "AlmanacTheme.h"
#include "../GameBalanceTuning.h"
#include "../Economy/ResourceTradePricing.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace AlmanacCalc;

namespace
{
    struct FSatisfactionDetailEntry
    {
        std::wstring Label;
        std::wstring Value;
        bool Highlight = false;
        FVector4 Tint = FVector4(0.31f, 0.27f, 0.21f, 1.f);
    };
    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);
        std::wstring Result;
        int GroupCount = 0;
        for (int Index = static_cast<int>(Digits.size()) - 1; Index >= 0; --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }
            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }
        if (Negative)
            Result.insert(Result.begin(), L'-');
        return Result;
    }

    std::wstring FormatSignedCompactCurrency(long long Value)
    {
        if (Value > 0)
            return L"+" + FormatCompactCurrency(Value);

        return FormatCompactCurrency(Value);
    }

    std::wstring FormatSignedPercentValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            std::to_wstring(Value) +
            L"%";
    }

    FVector4 ResolveMarketPressureTint(int Value)
    {
        if (Value > 0)
            return FVector4(0.82f, 0.52f, 0.20f, 0.95f);
        if (Value < 0)
            return FVector4(0.22f, 0.56f, 0.78f, 0.95f);

        return FVector4(0.62f, 0.60f, 0.52f, 0.90f);
    }

    std::wstring BuildStoragePressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"희소성 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"공급 과잉 " + FormatSignedPercentValue(BiasPercent);

        return L"안정";
    }

    std::wstring BuildBalancePressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"소비 우세 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"생산 우세 " + FormatSignedPercentValue(BiasPercent);

        return L"균형";
    }

    std::wstring BuildTemporalPressureText(int BiasPercent)
    {
        if (BiasPercent > 0)
            return L"상승 파동 " + FormatSignedPercentValue(BiasPercent);
        if (BiasPercent < 0)
            return L"하락 파동 " + FormatSignedPercentValue(BiasPercent);

        return L"횡보";
    }

    std::wstring BuildEventPressureText(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        int BiasPercent)
    {
        std::wstring SourceText;

        if (Snapshot.TaxEventStatus.Active &&
            !Snapshot.TaxEventStatus.Title.empty())
        {
            SourceText = Snapshot.TaxEventStatus.Title;
        }

        if (Snapshot.WorldCrisisStatus.Active &&
            !Snapshot.WorldCrisisStatus.Title.empty())
        {
            if (!SourceText.empty())
                SourceText += L" / ";

            SourceText += Snapshot.WorldCrisisStatus.Title;
        }

        if (SourceText.empty())
            return L"없음";

        return SourceText + L" " + FormatSignedPercentValue(BiasPercent);
    }

    std::wstring BuildTopBuildingMetricSummary(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        size_t MaxCount,
        const wchar_t* Suffix = nullptr)
    {
        std::wstring Result;

        for (size_t Index = 0;
            Index < Entries.size() && Index < MaxCount;
            ++Index)
        {
            if (Entries[Index].second <= 0)
                continue;

            if (!Result.empty())
                Result += L", ";

            Result += Entries[Index].first;
            Result += L" ";
            Result += FormatInteger(Entries[Index].second);

            if (Suffix && *Suffix)
                Result += Suffix;
        }

        return Result;
    }

    std::wstring BuildResourceLogisticsWarning(
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& Resource)
    {
        if (Resource.ShortagePressure > 0 &&
            !Resource.TopShortageBuildings.empty())
        {
            return L"병목: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopShortageBuildings,
                    2);
        }

        if (Resource.ReservedPickup > 0 &&
            !Resource.TopReservedBuildings.empty())
        {
            return L"운송 대기: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopReservedBuildings,
                    2);
        }

        if (!Resource.TopOverflowBuildings.empty())
        {
            return L"과잉 재고: " +
                BuildTopBuildingMetricSummary(
                    Resource.TopOverflowBuildings,
                    2,
                    L"%");
        }

        return L"흐름 안정";
    }

    std::wstring BuildFlowStageHeadline(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        const wchar_t* EmptyText)
    {
        if (Entries.empty())
            return EmptyText ? std::wstring(EmptyText) : std::wstring(L"-");

        return Entries.front().first;
    }

    std::wstring BuildFlowStageNotice(
        const AlmanacDataProvider::FAlmanacResourceTypeSnapshot& Resource)
    {
        return L"대표 흐름: " +
            BuildFlowStageHeadline(Resource.TopProducerBuildings, L"-") +
            L" -> " +
            BuildFlowStageHeadline(Resource.TopWarehouseBuildings, L"-") +
            L" -> " +
            BuildFlowStageHeadline(
                Resource.ShortagePressure > 0 ?
                    Resource.TopShortageBuildings :
                    Resource.TopConsumerBuildings,
                L"-") +
            L" -> " +
            BuildFlowStageHeadline(Resource.TopHarborBuildings, L"-");
    }

    std::wstring BuildOverviewElectionText(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        if (!Snapshot.HasMainWorld)
            return L"행정 데이터 없음";

        if (Snapshot.ElectionStatus.GameLost)
            return L"정권 상실";

        if (Snapshot.ElectionStatus.NextElectionYear <= 0 ||
            Snapshot.ElectionStatus.NextElectionMonth <= 0 ||
            Snapshot.ElectionStatus.NextElectionDay <= 0)
        {
            return L"선거 일정 없음";
        }

        std::wstring Result = FormatDate(
            Snapshot.ElectionStatus.NextElectionYear,
            Snapshot.ElectionStatus.NextElectionMonth,
            Snapshot.ElectionStatus.NextElectionDay);

        if (Snapshot.DaysUntilNextElection >= 0)
        {
            Result += L"\n";
            Result += std::to_wstring(Snapshot.DaysUntilNextElection);
            Result += L"일 남음";
        }

        return Result;
    }

    std::wstring BuildOverviewActiveEdictDetail(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        if (Snapshot.ActiveEdictLines.empty())
            return L"시행 중 없음";

        if (Snapshot.ActiveEdictLines.size() == 1)
            return Snapshot.ActiveEdictLines[0];

        return Snapshot.ActiveEdictLines[0] +
            L"\n외 " +
            std::to_wstring(Snapshot.ActiveEdictLines.size() - 1) +
            L"개";
    }
}
