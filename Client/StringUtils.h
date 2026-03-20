#pragma once

#include <Windows.h>
#include <cctype>
#include <cwctype>
#include <string>
#include <vector>

namespace StringUtils
{
    inline bool IsWhitespace(char Character)
    {
        return std::isspace(static_cast<unsigned char>(Character)) != 0;
    }

    inline bool IsWhitespace(wchar_t Character)
    {
        return iswspace(Character) != 0;
    }

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

    inline std::wstring FormatCurrency(long long Value)
    {
        if (Value < 0)
        {
            return L"-$" +
                FormatUnsignedIntegerWithCommas(AbsToUnsigned(Value));
        }

        return L"$" + FormatIntegerWithCommas(Value);
    }

    inline std::wstring FormatCurrencyDollarFirst(long long Value)
    {
        if (Value < 0)
        {
            return L"$-" +
                FormatUnsignedIntegerWithCommas(AbsToUnsigned(Value));
        }

        return L"$" + FormatIntegerWithCommas(Value);
    }

    inline std::wstring FormatSignedCurrency(long long Value)
    {
        return Value < 0 ?
            L"-$" + FormatUnsignedIntegerWithCommas(AbsToUnsigned(Value)) :
            L"+$" + FormatUnsignedIntegerWithCommas(AbsToUnsigned(Value));
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

    template <typename TChar>
    inline std::basic_string<TChar> Trim(
        const std::basic_string<TChar>& Text)
    {
        size_t Start = 0;

        while (Start < Text.size() && IsWhitespace(Text[Start]))
            ++Start;

        size_t End = Text.size();

        while (End > Start && IsWhitespace(Text[End - 1]))
            --End;

        return Text.substr(Start, End - Start);
    }

    template <typename TChar>
    inline bool StartsWith(
        const std::basic_string<TChar>& Text,
        const TChar* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = std::char_traits<TChar>::length(Prefix);
        return Text.size() >= PrefixLength &&
            Text.compare(0, PrefixLength, Prefix) == 0;
    }

    template <typename TChar>
    inline int ParseLeadingInteger(
        const std::basic_string<TChar>& Text,
        int DefaultValue = 0)
    {
        bool Negative = false;
        bool FoundDigit = false;
        int Value = 0;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const TChar Ch = Text[Index];

            if (!FoundDigit && Ch == static_cast<TChar>('-'))
            {
                Negative = true;
                continue;
            }

            if (Ch < static_cast<TChar>('0') ||
                Ch > static_cast<TChar>('9'))
            {
                if (FoundDigit)
                    break;

                continue;
            }

            FoundDigit = true;
            Value = Value * 10 + static_cast<int>(Ch - static_cast<TChar>('0'));
        }

        if (!FoundDigit)
            return DefaultValue;

        return Negative ? -Value : Value;
    }

    template <typename TChar>
    inline void AppendLine(
        std::basic_string<TChar>& Body,
        const std::basic_string<TChar>& Line)
    {
        if (Line.empty())
            return;

        if (!Body.empty())
            Body += static_cast<TChar>('\n');

        Body += Line;
    }

    template <typename TChar>
    inline void AppendLine(
        std::basic_string<TChar>& Body,
        const TChar* Line)
    {
        if (!Line || *Line == 0)
            return;

        AppendLine(Body, std::basic_string<TChar>(Line));
    }

    template <typename TChar>
    inline std::vector<std::basic_string<TChar>> SplitLines(
        const std::basic_string<TChar>& Text)
    {
        std::vector<std::basic_string<TChar>> Lines;
        std::basic_string<TChar> Current;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const TChar Character = Text[Index];

            if (Character == static_cast<TChar>('\r'))
                continue;

            if (Character == static_cast<TChar>('\n'))
            {
                Lines.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Character);
        }

        if (!Current.empty() || Text.empty())
            Lines.push_back(Current);

        return Lines;
    }

    template <typename TChar>
    inline std::basic_string<TChar> ExtractDetailValue(
        const std::basic_string<TChar>& DetailText,
        const TChar* Prefix)
    {
        if (!Prefix)
            return std::basic_string<TChar>();

        const std::vector<std::basic_string<TChar>> Lines =
            SplitLines(DetailText);
        const size_t PrefixLength = std::char_traits<TChar>::length(Prefix);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::basic_string<TChar> Line = Trim(Lines[Index]);

            if (!StartsWith(Line, Prefix))
                continue;

            return Trim(Line.substr(PrefixLength));
        }

        return std::basic_string<TChar>();
    }

    template <typename TChar>
    inline std::basic_string<TChar> JoinLines(
        const std::vector<std::basic_string<TChar>>& Lines)
    {
        std::basic_string<TChar> Result;

        for (const std::basic_string<TChar>& Line : Lines)
            AppendLine(Result, Line);

        return Result;
    }

    template <typename TChar>
    inline void ReplaceAll(
        std::basic_string<TChar>& Text,
        const std::basic_string<TChar>& Pattern,
        const std::basic_string<TChar>& Replacement)
    {
        if (Pattern.empty())
            return;

        size_t Position = 0;

        while ((Position = Text.find(Pattern, Position)) !=
            std::basic_string<TChar>::npos)
        {
            Text.replace(Position, Pattern.size(), Replacement);
            Position += Replacement.size();
        }
    }

    template <typename TChar>
    inline void ReplaceAll(
        std::basic_string<TChar>& Text,
        const TChar* Pattern,
        const TChar* Replacement)
    {
        const std::basic_string<TChar> PatternText =
            Pattern ? std::basic_string<TChar>(Pattern) :
                std::basic_string<TChar>();
        const std::basic_string<TChar> ReplacementText =
            Replacement ? std::basic_string<TChar>(Replacement) :
                std::basic_string<TChar>();

        ReplaceAll(
            Text,
            PatternText,
            ReplacementText);
    }

    template <typename TChar>
    inline unsigned long long HashFnv1a64(const TChar* Text)
    {
        constexpr unsigned long long OffsetBasis = 1469598103934665603ull;
        constexpr unsigned long long Prime = 1099511628211ull;
        unsigned long long Hash = OffsetBasis;

        if (!Text)
            return Hash;

        for (const TChar* Cursor = Text; *Cursor != 0; ++Cursor)
        {
            Hash ^= static_cast<unsigned long long>(*Cursor);
            Hash *= Prime;
        }

        return Hash;
    }

    template <typename TChar>
    inline unsigned long long HashFnv1a64(
        const std::basic_string<TChar>& Text)
    {
        return HashFnv1a64(Text.c_str());
    }
}
