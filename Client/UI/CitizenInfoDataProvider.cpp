#include "CitizenInfoDataProvider.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    struct FBuildingUiSnapshot
    {
        std::shared_ptr<CPlacementAreaObject> Building;
        const FBuildingCatalogEntry* CatalogEntry = nullptr;
        std::wstring ObjectName;
        std::wstring DisplayName;
        std::wstring CategoryName;
        std::wstring DetailText;
        std::wstring RequiredPowerText;
        std::wstring ProducedPowerText;
        std::wstring JobQualityText;
        std::wstring ServiceQualityText;
        std::wstring HousingQualityText;
        std::wstring WealthRequirementText;
        std::wstring TouristPreferenceText;
        std::wstring EffectText;
        std::wstring NoteText;
        std::wstring ServiceCapacityText;
        std::vector<std::wstring> NarrativeLines;
        std::vector<std::wstring> UpgradeHints;
        std::vector<std::wstring> OperationModes;
        std::vector<std::wstring> WarehouseSlotLines;
        std::vector<std::string> Residents;
        std::vector<std::string> AssignedEmployees;
        std::vector<std::string> WorkingEmployees;
        std::vector<std::string> AssignedVisitors;
        std::vector<std::string> ArrivedVisitors;
        std::vector<std::string> IncomingVisitors;
        int Capacity = 0;
        int BudgetLevel = 3;
        int DaysInMonth = 30;
        int MonthlyWageCost = 0;
        int MonthlyUpkeepCost = 0;
        int DailyWageCost = 0;
        int DailyUpkeepCost = 0;
        int HousingCap = 100;
        int JobCap = 100;
        int FoodCap = 100;
        int FunCap = 100;
        int ResourceStock = 0;
        int ExportableStock = 0;
        int MaxResourceStock = 0;
        int ServiceCapacity = 0;
        int TotalProducedPowerMW = 0;
        int TotalRequiredPowerMW = 0;
        float BudgetScale = 1.f;
        float AccessibilityScore = 0.f;
        float HarborShipProgressPercent = 0.f;
        bool Residential = false;
        bool WorkProvider = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        bool UsesResourceStock = false;
        bool Harbor = false;
        bool Warehouse = false;
    };

    constexpr int GBuildingTabCount = 5;

    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);

        if (RequiredCount <= 1)
            return std::wstring(Text.begin(), Text.end());

        std::wstring WideText;
        WideText.resize(RequiredCount - 1);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(),
            static_cast<int>(Text.size()),
            &WideText[0], RequiredCount - 1);
        return WideText;
    }

    std::wstring Trim(const std::wstring& Text)
    {
        size_t Start = 0;

        while (Start < Text.size() && iswspace(Text[Start]))
            ++Start;

        size_t End = Text.size();

        while (End > Start && iswspace(Text[End - 1]))
            --End;

        return Text.substr(Start, End - Start);
    }

    bool StartsWith(const std::wstring& Text, const wchar_t* Prefix)
    {
        if (!Prefix)
            return false;

        const size_t PrefixLength = wcslen(Prefix);

        if (Text.size() < PrefixLength)
            return false;

        return Text.compare(0, PrefixLength, Prefix) == 0;
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring Current;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (Ch == L'\r')
                continue;

            if (Ch == L'\n')
            {
                Lines.push_back(Current);
                Current.clear();
                continue;
            }

            Current.push_back(Ch);
        }

        if (!Current.empty() || Text.empty())
            Lines.push_back(Current);

        return Lines;
    }

    void AppendLine(std::wstring& Body, const std::wstring& Line)
    {
        if (Line.empty())
            return;

        if (!Body.empty())
            Body += L"\n";

        Body += Line;
    }

    std::wstring JoinLines(const std::vector<std::wstring>& Lines)
    {
        std::wstring Result;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
            AppendLine(Result, Lines[Index]);

        return Result;
    }

    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long UnsignedValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(UnsignedValue);
        std::wstring Result;
        int GroupCount = 0;

        for (int Index = static_cast<int>(Digits.size()) - 1; Index >= 0; --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }

            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }

        if (Negative)
            Result.insert(Result.begin(), L'-');

        return Result;
    }

    std::wstring FormatMoney(long long Value)
    {
        if (Value < 0)
            return L"-$" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMoneyDollarFirst(long long Value)
    {
        if (Value < 0)
            return L"$-" + FormatInteger(-Value);

        return L"$" + FormatInteger(Value);
    }

    std::wstring FormatMultiplier(float Value)
    {
        wchar_t Buffer[32] = {};
        swprintf_s(Buffer, L"x%.2f", Value);
        return Buffer;
    }

    int ParseLeadingInteger(const std::wstring& Text, int DefaultValue = 0)
    {
        bool Negative = false;
        bool FoundDigit = false;
        int Value = 0;

        for (size_t Index = 0; Index < Text.size(); ++Index)
        {
            const wchar_t Ch = Text[Index];

            if (!FoundDigit && Ch == L'-')
            {
                Negative = true;
                continue;
            }

            if (Ch < L'0' || Ch > L'9')
            {
                if (FoundDigit)
                    break;

                continue;
            }

            FoundDigit = true;
            Value = Value * 10 + static_cast<int>(Ch - L'0');
        }

        if (!FoundDigit)
            return DefaultValue;

        return Negative ? -Value : Value;
    }

    std::wstring ExtractDetailValue(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (!StartsWith(Line, Prefix))
                continue;

            return Trim(Line.substr(wcslen(Prefix)));
        }

        return std::wstring();
    }

    int ExtractPowerValueMW(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        return ParseLeadingInteger(
            ExtractDetailValue(DetailText, Prefix),
            0);
    }

    std::wstring NormalizeWealthRequirementText(const std::wstring& Value)
    {
        if (Value == L"유복")
            return L"유복함";
        if (Value == L"부유")
            return L"부유함";
        if (Value == L"가난")
            return L"가난함";

        return Value;
    }

    bool UseModernApartmentOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.DisplayName == L"현대식 아파트";
    }

    bool UseModernApartmentUpgradeCard(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.DisplayName == L"현대식 아파트";
    }

    bool UseHarborWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.Harbor && Snapshot.DisplayName == L"항구";
    }

    bool UseHydroponicFarmWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.DisplayName == L"대규모 수경 농장";
    }

    bool UseAquaParkWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.DisplayName == L"아쿠아 파크";
    }

    bool UseRestaurantWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return Snapshot.DisplayName == L"레스토랑";
    }

    int ResolveOverviewHousingQuality(const FBuildingUiSnapshot& Snapshot)
    {
        if (UseModernApartmentOverview(Snapshot))
            return 104;

        const int Parsed =
            ParseLeadingInteger(Snapshot.HousingQualityText, Snapshot.HousingCap);
        return (std::max)(0, Parsed);
    }

    long long ResolveOverviewMonthlyIncome(const FBuildingUiSnapshot& Snapshot)
    {
        if (UseModernApartmentOverview(Snapshot))
            return 54;

        return static_cast<long long>((std::max)(0, Snapshot.Capacity)) * 3LL;
    }

    int ResolveOverviewRequiredPower(const FBuildingUiSnapshot& Snapshot)
    {
        if (UseModernApartmentOverview(Snapshot))
            return 20;

        return (std::max)(
            0,
            ParseLeadingInteger(Snapshot.RequiredPowerText, 0));
    }

    std::vector<std::wstring> ExtractBulletSection(
        const std::wstring& DetailText,
        const wchar_t* HeaderPrefix)
    {
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines = SplitLines(DetailText);
        bool Capture = false;

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            const std::wstring Line = Trim(Lines[Index]);

            if (!Capture)
            {
                if (StartsWith(Line, HeaderPrefix))
                    Capture = true;

                continue;
            }

            if (Line.empty())
                break;

            if (!Line.empty() && Line[0] == L'-')
            {
                Result.push_back(Trim(Line.substr(1)));
                continue;
            }

            if (Line.find(L':') != std::wstring::npos)
                break;

            break;
        }

        return Result;
    }

    std::vector<std::wstring> ExtractNarrativeLines(
        const std::wstring& DetailText)
    {
        std::vector<std::wstring> Result;
        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (size_t Index = 0; Index < Lines.size(); ++Index)
        {
            std::wstring Line = Trim(Lines[Index]);

            if (Line.empty())
                continue;

            if (Line[0] == L'-' ||
                StartsWith(Line, L"건설 비용:") ||
                StartsWith(Line, L"설계도 비용:") ||
                StartsWith(Line, L"등장 시기:") ||
                StartsWith(Line, L"필요 인력:") ||
                StartsWith(Line, L"필요 전력:") ||
                StartsWith(Line, L"생산 전력:") ||
                StartsWith(Line, L"크기:") ||
                StartsWith(Line, L"직업 품질:") ||
                StartsWith(Line, L"서비스 품질:") ||
                StartsWith(Line, L"수용 인원:") ||
                StartsWith(Line, L"수용 가구:") ||
                StartsWith(Line, L"재산 요구치:") ||
                StartsWith(Line, L"필요 재산:") ||
                StartsWith(Line, L"관광객 재산:") ||
                StartsWith(Line, L"선호 관광객:") ||
                StartsWith(Line, L"운영 모드:") ||
                StartsWith(Line, L"업그레이드"))
            {
                continue;
            }

            if (StartsWith(Line, L"효과:"))
            {
                Result.push_back(
                    L"효과: " +
                    Trim(Line.substr(wcslen(L"효과:"))));
                continue;
            }

            if (StartsWith(Line, L"비고:"))
            {
                Result.push_back(
                    L"비고: " +
                    Trim(Line.substr(wcslen(L"비고:"))));
                continue;
            }

            if (Line.find(L':') != std::wstring::npos)
                continue;

            Result.push_back(Line);
        }

        return Result;
    }

    void PushUnique(std::vector<std::string>& Names, const std::string& Name)
    {
        if (Name.empty())
            return;

        if (std::find(Names.begin(), Names.end(), Name) == Names.end())
            Names.push_back(Name);
    }

    std::wstring SummarizeNames(const std::vector<std::string>& Names)
    {
        if (Names.empty())
            return L"-";

        constexpr size_t GMaxShownNames = 5;
        const size_t CountToShow = (std::min)(Names.size(), GMaxShownNames);
        std::wstring Summary;

        for (size_t Index = 0; Index < CountToShow; ++Index)
        {
            if (Index > 0)
                Summary += L", ";

            Summary += Utf8ToWide(Names[Index]);
        }

        if (Names.size() > GMaxShownNames)
        {
            Summary += L" (+";
            Summary += std::to_wstring(Names.size() - GMaxShownNames);
            Summary += L")";
        }

        return Summary;
    }

    const wchar_t* GetHousingClassDisplayName(
        EBuildingHousingClass HousingClass)
    {
        switch (HousingClass)
        {
        case EBuildingHousingClass::Collective:
            return L"서민 주거";
        case EBuildingHousingClass::Standard:
            return L"중산층 주거";
        case EBuildingHousingClass::Elite:
            return L"고급 주거";
        default:
            return L"일반";
        }
    }

    const wchar_t* GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass)
    {
        switch (LeisureClass)
        {
        case EBuildingLeisureClass::Cultural:
            return L"문화";
        case EBuildingLeisureClass::Luxury:
            return L"고급";
        case EBuildingLeisureClass::General:
            return L"일반";
        default:
            return L"미분류";
        }
    }

    std::wstring ResolveRoleSummary(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.Residential)
        {
            std::wstring Summary =
                Snapshot.DisplayName +
                L"은(는) " +
                std::to_wstring((std::max)(0, Snapshot.Capacity)) +
                L"명을 수용하는 주거 건물입니다.";

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += GetHousingClassDisplayName(
                    Snapshot.CatalogEntry->HousingClass);
                Summary += L" 계층을 주로 상대합니다.";
            }

            return Summary;
        }

        if (Snapshot.Harbor)
            return L"수입/수출과 입국 관문 역할을 담당하는 기반시설입니다.";

        if (Snapshot.EntertainmentProvider && Snapshot.FoodProvider)
            return L"음식과 유흥을 동시에 제공하는 서비스 건물입니다.";

        if (Snapshot.EntertainmentProvider)
        {
            std::wstring Summary =
                L"관광객과 시민에게 여가 서비스를 제공하는 시설입니다.";

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += GetLeisureClassDisplayName(
                    Snapshot.CatalogEntry->LeisureClass);
                Summary += L" 성향의 방문객에게 적합합니다.";
            }

            return Summary;
        }

        if (Snapshot.FoodProvider)
            return L"시민 소비와 외식 흐름을 담당하는 공급 건물입니다.";

        if (Snapshot.Building && Snapshot.Building->CanGenerateWorkOutput())
            return L"인력을 배치해 자원이나 상품을 생산하는 작업 시설입니다.";

        return L"도시 운영에 필요한 기능을 담당하는 보조 건물입니다.";
    }

    void AppendKeyValue(
        std::wstring& Body,
        const wchar_t* Key,
        const std::wstring& Value)
    {
        if (Value.empty())
            return;

        AppendLine(Body, std::wstring(Key) + L": " + Value);
    }

    bool BuildBuildingUiSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        FBuildingUiSnapshot& OutSnapshot)
    {
        if (!World || BuildingName.empty())
            return false;

        auto Building =
            World->FindObject<CPlacementAreaObject>(BuildingName).lock();

        if (!Building || !Building->GetAlive() || !Building->GetEnable())
            return false;

        OutSnapshot = FBuildingUiSnapshot();
        OutSnapshot.Building = Building;
        OutSnapshot.CatalogEntry =
            FindBuildingCatalogEntry(Building->GetBuildingId());
        OutSnapshot.ObjectName = Utf8ToWide(Building->GetName());
        OutSnapshot.DisplayName = Utf8ToWide(
            Building->GetBuildingDisplayName());
        OutSnapshot.CategoryName = Utf8ToWide(
            Building->GetBuildingCategoryName());
        OutSnapshot.DetailText = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->DetailText :
            std::wstring();
        OutSnapshot.Residential = Building->IsResidential();
        OutSnapshot.WorkProvider =
            !OutSnapshot.Residential &&
            Building->GetCapacity() > 0;
        OutSnapshot.FoodProvider = Building->IsFoodProvider();
        OutSnapshot.EntertainmentProvider =
            Building->IsEntertainmentProvider();
        OutSnapshot.Harbor = Building->IsHarbor();
        OutSnapshot.Warehouse = Building->IsWarehouse();
        OutSnapshot.Capacity = (std::max)(0, Building->GetCapacity());
        OutSnapshot.BudgetLevel = Building->GetBudgetLevel();
        OutSnapshot.BudgetScale = Building->GetBudgetSatisfactionScale();
        OutSnapshot.AccessibilityScore = Building->GetAccessibilityScore();
        OutSnapshot.HousingCap = Building->GetHousingSatisfactionCap();
        OutSnapshot.JobCap = Building->GetEffectiveJobSatisfactionCap();
        OutSnapshot.FoodCap = Building->GetFoodSatisfactionCap();
        OutSnapshot.FunCap = Building->GetFunSatisfactionCap();
        OutSnapshot.ResourceStock = Building->GetResourceStock();
        OutSnapshot.ExportableStock = Building->GetExportableResourceStock();
        OutSnapshot.MaxResourceStock = Building->GetMaxResourceStock();
        OutSnapshot.HarborShipProgressPercent =
            Building->GetHarborShipProgressPercent();
        OutSnapshot.UsesResourceStock =
            OutSnapshot.ResourceStock > 0 ||
            Building->CanGenerateWorkOutput() ||
            OutSnapshot.FoodProvider ||
            OutSnapshot.Harbor ||
            OutSnapshot.Warehouse;

        if (OutSnapshot.Warehouse)
        {
            for (int SlotIndex = 0;
                SlotIndex < Building->GetWarehouseSlotCount();
                ++SlotIndex)
            {
                const EResourceType SlotType =
                    Building->GetWarehouseSlotType(SlotIndex);
                std::wstring SlotLine =
                    L"슬롯 " + std::to_wstring(SlotIndex + 1) + L": ";

                if (SlotType == EResourceType::None)
                {
                    SlotLine += L"비어 있음";
                }
                else
                {
                    SlotLine += GetResourceTypeDisplayName(SlotType);
                    SlotLine += L" ";
                    SlotLine += FormatInteger(Building->GetResourceStock(SlotType));
                    SlotLine += L" / ";
                    SlotLine += FormatInteger(
                        Building->GetResourceTypeCapacity(SlotType));
                }

                OutSnapshot.WarehouseSlotLines.push_back(std::move(SlotLine));
            }
        }

        auto MainWorld =
            std::dynamic_pointer_cast<IMainWorldBuildMenuAccess>(World);

        if (MainWorld)
            OutSnapshot.DaysInMonth =
                (std::max)(1, MainWorld->GetSimulationMonthDayCount());

        OutSnapshot.MonthlyWageCost = Building->GetMonthlyWageCost();
        OutSnapshot.MonthlyUpkeepCost = Building->GetMonthlyUpkeepCost();
        OutSnapshot.DailyWageCost =
            Building->GetDailyWageCost(OutSnapshot.DaysInMonth);
        OutSnapshot.DailyUpkeepCost =
            Building->GetDailyUpkeepCost(OutSnapshot.DaysInMonth);

        OutSnapshot.RequiredPowerText =
            ExtractDetailValue(OutSnapshot.DetailText, L"필요 전력:");
        OutSnapshot.ProducedPowerText =
            ExtractDetailValue(OutSnapshot.DetailText, L"생산 전력:");
        OutSnapshot.JobQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"직업 품질:");
        OutSnapshot.ServiceQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"서비스 품질:");
        OutSnapshot.HousingQualityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"주거 품질:");
        OutSnapshot.WealthRequirementText =
            ExtractDetailValue(OutSnapshot.DetailText, L"재산 요구치:");

        if (OutSnapshot.WealthRequirementText.empty())
        {
            OutSnapshot.WealthRequirementText =
                ExtractDetailValue(OutSnapshot.DetailText, L"필요 재산:");
        }

        if (OutSnapshot.WealthRequirementText.empty())
        {
            OutSnapshot.WealthRequirementText =
                ExtractDetailValue(OutSnapshot.DetailText, L"관광객 재산:");
        }

        OutSnapshot.TouristPreferenceText =
            ExtractDetailValue(OutSnapshot.DetailText, L"선호 관광객:");
        OutSnapshot.EffectText =
            ExtractDetailValue(OutSnapshot.DetailText, L"효과:");
        OutSnapshot.NoteText =
            ExtractDetailValue(OutSnapshot.DetailText, L"비고:");
        OutSnapshot.ServiceCapacityText =
            ExtractDetailValue(OutSnapshot.DetailText, L"수용 인원:");

        if (OutSnapshot.ServiceCapacityText.empty())
        {
            OutSnapshot.ServiceCapacityText =
                ExtractDetailValue(OutSnapshot.DetailText, L"수용 가구:");
        }

        OutSnapshot.ServiceCapacity = (std::max)(
            0,
            ParseLeadingInteger(OutSnapshot.ServiceCapacityText, 0));
        OutSnapshot.OperationModes =
            ExtractBulletSection(OutSnapshot.DetailText, L"운영 모드");
        OutSnapshot.UpgradeHints = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->UpgradeHints :
            ExtractBulletSection(OutSnapshot.DetailText, L"업그레이드");
        OutSnapshot.NarrativeLines =
            ExtractNarrativeLines(OutSnapshot.DetailText);

        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        {
            for (size_t Index = 0; Index < BuildingList.size(); ++Index)
            {
                auto OtherBuilding = BuildingList[Index].lock();

                if (!OtherBuilding ||
                    !OtherBuilding->GetAlive() ||
                    !OtherBuilding->GetEnable() ||
                    !OtherBuilding->HasPlacedArea())
                {
                    continue;
                }

                const FBuildingCatalogEntry* Entry =
                    FindBuildingCatalogEntry(OtherBuilding->GetBuildingId());

                if (!Entry)
                    continue;

                OutSnapshot.TotalProducedPowerMW +=
                    (std::max)(
                        0,
                        (std::max)(
                            ExtractPowerValueMW(
                                Entry->DetailText,
                                L"생산 전력:"),
                            ExtractPowerValueMW(
                                Entry->DetailText,
                                L"발전량:")));
                OutSnapshot.TotalRequiredPowerMW +=
                    (std::max)(
                        0,
                        ExtractPowerValueMW(
                            Entry->DetailText,
                            L"필요 전력:"));
            }
        }

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return true;

        for (size_t Index = 0; Index < OrbList.size(); ++Index)
        {
            auto Orb = OrbList[Index].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            const std::string OrbName = Orb->GetName();

            if (Orb->GetHomeBuilding() == BuildingName)
                PushUnique(OutSnapshot.Residents, OrbName);

            if (OutSnapshot.WorkProvider &&
                Orb->GetWorkBuilding() == BuildingName)
            {
                PushUnique(OutSnapshot.AssignedEmployees, OrbName);

                if (Orb->GetCitizenState() == ECitizenState::AtWork)
                    PushUnique(OutSnapshot.WorkingEmployees, OrbName);
            }

            if (OutSnapshot.EntertainmentProvider &&
                Orb->GetFunBuilding() == BuildingName)
            {
                PushUnique(OutSnapshot.AssignedVisitors, OrbName);

                if (Orb->GetCitizenState() == ECitizenState::AtFun)
                    PushUnique(OutSnapshot.ArrivedVisitors, OrbName);
            }

            if (!OutSnapshot.FoodProvider)
                continue;

            const ECitizenState OrbState = Orb->GetCitizenState();

            if (OrbState == ECitizenState::AtFood)
            {
                const std::string& VisitFoodBuilding =
                    Orb->GetFoodVisitBuilding();
                const bool IsVisitBuildingMatched =
                    VisitFoodBuilding == BuildingName ||
                    (VisitFoodBuilding.empty() &&
                        Orb->GetFoodBuilding() == BuildingName);

                if (!IsVisitBuildingMatched)
                    continue;

                PushUnique(OutSnapshot.AssignedVisitors, OrbName);
                PushUnique(OutSnapshot.ArrivedVisitors, OrbName);
                continue;
            }

            if (OrbState != ECitizenState::GoingToFood ||
                Orb->GetFoodBuilding() != BuildingName)
            {
                continue;
            }

            FVector3 MarkerPos = FVector3::Zero;

            if (!Building->GetClosestMarkerWorldPos(
                Orb->GetWorldPos(), MarkerPos))
            {
                continue;
            }

            FVector3 OrbPos = Orb->GetWorldPos();
            OrbPos.z = MarkerPos.z;

            const float NearDistance =
                (std::max)(8.f, Orb->GetArrivalDistance() * 2.f);

            if (OrbPos.Distance(MarkerPos) > NearDistance)
                continue;

            PushUnique(OutSnapshot.AssignedVisitors, OrbName);
            PushUnique(OutSnapshot.IncomingVisitors, OrbName);
        }

        std::sort(
            OutSnapshot.Residents.begin(),
            OutSnapshot.Residents.end());
        std::sort(
            OutSnapshot.AssignedEmployees.begin(),
            OutSnapshot.AssignedEmployees.end());
        std::sort(
            OutSnapshot.WorkingEmployees.begin(),
            OutSnapshot.WorkingEmployees.end());
        std::sort(
            OutSnapshot.AssignedVisitors.begin(),
            OutSnapshot.AssignedVisitors.end());
        std::sort(
            OutSnapshot.ArrivedVisitors.begin(),
            OutSnapshot.ArrivedVisitors.end());
        std::sort(
            OutSnapshot.IncomingVisitors.begin(),
            OutSnapshot.IncomingVisitors.end());
        return true;
    }

    std::wstring BuildOverviewBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, L"운영 모드");
            AppendLine(Body, L"  " + Snapshot.OperationModes.front());
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"거주 현황: " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"노동자: " +
                std::to_wstring(Snapshot.AssignedEmployees.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendKeyValue(
                Body,
                L"요구 학력",
                GetCitizenEducationDisplayName(
                    Snapshot.Building->GetRequiredEducationLevel()));
            AppendKeyValue(Body, L"직업 품질", Snapshot.JobQualityText);
        }

        if (Snapshot.Residential && Snapshot.CatalogEntry)
        {
            AppendKeyValue(
                Body,
                L"주거 계층",
                GetHousingClassDisplayName(
                    Snapshot.CatalogEntry->HousingClass));
        }

        if (Snapshot.ServiceCapacity > 0 ||
            !Snapshot.ServiceQualityText.empty() ||
            !Snapshot.WealthRequirementText.empty() ||
            !Snapshot.TouristPreferenceText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"서비스");

            if (Snapshot.ServiceCapacity > 0)
            {
                AppendLine(
                    Body,
                    L"방문객: " +
                    std::to_wstring(Snapshot.AssignedVisitors.size()) +
                    L" / " +
                    std::to_wstring(Snapshot.ServiceCapacity));
            }

            AppendKeyValue(Body, L"서비스 품질", Snapshot.ServiceQualityText);
            AppendKeyValue(Body, L"재산 요구치", Snapshot.WealthRequirementText);
            AppendKeyValue(Body, L"선호 관광객", Snapshot.TouristPreferenceText);
        }

        if (!Snapshot.RequiredPowerText.empty() ||
            !Snapshot.ProducedPowerText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"전력");
            AppendKeyValue(Body, L"필요 전력", Snapshot.RequiredPowerText);
            AppendKeyValue(Body, L"생산 전력", Snapshot.ProducedPowerText);
        }

        if (Snapshot.UsesResourceStock || Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"보관 및 물류");

            if (Snapshot.UsesResourceStock)
            {
                AppendLine(
                    Body,
                    L"재고: " +
                    FormatInteger(Snapshot.ResourceStock) +
                    L" / " +
                    FormatInteger(Snapshot.MaxResourceStock));
            }

            if (Snapshot.Warehouse)
            {
                for (size_t SlotIndex = 0;
                    SlotIndex < Snapshot.WarehouseSlotLines.size();
                    ++SlotIndex)
                {
                    AppendLine(Body, Snapshot.WarehouseSlotLines[SlotIndex]);
                }
            }

            if (Snapshot.Harbor)
            {
                AppendLine(
                    Body,
                    L"수출 가능 재고: " +
                    FormatInteger(Snapshot.ExportableStock));
                AppendLine(
                    Body,
                    L"선박 진행: " +
                    std::to_wstring(static_cast<int>(roundf(
                        Snapshot.HarborShipProgressPercent * 100.f))) +
                    L"%");
            }
        }

        if (!Snapshot.EffectText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"주요 효과");
            AppendLine(Body, Snapshot.EffectText);
        }

        if (!Snapshot.NoteText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"비고");
            AppendLine(Body, Snapshot.NoteText);
        }

        return Body.empty() ? L"건물 데이터 준비 중입니다." : Body;
    }

    std::wstring BuildStatisticsBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        AppendLine(Body, L"월 인건비: " + FormatMoney(Snapshot.MonthlyWageCost));
        AppendLine(Body, L"월 유지비: " + FormatMoney(Snapshot.MonthlyUpkeepCost));
        AppendLine(
            Body,
            L"월 총비용: " +
            FormatMoney(
                static_cast<long long>(Snapshot.MonthlyWageCost) +
                static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
        AppendLine(Body, L"일 인건비: " + FormatMoney(Snapshot.DailyWageCost));
        AppendLine(Body, L"일 유지비: " + FormatMoney(Snapshot.DailyUpkeepCost));

        if (Snapshot.UsesResourceStock)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"현재 재고: " +
                FormatInteger(Snapshot.ResourceStock) +
                L" / " +
                FormatInteger(Snapshot.MaxResourceStock));
        }

        if (Snapshot.Warehouse && !Snapshot.WarehouseSlotLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"창고 슬롯");

            for (size_t SlotIndex = 0;
                SlotIndex < Snapshot.WarehouseSlotLines.size();
                ++SlotIndex)
            {
                AppendLine(Body, Snapshot.WarehouseSlotLines[SlotIndex]);
            }
        }

        if (Snapshot.Harbor)
        {
            AppendLine(
                Body,
                L"선박 도착 진행: " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.HarborShipProgressPercent * 100.f))) +
                L"%");
            AppendLine(
                Body,
                L"수출 가능 총량: " +
                FormatInteger(Snapshot.ExportableStock));
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"거주자 배정: " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendLine(
                Body,
                L"대표 거주자: " + SummarizeNames(Snapshot.Residents));
        }

        if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"배정 노동자: " +
                std::to_wstring(Snapshot.AssignedEmployees.size()));
            AppendLine(
                Body,
                L"근무 중: " +
                std::to_wstring(Snapshot.WorkingEmployees.size()));
            AppendLine(
                Body,
                L"대표 노동자: " +
                SummarizeNames(Snapshot.AssignedEmployees));
        }

        if (!Snapshot.AssignedVisitors.empty() ||
            !Snapshot.ArrivedVisitors.empty() ||
            !Snapshot.IncomingVisitors.empty())
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                L"배정 방문객: " +
                std::to_wstring(Snapshot.AssignedVisitors.size()));
            AppendLine(
                Body,
                L"현장 체류: " +
                std::to_wstring(Snapshot.ArrivedVisitors.size()));
            AppendLine(
                Body,
                L"도착 임박: " +
                std::to_wstring(Snapshot.IncomingVisitors.size()));
            AppendLine(
                Body,
                L"대표 방문객: " +
                SummarizeNames(Snapshot.ArrivedVisitors));
        }

        return Body;
    }

    std::wstring BuildUpgradesBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (!Snapshot.UpgradeHints.empty())
        {
            AppendLine(Body, L"등록된 업그레이드");

            for (size_t Index = 0; Index < Snapshot.UpgradeHints.size(); ++Index)
                AppendLine(Body, L"- " + Snapshot.UpgradeHints[Index]);
        }
        else
        {
            AppendLine(Body, L"등록된 업그레이드가 없습니다.");
        }

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, L"운영 모드 후보");

            for (size_t Index = 0; Index < Snapshot.OperationModes.size(); ++Index)
                AppendLine(Body, L"- " + Snapshot.OperationModes[Index]);
        }

        return Body;
    }

    std::wstring BuildEfficiencyBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;
        const int BudgetPercent =
            static_cast<int>(roundf(Snapshot.BudgetScale * 100.f));
        const int CapacityFillPercent =
            Snapshot.Capacity > 0 ?
            static_cast<int>(roundf(
                (static_cast<float>(
                    Snapshot.Residential ?
                    Snapshot.Residents.size() :
                    Snapshot.AssignedEmployees.size()) /
                    static_cast<float>(Snapshot.Capacity)) * 100.f)) :
            100;
        const int VisitorFillPercent =
            Snapshot.ServiceCapacity > 0 ?
            static_cast<int>(roundf(
                (static_cast<float>(Snapshot.AssignedVisitors.size()) /
                    static_cast<float>(Snapshot.ServiceCapacity)) * 100.f)) :
            0;

        AppendLine(
            Body,
            L"현재 효율: " +
            std::to_wstring(BudgetPercent) +
            L"%");
        AppendLine(
            Body,
            L"예산 보정: " + FormatMultiplier(Snapshot.BudgetScale));

        if (Snapshot.Building && !Snapshot.Building->IsRoad())
        {
            AppendLine(
                Body,
                L"도로 접근성: " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                L"%");
        }

        if (Snapshot.Residential)
        {
            AppendLine(
                Body,
                L"거주 충원율: " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                L"주거 만족도 상한: " +
                std::to_wstring(Snapshot.HousingCap));
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(
                Body,
                L"인력 충원율: " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                L"직업 만족도 상한: " +
                std::to_wstring(Snapshot.JobCap));
        }

        if (Snapshot.FoodProvider)
        {
            AppendLine(
                Body,
                L"음식 만족도 상한: " +
                std::to_wstring(Snapshot.FoodCap));
        }

        if (Snapshot.EntertainmentProvider)
        {
            AppendLine(
                Body,
                L"유흥 만족도 상한: " +
                std::to_wstring(Snapshot.FunCap));
        }

        if (Snapshot.ServiceCapacity > 0)
        {
            AppendLine(
                Body,
                L"방문 활용도: " +
                std::to_wstring((std::max)(0, VisitorFillPercent)) +
                L"%");
        }

        if (!Snapshot.RequiredPowerText.empty())
            AppendKeyValue(Body, L"전력 요구", Snapshot.RequiredPowerText);

        return Body;
    }

    std::wstring BuildInformationBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body = ResolveRoleSummary(Snapshot);

        AppendLine(Body, L"");
        AppendKeyValue(Body, L"카테고리", Snapshot.CategoryName);

        if (Snapshot.CatalogEntry)
        {
            AppendKeyValue(
                Body,
                L"등장 시대",
                GetBuildingEraDisplayName(Snapshot.CatalogEntry->UnlockEra));

            if (Snapshot.CatalogEntry->HousingClass !=
                EBuildingHousingClass::None)
            {
                AppendKeyValue(
                    Body,
                    L"주거 등급",
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass));
            }

            if (Snapshot.CatalogEntry->LeisureClass !=
                EBuildingLeisureClass::None)
            {
                AppendKeyValue(
                    Body,
                    L"여가 등급",
                    GetLeisureClassDisplayName(
                        Snapshot.CatalogEntry->LeisureClass));
            }

            if (Snapshot.CatalogEntry->PrimaryTouristPreference !=
                ETouristPreference::None)
            {
                AppendKeyValue(
                    Body,
                    L"대표 관광객",
                    GetTouristPreferenceDisplayName(
                        Snapshot.CatalogEntry->PrimaryTouristPreference));
            }
        }

        AppendKeyValue(
            Body,
            L"요구 학력",
            GetCitizenEducationDisplayName(
                Snapshot.Building->GetRequiredEducationLevel()));

        if (!Snapshot.NarrativeLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, JoinLines(Snapshot.NarrativeLines));
        }
        else if (!Snapshot.DetailText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Snapshot.DetailText);
        }

        return Body;
    }

    const wchar_t* GetCitizenProfileWealthDisplayName(
        ECitizenWealthLevel Level)
    {
        switch (Level)
        {
        case ECitizenWealthLevel::WellOff:
            return L"유복함";
        case ECitizenWealthLevel::Rich:
            return L"부유함";
        default:
            return L"가난함";
        }
    }

    const wchar_t* GetCitizenActivityDisplayName(ECitizenState State)
    {
        switch (State)
        {
        case ECitizenState::GoingToWork:
        case ECitizenState::AtWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return L"근무";
        case ECitizenState::GoingToFood:
        case ECitizenState::AtFood:
            return L"식사";
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
        case ECitizenState::GoingToFun:
        case ECitizenState::AtFun:
            return L"여가";
        default:
            return L"이동";
        }
    }

    std::wstring ResolveBuildingDisplayName(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName)
    {
        if (BuildingName.empty())
            return L"-";

        if (!World)
            return Utf8ToWide(BuildingName);

        auto Building =
            World->FindObject<CPlacementAreaObject>(BuildingName).lock();

        if (!Building || !Building->GetAlive() || !Building->GetEnable())
            return Utf8ToWide(BuildingName);

        const std::wstring DisplayName =
            Utf8ToWide(Building->GetBuildingDisplayName());

        return DisplayName.empty() ? Utf8ToWide(BuildingName) : DisplayName;
    }

    std::wstring ResolveCitizenLocationText(
        const std::shared_ptr<CWorld>& World,
        const std::shared_ptr<CBuildingMarkerOrb>& Citizen)
    {
        if (!Citizen)
            return L"-";

        auto MakeInteriorText =
            [&](const std::string& BuildingName)
        {
            const std::wstring DisplayName =
                ResolveBuildingDisplayName(World, BuildingName);

            if (DisplayName.empty() || DisplayName == L"-")
                return std::wstring(L"-");

            return DisplayName + L" 내부";
        };

        switch (Citizen->GetCitizenState())
        {
        case ECitizenState::AtHome:
        case ECitizenState::GoingHome:
            return MakeInteriorText(Citizen->GetHomeBuilding());
        case ECitizenState::AtWork:
        case ECitizenState::GoingToWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return MakeInteriorText(Citizen->GetWorkBuilding());
        case ECitizenState::AtFood:
        case ECitizenState::GoingToFood:
            return MakeInteriorText(Citizen->GetFoodBuilding());
        case ECitizenState::AtFun:
        case ECitizenState::GoingToFun:
            return MakeInteriorText(Citizen->GetFunBuilding());
        default:
            return L"트로피코 외부";
        }
    }

    const wchar_t* GetCitizenPoliticalIntensityDisplayName(
        EPoliticalAxis Axis,
        EPoliticalSupportLevel Support)
    {
        if (Support == EPoliticalSupportLevel::Strong)
        {
            return Axis == EPoliticalAxis::ReligionMilitarism ?
                L"열렬함" :
                L"강함";
        }

        return GetPoliticalSupportDisplayName(Support);
    }

    std::array<std::wstring, 3> BuildCitizenOpinionLines(
        const FNpcPoliticalProfile& PoliticalProfile)
    {
        struct FOpinionEntry
        {
            EPoliticalAxis Axis;
            FNpcPoliticalChoice Choice;
        };

        const std::array<FOpinionEntry, 4> OrderedEntries =
        {{
            { EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism },
            { EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry },
            { EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative },
            { EPoliticalAxis::Economy,
                PoliticalProfile.Economy }
        }};

        std::array<std::wstring, 3> Lines = {};
        int WriteIndex = 0;

        for (const FOpinionEntry& Entry : OrderedEntries)
        {
            if (WriteIndex >= static_cast<int>(Lines.size()))
                break;

            if (Entry.Choice.Stance == EPoliticalStance::Neutral)
                continue;

            Lines[static_cast<size_t>(WriteIndex)] =
                std::wstring(GetPoliticalFactionDisplayName(
                    Entry.Axis,
                    Entry.Choice.Stance)) +
                L" (" +
                GetCitizenPoliticalIntensityDisplayName(
                    Entry.Axis,
                    Entry.Choice.Support) +
                L")";
            ++WriteIndex;
        }

        return Lines;
    }

    float BuildCitizenSupportRatio(
        const FNpcSatisfaction& Satisfaction)
    {
        const float Overall =
            (std::max)(0.f, (std::min)(100.f, Satisfaction.Overall));
        const float Ratio = (Overall + 12.f) / 112.f;
        return (std::max)(0.f, (std::min)(1.f, Ratio));
    }
}

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile,
        int TabIndex)
    {
        const int ClampedTab = (std::max)(0, (std::min)(2, TabIndex));

        auto ToPercent = [](float Value)
        {
            const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
            return static_cast<int>(roundf(Clamped));
        };

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Citizen;
        Result.SelectedTabIndex = ClampedTab;
        Result.Title = Utf8ToWide(CitizenName);
        Result.ShowTabButtons = true;
        Result.ShowBudgetControls = false;
        Result.ShowActionButtons = false;
        Result.ShowSectionRibbon = (ClampedTab > 0);
        Result.ShowTitleIcon = false;

        // 부제 – 재산/학력/이민 여부
        {
            wchar_t SubBuf[128] = {};
            swprintf_s(SubBuf,
                L"%s · %s%s",
                GetCitizenWealthDisplayName(IdentityProfile.WealthLevel),
                GetCitizenEducationDisplayName(IdentityProfile.EducationLevel),
                IdentityProfile.IsImmigrant ? L" · 이민자" : L"");
            Result.Subtitle = SubBuf;
        }

        if (ClampedTab == 0)
        {
            // ── 탭 0: 기본 정보 (신원 + 여가/복지 만족도) ──────────
            Result.PageTitle = L"기본";
            wchar_t Buf[896] = {};
            swprintf_s(Buf,
                L"학력: %s  재산: %s%s\n\n"
                L"[ 여가 ]\n"
                L"음식: %d    보건: %d\n"
                L"유흥: %d    신앙: %d\n\n"
                L"[ 복지 서비스 ]\n"
                L"주거: %d    직업: %d\n"
                L"통근 부담: %d\n"
                L"자유: %d    치안: %d\n\n"
                L"종합 만족도: %d",
                GetCitizenEducationDisplayName(IdentityProfile.EducationLevel),
                GetCitizenWealthDisplayName(IdentityProfile.WealthLevel),
                IdentityProfile.IsImmigrant ? L"  (이민자)" : L"",
                ToPercent(Satisfaction.Food),
                ToPercent(Satisfaction.Health),
                ToPercent(Satisfaction.Fun),
                ToPercent(Satisfaction.Faith),
                ToPercent(Satisfaction.Housing),
                ToPercent(Satisfaction.Job),
                ToPercent(Satisfaction.CommuteTimePenalty),
                ToPercent(Satisfaction.Freedom),
                ToPercent(Satisfaction.Security),
                ToPercent(Satisfaction.Overall));
            Result.BodyText = Buf;
        }
        else if (ClampedTab == 1)
        {
            // ── 탭 1: 정치 (만족도 요약 + 신념) ───────────────────
            Result.PageTitle = L"정치";
            wchar_t Buf[768] = {};
            swprintf_s(Buf,
                L"[ 만족도 현황 ]\n"
                L"음식: %d  보건: %d  유흥: %d\n"
                L"신앙: %d  주거: %d  직업: %d\n"
                L"자유: %d  치안: %d  종합: %d\n\n"
                L"[ 신념 ]\n"
                L"경제: %s · %s\n"
                L"종교·군국: %s · %s\n"
                L"환경·산업: %s · %s\n"
                L"지식·보수: %s · %s",
                ToPercent(Satisfaction.Food),
                ToPercent(Satisfaction.Health),
                ToPercent(Satisfaction.Fun),
                ToPercent(Satisfaction.Faith),
                ToPercent(Satisfaction.Housing),
                ToPercent(Satisfaction.Job),
                ToPercent(Satisfaction.Freedom),
                ToPercent(Satisfaction.Security),
                ToPercent(Satisfaction.Overall),
                GetPoliticalFactionDisplayName(
                    EPoliticalAxis::Economy,
                    PoliticalProfile.Economy.Stance),
                GetPoliticalSupportDisplayName(
                    PoliticalProfile.Economy.Support),
                GetPoliticalFactionDisplayName(
                    EPoliticalAxis::ReligionMilitarism,
                    PoliticalProfile.ReligionMilitarism.Stance),
                GetPoliticalSupportDisplayName(
                    PoliticalProfile.ReligionMilitarism.Support),
                GetPoliticalFactionDisplayName(
                    EPoliticalAxis::EnvironmentIndustry,
                    PoliticalProfile.EnvironmentIndustry.Stance),
                GetPoliticalSupportDisplayName(
                    PoliticalProfile.EnvironmentIndustry.Support),
                GetPoliticalFactionDisplayName(
                    EPoliticalAxis::IntellectualConservative,
                    PoliticalProfile.IntellectualConservative.Stance),
                GetPoliticalSupportDisplayName(
                    PoliticalProfile.IntellectualConservative.Support));
            Result.BodyText = Buf;
        }
        else
        {
            // ── 탭 2: 성향 (생성형 서술) ───────────────────────────
            Result.PageTitle = L"성향";
            const int Overall = ToPercent(Satisfaction.Overall);

            const wchar_t* SatisfactionDesc =
                Overall >= 75 ? L"트로피코 생활에 크게 만족하고 있습니다." :
                Overall >= 55 ? L"트로피코 생활에 어느 정도 만족하고 있습니다." :
                Overall >= 35 ? L"트로피코 생활에 불만이 있습니다." :
                                L"트로피코 생활에 크게 불만족하고 있습니다.";

            // 지배적 정치 성향 탐색
            const wchar_t* DominantStance = nullptr;
            int MaxSupport = -1;
            auto CheckAxis =
                [&](EPoliticalAxis Axis, const FNpcPoliticalChoice& Profile)
            {
                const int Sup = static_cast<int>(Profile.Support);
                if (Sup > MaxSupport)
                {
                    MaxSupport = Sup;
                    DominantStance = GetPoliticalFactionDisplayName(
                        Axis, Profile.Stance);
                }
            };
            CheckAxis(EPoliticalAxis::Economy,
                PoliticalProfile.Economy);
            CheckAxis(EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism);
            CheckAxis(EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry);
            CheckAxis(EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative);

            wchar_t Buf[512] = {};
            swprintf_s(Buf,
                L"%s은/는 %s %s 시민입니다%s.\n\n"
                L"%s\n\n"
                L"지배적 정치 성향: %s",
                Result.Title.c_str(),
                GetCitizenWealthDisplayName(IdentityProfile.WealthLevel),
                GetCitizenEducationDisplayName(IdentityProfile.EducationLevel),
                IdentityProfile.IsImmigrant ? L" (이민자)" : L"",
                SatisfactionDesc,
                DominantStance ? DominantStance : L"-");
            Result.BodyText = Buf;
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int TabIndex)
    {
        if (!World || CitizenName.empty())
            return FCitizenInfoSnapshot();

        auto Citizen = World->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

        if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
            return FCitizenInfoSnapshot();

        FCitizenInfoSnapshot Result = BuildCitizenSnapshot(
            CitizenName,
            Citizen->GetSatisfaction(),
            Citizen->GetIdentityProfile(),
            Citizen->GetPoliticalProfile(),
            TabIndex);

        if (Result.Valid && Result.SelectedTabIndex == 0)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText = L"↠ 트로피코인 ↞";
            Result.ShowCitizenProfileOverview = true;
            Result.ShowCitizenActionButtons = true;
            Result.ShowSectionDivider = true;
            Result.CitizenPortraitSlotCount = 11;
            Result.CitizenPortraitOccupiedSlot = 4;
            Result.CitizenPortraitVariant =
                static_cast<int>(CitizenName.size() % 4);
            Result.CitizenFooterText = L"TROPICO EXEC. 16FA-923";

            Result.OverviewMetricLabels[0] = L"활동";
            Result.OverviewMetricValues[0] =
                GetCitizenActivityDisplayName(Citizen->GetCitizenState());
            Result.OverviewMetricLabels[1] = L"위치";
            Result.OverviewMetricValues[1] =
                ResolveCitizenLocationText(World, Citizen);
            Result.OverviewMetricLabels[2] = L"연령";
            Result.OverviewMetricValues[2] = L"30 (성인)";
            Result.OverviewMetricLabels[3] = L"출신";
            Result.OverviewMetricValues[3] =
                Citizen->GetIdentityProfile().IsImmigrant ?
                    L"프랑스" :
                    L"트로피코";
            Result.OverviewMetricLabels[4] = L"재산";
            Result.OverviewMetricValues[4] =
                GetCitizenProfileWealthDisplayName(
                    Citizen->GetIdentityProfile().WealthLevel);
            Result.OverviewMetricLabels[5] = L"교육";
            Result.OverviewMetricValues[5] =
                GetCitizenEducationDisplayName(
                    Citizen->GetIdentityProfile().EducationLevel);
            Result.OverviewMetricLabels[6] = L"직장";
            Result.OverviewMetricValues[6] =
                ResolveBuildingDisplayName(World, Citizen->GetWorkBuilding());
            Result.OverviewMetricLabels[7] = L"집";
            Result.OverviewMetricValues[7] =
                ResolveBuildingDisplayName(World, Citizen->GetHomeBuilding());
            Result.OverviewMetricAccentValues[6] =
                !Citizen->GetWorkBuilding().empty();
            Result.OverviewMetricAccentValues[7] =
                !Citizen->GetHomeBuilding().empty();

            Result.CitizenActionLabels[0] = L"매수";
            Result.CitizenActionLabels[1] = L"살해";
            Result.CitizenActionLabels[2] = L"사고사로 위장";
            Result.CitizenActionLabels[3] = L"체포";
            Result.CitizenActionLabels[4] = L"보호 시설 격리";
            Result.CitizenActionLabels[5] = L"우주 파견";
        }
        else if (Result.Valid && Result.SelectedTabIndex == 1)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText = L"↠ 트로피코인 ↞";
            Result.ShowCitizenPoliticsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();

            Result.CitizenPoliticsSatisfactionLabels =
            {{
                L"종합 만족도",
                L"음식",
                L"보건",
                L"유흥",
                L"신앙",
                L"주거",
                L"직업",
                L"자유",
                L"치안"
            }};
            Result.CitizenPoliticsSatisfactionRatios =
            {{
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Overall / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Food / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Health / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Fun / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Faith / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Housing / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Job / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Freedom / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen->GetSatisfaction().Security / 100.f))
            }};
            Result.CitizenPoliticsOpinionLines =
                BuildCitizenOpinionLines(Citizen->GetPoliticalProfile());
            Result.CitizenPoliticsSupportRatio =
                BuildCitizenSupportRatio(Citizen->GetSatisfaction());
        }
        else if (Result.Valid && Result.SelectedTabIndex == 2)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText = L"↠ 트로피코인 ↞";
            Result.ShowCitizenThoughtsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenPoliticsOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();
            Result.CitizenThoughtLines =
            {{
                L"우리 군대의 군복이 아주 끝내줘!",
                L"잠을 자면서 돈을 받는 게 내가 바라는\n직업이지.",
                L"내가 무슨 생각을 하고 싶은지 생각할 수\n있다니!",
                L"난 노숙자가 아니야. 집을 찾아다니는 게\n직업일 뿐...",
                L"강국이라면 군대에 아무도 사용법을\n모르는 무기도 많아야 마땅하지."
            }};
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedTabIndex)
    {
        FBuildingUiSnapshot BuildingSnapshot;

        if (!BuildBuildingUiSnapshot(World, BuildingName, BuildingSnapshot))
            return FCitizenInfoSnapshot();

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Building;
        Result.SelectedTabIndex = (std::max)(0, (std::min)(GBuildingTabCount - 1, SelectedTabIndex));
        Result.BudgetLevel = BuildingSnapshot.BudgetLevel;
        Result.Title = BuildingSnapshot.DisplayName.empty() ?
            BuildingSnapshot.ObjectName :
            BuildingSnapshot.DisplayName;

        if (UseHydroponicFarmWorkOverview(BuildingSnapshot) ||
            UseAquaParkWorkOverview(BuildingSnapshot) ||
            UseRestaurantWorkOverview(BuildingSnapshot))
        {
            Result.Title = L"II " + Result.Title;
        }

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.Subtitle =
                std::wstring(GetBuildingEraDisplayName(
                    BuildingSnapshot.CatalogEntry->UnlockEra)) +
                L"  |  " +
                BuildingSnapshot.CategoryName;
        }
        else
        {
            Result.Subtitle = BuildingSnapshot.CategoryName;
        }

        static const wchar_t* GPageTitles[GBuildingTabCount] =
        {
            L"",
            L"통계",
            L"업그레이드",
            L"효율",
            L"정보"
        };

        Result.PageTitle = GPageTitles[Result.SelectedTabIndex];
        Result.ShowTabButtons = true;
        Result.ShowSectionRibbon = Result.SelectedTabIndex != 0;
        Result.ShowBudgetControls = Result.SelectedTabIndex == 0;
        Result.ShowActionButtons = Result.SelectedTabIndex == 0;
        Result.ShowBuildingOverview =
            Result.SelectedTabIndex == 0 &&
            BuildingSnapshot.Residential;
        Result.ShowBuildingWorkOverview =
            Result.SelectedTabIndex == 0 &&
            (UseHarborWorkOverview(BuildingSnapshot) ||
                UseHydroponicFarmWorkOverview(BuildingSnapshot) ||
                UseAquaParkWorkOverview(BuildingSnapshot) ||
                UseRestaurantWorkOverview(BuildingSnapshot));
        Result.ShowBuildingMetricRows =
            (Result.SelectedTabIndex == 1 ||
                (Result.SelectedTabIndex == 3 &&
                    UseModernApartmentOverview(BuildingSnapshot))) &&
            BuildingSnapshot.Residential;
        Result.ShowBuildingUpgradeCard =
            Result.SelectedTabIndex == 2 &&
            UseModernApartmentUpgradeCard(BuildingSnapshot);
        Result.ShowBuildingInformationParagraphs =
            Result.SelectedTabIndex == 4 &&
            UseModernApartmentOverview(BuildingSnapshot);
        Result.ShowSectionRibbon =
            Result.SelectedTabIndex != 0 &&
            !Result.ShowBuildingInformationParagraphs;
        Result.ShowOverviewCommandButton =
            Result.SelectedTabIndex == 0 &&
            UseHydroponicFarmWorkOverview(BuildingSnapshot);
        Result.OverviewCommandButtonText =
            Result.ShowOverviewCommandButton ?
                L"자원 변경" :
                std::wstring();

        if (UseHydroponicFarmWorkOverview(BuildingSnapshot))
        {
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText = L"< 코코아 >";
        }

        const long long TotalMonthlyCost =
            static_cast<long long>(BuildingSnapshot.MonthlyWageCost) +
            static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
        Result.BudgetText =
            L"예산 단계 " +
            std::to_wstring(BuildingSnapshot.BudgetLevel) +
            L"  |  " +
            FormatMultiplier(BuildingSnapshot.BudgetScale) +
            L"  |  월 비용 " +
            FormatMoney(TotalMonthlyCost);

        if (BuildingSnapshot.CatalogEntry)
        {
            Result.TitleIconPath = GetCatalogEntryIconPath(
                BuildingSnapshot.CatalogEntry->Category,
                BuildingSnapshot.CatalogEntry->CategoryLocalIndex);

            if (Result.TitleIconPath)
            {
                Result.ShowTitleIcon = true;
                Result.TitleIconTextureKey =
                    "CitizenInfoTitleIcon_" +
                    BuildingSnapshot.CatalogEntry->Id;
            }
        }

        if (Result.ShowBuildingOverview)
        {
            const int PowerSurplusMW =
                BuildingSnapshot.TotalProducedPowerMW -
                BuildingSnapshot.TotalRequiredPowerMW;
            const int HousingQuality =
                ResolveOverviewHousingQuality(BuildingSnapshot);
            const long long MonthlyIncome =
                ResolveOverviewMonthlyIncome(BuildingSnapshot);
            const int RequiredPowerMW =
                ResolveOverviewRequiredPower(BuildingSnapshot);

            Result.OverviewBudgetLabel = L"예산";
            Result.OverviewBudgetValue =
                FormatMoney(BuildingSnapshot.MonthlyUpkeepCost);
            Result.OverviewOccupancyLabel = L"거주지";
            Result.OverviewOccupancyValue =
                std::to_wstring(BuildingSnapshot.Residents.size()) +
                L" / " +
                std::to_wstring((std::max)(0, BuildingSnapshot.Capacity));
            Result.OverviewResidentCount =
                static_cast<int>(BuildingSnapshot.Residents.size());
            Result.OverviewResidentCapacity =
                (std::max)(0, BuildingSnapshot.Capacity);
            Result.OverviewMetricLabels =
            {
                L"주거 품질",
                L"필요 재산",
                L"월간 수입",
                L"전기",
                L"전력의",
                L"전력망 현황"
            };
            Result.OverviewMetricValues =
            {
                std::to_wstring(HousingQuality),
                NormalizeWealthRequirementText(
                    BuildingSnapshot.WealthRequirementText),
                FormatMoney(MonthlyIncome),
                L"-" + std::to_wstring(RequiredPowerMW) + L"메가와트",
                L"#1",
                std::wstring(PowerSurplusMW >= 0 ? L"+" : L"") +
                std::to_wstring(PowerSurplusMW) +
                    L"메가와트"
            };
        }

        if (Result.ShowBuildingWorkOverview)
        {
            if (UseHydroponicFarmWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel = L"근무 형태";
                Result.OverviewWorkModeValue = L"대체로 천연";
                Result.OverviewBudgetLabel = L"예산";
                Result.OverviewBudgetValue = L"$95";
                Result.OverviewOccupancyLabel = L"노동자";
                Result.OverviewOccupancyValue = L"4 / 4";
                Result.OverviewResidentCount = 4;
                Result.OverviewResidentCapacity = 4;
                Result.OverviewMetricLabels =
                {
                    L"직업 품질",
                    L"요구 학력",
                    L"임금",
                    L"효율",
                    L"전기",
                    L"전력망",
                    L"전력망 현황",
                    L"보관소",
                    L"생산",
                    L"코코아",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"65",
                    L"고등학교",
                    L"$18",
                    L"145%",
                    L"-30메가와트",
                    L"#1",
                    L"+785메가와트",
                    L"",
                    L"",
                    L"0 / 2320",
                    L"",
                    L""
                };
            }
            else if (UseAquaParkWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel = L"근무 형태";
                Result.OverviewWorkModeValue = L"산뜻하고 깔끔하게";
                Result.OverviewBudgetLabel = L"예산";
                Result.OverviewBudgetValue = L"$57 / $75";
                Result.OverviewOccupancyLabel = L"노동자";
                Result.OverviewOccupancyValue = L"1 / 3";
                Result.OverviewResidentCount = 1;
                Result.OverviewResidentCapacity = 3;
                Result.ShowBuildingVisitorIcons = true;
                Result.OverviewVisitorCount = 9;
                Result.OverviewVisitorCapacity = 12;
                Result.OverviewMetricLabels =
                {
                    L"직업 품질",
                    L"요구 학력",
                    L"임금",
                    L"효율",
                    L"전기",
                    L"전력망",
                    L"전력망 현황",
                    L"방문객",
                    L"서비스 품질",
                    L"필요 재산",
                    L"선호하는 유형:",
                    L"요금/수입",
                    L"□ 관광객 전용",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"65",
                    L"무학력",
                    L"$9",
                    L"125%",
                    L"-15메가와트",
                    L"#1",
                    L"+785메가와트",
                    L"9 / 6 (18)",
                    L"88",
                    L"유복함",
                    L"휴양",
                    L"$15 ($0)",
                    L" ",
                    L""
                };
            }
            else if (UseRestaurantWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel = L"근무 형태";
                Result.OverviewWorkModeValue = L"천 넘긴";
                Result.OverviewBudgetLabel = L"예산";
                Result.OverviewBudgetValue = L"$64";
                Result.OverviewOccupancyLabel = L"노동자";
                Result.OverviewOccupancyValue = L"4 / 4";
                Result.OverviewResidentCount = 4;
                Result.OverviewResidentCapacity = 4;
                Result.ShowBuildingVisitorIcons = true;
                Result.OverviewVisitorCount = 12;
                Result.OverviewVisitorCapacity = 12;
                Result.OverviewMetricLabels =
                {
                    L"직업 품질",
                    L"요구 학력",
                    L"임금",
                    L"효율",
                    L"전기",
                    L"전력망",
                    L"전력망 현황",
                    L"방문객",
                    L"서비스 품질",
                    L"필요 재산",
                    L"요금 수입",
                    L"□ 관광객 전용",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"55",
                    L"무학력",
                    L"$13",
                    L"125%",
                    L"-10메가와트",
                    L"#1",
                    L"+785메가와트",
                    L"12 / 12",
                    L"96",
                    L"유복함",
                    L"$18 ($0)",
                    L" ",
                    L"",
                    L""
                };
            }
            else
            {
                Result.OverviewWorkModeLabel = L"근무 형태";
                Result.OverviewWorkModeValue =
                    !BuildingSnapshot.OperationModes.empty() ?
                    BuildingSnapshot.OperationModes.front() :
                    L"일반 통제";
                Result.OverviewBudgetLabel = L"예산";
                Result.OverviewBudgetValue = L"$120";
                Result.OverviewOccupancyLabel = L"노동자";
                Result.OverviewOccupancyValue =
                    std::to_wstring(BuildingSnapshot.AssignedEmployees.size()) +
                    L" / " +
                    std::to_wstring((std::max)(0, BuildingSnapshot.Capacity));
                Result.OverviewResidentCount =
                    static_cast<int>(BuildingSnapshot.AssignedEmployees.size());
                Result.OverviewResidentCapacity =
                    (std::max)(0, BuildingSnapshot.Capacity);
                Result.OverviewMetricLabels =
                {
                    L"직업 품질",
                    L"요구 학력",
                    L"임금",
                    L"효율",
                    L"항구",
                    L"다음 도착 시각:",
                    L"전체 비용:",
                    L"예상 수익:",
                    L"보관소",
                    L"설탕",
                    L"옥수수",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"50",
                    GetCitizenEducationDisplayName(
                        BuildingSnapshot.Building->GetRequiredEducationLevel()),
                    L"$18",
                    L"135%",
                    L"",
                    L"20일",
                    L"$25,008",
                    L"$12,561",
                    L"",
                    L"3000 / 10000",
                    L"6000 / 10000",
                    L""
                };
            }
        }

        if (Result.ShowBuildingMetricRows && Result.SelectedTabIndex == 1)
        {
            if (UseModernApartmentOverview(BuildingSnapshot))
            {
                Result.OverviewMetricLabels =
                {
                    L"수입 (전체)",
                    L"수입 (전월)",
                    L"",
                    L"",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"$5,130",
                    FormatMoneyDollarFirst(-90),
                    L"",
                    L"",
                    L"",
                    L""
                };
            }
            else
            {
                const long long IncomeTotal =
                    ResolveOverviewMonthlyIncome(BuildingSnapshot) * 12LL;
                const long long IncomePrevious =
                    ResolveOverviewMonthlyIncome(BuildingSnapshot) -
                    static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
                Result.OverviewMetricLabels =
                {
                    L"수입 (전체)",
                    L"수입 (전월)",
                    L"",
                    L"",
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    FormatMoney(IncomeTotal),
                    FormatMoneyDollarFirst(IncomePrevious),
                    L"",
                    L"",
                    L"",
                    L""
                };
            }
        }

        if (Result.ShowBuildingMetricRows && Result.SelectedTabIndex == 3)
        {
            Result.ShowHeaderNote = true;
            Result.ShowSectionDivider = true;
            Result.HeaderNoteText =
                L"주거 품질은 효율에 따라 변합니다.";
            Result.OverviewMetricLabels =
            {
                L"효율",
                L"",
                L"",
                L"",
                L"",
                L""
            };
            Result.OverviewMetricValues =
            {
                L"100%",
                L"",
                L"",
                L"",
                L"",
                L""
            };
        }

        if (Result.ShowBuildingUpgradeCard)
        {
            Result.UpgradeCardTitle = L"태양광 패널 창문";
            Result.UpgradeCardDescription =
                L"건물의 전력 필요량이 -30메가와트\n감소합니다.";
            Result.UpgradeCardIconPath = TEXT(
                "TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_solarPowerPlant.png");
            Result.UpgradeCardIconTextureKey =
                "CitizenInfoUpgradeCardIcon_ModernApartment";
        }

        if (Result.ShowBuildingInformationParagraphs)
        {
            Result.ShowSectionDivider = true;
            Result.InformationAccentText =
                std::to_wstring((std::max)(0, BuildingSnapshot.Capacity));
            Result.InformationTopText =
                std::wstring(
                    L"가구를 수용할 수 있는 고급 주거\n"
                    L"건물입니다. 입주자의 재산 수준이\n"
                    L"최소 ") +
                NormalizeWealthRequirementText(
                    BuildingSnapshot.WealthRequirementText) +
                L" 이상이어야 합니다. 적은\n"
                L"공해를 배출합니다.";
            Result.InformationBottomText =
                L"콘크리트와 강철로 이루어진 괴물,\n"
                L"현대식 아파트는 '현대적인 편의 시설\n"
                L"완비'라는 광고 문구를 이용해\n"
                L"제정신이 아닌 듯한 월세를 제시합니다.";
        }

        switch (Result.SelectedTabIndex)
        {
        case 1:
            Result.BodyText = Result.ShowBuildingMetricRows ?
                std::wstring() :
                BuildStatisticsBody(BuildingSnapshot);
            break;
        case 2:
            Result.BodyText = Result.ShowBuildingUpgradeCard ?
                std::wstring() :
                BuildUpgradesBody(BuildingSnapshot);
            break;
        case 3:
            Result.BodyText = Result.ShowBuildingMetricRows ?
                std::wstring() :
                BuildEfficiencyBody(BuildingSnapshot);
            break;
        case 4:
            Result.BodyText = Result.ShowBuildingInformationParagraphs ?
                std::wstring() :
                BuildInformationBody(BuildingSnapshot);
            break;
        case 0:
        default:
            Result.BodyText = Result.ShowBuildingOverview ?
                std::wstring() :
                BuildOverviewBody(BuildingSnapshot);
            break;
        }

        return Result;
    }
}
