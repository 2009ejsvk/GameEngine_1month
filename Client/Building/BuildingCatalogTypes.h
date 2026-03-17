#pragma once

enum class EBuildingCostState : unsigned char
{
    None,
    Known,
    Unknown
};

enum class EBuildingProductionChainStage : unsigned char
{
    None,
    Primary,
    Intermediate,
    Final
};
