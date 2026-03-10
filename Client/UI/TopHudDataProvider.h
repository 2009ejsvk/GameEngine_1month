#pragma once

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
        std::wstring ElectionText = L"차기 선거 -";
        FVector4 ElectionTextColor = FVector4(1.f, 1.f, 1.f, 1.f);
        std::wstring TaxPolicyText = L"세금 -";
        std::wstring EventText = L"현재 상태 안정";
        FVector4 EventTextColor = FVector4(1.f, 1.f, 1.f, 1.f);
        std::wstring NpcText = L"0";
        std::wstring SupportText = L"0%";
        std::wstring GameOverTitleText = L"정권 상실";
        std::wstring GameOverBodyText;
        float MonthProgress = 0.f;
        bool GameLost = false;
        bool CanUseButtons = true;
    };

    FTopHudSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World);
}
