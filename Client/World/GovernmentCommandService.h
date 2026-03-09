#pragma once

#include "../Politics/PoliticalTypes.h"
#include <string>

class IGovernmentCommandService
{
public:
    virtual ~IGovernmentCommandService() = default;

    virtual bool TryApplyEdict(
        EGovernmentEdictType Type,
        std::wstring& OutMessage) = 0;
    virtual bool AdjustTaxPolicy(
        ETaxPolicyType Type,
        int DeltaPercent,
        std::wstring& OutMessage) = 0;
};
