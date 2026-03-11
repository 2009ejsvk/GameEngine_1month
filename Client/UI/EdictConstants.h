#pragma once

#include "UILayoutConfig.h"

namespace EdictConstants
{
    constexpr int GEdictCategoryCount = 4;
    constexpr int GEdictSlotsPerPage = 14;
    constexpr int GEdictSlotColumnCount = 7;
    constexpr int GEdictSlotRowCount = 2;
    constexpr int GTaxPolicyRowCount = 3;

    inline bool IsTaxPolicyPanelEnabled()
    {
        return UIConfig::EdictEnableTaxPolicyPanel;
    }
}
