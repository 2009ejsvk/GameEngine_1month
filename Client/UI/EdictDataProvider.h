#pragma once

#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

class CWorld;
enum class EEdictUiCategory : int;

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
        std::wstring TitleText = L"세금 정책";
        std::wstring SummaryText = L"세금 보고 준비 중";
        std::vector<FEdictTaxPolicyRowSnapshot> Rows;
        bool ShowPanel = false;
    };

    struct FEdictDetailSnapshot
    {
        bool HasSelection = false;
        std::wstring Title = L"칙령 선택";
        std::wstring CostText = L"$0";
        std::wstring InfoText = L"왼쪽 카드에서 칙령을 선택하세요.";
        std::wstring BodyText =
            L"카드를 고르면 효과와 제약 조건이 아래 패널에 표시됩니다.\n"
            L"현재 시행 중인 칙령은 금색 강조와 체크 표시로 구분됩니다.";
        std::wstring FeedbackText;
        std::wstring RequirementText;
        EEdictRequirementTone RequirementTone =
            EEdictRequirementTone::None;
        EEdictActionVisualMode ActionMode =
            EEdictActionVisualMode::Waiting;
        std::wstring ActionLabel = L"시행";
        bool ActionEnabled = false;
    };

    struct FEdictCatalogSnapshot
    {
        EEdictUiCategory SelectedCategory;
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
        EEdictUiCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex,
        int SelectedEntryIndex,
        const std::wstring& FeedbackMessage);
}
