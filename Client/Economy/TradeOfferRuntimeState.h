#pragma once

#include "../Building/BuildingTypes.h"

struct FTradeOfferRuntimeState
{
    int OfferId = 0;
    int ScenarioTag = 0;
    bool ImportRoute = false;
    EResourceType ResourceType = EResourceType::None;
    EResourceMarketClass MarketClass = EResourceMarketClass::None;
    int ForeignPowerIndex = 0;
    int BasePricePerThousand = 0;
    int OfferPricePerThousand = 0;
    int MarginPercent = 0;
    int MaxAmount = 0;
    int AvailabilityUnits = 0;
    int Score = 0;
};
