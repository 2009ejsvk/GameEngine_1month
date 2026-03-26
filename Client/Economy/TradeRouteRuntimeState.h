#pragma once

#include "../Building/BuildingTypes.h"

enum class ETradeRouteEndReason
{
    Completed = 0,
    Cancelled,
    Expired,
    EraTransitioned
};

inline const wchar_t* GetTradeRouteEndReasonDisplayName(ETradeRouteEndReason Reason)
{
    switch (Reason)
    {
    case ETradeRouteEndReason::Completed:
        return L"이행 완료";
    case ETradeRouteEndReason::Cancelled:
        return L"계약 취소";
    case ETradeRouteEndReason::Expired:
        return L"기간 만료";
    case ETradeRouteEndReason::EraTransitioned:
        return L"시대 전환";
    default:
        return L"종료";
    }
}

struct FTradeRouteRuntimeState
{
    int RouteId = 0;
    int ScenarioTag = 0;
    bool ImportRoute = false;
    EResourceType ResourceType = EResourceType::None;
    EResourceMarketClass MarketClass = EResourceMarketClass::None;
    int ForeignPowerIndex = 0;
    int ContractUnits = 0;
    int FulfilledUnits = 0;
    int RemainingDays = 0;
    int TotalDurationDays = 0;
    float TimerProgressAccum = 0.f;
    int RoutePricePerThousandUnits = 0;
    int SignedStandardPricePerThousandUnits = 0;
};

struct FTradeRouteCompletionRecord
{
    int RecordId = 0;
    int RouteId = 0;
    int ScenarioTag = 0;
    bool ImportRoute = false;
    EResourceType ResourceType = EResourceType::None;
    EResourceMarketClass MarketClass = EResourceMarketClass::None;
    int ForeignPowerIndex = 0;
    int ContractUnits = 0;
    int FulfilledUnits = 0;
    int ElapsedDays = 0;
    int TotalDurationDays = 0;
    long long SettledValue = 0;
    ETradeRouteEndReason EndReason = ETradeRouteEndReason::Completed;
    int CompletionRewardModifier = 0;
    int SecondaryRelationModifier = 0;
    int StandingModifier = 0;
};
