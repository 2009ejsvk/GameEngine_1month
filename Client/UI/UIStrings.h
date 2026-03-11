#pragma once

#include <initializer_list>
#include <string>
#include <vector>

namespace UIStrings
{
    const std::wstring& Get(const wchar_t* Key);

    std::wstring Format(
        const wchar_t* Key,
        const std::vector<std::wstring>& Args);

    inline std::wstring Format(
        const wchar_t* Key,
        std::initializer_list<std::wstring> Args)
    {
        return Format(
            Key,
            std::vector<std::wstring>(Args.begin(), Args.end()));
    }
}
