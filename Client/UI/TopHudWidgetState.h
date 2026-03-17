#pragma once

#include "../Politics/ConstitutionTypes.h"

struct FTopHudWidgetState
{
    float MonthProgress = 0.f;
    bool GameLost = false;
    bool GameOverMenusClosed = false;
    bool ManualEraTransitionPopupOpen = false;
    bool ConstitutionPopupActive = false;
    bool EraTransitionPopupOpen = false;
    EConstitutionOptionId ConstitutionConfirmOptionId =
        EConstitutionOptionId::None;
    EConstitutionOptionId ConstitutionCancelOptionId =
        EConstitutionOptionId::None;
};
