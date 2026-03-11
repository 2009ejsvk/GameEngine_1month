#pragma once

#include "../Building/BuildingTypes.h"
#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

class CWorld;

namespace BuildMenuDataProvider
{
    struct FBuildMenuStatusRecord
    {
        int AliveNpcCount = 0;
        long long NationalBudget = 0;
        int SimulationYear = 2000;
        int SimulationMonth = 1;
        int SimulationDay = 1;
        int SimulationMonthDayCount = 30;
        float SimulationMonthProgress = 0.f;
        bool HasSimulationData = false;
        std::wstring YearbookBodyText;
    };

    struct FBuildMenuStatusSnapshot
    {
        std::wstring NpcCountText = L"NPC: 0";
        std::wstring BudgetText = L"국가 예산: $0";
        std::wstring DateText = L"날짜: 2000-01-01";
        std::wstring MonthProgressText = L"월 진행: 0%";
        float MonthProgress = 0.f;
        std::wstring YearbookBodyText;
    };

    struct FBuildMenuSlotSnapshot
    {
        bool Enabled = false;
        int EntryIndex = -1;
        std::wstring DisplayName;
        const TCHAR* IconPath = nullptr;
        bool Previewed = false;
    };

    struct FBuildMenuDetailSnapshot
    {
        bool HasSelection = false;
        std::wstring Title = L"건물 정보";
        std::wstring BlueprintCost = L"-";
        std::wstring ConstructionCost = L"-";
        std::wstring InfoText =
            L"핵심 정보\n- 건물을 선택하면 표시됩니다.";
        std::wstring BodyText =
            L"왼쪽 카드에 마우스를 올리면 건물 설명과 핵심 정보가 함께 표시됩니다.";
    };

    struct FBuildMenuCatalogSnapshot
    {
        EBuildingCategory SelectedCategory =
            EBuildingCategory::Infrastructure;
        int CurrentPage = 0;
        int PageCount = 1;
        int PreviewEntryIndex = -1;
        std::wstring TitleText;
        std::wstring PageText = L"1 / 1";
        std::vector<int> VisibleEntryIndices;
        std::vector<FBuildMenuSlotSnapshot> Slots;
    };

    struct FBuildMenuSnapshot
    {
        FBuildMenuStatusSnapshot Status;
        FBuildMenuCatalogSnapshot Catalog;
        FBuildMenuDetailSnapshot Detail;
    };

    class IBuildMenuQuerySource
    {
    public:
        virtual ~IBuildMenuQuerySource() = default;

        virtual FBuildMenuStatusRecord QueryStatus() const = 0;
    };

    FBuildMenuSnapshot BuildSnapshot(
        const std::shared_ptr<IBuildMenuQuerySource>& QuerySource,
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex);

    FBuildMenuSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex);
}
