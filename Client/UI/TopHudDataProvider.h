#pragma once

#include "UIStrings.h"
#include "Vector4.h"
#include <memory>
#include <string>

class CWorld;

namespace TopHudDataProvider
{
    struct FTopHudSnapshot
    {
        std::wstring DateText = L"-";
        std::wstring BudgetText = L"$0";
        std::wstring ElectionText =
            UIStrings::Get(L"top_hud.placeholder.election");
        FVector4 ElectionTextColor = FVector4(1.f, 1.f, 1.f, 1.f);
        std::wstring TaxPolicyText =
            UIStrings::Get(L"top_hud.placeholder.tax_policy");
        std::wstring EventText =
            UIStrings::Get(L"top_hud.placeholder.event_stable");
        FVector4 EventTextColor = FVector4(1.f, 1.f, 1.f, 1.f);
        std::wstring NpcText = L"0";
        std::wstring SupportText = L"0%";
        std::wstring GameOverTitleText =
            UIStrings::Get(L"top_hud.game_over.title");
        std::wstring GameOverBodyText;
        float MonthProgress = 0.f;
        bool GameLost = false;
        bool CanUseButtons = true;
    };

    FTopHudSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World);
}
