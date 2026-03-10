#include "EdictDataProvider.h"
#include "EdictWidget.h"
#include "../Politics/EdictSystem.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <algorithm>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    constexpr int GEdictSlotsPerPage = 14;
    constexpr int GTaxPolicyRowCount = 3;
    constexpr bool GEnableTaxPolicyPanel = false;
    constexpr const TCHAR* GCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");

    const wchar_t* GCategoryLabels[] =
    {
        L"식민지 시대",
        L"세계대전 시대",
        L"냉전 시대",
        L"현대 시대"
    };

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
        std::wstring StatusText = L"상태 확인 중";
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
        const int Index = static_cast<int>(Category);

        if (Index < 0 || Index >= static_cast<int>(std::size(GCategoryLabels)))
            return L"칙령";

        return GCategoryLabels[Index];
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
            return L"근로층 세금 파업";
        case ETaxPolicyEventType::PropertyTaxBacklash:
            return L"재산세 반발";
        case ETaxPolicyEventType::BudgetCrisis:
            return L"국고 위기";
        default:
            return L"세금 사건";
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
            Info.StatusText = L"준비 중";
            Info.RequirementText = L"아직 구현되지 않은 칙령입니다.";
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
                Info.StatusText = L"활성";
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"사용 가능";
        }
        else
        {
            if (State->Active)
            {
                Info.StatusText =
                    L"시행 중 (" +
                    std::to_wstring((std::max)(0, State->RemainingDays)) +
                    L"일 남음)";
                return Info;
            }

            if (State->CooldownDays > 0)
            {
                Info.StatusText =
                    L"재사용 대기 (" +
                    std::to_wstring(State->CooldownDays) +
                    L"일)";
                return Info;
            }

            Info.CanApply = true;
            Info.StatusText = L"시행 가능";
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
                Info.StatusText = L"조건 미충족";
                Info.RequirementText =
                    std::wstring(L"대응 사건 필요: ") +
                    GetTaxPolicyEventDisplayName(RequiredTaxEvent);
                return Info;
            }
        }

        if (!State->Active &&
            Info.ActivationCost > MainWorld->GetNationalBudget())
        {
            Info.CanApply = false;
            Info.StatusText = L"예산 부족";
            Info.RequirementText = L"예산 부족";
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
        Result.ShowPanel = GEnableTaxPolicyPanel;
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
                L"오늘 세수 " +
                FormatCurrency(MainWorld->GetLastDailyTaxIncome()) +
                L"\n다음 일일 정산부터 반영";

            if (TaxEventStatus.Active)
            {
                Result.SummaryText +=
                    L"\n활성 사건: " +
                    TaxEventStatus.Title +
                    L" (" +
                    std::to_wstring((std::max)(0, TaxEventStatus.RemainingDays)) +
                    L"일)";
            }
        }
        else
        {
            Result.SummaryText = L"세금 보고 준비 중";
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
            Result.InfoText = L"준비 중  |  참고용 칙령";
            Result.BodyText =
                L"아이콘과 시대 배치만 연결된 칙령입니다.\n"
                L"실제 효과와 적용 로직은 아직 연결되지 않았습니다.";
            Result.RequirementText =
                L"미구현: 아직 게임 로직이 연결되지 않았습니다.";
            Result.RequirementTone =
                EdictDataProvider::EEdictRequirementTone::Warning;
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Waiting;
            Result.ActionLabel = L"준비 중";
            return Result;
        }

        const ETaxPolicyEventType RequiredTaxEvent =
            ResolveRequiredTaxPolicyEvent(Definition.Type);
        Result.BodyText = Definition.Summary;
        Result.InfoText =
            Availability.StatusText +
            L"  |  " +
            (Definition.Mode == EGovernmentEdictMode::Passive ?
                L"상시 칙령" :
                L"기간 칙령");
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
                L"\n\n매달 유지비 " +
                FormatCurrency(Definition.MonthlyUpkeep) +
                L"이 소요됩니다.";
        }

        if (Definition.Mode == EGovernmentEdictMode::Active)
        {
            const int DurationMonths =
                FormatDaysToMonths(Definition.DurationDays);
            Result.InfoText +=
                L"  |  지속 " +
                std::to_wstring(DurationMonths) +
                L"개월";
        }

        if (Definition.CooldownDays > 0)
        {
            const int CooldownMonths =
                FormatDaysToMonths(Definition.CooldownDays);
            Result.InfoText +=
                L"  |  재사용 " +
                std::to_wstring(CooldownMonths) +
                L"개월";
        }

        if (!Availability.CanApply &&
            !Availability.Active &&
            !Availability.CoolingDown &&
            !Availability.RequirementText.empty())
        {
            Result.RequirementText =
                L"미충족: " + Availability.RequirementText;
        }

        if (MainWorld && RequiredTaxEvent != ETaxPolicyEventType::None)
        {
            Result.BodyText +=
                L"\n\n필요 사건: " +
                std::wstring(GetTaxPolicyEventDisplayName(RequiredTaxEvent));

            const FTaxPolicyEventStatus& TaxEventStatus =
                MainWorld->GetTaxPolicyEventStatus();

            if (TaxEventStatus.Active &&
                TaxEventStatus.Type == RequiredTaxEvent)
            {
                Result.InfoText += L"  |  대응 가능";
            }
            else if (!Availability.CanApply)
            {
                if (TaxEventStatus.Active)
                {
                    Result.RequirementText =
                        L"미충족: 현재 사건은 " + TaxEventStatus.Title;
                }
                else if (Result.RequirementText.empty())
                {
                    Result.RequirementText =
                        L"미충족: " +
                        std::wstring(
                            GetTaxPolicyEventDisplayName(RequiredTaxEvent)) +
                        L" 발생 필요";
                }
            }
        }

        if (!Result.RequirementText.empty() &&
            !Availability.Active &&
            !Availability.CoolingDown)
        {
            Result.RequirementTone =
                Availability.RequirementText == L"예산 부족" ?
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
            Result.ActionLabel = L"활성";
        }
        else if (Availability.CoolingDown)
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::CoolingDown;
            Result.ActionLabel = L"대기";
        }
        else if (Availability.CanApply)
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Primary;
            Result.ActionLabel = L"시행";
            Result.ActionEnabled = true;
        }
        else if (Availability.RequirementText == L"예산 부족")
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::BudgetShortage;
            Result.ActionLabel = L"예산 부족";
        }
        else if (!Result.RequirementText.empty())
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Requirement;
            Result.ActionLabel = L"조건 필요";
        }
        else
        {
            Result.ActionMode =
                EdictDataProvider::EEdictActionVisualMode::Waiting;
            Result.ActionLabel = MainWorld ? L"대기" : L"정보 없음";
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
