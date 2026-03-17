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
            return Prefix + Buffer +
                UIStrings::Get(L"almanac.format.currency.million_suffix");
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
        return UIStrings::Format(
            L"almanac.format.count_with_percent",
            {
                std::to_wstring(Count),
                FormatPercent(Ratio * 100.0)
            });
    }

    std::wstring FormatTaxPolicySummary(const FTaxPolicy& TaxPolicy)
    {
        return UIStrings::Format(
            L"almanac.tax_policy.summary_template",
            {
                std::to_wstring(TaxPolicy.ConsumptionRatePercent),
                std::to_wstring(TaxPolicy.IncomeRatePercent),
                std::to_wstring(TaxPolicy.PropertyRatePercent)
            });
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

    const wchar_t* GetPoliticalFactionCompactName(
        EPoliticalAxis Axis,
        EPoliticalStance Stance)
    {
        switch (Axis)
        {
        case EPoliticalAxis::Economy:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.compact.economy.left").c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.compact.economy.right").c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.compact.religion_militarism.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.compact.religion_militarism.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.compact.environment_industry.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.compact.environment_industry.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.compact.intellectual_conservative.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.compact.intellectual_conservative.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        default:
            return UIStrings::Get(L"almanac.politics.compact.neutral").c_str();
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
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.verbose.economy.left").c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.verbose.economy.right").c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::ReligionMilitarism:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.verbose.religion_militarism.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.verbose.religion_militarism.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::EnvironmentIndustry:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.verbose.environment_industry.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.verbose.environment_industry.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        case EPoliticalAxis::IntellectualConservative:
            switch (Stance)
            {
            case EPoliticalStance::Left:
                return UIStrings::Get(
                    L"almanac.politics.verbose.intellectual_conservative.left")
                    .c_str();
            case EPoliticalStance::Right:
                return UIStrings::Get(
                    L"almanac.politics.verbose.intellectual_conservative.right")
                    .c_str();
            default:
                return UIStrings::Get(
                    L"almanac.politics.compact.neutral").c_str();
            }
        default:
            return UIStrings::Get(L"almanac.politics.compact.neutral").c_str();
        }
    }
}
