#pragma once

#include <Windows.h>
#include <string>

namespace StringUtils
{
    inline unsigned long long AbsToUnsigned(long long Value)
    {
        return Value < 0 ?
            static_cast<unsigned long long>(-(Value + 1)) + 1ULL :
            static_cast<unsigned long long>(Value);
    }

    inline std::wstring FormatUnsignedIntegerWithCommas(
        unsigned long long Value)
    {
        std::wstring Digits = std::to_wstring(Value);
        std::wstring Result;
        int GroupCount = 0;

        for (int Index = static_cast<int>(Digits.size()) - 1;
            Index >= 0;
            --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }

            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }

        return Result;
    }

    inline std::wstring FormatIntegerWithCommas(long long Value)
    {
        const bool Negative = Value < 0;
        std::wstring Result =
            FormatUnsignedIntegerWithCommas(AbsToUnsigned(Value));

        if (Negative)
            Result.insert(Result.begin(), L'-');

        return Result;
    }

    inline std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0);

        if (RequiredCount <= 0)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &WideText[0], RequiredCount);
        return WideText;
    }

    inline std::string WideToUtf8(const std::wstring& Text)
    {
        if (Text.empty())
            return std::string();

        const int RequiredBytes = WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0, nullptr, nullptr);

        if (RequiredBytes <= 0)
        {
            std::string Fallback;
            Fallback.reserve(Text.size());

            for (wchar_t Character : Text)
            {
                Fallback.push_back(
                    Character >= 0 && Character <= 0x7F ?
                        static_cast<char>(Character) :
                        '?');
            }

            return Fallback;
        }

        std::string Result;
        Result.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Result[0], RequiredBytes, nullptr, nullptr);
        return Result;
    }
}
