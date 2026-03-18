#pragma once

#include "../Politics/PoliticalTypes.h"
#include <string>

struct FEventWidgetState
{
    bool Visible = false;
    EPoliticalDemandIssuerType IssuerType =
        EPoliticalDemandIssuerType::None;
    int IssuerIndex = -1;
    std::wstring Title;
    std::wstring Body;
    std::wstring AcceptConsequence;
    std::wstring RejectConsequence;
    bool ShowAcceptButton = true;
    bool ShowRejectButton = true;
    float AutoCloseSeconds = 0.f;
};
