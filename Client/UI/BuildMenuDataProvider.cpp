#include "BuildMenuDataProvider.h"
#include "AlmanacDataProvider.h"
#include "../Building/BuildingCatalog.h"
#include "../Building/BuildingCategoryInfo.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <string>
#include <vector>

namespace
{
    constexpr int GSlotsPerPage = 15;

    struct FParsedDetailInfo
    {
        std::wstring BlueprintCost;
        std::wstring ConstructionCost;
        std::vector<std::wstring> Highlights;
        std::wstring Description;
    };

    std::wstring BuildDefaultYearbookBodyText()
    {
        return
            L"종합 만족도: -\n"
            L"음식: -\n"
            L"보건: -\n"
            L"유흥: -\n"
            L"신앙: -\n"
            L"주거: -\n"
            L"직업: -\n"
            L"자유: -\n"
            L"치안: -\n"
            L"무주택자 수: 0명\n"
            L"실업자 수: 0명";
    }

    bool StartsWith(
        const std::wstring& Text,
        const wchar_t* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = wcslen(Prefix);
        return Text.size() >= PrefixLength &&
            Text.compare(0, PrefixLength, Prefix) == 0;
    }

    std::wstring TrimCopy(const std::wstring& Text)
    {
        size_t Begin = 0;
        size_t End = Text.size();

        while (Begin < End && iswspace(Text[Begin]))
            ++Begin;

        while (End > Begin && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Begin, End - Begin);
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        size_t Start = 0;

        while (Start <= Text.size())
        {
            const size_t End = Text.find(L'\n', Start);

            if (End == std::wstring::npos)
            {
                Lines.push_back(Text.substr(Start));
                break;
            }

            Lines.push_back(Text.substr(Start, End - Start));
            Start = End + 1;
        }

        return Lines;
    }

    std::wstring JoinLines(const std::vector<std::wstring>& Lines)
    {
        std::wstring Result;

        for (size_t i = 0; i < Lines.size(); ++i)
        {
            if (Lines[i].empty())
                continue;

            if (!Result.empty())
                Result += L"\n";

            Result += Lines[i];
        }

        return Result;
    }

    std::wstring BuildHighlightsBlockText(
        const std::vector<std::wstring>& Highlights)
    {
        std::wstring Result = L"핵심 정보";

        if (Highlights.empty())
        {
            Result += L"\n- 준비 중";
            return Result;
        }

        for (size_t i = 0; i < Highlights.size(); ++i)
        {
            Result += L"\n- ";
            Result += Highlights[i];
        }

        return Result;
    }

    void AddHighlightFallbacks(
        const FBuildingCatalogEntry& Entry,
        std::vector<std::wstring>& Highlights)
    {
        if (Highlights.size() < 3)
        {
            Highlights.push_back(
                L"해금 시대: " +
                std::wstring(GetBuildingEraDisplayName(Entry.UnlockEra)));
        }

        if (Highlights.size() < 3 &&
            Entry.RequiredEducationLevel !=
                ECitizenEducationLevel::Uneducated)
        {
            Highlights.push_back(
                L"필요 학력: " +
                std::wstring(GetCitizenEducationDisplayName(
                    Entry.RequiredEducationLevel)));
        }

        if (Highlights.size() < 3 &&
            Entry.ProducedResourceType != EResourceType::None)
        {
            Highlights.push_back(
                L"생산 자원: " +
                std::wstring(GetResourceTypeDisplayName(
                    Entry.ProducedResourceType)));
        }

        if (Entry.Residential &&
            Highlights.size() < 3 &&
            Entry.HousingSatisfactionCap > 0)
        {
            Highlights.push_back(
                L"주거 품질: " +
                std::to_wstring(Entry.HousingSatisfactionCap));
        }

        if (Highlights.size() < 3 &&
            Entry.JobSatisfactionCap > 0 &&
            Entry.JobSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"직업 품질: " +
                std::to_wstring(Entry.JobSatisfactionCap));
        }

        if (Highlights.size() < 3 &&
            Entry.FoodSatisfactionCap > 0 &&
            Entry.FoodSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"음식 품질: " +
                std::to_wstring(Entry.FoodSatisfactionCap));
        }

        if (Highlights.size() < 3 &&
            Entry.FunSatisfactionCap > 0 &&
            Entry.FunSatisfactionCap < 100)
        {
            Highlights.push_back(
                L"서비스 품질: " +
                std::to_wstring(Entry.FunSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.Capacity > 0)
        {
            Highlights.push_back(
                L"수용 인원: " +
                std::to_wstring(Entry.Capacity));
        }
    }

    FParsedDetailInfo ParseDetailInfo(
        const FBuildingCatalogEntry& Entry)
    {
        FParsedDetailInfo Result;
        std::vector<std::wstring> DescriptionLines;
        const std::vector<std::wstring> Lines = SplitLines(Entry.DetailText);
        static const wchar_t* HighlightPrefixes[] =
        {
            L"주거 품질:",
            L"직업 품질:",
            L"음식 품질:",
            L"오락 품질:",
            L"서비스 품질:",
            L"수용 인원:",
            L"수용 가구:",
            L"생산 전력:",
            L"발전량:",
            L"필요 전력:",
            L"방문객:",
            L"미관:",
            L"효과:"
        };

        for (size_t i = 0; i < Lines.size(); ++i)
        {
            const std::wstring Line = TrimCopy(Lines[i]);

            if (Line.empty())
                continue;

            if (StartsWith(Line, L"설계도 비용:"))
            {
                Result.BlueprintCost =
                    TrimCopy(Line.substr(wcslen(L"설계도 비용:")));
                continue;
            }

            if (StartsWith(Line, L"건설 비용:"))
            {
                Result.ConstructionCost =
                    TrimCopy(Line.substr(wcslen(L"건설 비용:")));
                continue;
            }

            bool HighlightLine = false;

            for (size_t PrefixIndex = 0;
                PrefixIndex <
                sizeof(HighlightPrefixes) / sizeof(HighlightPrefixes[0]);
                ++PrefixIndex)
            {
                if (!StartsWith(Line, HighlightPrefixes[PrefixIndex]))
                    continue;

                if (Result.Highlights.size() < 3)
                    Result.Highlights.push_back(Line);

                HighlightLine = true;
                break;
            }

            if (!HighlightLine)
                DescriptionLines.push_back(Line);
        }

        AddHighlightFallbacks(Entry, Result.Highlights);
        Result.Description = JoinLines(DescriptionLines);

        if (Result.Description.empty())
            Result.Description = L"세부 데이터 준비 중";

        return Result;
    }

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

    int CollectAliveNpcCount(CWorld* World)
    {
        if (!World)
            return 0;

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
        int NpcCount = 0;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        {
            for (size_t i = 0; i < OrbList.size(); ++i)
            {
                auto Orb = OrbList[i].lock();

                if (Orb && Orb->GetAlive())
                    ++NpcCount;
            }
        }

        return NpcCount;
    }

    std::vector<int> CollectCategoryEntryIndices(
        EBuildingCategory SelectedCategory)
    {
        std::vector<int> Result;
        const auto& Catalog = GetBuildingCatalog();

        for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
        {
            if (Catalog[i].Category != SelectedCategory)
                continue;

            if (Catalog[i].IsHiddenFromBuildMenu)
                continue;

            Result.push_back(i);
        }

        return Result;
    }

    BuildMenuDataProvider::FBuildMenuStatusSnapshot BuildStatusSnapshot(
        const std::shared_ptr<CWorld>& World)
    {
        BuildMenuDataProvider::FBuildMenuStatusSnapshot Result;
        Result.YearbookBodyText = BuildDefaultYearbookBodyText();

        Result.NpcCountText =
            L"NPC: " + std::to_wstring(CollectAliveNpcCount(World.get()));

        if (!World)
            return Result;

        auto MainWorldBuildMenu =
            std::dynamic_pointer_cast<IMainWorldBuildMenuAccess>(World);

        if (MainWorldBuildMenu)
        {
            Result.BudgetText =
                L"국가 예산: " +
                FormatCurrency(MainWorldBuildMenu->GetNationalBudget());

            wchar_t DateBuffer[64] = {};
            swprintf_s(
                DateBuffer,
                L"날짜: %04d-%02d-%02d",
                MainWorldBuildMenu->GetSimulationYear(),
                MainWorldBuildMenu->GetSimulationMonth(),
                MainWorldBuildMenu->GetSimulationDay());
            Result.DateText = DateBuffer;

            Result.MonthProgress =
                MainWorldBuildMenu->GetSimulationMonthProgress();

            wchar_t ProgressBuffer[96] = {};
            swprintf_s(
                ProgressBuffer,
                L"월 진행: %d%%  |  %d / %d일",
                static_cast<int>(roundf(Result.MonthProgress * 100.f)),
                MainWorldBuildMenu->GetSimulationDay(),
                MainWorldBuildMenu->GetSimulationMonthDayCount());
            Result.MonthProgressText = ProgressBuffer;
        }

        const auto MainWorldAlmanac =
            std::dynamic_pointer_cast<IMainWorldAlmanacAccess>(World);
        const AlmanacDataProvider::FAlmanacSnapshot AlmanacSnapshot =
            AlmanacDataProvider::BuildSnapshot(World, MainWorldAlmanac);
        Result.YearbookBodyText =
            AlmanacDataProvider::BuildYearbookSummaryText(AlmanacSnapshot);
        return Result;
    }

    BuildMenuDataProvider::FBuildMenuCatalogSnapshot BuildCatalogSnapshot(
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex)
    {
        BuildMenuDataProvider::FBuildMenuCatalogSnapshot Result;
        Result.SelectedCategory = SelectedCategory;
        Result.TitleText = BuildingCategoryInfo::GetDisplayName(
            SelectedCategory);
        Result.VisibleEntryIndices.assign(GSlotsPerPage, -1);
        Result.Slots.resize(GSlotsPerPage);

        const std::vector<int> CategoryEntries =
            CollectCategoryEntryIndices(SelectedCategory);
        const int EntryCount = static_cast<int>(CategoryEntries.size());
        const int PageCount = (std::max)(
            1,
            (EntryCount + GSlotsPerPage - 1) / GSlotsPerPage);

        Result.PageCount = PageCount;
        Result.CurrentPage = RequestedPage;

        if (Result.CurrentPage < 0)
            Result.CurrentPage = 0;
        else if (Result.CurrentPage >= PageCount)
            Result.CurrentPage = PageCount - 1;

        const int BeginIndex = Result.CurrentPage * GSlotsPerPage;

        for (int i = 0; i < GSlotsPerPage; ++i)
        {
            const int CategoryListIndex = BeginIndex + i;

            if (CategoryListIndex < 0 || CategoryListIndex >= EntryCount)
                continue;

            const int EntryIndex = CategoryEntries[CategoryListIndex];
            Result.VisibleEntryIndices[i] = EntryIndex;

            if (EntryIndex < 0 ||
                EntryIndex >= static_cast<int>(GetBuildingCatalog().size()))
            {
                continue;
            }

            const FBuildingCatalogEntry& Entry = GetBuildingCatalog()[EntryIndex];
            auto& Slot = Result.Slots[i];
            Slot.Enabled = true;
            Slot.EntryIndex = EntryIndex;
            Slot.DisplayName = Entry.DisplayName;
            Slot.IconPath = GetCatalogEntryIconPath(
                Entry.Category,
                Entry.CategoryLocalIndex);
        }

        bool HasPreviewOnPage = false;

        for (int i = 0; i < static_cast<int>(Result.VisibleEntryIndices.size()); ++i)
        {
            if (Result.VisibleEntryIndices[i] == PreviewEntryIndex)
            {
                HasPreviewOnPage = true;
                Result.Slots[i].Previewed = true;
                break;
            }
        }

        Result.PreviewEntryIndex = HasPreviewOnPage ? PreviewEntryIndex : -1;

        wchar_t PageBuffer[64] = {};
        swprintf_s(
            PageBuffer,
            L"%d / %d",
            Result.CurrentPage + 1,
            Result.PageCount);
        Result.PageText = PageBuffer;
        return Result;
    }

    BuildMenuDataProvider::FBuildMenuDetailSnapshot BuildDetailSnapshot(
        int PreviewEntryIndex)
    {
        BuildMenuDataProvider::FBuildMenuDetailSnapshot Result;
        const auto& Catalog = GetBuildingCatalog();

        if (PreviewEntryIndex < 0 ||
            PreviewEntryIndex >= static_cast<int>(Catalog.size()))
        {
            return Result;
        }

        const FBuildingCatalogEntry& Entry = Catalog[PreviewEntryIndex];
        const FParsedDetailInfo ParsedDetail = ParseDetailInfo(Entry);

        Result.HasSelection = true;
        Result.Title = Entry.DisplayName;
        Result.BlueprintCost =
            ParsedDetail.BlueprintCost.empty() ?
            L"-" :
            ParsedDetail.BlueprintCost;
        Result.ConstructionCost =
            ParsedDetail.ConstructionCost.empty() ?
            L"-" :
            ParsedDetail.ConstructionCost;
        Result.InfoText = BuildHighlightsBlockText(ParsedDetail.Highlights);
        Result.BodyText = ParsedDetail.Description;
        return Result;
    }
}

namespace BuildMenuDataProvider
{
    FBuildMenuSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex)
    {
        FBuildMenuSnapshot Result;
        Result.Status = BuildStatusSnapshot(World);
        Result.Catalog = BuildCatalogSnapshot(
            SelectedCategory,
            RequestedPage,
            PreviewEntryIndex);
        Result.Detail = BuildDetailSnapshot(Result.Catalog.PreviewEntryIndex);
        return Result;
    }
}
