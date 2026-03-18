#pragma once

#include "World/WorldSubsystem.h"
#include "MainWorldWorldCrisisService.h"
#include <memory>

class CCrisisSubsystem : public CWorldSubsystem
{
public:
    using CWorldSubsystem::CWorldSubsystem;

    void Reset();
    void ApplyDailyWorldCrisisEffects();
    void TickWorldCrises();

    std::unique_ptr<CMainWorldWorldCrisisService> WorldCrisisService;
};
