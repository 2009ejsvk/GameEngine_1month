#pragma once

#include "../Citizen/CitizenTypes.h"

enum class EPoliticalScope
{
    Global = 0,
    Worker,
    Resident,
    Visitor
};

struct FPoliticalSignalDef
{
    EPoliticalAxis Axis = EPoliticalAxis::Economy;
    EPoliticalStance FavoredStance = EPoliticalStance::Neutral;
    float Strength = 0.f;
    EPoliticalScope Scope = EPoliticalScope::Global;
};
