#include "BuildMenuDataProvider.h"
#include "BuildMenuQueryService.h"
#include "DescriptionFileUtils.h"
#include "../Building/BuildingCatalog.h"
#include "../Building/BuildingCategoryInfo.h"
#include "../StringUtils.h"
#include "Asset/PathManager.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr int GSlotsPerPage = 15;
    using DescriptionFileUtils::GetParentDirectoryPath;
    using DescriptionFileUtils::JoinPath;
    using DescriptionFileUtils::LoadUtf8TextFile;
    using StringUtils::SplitLines;
    using StringUtils::Trim;
    using StringUtils::Utf8ToWide;

    std::vector<std::wstring> BuildDescriptionOverrideCandidatePaths()
    {
        std::vector<std::wstring> Paths;

        if (const TCHAR* RootPath = CPathManager::FindPath("Root"))
        {
            const std::wstring RepoRoot =
                GetParentDirectoryPath(RootPath);

            if (!RepoRoot.empty())
            {
                Paths.push_back(JoinPath(
                    RepoRoot,
                    L"Client\\Building\\Data\\BuildingDescriptionOverrides.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingDescriptionOverrides.tsv"));
        }

        return Paths;
    }

    std::vector<std::string> SplitTabSeparatedLine(const std::string& Line)
    {
        std::vector<std::string> Fields;
        std::string Current;

        for (size_t Index = 0; Index < Line.size(); ++Index)
        {
            if (Line[Index] == '\t')
            {
                Fields.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Line[Index]);
        }

        Fields.push_back(Current);
        return Fields;
    }

    std::string UnescapeTsvField(const std::string& Text)
    {
        std::string Result;
        Result.reserve(Text.size());

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const char Ch = Text[Index];

            if (Ch != '\\' || Index + 1 >= Text.size())
            {
                Result.push_back(Ch);
                continue;
            }

            const char Next = Text[++Index];

            switch (Next)
            {
            case 'n':
                Result.push_back('\n');
                break;
            case 'r':
                Result.push_back('\r');
                break;
            case 't':
                Result.push_back('\t');
                break;
            case '\\':
                Result.push_back('\\');
                break;
            default:
                Result.push_back(Next);
                break;
            }
        }

        return Result;
    }

    std::unordered_map<std::string, std::wstring>
        LoadDescriptionOverrides()
    {
        std::unordered_map<std::string, std::wstring> Result;
        const std::vector<std::wstring> CandidatePaths =
            BuildDescriptionOverrideCandidatePaths();

        for (size_t PathIndex = 0;
            PathIndex < CandidatePaths.size();
            ++PathIndex)
        {
            std::string FileContent;

            if (!LoadUtf8TextFile(CandidatePaths[PathIndex], FileContent))
                continue;

            size_t Cursor = 0;

            while (Cursor <= FileContent.size())
            {
                const size_t LineEnd = FileContent.find('\n', Cursor);
                std::string Line =
                    LineEnd == std::string::npos ?
                    FileContent.substr(Cursor) :
                    FileContent.substr(Cursor, LineEnd - Cursor);

                if (!Line.empty() && Line.back() == '\r')
                    Line.pop_back();

                if (!Line.empty() && Line[0] != '#')
                {
                    const std::vector<std::string> Fields =
                        SplitTabSeparatedLine(Line);

                    if (Fields.size() >= 2 && !Fields[0].empty())
                    {
                        Result[Fields[0]] =
                            Utf8ToWide(UnescapeTsvField(Fields[1]));
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            if (!Result.empty())
                break;
        }

        return Result;
    }

    const std::unordered_map<std::string, std::wstring>&
        GetDescriptionOverrides()
    {
        static const std::unordered_map<std::string, std::wstring>
            GDescriptionOverrides = LoadDescriptionOverrides();
        return GDescriptionOverrides;
    }

    const std::wstring* FindDescriptionOverride(
        const std::string& EntryId)
    {
        const auto& Overrides = GetDescriptionOverrides();
        const auto It = Overrides.find(EntryId);
        return It != Overrides.end() ? &It->second : nullptr;
    }

    std::string BuildCatalogIconTextureKey(
        const FBuildingCatalogEntry& Entry)
    {
        return
            "BuildingCatalogIcon_" +
            Entry.Id +
            "_Gen_" +
            std::to_string(::GetRuntimeConfigGeneration());
    }

    struct FParsedDetailInfo
    {
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

    std::wstring FormatEraProgressText(
        const FEraProgressState& EraProgress)
    {
        if (!EraProgress.HasNextEra)
            return L"시대 진행: 최종 시대 도달";

        const FEraUnlockRequirement& Requirement =
            EraProgress.NextRequirement;

        switch (EraProgress.NextEra)
        {
        case EBuildingEra::WorldWars:
        {
            std::wstring Result =
                L"시대 진행: 인구 " +
                std::to_wstring(EraProgress.Population) +
                L"/" +
                std::to_wstring(Requirement.MinPopulation) +
                L" | 건물 " +
                std::to_wstring(EraProgress.TotalBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinTotalBuildings) +
                L" | 식량 " +
                std::to_wstring(EraProgress.FoodProviders) +
                L"/" +
                std::to_wstring(Requirement.MinFoodProviders);
            if (EraProgress.NextEraReady)
                Result += L" | 전환 승인 가능";
            return Result;
        }
        case EBuildingEra::ColdWar:
        {
            std::wstring Result =
                L"시대 진행: 인구 " +
                std::to_wstring(EraProgress.Population) +
                L"/" +
                std::to_wstring(Requirement.MinPopulation) +
                L" | 건물 " +
                std::to_wstring(EraProgress.TotalBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinTotalBuildings) +
                L" | 산업 " +
                std::to_wstring(EraProgress.IndustryBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinIndustryBuildings) +
                L" | 전력 " +
                std::to_wstring(EraProgress.PowerMW) +
                L"/" +
                std::to_wstring(Requirement.MinPowerMW) +
                L"MW";
            if (EraProgress.NextEraReady)
                Result += L" | 전환 승인 가능";
            return Result;
        }
        case EBuildingEra::Modern:
        {
            std::wstring Result =
                L"시대 진행: 인구 " +
                std::to_wstring(EraProgress.Population) +
                L"/" +
                std::to_wstring(Requirement.MinPopulation) +
                L" | 건물 " +
                std::to_wstring(EraProgress.TotalBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinTotalBuildings) +
                L" | 공공 " +
                std::to_wstring(EraProgress.PublicServiceBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinPublicServiceBuildings) +
                L" | 오락 " +
                std::to_wstring(EraProgress.EntertainmentBuildings) +
                L"/" +
                std::to_wstring(Requirement.MinEntertainmentBuildings) +
                L" | 전력 " +
                std::to_wstring(EraProgress.PowerMW) +
                L"/" +
                std::to_wstring(Requirement.MinPowerMW) +
                L"MW";
            if (EraProgress.NextEraReady)
                Result += L" | 전환 승인 가능";
            return Result;
        }
        case EBuildingEra::Colonial:
        default:
            break;
        }

        return L"시대 진행: -";
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

        if (Highlights.size() < 3)
        {
            const wchar_t* const ProductionStageText =
                GetProductionChainStageDisplayName(
                    Entry.ProductionChainStage);
            const std::wstring ProductionSummary =
                BuildProductionChainSummary(Entry);

            if (ProductionStageText && *ProductionStageText &&
                !ProductionSummary.empty())
            {
                Highlights.push_back(
                    L"생산 단계: " +
                    std::wstring(ProductionStageText) +
                    L" | " +
                    ProductionSummary);
            }
            else if (!ProductionSummary.empty())
            {
                Highlights.push_back(
                    L"생산 체인: " + ProductionSummary);
            }
        }

        if (Entry.Residential &&
            Highlights.size() < 3 &&
            Entry.HousingSatisfactionCap > 0)
        {
            Highlights.push_back(
                L"주거 품질: " +
                std::to_wstring(Entry.HousingSatisfactionCap));
        }

        if (Highlights.size() < 3 && Entry.BaseProducedPowerMW > 0)
        {
            Highlights.push_back(
                L"생산 전력: " +
                std::to_wstring(Entry.BaseProducedPowerMW) +
                L"MW");
        }

        if (Highlights.size() < 3 && Entry.BaseRequiredPowerMW > 0)
        {
            Highlights.push_back(
                L"필요 전력: " +
                std::to_wstring(Entry.BaseRequiredPowerMW) +
                L"MW");
        }

        if (Highlights.size() < 3 && Entry.BasePollutionOutput > 0)
        {
            Highlights.push_back(
                L"공해 배출: " +
                std::to_wstring(Entry.BasePollutionOutput));
        }

        if (Highlights.size() < 3 && Entry.BasePollutionMitigation > 0)
        {
            Highlights.push_back(
                L"공해 정화: " +
                std::to_wstring(Entry.BasePollutionMitigation));
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

        if (Highlights.size() < 3 &&
            Entry.Residential &&
            Entry.HouseholdCapacity > 0)
        {
            Highlights.push_back(
                L"수용 가구: " +
                std::to_wstring(Entry.HouseholdCapacity));
        }

        if (Highlights.size() < 3 &&
            !Entry.Residential &&
            Entry.ServiceCapacity > 0)
        {
            Highlights.push_back(
                std::wstring(
                    Entry.ServiceCapacityUsesHouseholds ?
                        L"수용 가구: " :
                        L"수용 인원: ") +
                std::to_wstring(Entry.ServiceCapacity));
        }

        if (Highlights.size() < 3 && Entry.Capacity > 0)
        {
            Highlights.push_back(
                std::wstring(
                    Entry.Residential ? L"수용 인원: " : L"필요 인력: ") +
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
            L"필요 인력:",
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
            const std::wstring Line = Trim(Lines[i]);

            if (Line.empty())
                continue;

            if (StartsWith(Line, L"설계도 비용:") ||
                StartsWith(Line, L"건설 비용:"))
                continue;

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

    std::wstring FormatCatalogCostValue(
        EBuildingCostState State,
        int Cost,
        const wchar_t* ZeroLabel)
    {
        switch (State)
        {
        case EBuildingCostState::Known:
            return Cost <= 0 ?
                (ZeroLabel ? std::wstring(ZeroLabel) : std::wstring(L"0")) :
                std::to_wstring(Cost);
        case EBuildingCostState::Unknown:
            return L"미기재";
        case EBuildingCostState::None:
        default:
            break;
        }

        return L"-";
    }

    std::wstring FormatCurrency(long long Value)
    {
        return StringUtils::FormatCurrency(Value);
    }

    std::vector<int> CollectCategoryEntryIndices(
        EBuildingCategory SelectedCategory,
        EBuildingEra CurrentEra)
    {
        std::vector<int> Result;
        const auto& Catalog = GetBuildingCatalog();

        auto IsGovernmentFinanceProxyEntry =
            [](const FBuildingCatalogEntry& Entry)
            {
                return GetEffectiveBuildMenuCategory(Entry) ==
                        EBuildingCategory::GovernmentFinance &&
                    IsCustomsOfficeCatalogEntry(Entry);
            };

        auto ShouldIncludeEntry =
            [&](const FBuildingCatalogEntry& Entry)
            {
                if (!IsBuildingEraUnlocked(CurrentEra, Entry.UnlockEra))
                    return false;

                const EBuildingCategory BuildMenuCategory =
                    GetEffectiveBuildMenuCategory(Entry);

                switch (SelectedCategory)
                {
                case EBuildingCategory::Entertainment:
                    return BuildMenuCategory ==
                        EBuildingCategory::Entertainment;
                case EBuildingCategory::LuxuryEntertainment:
                    return BuildMenuCategory ==
                        EBuildingCategory::LuxuryEntertainment;
                case EBuildingCategory::PublicService:
                    return BuildMenuCategory ==
                            EBuildingCategory::PublicService &&
                        !IsGovernmentFinanceProxyEntry(Entry);
                case EBuildingCategory::GovernmentFinance:
                    return BuildMenuCategory ==
                        EBuildingCategory::GovernmentFinance;
                default:
                    return BuildMenuCategory == SelectedCategory;
                }
            };

        if (SelectedCategory == EBuildingCategory::GovernmentFinance)
        {
            int CustomsEntryIndex = -1;

            for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
            {
                const FBuildingCatalogEntry& Entry = Catalog[i];

                if (Entry.IsHiddenFromBuildMenu ||
                    !IsBuildingEraUnlocked(CurrentEra, Entry.UnlockEra))
                {
                    continue;
                }

                if (IsGovernmentFinanceProxyEntry(Entry))
                {
                    CustomsEntryIndex = i;
                    break;
                }
            }

            for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
            {
                const FBuildingCatalogEntry& Entry = Catalog[i];

                if (Entry.IsHiddenFromBuildMenu ||
                    !IsBuildingEraUnlocked(CurrentEra, Entry.UnlockEra) ||
                    GetEffectiveBuildMenuCategory(Entry) !=
                        EBuildingCategory::GovernmentFinance ||
                    IsGovernmentFinanceProxyEntry(Entry))
                {
                    continue;
                }

                if (Entry.CategoryLocalIndex < 5)
                    Result.push_back(i);
            }

            if (CustomsEntryIndex >= 0)
                Result.push_back(CustomsEntryIndex);

            for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
            {
                const FBuildingCatalogEntry& Entry = Catalog[i];

                if (Entry.IsHiddenFromBuildMenu ||
                    !IsBuildingEraUnlocked(CurrentEra, Entry.UnlockEra) ||
                    GetEffectiveBuildMenuCategory(Entry) !=
                        EBuildingCategory::GovernmentFinance ||
                    IsGovernmentFinanceProxyEntry(Entry))
                {
                    continue;
                }

                if (Entry.CategoryLocalIndex >= 5)
                    Result.push_back(i);
            }

            return Result;
        }

        for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
        {
            if (!ShouldIncludeEntry(Catalog[i]))
                continue;

            if (Catalog[i].IsHiddenFromBuildMenu)
                continue;

            Result.push_back(i);
        }

        return Result;
    }

    BuildMenuDataProvider::FBuildMenuStatusSnapshot BuildStatusSnapshot(
        const BuildMenuDataProvider::FBuildMenuStatusRecord& StatusRecord)
    {
        BuildMenuDataProvider::FBuildMenuStatusSnapshot Result;
        Result.YearbookBodyText = BuildDefaultYearbookBodyText();
        Result.NpcCountText =
            L"NPC: " + std::to_wstring(StatusRecord.AliveNpcCount);

        if (StatusRecord.HasSimulationData)
        {
            Result.BudgetText =
                L"국가 예산: " +
                FormatCurrency(StatusRecord.NationalBudget);

            wchar_t DateBuffer[64] = {};
            swprintf_s(
                DateBuffer,
                L"날짜: %04d-%02d-%02d",
                StatusRecord.SimulationYear,
                StatusRecord.SimulationMonth,
                StatusRecord.SimulationDay);
            Result.DateText = DateBuffer;

            Result.MonthProgress = StatusRecord.SimulationMonthProgress;

            wchar_t ProgressBuffer[96] = {};
            swprintf_s(
                ProgressBuffer,
                L"월 진행: %d%%  |  %d / %d일",
                static_cast<int>(roundf(Result.MonthProgress * 100.f)),
                StatusRecord.SimulationDay,
                StatusRecord.SimulationMonthDayCount);
            Result.MonthProgressText = ProgressBuffer;
        }

        const std::wstring EraHeader =
            L"현재 시대: " +
            std::wstring(GetBuildingEraDisplayName(StatusRecord.CurrentEra)) +
            L"\n" +
            (StatusRecord.EraProgress.HasNextEra ?
                L"다음 시대: " +
                    std::wstring(GetBuildingEraDisplayName(
                        StatusRecord.EraProgress.NextEra)) :
                L"다음 시대: 없음") +
            L"\n" +
            FormatEraProgressText(StatusRecord.EraProgress);

        if (!StatusRecord.YearbookBodyText.empty())
            Result.YearbookBodyText = EraHeader + L"\n" + StatusRecord.YearbookBodyText;
        else
            Result.YearbookBodyText = EraHeader + L"\n" + Result.YearbookBodyText;

        return Result;
    }

    BuildMenuDataProvider::FBuildMenuCatalogSnapshot BuildCatalogSnapshot(
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex,
        EBuildingEra CurrentEra)
    {
        BuildMenuDataProvider::FBuildMenuCatalogSnapshot Result;
        Result.SelectedCategory = SelectedCategory;
        Result.TitleText = BuildingCategoryInfo::GetDisplayName(
            SelectedCategory);
        Result.VisibleEntryIndices.assign(GSlotsPerPage, -1);
        Result.Slots.resize(GSlotsPerPage);

        const std::vector<int> CategoryEntries =
            CollectCategoryEntryIndices(
                SelectedCategory,
                CurrentEra);
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
            Slot.IconPath = GetCatalogEntryIconPath(Entry);
            Slot.IconTextureKey = BuildCatalogIconTextureKey(Entry);
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
        Result.BlueprintCost = FormatCatalogCostValue(
            Entry.BlueprintCostState,
            Entry.BlueprintCost,
            L"없음");
        Result.ConstructionCost = FormatCatalogCostValue(
            Entry.ConstructionCostState,
            Entry.ConstructionCost,
            L"무료");
        Result.InfoText = BuildHighlightsBlockText(ParsedDetail.Highlights);
        if (const std::wstring* OverrideDescription =
                FindDescriptionOverride(Entry.Id))
        {
            Result.BodyText = *OverrideDescription;
        }
        else
        {
            Result.BodyText = ParsedDetail.Description;
        }
        return Result;
    }
}

namespace BuildMenuDataProvider
{
    FBuildMenuSnapshot BuildSnapshot(
        const std::shared_ptr<IBuildMenuQuerySource>& QuerySource,
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex)
    {
        FBuildMenuSnapshot Result;
        FBuildMenuStatusRecord StatusRecord;

        if (QuerySource)
            StatusRecord = QuerySource->QueryStatus();

        Result.Status = BuildStatusSnapshot(StatusRecord);
        Result.Catalog = BuildCatalogSnapshot(
            SelectedCategory,
            RequestedPage,
            PreviewEntryIndex,
            StatusRecord.CurrentEra);
        Result.Detail = BuildDetailSnapshot(Result.Catalog.PreviewEntryIndex);
        return Result;
    }

    FBuildMenuSnapshot BuildSnapshot(
        const std::shared_ptr<CWorld>& World,
        EBuildingCategory SelectedCategory,
        int RequestedPage,
        int PreviewEntryIndex)
    {
        return BuildSnapshot(
            BuildMenuQueryService::CreateWorldQuerySource(World),
            SelectedCategory,
            RequestedPage,
            PreviewEntryIndex);
    }
}
