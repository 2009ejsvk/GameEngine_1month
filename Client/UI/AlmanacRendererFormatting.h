#pragma once

#include "AlmanacRendererConstants.h"
#include "../Politics/PoliticalTypes.h"
#include "TropicoUiTheme.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <string>

namespace
{
    float Clamp01(float Value)
    {
        return (std::max)(0.f, (std::min)(1.f, Value));
    }

    double Clamp01(double Value)
    {
        return (std::max)(0.0, (std::min)(1.0, Value));
    }

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatCompactCurrency(long long Value)
    {
        const bool Negative = Value < 0;
        const double AbsValue =
            static_cast<double>(Negative ? -Value : Value);

        std::wstring Prefix = Negative ? L"-$" : L"$";

        if (AbsValue >= 1000000.0)
        {
            const double InMillions = AbsValue / 1000000.0;
            wchar_t Buffer[32] = {};
            swprintf_s(Buffer, L"%.2f", InMillions);
            return Prefix + Buffer + L"백만";
        }

        return FormatCurrency(Value);
    }

    std::wstring FormatPercent(double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.0f%%", Value);
        return Buffer;
    }

    std::wstring FormatFixed1(double Value)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%.1f", Value);
        return Buffer;
    }

    std::wstring FormatSignedFixed1(double Value)
    {
        if (Value > 0.0)
            return L"+" + FormatFixed1(Value);

        return FormatFixed1(Value);
    }

    std::wstring FormatSignedPercentUnit(double Value)
    {
        const double PercentValue = Value * 100.0;

        if (std::fabs(PercentValue) < 0.5)
            return L"0%";

        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%+.0f%%", PercentValue);
        return Buffer;
    }

    std::wstring FormatDate(int Year, int Month, int Day)
    {
        wchar_t Buffer[64] = {};
        swprintf_s(Buffer, L"%04d.%02d.%02d", Year, Month, Day);
        return Buffer;
    }

    std::wstring FormatCountWithPercent(int Count, double Ratio)
    {
        return std::to_wstring(Count) +
            L"명 (" + FormatPercent(Ratio * 100.0) + L")";
    }

    std::wstring FormatTaxPolicySummary(const FTaxPolicy& TaxPolicy)
    {
        return
            L"소비 " + std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L"% / 소득 " + std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L"% / 재산 " + std::to_wstring(TaxPolicy.PropertyRatePercent) +
            L"%";
    }

    std::wstring FormatTaxPolicyCompact(const FTaxPolicy& TaxPolicy)
    {
        return
            std::to_wstring(TaxPolicy.ConsumptionRatePercent) +
            L" / " +
            std::to_wstring(TaxPolicy.IncomeRatePercent) +
            L" / " +
            std::to_wstring(TaxPolicy.PropertyRatePercent);
    }

    const wchar_t* GetElectionWarningTierLabel(double Score)
    {
        if (Score >= 0.78)
            return L"재선 위험 높음";
        if (Score >= 0.52)
            return L"재선 주의";
        if (Score >= 0.32)
            return L"선거 점검";
        return L"안정";
    }

    FVector4 ResolveElectionWarningTint(double Score)
    {
        if (Score >= 0.78)
            return TropicoUiTheme::GStatusDangerTint;
        if (Score >= 0.52)
            return TropicoUiTheme::GStatusWarningTint;
        if (Score >= 0.32)
            return TropicoUiTheme::GStatusCautionTint;
        return TropicoUiTheme::GStatusSuccessTint;
    }

    std::wstring BuildElectionWarningSummary(
        bool GameLost,
        int DaysUntilNextElection,
        double ElectionWarningScore,
        const FTaxPolicyEventStatus& TaxEventStatus)
    {
        if (GameLost)
            return L"정권 상실";

        if (DaysUntilNextElection < 0)
            return L"선거 일정 없음";

        if (DaysUntilNextElection > 180 && ElectionWarningScore < 0.32)
            return L"안정";

        std::wstring Summary =
            std::wstring(GetElectionWarningTierLabel(ElectionWarningScore)) +
            L" / " +
            std::to_wstring(DaysUntilNextElection) +
            L"일 남음";

        if (TaxEventStatus.Active && !TaxEventStatus.Title.empty())
        {
            Summary += L" / " + TaxEventStatus.Title;
        }
        else if (ElectionWarningScore >= 0.78)
        {
            Summary += L" / 지지 기반 급락";
        }
        else if (ElectionWarningScore >= 0.52)
        {
            Summary += L" / 야권 결집";
        }
        else if (DaysUntilNextElection <= 90)
        {
            Summary += L" / 박빙 진입 가능";
        }

        return Summary;
    }

    const wchar_t* GetPoliticalFactionCompactName(
        EPoliticalAxis Axis,
        EPoliticalStance Stance)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"자본";
            case EPoliticalStance::Right: return L"공산";
            default: return L"중립";
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"종교";
            case EPoliticalStance::Right: return L"군국";
            default: return L"중립";
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"환경";
            case EPoliticalStance::Right: return L"산업";
            default: return L"중립";
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"지식";
            case EPoliticalStance::Right: return L"보수";
            default: return L"중립";
            }
        default:
            return L"중립";
        }
    }

    const wchar_t* GetPoliticalFactionVerboseName(
        EPoliticalAxis Axis,
        EPoliticalStance Stance)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"자본주의자";
            case EPoliticalStance::Right: return L"공산주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"종교인";
            case EPoliticalStance::Right: return L"군국주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"환경주의자";
            case EPoliticalStance::Right: return L"산업주의자";
            default: return L"중립";
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:  return L"지식인";
            case EPoliticalStance::Right: return L"보수주의자";
            default: return L"중립";
            }
        default:
            return L"중립";
        }
    }
}
