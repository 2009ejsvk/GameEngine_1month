#pragma once

#include "UIStrings.h"

namespace UIStringShorthand
{
    inline const std::wstring& Ui(const wchar_t* Key)
    {
        return UIStrings::Get(Key);
    }

    inline const wchar_t* UiText(const wchar_t* Key)
    {
        return UIStrings::Get(Key).c_str();
    }
}
