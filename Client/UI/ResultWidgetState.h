#pragma once

#include <string>

struct FResultWidgetState
{
    bool Visible = false;
    bool Victory = false;
    std::wstring Title;
    std::wstring Summary;
    std::wstring DetailPrimary;
    std::wstring DetailSecondary;
    std::wstring DetailTertiary;
    std::wstring DetailQuaternary;
};
