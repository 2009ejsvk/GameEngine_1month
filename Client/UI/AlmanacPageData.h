#pragma once

#include "Vector4.h"
#include <array>
#include <string>

namespace AlmanacPageData
{
    constexpr int GPoliticsFactionCount = 8;
    constexpr int GPoliticsNeutralCount = 4;
    constexpr int GPoliticsSupportRowCount = 4;
    constexpr int GPoliticsModifierCount = 6;
    constexpr int GForeignPowerCount = 5;
    constexpr int GForeignBuildingLineCount = 3;
    constexpr int GForeignEdictLineCount = 2;
    constexpr int GForeignMiscLineCount = 2;
    constexpr int GBuildingCategoryCount = 13;
    constexpr int GBuildingDetailCount = 8;

    struct FModifierEntry
    {
        std::wstring Label;
        int Value = 0;
    };

    struct FLabelValueEntry
    {
        std::wstring Label;
        std::wstring Value;
    };

    struct FPoliticsFactionEntry
    {
        std::wstring Label;
        int Count = 0;
        int Favor = 0;
        std::wstring Leader;
        int ModerateCount = 0;
        int StrongCount = 0;
        int FerventCount = 0;
        int BuildingModifierTotal = 0;
        FVector4 IconTint = FVector4(0.90f, 0.72f, 0.18f, 0.96f);
        std::array<FModifierEntry, GPoliticsModifierCount> Modifiers = {};
    };

    struct FForeignPowerEntry
    {
        std::wstring Name;
        int Relation = 0;
        std::wstring DisplayValue = L"0";
        std::wstring Status = L"미확인";
        int EconomicAid = 0;
        int BuildingModifier = 0;
        int EdictModifier = 0;
        int ConstitutionModifier = 0;
        int MiscModifier = 0;
        int TradeModifier = 0;
        std::array<FModifierEntry, GForeignBuildingLineCount> BuildingLines = {};
        std::array<FModifierEntry, GForeignEdictLineCount> EdictLines = {};
        std::array<FModifierEntry, GForeignMiscLineCount> MiscLines = {};
    };

    struct FBuildingCategoryEntry
    {
        std::wstring Label;
        int Count = 0;
        std::array<FModifierEntry, GBuildingDetailCount> Details = {};
    };

    struct FDataSet
    {
        std::array<int, GPoliticsNeutralCount> PoliticsNeutralCounts = {};
        std::array<FLabelValueEntry, GPoliticsSupportRowCount> PoliticsSupportRows = {};
        std::wstring PoliticsElectionText;
        std::array<FPoliticsFactionEntry, GPoliticsFactionCount> PoliticsFactions = {};
        std::array<FForeignPowerEntry, GForeignPowerCount> ForeignPowers = {};
        std::array<FBuildingCategoryEntry, GBuildingCategoryCount> BuildingCategories = {};
    };

    const FDataSet& Get();
    bool ReloadIfChanged(float DeltaTime);
}
