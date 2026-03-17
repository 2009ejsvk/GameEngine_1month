#include "CitizenInfoPresentationInternal.h"
#include "UIStrings.h"
#include "../StringUtils.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

using namespace CitizenInfoPresentationInternal;

namespace CitizenInfoPresentation
{
    std::wstring FormatInteger(long long Value)
    {
        return StringUtils::FormatIntegerWithCommas(Value);
    }

    std::wstring FormatMoney(long long Value)
    {
        if (Value < 0)
            return L"-$" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMoneyDollarFirst(long long Value)
    {
        if (Value < 0)
            return L"$-" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMultiplier(float Value)
    {
        wchar_t Buffer[32] = {};
        swprintf_s(Buffer, L"x%.2f", Value);
        return Buffer;
    }

    std::wstring FormatMegawattValue(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.megawatt");
    }

    std::wstring FormatSignedMegawattValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            FormatMegawattValue(Value);
    }

    std::wstring FormatFlowRateValue(float Value, const wchar_t* SuffixKey)
    {
        const float SafeValue = (std::max)(0.f, Value);
        const std::wstring Suffix = Ui(SuffixKey);

        if (SafeValue < 0.05f)
            return L"0" + Suffix;

        const float RoundedValue =
            roundf(SafeValue * 10.f) / 10.f;
        wchar_t Buffer[64] = {};

        if (fabsf(RoundedValue - roundf(RoundedValue)) < 0.05f)
        {
            swprintf_s(
                Buffer,
                L"%d%s",
                static_cast<int>(roundf(RoundedValue)),
                Suffix.c_str());
        }
        else
        {
            swprintf_s(Buffer, L"%.1f%s", RoundedValue, Suffix.c_str());
        }

        return Buffer;
    }

    std::wstring FormatFlowVolumeValue(int Value, const wchar_t* SuffixKey)
    {
        return CitizenInfoPresentation::FormatInteger((std::max)(0, Value)) +
            Ui(SuffixKey);
    }
}
