#pragma once

#include "CitizenInfoBuildingRuntime.h"
#include "CitizenInfoDataProvider.h"

namespace CitizenInfoDataProviderInternal
{
    using FBuildingUiSnapshot = CitizenInfoBuildingRuntime::FBuildingUiSnapshot;

    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry);
    const std::wstring& Ui(const wchar_t* Key);
    const wchar_t* UiText(const wchar_t* Key);
    std::wstring GetDamageLevelDisplayName(EBuildingDamageLevel Level);
    std::wstring FormatMoney(long long Value);
    std::wstring FormatMoneyDollarFirst(long long Value);
    std::wstring FormatMultiplier(float Value);
}
