#include "BuildingCatalog.h"
#include "Asset/PathManager.h"
#include <Windows.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <string>
#include <vector>

namespace
{
    std::string WideToUtf8(const std::wstring& Text)
    {
        if (Text.empty())
            return std::string();

        const int RequiredBytes = WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0, nullptr, nullptr);

        if (RequiredBytes <= 0)
        {
            std::string Fallback;
            Fallback.reserve(Text.size());

            for (size_t i = 0; i < Text.size(); ++i)
            {
                const wchar_t Ch = Text[i];

                if (Ch >= 0 && Ch <= 0x7f)
                    Fallback.push_back(static_cast<char>(Ch));
                else
                    Fallback.push_back('?');
            }

            return Fallback;
        }

        std::string Utf8;
        Utf8.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Utf8[0], RequiredBytes, nullptr, nullptr);
        return Utf8;
    }
} // namespace

namespace
{
    constexpr int CategoryCount = 8;

    const wchar_t* CategoryLabels[CategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"오락",
        L"미디어 및 교육",
        L"관광업",
        L"공익 서비스"
    };

    EPlacementTemplateType ResolveTemplateTypeByBuildingId(
        const std::string& BuildingId)
    {
        // 기본값은 기존 동작과 동일한 3x3 단일 마커 템플릿이다.
        struct FTemplateRule
        {
            const char* BuildingId;
            EPlacementTemplateType TemplateType;
        };

        static const std::vector<FTemplateRule> GRules =
        {
            // 필요 시 건물별 템플릿 규칙을 여기에 추가한다.
            // { "build_1_3", EPlacementTemplateType::Diamond5x5TwoMarker },
            // { "build_4_3", EPlacementTemplateType::Diamond5x5FourMarker },
            // { "build_7_5", EPlacementTemplateType::Diamond7x7ThreeMarker },
        };

        for (const FTemplateRule& Rule : GRules)
        {
            if (BuildingId == Rule.BuildingId)
                return Rule.TemplateType;
        }

        return EPlacementTemplateType::Diamond3x3SingleMarker;
    }

    struct FExternalCatalogRecord
    {
        EBuildingCategory Category = EBuildingCategory::Infrastructure;
        int LocalIndex = 0;
        std::wstring DisplayName;
        std::wstring DetailText;
    };

    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0);

        if (RequiredCount <= 0)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &WideText[0], RequiredCount);
        return WideText;
    }

    std::wstring TrimTrailingSeparators(const std::wstring& Path)
    {
        std::wstring Result = Path;

        while (!Result.empty() &&
            (Result.back() == L'\\' || Result.back() == L'/'))
        {
            Result.pop_back();
        }

        return Result;
    }

    std::wstring GetParentDirectoryPath(const std::wstring& Path)
    {
        const std::wstring TrimmedPath = TrimTrailingSeparators(Path);
        const size_t SlashPos = TrimmedPath.find_last_of(L"\\/");

        if (SlashPos == std::wstring::npos)
            return std::wstring();

        return TrimmedPath.substr(0, SlashPos);
    }

    std::wstring JoinPath(
        const std::wstring& BasePath,
        const wchar_t* Suffix)
    {
        if (BasePath.empty())
            return std::wstring(Suffix ? Suffix : L"");

        std::wstring Result = TrimTrailingSeparators(BasePath);

        if (!Suffix || !*Suffix)
            return Result;

        Result += L"\\";
        Result += Suffix;
        return Result;
    }

    std::vector<std::wstring> BuildCatalogDataCandidatePaths()
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
                    L"Client\\Building\\Data\\BuildingCatalog.tsv"));
            }
        }

        if (const TCHAR* AssetPath = CPathManager::FindPath("Asset"))
        {
            Paths.push_back(JoinPath(
                AssetPath,
                L"Data\\BuildingCatalog.tsv"));
        }

        return Paths;
    }

    bool LoadUtf8TextFile(
        const std::wstring& FullPath,
        std::string& OutText)
    {
        OutText.clear();

        FILE* File = nullptr;
        _wfopen_s(&File, FullPath.c_str(), L"rb");

        if (!File)
            return false;

        fseek(File, 0, SEEK_END);
        const long FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        if (FileSize < 0)
        {
            fclose(File);
            return false;
        }

        OutText.resize(static_cast<size_t>(FileSize));

        if (FileSize > 0)
        {
            const size_t ReadSize = fread(
                &OutText[0],
                1,
                static_cast<size_t>(FileSize),
                File);

            if (ReadSize != static_cast<size_t>(FileSize))
            {
                fclose(File);
                OutText.clear();
                return false;
            }
        }

        fclose(File);

        if (OutText.size() >= 3 &&
            static_cast<unsigned char>(OutText[0]) == 0xEF &&
            static_cast<unsigned char>(OutText[1]) == 0xBB &&
            static_cast<unsigned char>(OutText[2]) == 0xBF)
        {
            OutText.erase(0, 3);
        }

        return true;
    }

    std::vector<std::string> SplitTabSeparatedLine(const std::string& Line)
    {
        std::vector<std::string> Fields;
        std::string Current;

        for (size_t Index = 0; Index < Line.size(); ++Index)
        {
            const char Ch = Line[Index];

            if (Ch == '\t')
            {
                Fields.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Ch);
        }

        Fields.push_back(Current);
        return Fields;
    }

    std::string UnescapeCatalogField(const std::string& Text)
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

    bool TryParseBuildingCategoryKey(
        const std::string& Key,
        EBuildingCategory& OutCategory)
    {
        if (Key == "Infrastructure")
            OutCategory = EBuildingCategory::Infrastructure;
        else if (Key == "FoodResource")
            OutCategory = EBuildingCategory::FoodResource;
        else if (Key == "Industry")
            OutCategory = EBuildingCategory::Industry;
        else if (Key == "Housing")
            OutCategory = EBuildingCategory::Housing;
        else if (Key == "Entertainment")
            OutCategory = EBuildingCategory::Entertainment;
        else if (Key == "MediaEducation")
            OutCategory = EBuildingCategory::MediaEducation;
        else if (Key == "Tourism")
            OutCategory = EBuildingCategory::Tourism;
        else if (Key == "PublicService")
            OutCategory = EBuildingCategory::PublicService;
        else
            return false;

        return true;
    }

    void ResolveCategoryDefaultFlags(
        EBuildingCategory Category,
        bool& OutResidential,
        bool& OutFoodProvider,
        bool& OutEntertainmentProvider)
    {
        OutResidential = false;
        OutFoodProvider = false;
        OutEntertainmentProvider = false;

        switch (Category)
        {
        case EBuildingCategory::Housing:
            OutResidential = true;
            break;
        case EBuildingCategory::FoodResource:
            OutFoodProvider = true;
            break;
        case EBuildingCategory::Entertainment:
            OutEntertainmentProvider = true;
            break;
        default:
            break;
        }
    }

    bool LoadExternalCatalogRecords(
        std::vector<FExternalCatalogRecord>& OutRecords)
    {
        OutRecords.clear();
        const std::vector<std::wstring> CandidatePaths =
            BuildCatalogDataCandidatePaths();

        for (size_t PathIndex = 0; PathIndex < CandidatePaths.size(); ++PathIndex)
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

                    if (Fields.size() >= 4)
                    {
                        EBuildingCategory Category;
                        char* EndPtr = nullptr;
                        const long ParsedIndex =
                            strtol(Fields[1].c_str(), &EndPtr, 10);

                        if (TryParseBuildingCategoryKey(Fields[0], Category) &&
                            EndPtr && *EndPtr == 0 &&
                            ParsedIndex >= 0)
                        {
                            FExternalCatalogRecord Record;
                            Record.Category = Category;
                            Record.LocalIndex =
                                static_cast<int>(ParsedIndex);
                            Record.DisplayName = Utf8ToWide(
                                UnescapeCatalogField(Fields[2]));
                            Record.DetailText = Utf8ToWide(
                                UnescapeCatalogField(Fields[3]));
                            OutRecords.push_back(Record);
                        }
                    }
                }

                if (LineEnd == std::string::npos)
                    break;

                Cursor = LineEnd + 1;
            }

            if (!OutRecords.empty())
            {
                std::stable_sort(
                    OutRecords.begin(),
                    OutRecords.end(),
                    [](const FExternalCatalogRecord& A,
                        const FExternalCatalogRecord& B)
                    {
                        if (A.Category != B.Category)
                        {
                            return static_cast<int>(A.Category) <
                                static_cast<int>(B.Category);
                        }

                        return A.LocalIndex < B.LocalIndex;
                    });
                return true;
            }
        }

        OutputDebugStringW(
            L"[BuildingCatalog] Failed to load BuildingCatalog.tsv\n");
        return false;
    }

    bool NameContains(
        const std::wstring& Name,
        const wchar_t* Pattern)
    {
        return Pattern && Name.find(Pattern) != std::wstring::npos;
    }

    void AddPoliticalSignal(
        FBuildingCatalogEntry& Entry,
        EPoliticalAxis Axis,
        EPoliticalStance FavoredStance,
        float Strength,
        EPoliticalScope Scope = EPoliticalScope::Global)
    {
        if (Strength <= 0.f)
            return;

        FPoliticalSignalDef Signal;
        Signal.Axis = Axis;
        Signal.FavoredStance = FavoredStance;
        Signal.Strength = Strength;
        Signal.Scope = Scope;
        Entry.PoliticalSignals.push_back(Signal);
    }

    std::vector<std::wstring> SplitDetailLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring CurrentLine;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'\r')
                continue;

            if (Ch == L'\n')
            {
                Lines.push_back(CurrentLine);
                CurrentLine.clear();
                continue;
            }

            CurrentLine.push_back(Ch);
        }

        if (!CurrentLine.empty() || Text.empty())
            Lines.push_back(CurrentLine);

        return Lines;
    }

    bool IsCatalogIdOneOf(
        const std::string& Id,
        std::initializer_list<const char*> Candidates)
    {
        for (const char* Candidate : Candidates)
        {
            if (Candidate && Id == Candidate)
                return true;
        }

        return false;
    }

    EBuildingEra ParseUnlockEra(const std::wstring& DetailText)
    {
        if (DetailText.find(L"식민지 시대") != std::wstring::npos)
            return EBuildingEra::Colonial;

        if (DetailText.find(L"세계대전 시대") != std::wstring::npos)
            return EBuildingEra::WorldWars;

        if (DetailText.find(L"냉전 시대") != std::wstring::npos)
            return EBuildingEra::ColdWar;

        if (DetailText.find(L"현대 시대") != std::wstring::npos)
            return EBuildingEra::Modern;

        return EBuildingEra::Colonial;
    }

    ECitizenEducationLevel ParseRequiredEducationLevel(
        const std::wstring& DetailText)
    {
        if (DetailText.find(L"대졸") != std::wstring::npos)
            return ECitizenEducationLevel::College;

        if (DetailText.find(L"고졸") != std::wstring::npos)
            return ECitizenEducationLevel::HighSchool;

        return ECitizenEducationLevel::Uneducated;
    }

    EBuildingHousingClass ResolveHousingClass(const std::string& Id)
    {
        if (IsCatalogIdOneOf(Id,
            {
                "build_4_1",
                "build_4_3",
                "build_4_6",
                "build_4_8",
                "build_4_9"
            }))
        {
            return EBuildingHousingClass::Collective;
        }

        if (IsCatalogIdOneOf(Id,
            {
                "build_4_2",
                "build_4_4",
                "build_4_5",
                "build_4_11",
                "build_4_12"
            }))
        {
            return EBuildingHousingClass::Elite;
        }

        if (IsCatalogIdOneOf(Id, { "build_4_7", "build_4_10" }))
            return EBuildingHousingClass::Standard;

        return EBuildingHousingClass::None;
    }

    EBuildingLeisureClass ResolveLeisureClass(
        EBuildingCategory Category,
        const std::string& Id)
    {
        if (Category != EBuildingCategory::Entertainment)
            return EBuildingLeisureClass::None;

        if (IsCatalogIdOneOf(Id, { "build_5_13", "build_5_14", "build_5_24" }))
            return EBuildingLeisureClass::Cultural;

        if (IsCatalogIdOneOf(Id,
            {
                "build_5_16",
                "build_5_17",
                "build_5_18",
                "build_5_19",
                "build_5_20",
                "build_5_25"
            }))
        {
            return EBuildingLeisureClass::Luxury;
        }

        return EBuildingLeisureClass::General;
    }

    ETouristPreference ParsePrimaryTouristPreference(
        const std::wstring& DetailText)
    {
        const size_t MarkerPos = DetailText.find(L"선호 관광객:");

        if (MarkerPos == std::wstring::npos)
            return ETouristPreference::None;

        const std::wstring TouristLine =
            DetailText.substr(MarkerPos);

        if (TouristLine.find(L"문화") != std::wstring::npos)
            return ETouristPreference::Cultural;
        if (TouristLine.find(L"아동") != std::wstring::npos)
            return ETouristPreference::Family;
        if (TouristLine.find(L"배낭여행") != std::wstring::npos)
            return ETouristPreference::Backpacker;
        if (TouristLine.find(L"휴양") != std::wstring::npos)
            return ETouristPreference::Relaxation;
        if (TouristLine.find(L"스릴중독") != std::wstring::npos)
            return ETouristPreference::ThrillSeeker;
        if (TouristLine.find(L"유명인") != std::wstring::npos)
            return ETouristPreference::Celebrity;

        return ETouristPreference::None;
    }

    std::vector<std::wstring> ExtractUpgradeHints(
        const std::wstring& DetailText)
    {
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines =
            SplitDetailLines(DetailText);
        bool Capture = false;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring& Line = Lines[Index];

            if (Line.find(L"업그레이드") == 0)
            {
                Capture = true;
                continue;
            }

            if (!Capture)
                continue;

            if (Line.empty())
                break;

            if (Line[0] != L'-')
                break;

            Result.push_back(Line.substr(1));
        }

        return Result;
    }

    EResourceType ResolveProducedResourceType(
        const FBuildingCatalogEntry& Entry)
    {
        if (Entry.Residential ||
            Entry.BuildingKind == EPlacementBuildingKind::Harbor ||
            Entry.BuildingKind == EPlacementBuildingKind::TransportOffice)
        {
            return EResourceType::None;
        }

        if (Entry.FoodProvider)
            return EResourceType::Food;

        if (Entry.Category == EBuildingCategory::FoodResource)
            return EResourceType::RawGoods;

        if (Entry.Category == EBuildingCategory::Industry)
            return EResourceType::ManufacturedGoods;

        return EResourceType::None;
    }

    EResourceType ResolveVisitConsumptionResourceType(
        const FBuildingCatalogEntry& Entry)
    {
        return Entry.FoodProvider ?
            EResourceType::Food :
            EResourceType::None;
    }

    void AssignPoliticalSignals(FBuildingCatalogEntry& Entry)
    {
        Entry.PoliticalSignals.clear();

        const std::wstring& Name = Entry.DisplayName;

        switch (Entry.Category)
        {
        case EBuildingCategory::Infrastructure:
            if (Name == L"항구" || Name == L"화물창고" || Name == L"창고")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 5.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.5f);
            }
            else if (Name == L"운송업자 사무소" || Name == L"건설 사무소")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.0f, EPoliticalScope::Worker);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (NameContains(Name, L"풍력") || NameContains(Name, L"태양광"))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 3.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 1.0f);
            }
            else if (NameContains(Name, L"발전소") || NameContains(Name, L"원자력"))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 3.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 1.5f);
            }
            break;

        case EBuildingCategory::FoodResource:
            if (NameContains(Name, L"광산") ||
                NameContains(Name, L"석유") ||
                NameContains(Name, L"정유") ||
                Name == L"벌목소")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 4.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 2.0f, EPoliticalScope::Worker);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 2.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 1.5f);
            }
            break;

        case EBuildingCategory::Industry:
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 3.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::EnvironmentIndustry,
                EPoliticalStance::Right, 3.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 1.5f, EPoliticalScope::Worker);
            break;

        case EBuildingCategory::Housing:
            if (Entry.HousingClass == EBuildingHousingClass::Collective)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 4.0f, EPoliticalScope::Resident);
            }
            else if (Entry.HousingClass == EBuildingHousingClass::Elite)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.5f, EPoliticalScope::Resident);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 2.0f, EPoliticalScope::Resident);
            }
            break;

        case EBuildingCategory::Entertainment:
            if (Entry.LeisureClass == EBuildingLeisureClass::Luxury)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 2.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 1.5f);
            }
            else if (Entry.LeisureClass == EBuildingLeisureClass::Cultural)
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 3.0f);
            }
            break;

        case EBuildingCategory::MediaEducation:
            if (Name == L"영묘")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Left, 2.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 1.5f);
            }
            else if (Name == L"격려용 광고판" || Name == L"격려용 동상")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (Name == L"핵 개발 프로그램")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Right, 3.0f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Right, 2.0f);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Left, 3.5f);
            }
            break;

        case EBuildingCategory::Tourism:
            AddPoliticalSignal(
                Entry, EPoliticalAxis::Economy,
                EPoliticalStance::Left, 2.5f);
            AddPoliticalSignal(
                Entry, EPoliticalAxis::IntellectualConservative,
                EPoliticalStance::Right, 1.0f);
            break;

        case EBuildingCategory::PublicService:
            if (Name == L"예배당" || Name == L"교회" || Name == L"성당")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Left, 3.5f);
            }
            else if (Name == L"소방서")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::ReligionMilitarism,
                    EPoliticalStance::Right, 2.0f);
            }
            else if (Name == L"쇼핑몰")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 3.0f);
            }
            else if (Name == L"쓰레기장" || Name == L"폐기물 처리장")
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::EnvironmentIndustry,
                    EPoliticalStance::Left, 2.5f);
            }
            else
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 2.5f);
            }
            break;

        default:
            break;
        }
    }

    const TCHAR* const GInfrastructureIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\T_ICO_Road.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\T_ICO_Demolish.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_dock.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_constructionOffice.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_teamstersOffice.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_landing.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_storageQuay.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_DLC_warehouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_electricSubstation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_powerPlant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_parkDeck.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_busGarage.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_busStop.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\T_ICO_Tunnel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_NuclearPowerPlant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_metroStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_telefericStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_windFarm.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\ICO_OffshoreWindTurbine.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_solarPowerPlant.png")
    };

    const TCHAR* const GFoodResourceIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_CoconutHarvester.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_loggingCamp.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_wharf.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_plantation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_ranch.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_ranch.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_mine.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_oilWell.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_fishFarm.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_oilRig.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_hydrophobicPlantation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_factoryRanch.png")
    };

    const TCHAR* const GIndustryIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_automatedMine.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_lumberMill.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_rumDistillery.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_tannery.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_cannery.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_creamery.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_cigarFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_shipyard.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_steelMill.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_textileMill.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_weaponsFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_chocolateFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_furnitureFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_jewelryFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_plasticPlant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_vehicleFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_electronicsFactory.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_fashionCompany.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_pharmaceuticalCompany.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_juicer.png")
    };

    const TCHAR* const GHousingIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\ICO_Flophouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_countryHouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_bunkhouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_mansion.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_house.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\ICO_Flophouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_apartment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_tenment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_Conventillo.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_modernApartment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_modernMansion.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\ICO_ColdWarMansion.png")
    };

    const TCHAR* const GEntertainmentIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_tavern.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_circus.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_theatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_botanicalGarden.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_funFairPier.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_restaurant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_arcade.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_fastFoodJoint.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_movieTheatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_aquaPark.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_rollerCoaster.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_stadium.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_theatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_operaHouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_cabaret.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_casino.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_cocktailBar.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_golfcourse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_gourmetRestaurant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_nightClub.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_snorkelBay.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_beachResort.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_hangGlider.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_museumOfModernArt.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_yachtClub.png")
    };

    const TCHAR* const GMediaEducationIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_newspaper.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_library.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_highschool.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_college.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_radiostation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_childhoodMuseum.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_mausoleum.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_TVStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\T_ICO_billboard.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\T_ICO_inspiringStatue.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_researchLab.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_spaceProgram.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_nuclearProgram.png")
    };

    const TCHAR* const GTourismIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_turistDock.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_cabin.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_beachVilla.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_cabanaVillage.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_motel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_hotel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_scenicOutlook.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_souvenirShop.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_touristOffice.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_ancientRuins.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_ethnicEnclave.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_airport.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_economyHotel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_familyResort.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_luxuryHotel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_skyscraperHotel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_cruiseShip.png")
    };

    const TCHAR* const GPublicServiceIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_chapel.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_grocery.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_church.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_clinic.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_fireStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_cathedral.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_hospital.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_asylum.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_garbageDump.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_shoppingMall.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_wasteTreatmentFacility.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsDLC\\T_ICO_DLC_rehabCenter.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsDLC\\T_ICO_DLC_beautyFarm.png")
    };

    const TCHAR* PickIcon(const TCHAR* const* Icons, int Count, int LocalIndex)
    {
        if (!Icons || Count <= 0)
            return nullptr;

        int Index = LocalIndex % Count;

        if (Index < 0)
            Index += Count;

        return Icons[Index];
    }

    const wchar_t* GetCatalogEntryIconPathInternal(
        int CategoryIndex,
        int CategoryLocalIndex)
    {
        switch (CategoryIndex)
        {
        case 0:
            return PickIcon(
                GInfrastructureIcons,
                _countof(GInfrastructureIcons),
                CategoryLocalIndex);
        case 1:
            return PickIcon(
                GFoodResourceIcons,
                _countof(GFoodResourceIcons),
                CategoryLocalIndex);
        case 2:
            return PickIcon(
                GIndustryIcons,
                _countof(GIndustryIcons),
                CategoryLocalIndex);
        case 3:
            return PickIcon(
                GHousingIcons,
                _countof(GHousingIcons),
                CategoryLocalIndex);
        case 4:
            return PickIcon(
                GEntertainmentIcons,
                _countof(GEntertainmentIcons),
                CategoryLocalIndex);
        case 5:
            return PickIcon(
                GMediaEducationIcons,
                _countof(GMediaEducationIcons),
                CategoryLocalIndex);
        case 6:
            return PickIcon(
                GTourismIcons,
                _countof(GTourismIcons),
                CategoryLocalIndex);
        case 7:
            return PickIcon(
                GPublicServiceIcons,
                _countof(GPublicServiceIcons),
                CategoryLocalIndex);
        default:
            break;
        }

        return nullptr;
    }
} // namespace

const wchar_t* GetCatalogEntryIconPath(
    EBuildingCategory Category,
    int CategoryLocalIndex)
{
    return GetCatalogEntryIconPathInternal(
        static_cast<int>(Category), CategoryLocalIndex);
}

const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog()
{
    static std::vector<FBuildingCatalogEntry> Catalog = []()
    {
        static const int HousingCaps[] =
        {
            10, // 판잣집
            20, // 시골 주택
            30, // 합숙소
            75, // 저택
            50, // 단독주택
            25, // 간이 숙박소
            60, // 아파트
            40, // 서민 아파트
            45, // 공동주택
            70, // 현대식 아파트
            85, // 현대식 저택
            90  // 보안 저택
        };

        static const int EntertainmentFunCaps[] =
        {
            45, // 주점
            70, // 서커스
            40, // 버스커
            55, // 식물원
            68, // 유원지 부두
            60, // 레스토랑
            58, // 오락실
            52, // 패스트푸드 체인점
            65, // 영화관
            72, // 아쿠아 파크
            78, // 롤러코스터
            80, // 경기장
            60, // 극장
            75, // 오페라 극장
            65, // 카바레
            70, // 카지노
            70, // 칵테일 바
            60, // 골프 코스
            80, // 고급 레스토랑
            65, // 나이트클럽
            65, // 스노클 베이
            65, // 해변 휴양지
            75, // 행글라이딩
            70, // 현대 미술관
            95  // 요트 클럽
        };

        std::vector<FBuildingCatalogEntry> Entries;
        std::vector<FExternalCatalogRecord> SourceRecords;

        if (!LoadExternalCatalogRecords(SourceRecords))
            return Entries;

        Entries.reserve(SourceRecords.size());

        for (size_t RecordIndex = 0;
            RecordIndex < SourceRecords.size();
            ++RecordIndex)
        {
            const FExternalCatalogRecord& Record = SourceRecords[RecordIndex];
            const int CategoryIndex = static_cast<int>(Record.Category);
            const int i = Record.LocalIndex;

            if (CategoryIndex < 0 || CategoryIndex >= CategoryCount || i < 0)
                continue;

            bool Residential = false;
            bool FoodProvider = false;
            bool EntertainmentProvider = false;
            ResolveCategoryDefaultFlags(
                Record.Category,
                Residential,
                FoodProvider,
                EntertainmentProvider);

            FBuildingCatalogEntry Entry;
            Entry.Id = "build_" + std::to_string(CategoryIndex + 1) +
                "_" + std::to_string(i + 1);
            Entry.DisplayName = Record.DisplayName;
            Entry.CategoryName = CategoryLabels[CategoryIndex];
            Entry.DetailText =
                Record.DetailText.empty() ?
                L"세부 데이터 준비 중" :
                Record.DetailText;
            Entry.Residential = Residential;
            Entry.FoodProvider = FoodProvider;
            Entry.EntertainmentProvider = EntertainmentProvider;
            Entry.Category = Record.Category;
            Entry.CategoryLocalIndex = i;
            Entry.TemplateType =
                ResolveTemplateTypeByBuildingId(Entry.Id);

            if (Entry.Id == "build_1_3")
                Entry.BuildingKind = EPlacementBuildingKind::Harbor;
            else if (Entry.Id == "build_1_5")
                Entry.BuildingKind = EPlacementBuildingKind::TransportOffice;
            else
                Entry.BuildingKind =
                    (Residential || (i % 2 == 0)) ?
                    EPlacementBuildingKind::BuildingA :
                    EPlacementBuildingKind::BuildingB;

            if (Residential)
            {
                Entry.Capacity = 20 + (i % 5) * 8 + (i / 5) * 4;
            }
            else
            {
                Entry.Capacity = 15 + (i % 6) * 6 + (i / 6) * 3;
            }

            Entry.HousingSatisfactionCap = 100;
            Entry.JobSatisfactionCap = 100;
            Entry.FoodSatisfactionCap = 100;
            Entry.FunSatisfactionCap = 100;

            if (Record.Category == EBuildingCategory::Infrastructure)
            {
                Entry.JobSatisfactionCap = (std::min)(
                    85, 45 + (i % 7) * 5 + (i / 7) * 3);
            }
            else if (Record.Category == EBuildingCategory::FoodResource)
            {
                Entry.FoodSatisfactionCap = (std::min)(
                    82, 35 + (i % 7) * 6 + (i / 7) * 4);
                Entry.JobSatisfactionCap = (std::min)(
                    70, 40 + (i % 5) * 5 + (i / 5) * 2);
            }
            else if (Record.Category == EBuildingCategory::Industry)
            {
                Entry.JobSatisfactionCap = (std::min)(
                    90, 50 + (i % 8) * 5 + (i / 8) * 4);
            }
            else if (Record.Category == EBuildingCategory::Housing)
            {
                if (i < static_cast<int>(
                    sizeof(HousingCaps) / sizeof(HousingCaps[0])))
                {
                    Entry.HousingSatisfactionCap = HousingCaps[i];
                }
            }
            else if (Record.Category == EBuildingCategory::Entertainment)
            {
                if (i < static_cast<int>(
                    sizeof(EntertainmentFunCaps) /
                    sizeof(EntertainmentFunCaps[0])))
                {
                    Entry.FunSatisfactionCap =
                        EntertainmentFunCaps[i];
                }
            }
            else if (Record.Category == EBuildingCategory::PublicService)
            {
                Entry.JobSatisfactionCap = (std::min)(
                    88, 48 + (i % 6) * 5 + (i / 6) * 4);
            }

            if (Record.Category == EBuildingCategory::Entertainment &&
                IsCatalogIdOneOf(
                    Entry.Id,
                    { "build_5_6", "build_5_8", "build_5_19" }))
            {
                Entry.FoodProvider = true;

                if (Entry.Id == "build_5_6")
                    Entry.FoodSatisfactionCap = 65;
                else if (Entry.Id == "build_5_19")
                    Entry.FoodSatisfactionCap = 75;
                else
                    Entry.FoodSatisfactionCap = 55;
            }
            else if (Record.Category == EBuildingCategory::PublicService &&
                IsCatalogIdOneOf(
                    Entry.Id,
                    { "build_8_2", "build_8_10" }))
            {
                Entry.FoodProvider = true;
                Entry.FoodSatisfactionCap =
                    (Entry.Id == "build_8_10") ? 70 : 55;
            }

            if (Record.Category == EBuildingCategory::PublicService &&
                IsCatalogIdOneOf(
                    Entry.Id,
                    { "build_8_12", "build_8_13" }))
            {
                Entry.EntertainmentProvider = true;
                Entry.FunSatisfactionCap = 68;
            }

            Entry.UnlockEra = ParseUnlockEra(Entry.DetailText);
            Entry.RequiredEducationLevel =
                ParseRequiredEducationLevel(Entry.DetailText);
            Entry.HousingClass = ResolveHousingClass(Entry.Id);
            Entry.LeisureClass =
                ResolveLeisureClass(Entry.Category, Entry.Id);
            Entry.PrimaryTouristPreference =
                ParsePrimaryTouristPreference(Entry.DetailText);
            Entry.ProducedResourceType =
                ResolveProducedResourceType(Entry);
            Entry.VisitConsumptionResourceType =
                ResolveVisitConsumptionResourceType(Entry);
            Entry.CanExportStoredResources =
                Entry.BuildingKind == EPlacementBuildingKind::Harbor;
            Entry.SupportsTeamsterPickup =
                Entry.ProducedResourceType != EResourceType::None &&
                !Entry.Residential &&
                !Entry.CanExportStoredResources &&
                Entry.BuildingKind !=
                    EPlacementBuildingKind::TransportOffice;
            Entry.SupportsImmigration =
                Entry.DetailText.find(L"이민/이주 처리") !=
                std::wstring::npos;
            Entry.UpgradeHints = ExtractUpgradeHints(Entry.DetailText);

            AssignPoliticalSignals(Entry);
            Entries.push_back(Entry);
        }

        // ── UI 동작 플래그 후처리 ─────────────────────────────────────────
        // DisplayName 기반 동작 분기를 외부 레이어(BuildMenuWidget 등)가
        // 직접 비교하지 않도록 카탈로그 구성 시점에 플래그로 굳힌다.
        for (auto& Entry : Entries)
        {
            if (Entry.Id == "build_1_2")
                Entry.IsDemolish = true;

            if (Entry.Id == "build_4_1")
                Entry.IsHiddenFromBuildMenu = true;
        }

        return Entries;
    }();

    return Catalog;
}

const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId)
{
    const auto& Catalog = GetBuildingCatalog();
    const auto It = std::find_if(
        Catalog.begin(), Catalog.end(),
        [&](const FBuildingCatalogEntry& Entry)
        {
            return Entry.Id == EntryId;
        });

    return It != Catalog.end() ? &(*It) : nullptr;
}

std::string GetCatalogEntryIconPathUtf8(EBuildingCategory Category, int CategoryLocalIndex)
{
    const wchar_t* IconPath = GetCatalogEntryIconPath(Category, CategoryLocalIndex);

    if (!IconPath)
        return std::string();

    return WideToUtf8(std::wstring(IconPath));
}
