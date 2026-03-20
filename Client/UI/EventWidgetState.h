#pragma once

#include "../Politics/PoliticalTypes.h"
#include <string>
#include <vector>

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
    bool UseBodyFormulaTermWrap = false;
    std::vector<std::wstring> BodyFormulaTerms;
    bool ShowAcceptButton = true;
    bool ShowRejectButton = true;
    bool ShowCornerCloseButton = false;
    std::wstring CornerCloseConfigSection;
    float AutoCloseSeconds = 0.f;
};
