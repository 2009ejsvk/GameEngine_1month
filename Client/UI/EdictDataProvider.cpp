#include "EdictDataProvider.h"
#include "EdictConstants.h"
#include "EdictWidget.h"
#include "UIStrings.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <algorithm>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    using EdictConstants::GEdictSlotsPerPage;
    using EdictConstants::GTaxPolicyRowCount;
    constexpr const TCHAR* GCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");

    const ETaxPolicyType GTaxPolicyTypes[GTaxPolicyRowCount] =
    {
        ETaxPolicyType::Consumption,
        ETaxPolicyType::Income,
        ETaxPolicyType::Property
    };

    struct FEdictAvailabilityInfo
    {
        bool Active = false;
        bool CoolingDown = false;
        bool CanApply = false;
        long long ActivationCost = 0;
        std::wstring StatusText =
            UIStrings::Get(L"edict.status.checking");
        std::wstring RequirementText;
    };

    std::wstring FormatCurrency(long long Value)
    {
        bool Negative = false;
        unsigned long long AbsValue = 0;

        if (Value < 0)
        {
            Negative = true;
            AbsValue = static_cast<unsigned long long>(-Value);
        }
        else
        {
            AbsValue = static_cast<unsigned long long>(Value);
        }

        std::wstring Digits = std::to_wstring(AbsValue);

        for (int i = static_cast<int>(Digits.size()) - 3; i > 0; i -= 3)
        {
            Digits.insert(static_cast<size_t>(i), 1, L',');
        }

        if (Negative)
            Digits.insert(Digits.begin(), L'-');

        return L"$" + Digits;
    }

    std::wstring FormatPercentValue(int Value)
    {
        return std::to_wstring(Value) + L"%";
    }

    int FormatDaysToMonths(int Days)
    {
        return (std::max)(1, (Days + 29) / 30);
    }

    EEdictUiCategory ResolveEdictUiCategory(EEdictEra Era)
    {
        switch (Era)
        {
        case EEdictEra::Colonial:
            return EEdictUiCategory::Colonial;
        case EEdictEra::WorldWars:
            return EEdictUiCategory::WorldWars;
        case EEdictEra::ColdWar:
            return EEdictUiCategory::ColdWar;
        case EEdictEra::Modern:
            return EEdictUiCategory::Modern;
        default:
            return EEdictUiCategory::Colonial;
        }
    }

    const wchar_t* GetCategoryLabel(EEdictUiCategory Category)
    {
        switch (Category)
        {
        case EEdictUiCategory::Colonial:
            return UIStrings::Get(L"edict.category.colonial").c_str();
        case EEdictUiCategory::WorldWars:
            return UIStrings::Get(L"edict.category.world_wars").c_str();
        case EEdictUiCategory::ColdWar:
            return UIStrings::Get(L"edict.category.cold_war").c_str();
        case EEdictUiCategory::Modern:
            return UIStrings::Get(L"edict.category.modern").c_str();
        default:
            return UIStrings::Get(L"edict.category.fallback").c_str();
        }
    }

    ETaxPolicyEventType ResolveRequiredTaxPolicyEvent(
        EGovernmentEdictType Type)
    {
        switch (Type)
        {
        case EGovernmentEdictType::LaborTaxRelief:
            return ETaxPolicyEventType::WorkerTaxStrike;
        case EGovernmentEdictType::PropertyTaxRelief:
            return ETaxPolicyEventType::PropertyTaxBacklash;
        case EGovernmentEdictType::EmergencyAusterity:
            return ETaxPolicyEventType::BudgetCrisis;
        default:
            return ETaxPolicyEventType::None;
        }
    }

    const wchar_t* GetTaxPolicyEventDisplayName(
        ETaxPolicyEventType Type)
    {
        switch (Type)
        {
        case ETaxPolicyEventType::WorkerTaxStrike:
            return UIStrings::Get(L"edict.tax_event.worker_tax_strike").c_str();
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return UIStrings::Get(L"edict.tax_event.property_tax_backlash").c_str();
        case ETaxPolicyEventType::BudgetCrisis:
            return UIStrings::Get(L"edict.tax_event.budget_crisis").c_str();
        default:
            return UIStrings::Get(L"edict.tax_event.generic").c_str();
        }
    }

    FEdictAvailabilityInfo EvaluateEdictAvailability(
        const FGovernmentEdictDefinition& Definition,
        const FGovernmentEdictState* State,
        const IMainWorldEdictReadAccess* MainWorld)
    {
        FEdictAvailabilityInfo Info;
        Info.ActivationCost = Definition.BaseCost;

        if (!Definition.Implemented)
        {
            Info.StatusText = UIStrings::Get(L"edict.status.preparing");
            Info.RequirementText =
                UIStrings::Get(L"edict.requirement.not_implemented");
            Info.ActivationCost = 0;
            return Info;
        }

        if (!State || !MainWorld)
            return Info;

        const int ActiveCitizenCount = (std::max)(
            0,
            MainWorld->GetPoliticalSnapshot().ActiveCitizenCount);
        Info.ActivationCost = EdictSystem::ResolveEdictActivationCost(
            Definition,
            ActiveCitizenCount);
        Info.Active = State->Active;
        Info.CoolingDown =
            !State->Active &&
            Definition.Mode == EGovernmentEdictMode::Active &&
            State->CooldownDays > 0;

        if (Definition.Mode == EGovernmentEdictMode::Passive)
        {
            if (State->Active)
            {
                Info.CanApply = true;
                Info.StatusText = UIStrings::Get(L"edict.status.active");
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = UIStrings::Get(L"edict.status.available");
        }
        else
        {
            if (State->Active)
            {
                Info.StatusText = UIStrings::Format(
                    L"edict.status.active_template",
                    { std::to_wstring((std::max)(0, State->RemainingDays)) });
                return Info;
            }

            if (State->CooldownDays > 0)
            {
                Info.StatusText = UIStrings::Format(
                    L"edict.status.cooldown_template",
                    { std::to_wstring(State->CooldownDays) });
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = UIStrings::Get(L"edict.status.can_apply");
        }

        const ETaxPolicyEventType RequiredTaxEvent =
            ResolveRequiredTaxPolicyEvent(Definition.Type);

        if (RequiredTaxEvent != ETaxPolicyEventType::None)
        {
            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            if (!TaxEventStatus.Active ||
                TaxEventStatus.Type != RequiredTaxEvent)
            {
                Info.CanApply = false;
                Info.StatusText =
                    UIStrings::Get(L"edict.status.requirement_missing");
                Info.RequirementText =
                    UIStrings::Get(L"edict.requirement.event_prefix") +
                    GetTaxPolicyEventDisplayName(RequiredTaxEvent);
                return Info;
            }
        }

        if (!State->Active &&
            Info.ActivationCost > MainWorld->GetNationalBudget())
        {
            Info.CanApply = false;
            Info.StatusText =
                UIStrings::Get(L"edict.status.budget_shortage");
            Info.RequirementText =
                UIStrings::Get(L"edict.status.budget_shortage");
            return Info;
        }

        return Info;
    }

    std::vector<int> CollectCategoryEntryIndices(
        EEdictUiCategory Category)
    {
        std::vector<int> Result;
        const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();

        for (int i = 0; i < static_cast<int>(Definitions.size()); ++i)
        {
            if (ResolveEdictUiCategory(Definitions[i].Era) != Category)
                continue;

            Result.push_back(i);
        }

        return Result;
    }

    int GetTaxRatePercent(
        const FTaxPolicy& TaxPolicy,
        ETaxPolicyType TaxType)
    {
        if (TaxType == ETaxPolicyType::Consumption)
            return TaxPolicy.ConsumptionRatePercent;
        if (TaxType == ETaxPolicyType::Income)
            return TaxPolicy.IncomeRatePercent;
        return TaxPolicy.PropertyRatePercent;
    }

    EdictDataProvider::FEdictTaxPolicySnapshot BuildTaxPolicySnapshot(
        const std::shared_ptr<IMainWorldEdictReadAccess>& MainWorld)
    {
        EdictDataProvider::FEdictTaxPolicySnapshot Result;
        Result.ShowPanel = EdictConstants::IsTaxPolicyPanelEnabled();

        if (!Result.ShowPanel)
            return Result;

        Result.Rows.resize(GTaxPolicyRowCount);

        const FTaxPolicy* TaxPolicy = nullptr;

        if (MainWorld)
            TaxPolicy = &MainWorld->GetTaxPolicy();

        for (int i = 0; i < GTaxPolicyRowCount; ++i)
        {
            const ETaxPolicyType TaxType = GTaxPolicyTypes[i];
            const int TaxRatePercent =
                TaxPolicy ? GetTaxRatePercent(*TaxPolicy, TaxType) : 0;

            Result.Rows[i].Text =
                std::wstring(GetTaxPolicyDisplayName(TaxType)) +
                L" " +
                FormatPercentValue(TaxRatePercent);
            Result.Rows[i].CanDecrease =
                TaxPolicy &&
                TaxRatePercent > GetTaxPolicyMinPercent(TaxType);
            Result.Rows[i].CanIncrease =
                TaxPolicy &&
                TaxRatePercent < GetTaxPolicyMaxPercent(TaxType);
        }

        if (MainWorld)
        {
            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            Result.SummaryText =
                UIStrings::Format(
                    L"edict.tax_policy.daily_income_template",
                    { FormatCurrency(MainWorld->GetLastDailyTaxIncome()) });

            if (TaxEventStatus.Active)
            {
                Result.SummaryText +=
                    UIStrings::Format(
                        L"edict.tax_policy.active_event_template",
                        {
                            TaxEventStatus.Title,
                            std::to_wstring(
                                (std::max)(0, TaxEventStatus.RemainingDays))
                        });
            }
        }
        else
        {
            Result.SummaryText =
                UIStrings::Get(L"edict.tax_policy.summary_pending");
        }

        return Result;
    }

    EdictDataProvider::FEdictDetailSnapshot BuildDetailSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::shared_ptr<IMainWorldEdictReadAccess>& MainWorld,
        int PreviewEntryIndex,
        int SelectedEntryIndex,
        const std::wstring& FeedbackMessage)
    {
        EdictDataProvider::FEdictDetailSnapshot Result;
        Result.FeedbackText = FeedbackMessage;

        const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
        const int DetailEntryIndex =
            PreviewEntryIndex >= 0 ? PreviewEntryIndex : SelectedEntryIndex;

        if (DetailEntryIndex < 0 ||
            DetailEntryIndex >= static_cast<int>(Definitions.size()))
        {
            return Result;
        }

        Result.HasSelection = true;
        const FGovernmentEdictDefinition& Definition =
            Definitions[DetailEntryIndex];
        Result.Title = Definition.DisplayName;

        const FGovernmentEdictState* State = nullptr;

        if (MainWorld)
            State = MainWorld->GetGovernmentEdictState(Definition.Type);

        const FEdictAvailabilityInfo Availability =
            EvaluateEdictAvailability(
                Definition,
                State,
                MainWorld.get());

        if (!Definition.Implemented)
        {
            Result.CostText = L"$0";
            Result.InfoText = UIStrings::Get(L"edict.detail.reference_only");
            Result.BodyText = UIStrings::Get(L"edict.detail.reference_body");
            Result.RequirementText =
                UIStrings::Get(L"edict.detail.not_implemented");
            Result.RequirementTone =
                EdictDataProvider::EEdictRequirementTone::Warning;
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Waiting;
            Result.ActionLabel = UIStrings::Get(L"edict.action.preparing");
            return Result;
        }

        const ETaxPolicyEventType RequiredTaxEvent =
            ResolveRequiredTaxPolicyEvent(Definition.Type);
        Result.BodyText = Definition.Summary;
        Result.InfoText =
            Availability.StatusText +
            L"  |  " +
            (Definition.Mode == EGovernmentEdictMode::Passive ?
                UIStrings::Get(L"edict.detail.passive") :
                UIStrings::Get(L"edict.detail.active"));
        Result.CostText = FormatCurrency(Availability.ActivationCost);

        if (!Definition.EffectText.empty())
        {
            if (!Result.BodyText.empty())
                Result.BodyText += L"\n";

            Result.BodyText += Definition.EffectText;
        }

        if (Definition.MonthlyUpkeep > 0)
        {
            Result.BodyText +=
                UIStrings::Get(L"edict.detail.monthly_upkeep_prefix") +
                FormatCurrency(Definition.MonthlyUpkeep) +
                UIStrings::Get(L"edict.detail.monthly_upkeep_suffix");
        }

        if (Definition.Mode == EGovernmentEdictMode::Active)
        {
            const int DurationMonths =
                FormatDaysToMonths(Definition.DurationDays);
            Result.InfoText +=
                UIStrings::Get(L"edict.detail.duration_prefix") +
                std::to_wstring(DurationMonths) +
                UIStrings::Get(L"edict.detail.duration_suffix");
        }

        if (Definition.CooldownDays > 0)
        {
            const int CooldownMonths =
                FormatDaysToMonths(Definition.CooldownDays);
            Result.InfoText +=
                UIStrings::Get(L"edict.detail.cooldown_prefix") +
                std::to_wstring(CooldownMonths) +
                UIStrings::Get(L"edict.detail.cooldown_suffix");
        }

        if (!Availability.CanApply &&
            !Availability.Active &&
            !Availability.CoolingDown &&
            !Availability.RequirementText.empty())
        {
            Result.RequirementText =
                UIStrings::Get(L"edict.requirement.unmet_prefix") +
                Availability.RequirementText;
        }

        if (MainWorld && RequiredTaxEvent != ETaxPolicyEventType::None)
        {
            Result.BodyText +=
                UIStrings::Get(L"edict.detail.required_event_prefix") +
                GetTaxPolicyEventDisplayName(RequiredTaxEvent);

            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            if (TaxEventStatus.Active &&
                TaxEventStatus.Type == RequiredTaxEvent)
            {
                Result.InfoText += UIStrings::Get(L"edict.detail.event_ready");
            }
            else if (!Availability.CanApply)
            {
                if (TaxEventStatus.Active)
                {
                    Result.RequirementText =
                        UIStrings::Get(
                            L"edict.requirement.current_event_prefix") +
                        TaxEventStatus.Title;
                }
                else if (Result.RequirementText.empty())
                {
                    Result.RequirementText =
                        UIStrings::Get(L"edict.requirement.unmet_prefix") +
                        std::wstring(
                            GetTaxPolicyEventDisplayName(RequiredTaxEvent)) +
                        UIStrings::Get(
                            L"edict.requirement.event_needed_suffix");
                }
            }
        }

        if (!Result.RequirementText.empty() &&
            !Availability.Active &&
            !Availability.CoolingDown)
        {
            Result.RequirementTone =
                Availability.RequirementText ==
                    UIStrings::Get(L"edict.status.budget_shortage") ?
                EdictDataProvider::EEdictRequirementTone::BudgetShortage :
                EdictDataProvider::EEdictRequirementTone::Warning;
        }
        else
        {
            Result.RequirementText.clear();
        }

        if (Availability.Active)
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Active;
            Result.ActionLabel = UIStrings::Get(L"edict.action.active");
        }
        else if (Availability.CoolingDown)
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::CoolingDown;
            Result.ActionLabel = UIStrings::Get(L"edict.action.waiting");
        }
        else if (Availability.CanApply)
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Primary;
            Result.ActionLabel = UIStrings::Get(L"edict.action.apply");
            Result.ActionEnabled = true;
        }
        else if (
            Availability.RequirementText ==
                UIStrings::Get(L"edict.status.budget_shortage"))
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::BudgetShortage;
            Result.ActionLabel =
                UIStrings::Get(L"edict.action.budget_shortage");
        }
        else if (!Result.RequirementText.empty())
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Requirement;
            Result.ActionLabel =
                UIStrings::Get(L"edict.action.requirement");
        }
        else
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Waiting;
            Result.ActionLabel = MainWorld ?
                UIStrings::Get(L"edict.action.waiting") :
                UIStrings::Get(L"edict.action.no_info");
        }

        return Result;
    }

    EdictDataProvider::FEdictCatalogSnapshot BuildCatalogSnapshot(
        const std::shared_ptr<IMainWorldEdictReadAccess>& MainWorld,
        EEdictUiCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex,
        int SelectedEntryIndex)
    {
        EdictDataProvider::FEdictCatalogSnapshot Result;
        Result.SelectedCategory = SelectedCategory;
        Result.TitleText = GetCategoryLabel(SelectedCategory);
        Result.VisibleEntryIndices.assign(GEdictSlotsPerPage, -1);
        Result.Slots.resize(GEdictSlotsPerPage);

        const auto& Definitions = EdictSystem::GetGovernmentEdictDefinitions();
        const std::vector<int> CategoryEntries =
            CollectCategoryEntryIndices(SelectedCategory);
        const int EntryCount = static_cast<int>(CategoryEntries.size());
        const int PageCount = (std::max)(
            1,
            (EntryCount + GEdictSlotsPerPage - 1) / GEdictSlotsPerPage);

        Result.PageCount = PageCount;
        Result.CurrentPage = RequestedPage;

        if (Result.CurrentPage < 0)
            Result.CurrentPage = 0;
        else if (Result.CurrentPage >= PageCount)
            Result.CurrentPage = PageCount - 1;

        const int BeginIndex = Result.CurrentPage * GEdictSlotsPerPage;
        bool HasSelectedOnPage = false;
        bool HasPreviewOnPage = false;

        for (int i = 0; i < GEdictSlotsPerPage; ++i)
        {
            const int CategoryListIndex = BeginIndex + i;

            if (CategoryListIndex < 0 || CategoryListIndex >= EntryCount)
                continue;

            const int EntryIndex = CategoryEntries[CategoryListIndex];

            if (EntryIndex == SelectedEntryIndex)
                HasSelectedOnPage = true;
            if (EntryIndex == PreviewEntryIndex)
                HasPreviewOnPage = true;

            Result.VisibleEntryIndices[i] = EntryIndex;
        }

        Result.SelectedEntryIndex = HasSelectedOnPage ? SelectedEntryIndex : -1;
        Result.PreviewEntryIndex = HasPreviewOnPage ? PreviewEntryIndex : -1;

        const int FocusedEntryIndex =
            Result.PreviewEntryIndex >= 0 ?
                Result.PreviewEntryIndex :
                Result.SelectedEntryIndex;

        for (int i = 0; i < GEdictSlotsPerPage; ++i)
        {
            const int EntryIndex = Result.VisibleEntryIndices[i];

            if (EntryIndex < 0 || EntryIndex >= static_cast<int>(Definitions.size()))
                continue;

            const auto& Definition = Definitions[EntryIndex];
            const FGovernmentEdictState* State = nullptr;

            if (MainWorld)
                State = MainWorld->GetGovernmentEdictState(Definition.Type);

            const FEdictAvailabilityInfo Availability =
                EvaluateEdictAvailability(
                    Definition,
                    State,
                    MainWorld.get());

            auto& Slot = Result.Slots[i];
            Slot.HasEntry = true;
            Slot.EntryIndex = EntryIndex;
            Slot.DisplayName = Definition.DisplayName;
            Slot.IconPath = Definition.IconPath ?
                Definition.IconPath :
                GCostIconTexture;
            Slot.Focused = FocusedEntryIndex == EntryIndex;
            Slot.Active = Availability.Active;
            Slot.CoolingDown = Availability.CoolingDown;
            Slot.Available =
                Availability.CanApply || Availability.Active;
        }

        wchar_t PageBuffer[64] = {};
        swprintf_s(
            PageBuffer,
            L"%d / %d",
            Result.CurrentPage + 1,
            Result.PageCount);
        Result.PageText = PageBuffer;
        Result.CanMovePrev = Result.CurrentPage > 0;
        Result.CanMoveNext = Result.CurrentPage < Result.PageCount - 1;

        return Result;
    }
}

namespace EdictDataProvider
{
    FEdictSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        EEdictUiCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex,
        int SelectedEntryIndex,
        const std::wstring& FeedbackMessage)
    {
        FEdictSnapshot Result;
        const auto MainWorld =
            std::dynamic_pointer_cast<IMainWorldEdictReadAccess>(World);

        Result.Catalog = BuildCatalogSnapshot(
            MainWorld,
            SelectedCategory,
            RequestedPage,
            PreviewEntryIndex,
            SelectedEntryIndex);
        Result.Detail = BuildDetailSnapshot(
            World,
            MainWorld,
            Result.Catalog.PreviewEntryIndex,
            Result.Catalog.SelectedEntryIndex,
            FeedbackMessage);
        Result.TaxPolicy = BuildTaxPolicySnapshot(MainWorld);
        return Result;
    }
}
