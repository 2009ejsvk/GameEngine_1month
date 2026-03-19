#pragma once

#include "../Politics/ConstitutionTypes.h"

struct FTopHudWidgetState
{
    float MonthProgress = 0.f;
    bool GameLost = false;
    bool GameOverMenusClosed = false;
    bool ConstitutionPanelOpen = false;
    bool ConstitutionOverviewMode = true;
    bool ManualEraTransitionPopupOpen = false;
    bool ConstitutionPopupActive = false;
    bool EraTransitionPopupOpen = false;
    EConstitutionTopic ConstitutionViewedTopic =
        EConstitutionTopic::VotingRights;
    EConstitutionOptionId ConstitutionLeftOptionId =
        EConstitutionOptionId::None;
    EConstitutionOptionId ConstitutionRightOptionId =
        EConstitutionOptionId::None;
};
