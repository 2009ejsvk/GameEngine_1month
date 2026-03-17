#pragma once

#include "EdictWidgetState.h"
#include "UIStrings.h"
#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

class CWorld;

namespace EdictDataProvider
{
    enum class EEdictActionVisualMode : int
    {
        Neutral = 0,
        Primary,
        Active,
        CoolingDown,
        Requirement,
        BudgetShortage,
        Waiting
    };

    enum class EEdictRequirementTone : int
    {
        None = 0,
        Warning,
        BudgetShortage
    };

    struct FEdictSlotSnapshot
    {
        bool HasEntry = false;
        int EntryIndex = -1;
        std::wstring DisplayName;
        const TCHAR* IconPath = nullptr;
        bool Focused = false;
        bool Active = false;
        bool CoolingDown = false;
        bool Available = false;
    };

    struct FEdictTaxPolicyRowSnapshot
    {
        std::wstring Text = L"-";
        bool CanDecrease = false;
        bool CanIncrease = false;
    };

    struct FEdictTaxPolicySnapshot
    {
        std::wstring TitleText = UIStrings::Get(L"edict.tax_policy.title");
        std::wstring SummaryText =
            UIStrings::Get(L"edict.tax_policy.summary_pending");
        std::vector<FEdictTaxPolicyRowSnapshot> Rows;
        bool ShowPanel = false;
    };

    struct FEdictDetailSnapshot
    {
        bool HasSelection = false;
        std::wstring Title = UIStrings::Get(L"edict.detail.default_title");
        std::wstring CostText = L"$0";
        std::wstring InfoText = UIStrings::Get(L"edict.detail.default_info");
        std::wstring BodyText = UIStrings::Get(L"edict.detail.default_body");
        std::wstring FeedbackText;
        std::wstring RequirementText;
        EEdictRequirementTone RequirementTone =
            EEdictRequirementTone::None;
        EEdictActionVisualMode ActionMode =
            EEdictActionVisualMode::Waiting;
        std::wstring ActionLabel = UIStrings::Get(L"edict.action.apply");
        bool ActionEnabled = false;
    };

    struct FEdictCatalogSnapshot
    {
        EEdictUiCategory SelectedCategory;
        int MaxUnlockedCategoryIndex = 0;
        int CurrentPage = 0;
        int PageCount = 1;
        int PreviewEntryIndex = -1;
        int SelectedEntryIndex = -1;
        std::wstring TitleText;
        std::wstring PageText = L"1 / 1";
        bool CanMovePrev = false;
        bool CanMoveNext = false;
        std::vector<int> VisibleEntryIndices;
        std::vector<FEdictSlotSnapshot> Slots;
    };

    struct FEdictSnapshot
    {
        FEdictCatalogSnapshot Catalog;
        FEdictDetailSnapshot Detail;
        FEdictTaxPolicySnapshot TaxPolicy;
    };

    FEdictSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        const FEdictWidgetState& State);
}
