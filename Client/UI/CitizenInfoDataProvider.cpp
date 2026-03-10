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
        std::wstring WealthRequirementText;
        std::wstring TouristPreferenceText;
        std::wstring EffectText;
        std::wstring NoteText;
        std::wstring ServiceCapacityText;
        std::vector<std::wstring> NarrativeLines;
        std::vector<std::wstring> UpgradeHints;
        std::vector<std::wstring> OperationModes;
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
        float BudgetScale = 1.f;
        float HarborShipProgressPercent = 0.f;
        bool Residential = false;
        bool WorkProvider = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        bool UsesResourceStock = false;
        bool Harbor = false;
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
        OutSnapshot.Capacity = (std::max)(0, Building->GetCapacity());
        OutSnapshot.BudgetLevel = Building->GetBudgetLevel();
        OutSnapshot.BudgetScale = Building->GetBudgetSatisfactionScale();
        OutSnapshot.HousingCap = Building->GetHousingSatisfactionCap();
        OutSnapshot.JobCap = Building->GetJobSatisfactionCap();
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
            OutSnapshot.Harbor;

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
}

namespace CitizenInfoDataProvider
{
    FCitizenInfoSnapshot BuildCitizenSnapshot(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FCitizenIdentityProfile& IdentityProfile,
        const FNpcPoliticalProfile& PoliticalProfile)
    {
        auto ToPercent = [](float Value)
        {
            const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
            return static_cast<int>(roundf(Clamped));
        };

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Citizen;
        Result.Title = Utf8ToWide(CitizenName);
        Result.Subtitle = L"시민 정보";
        Result.ShowTabButtons = false;
        Result.ShowBudgetControls = false;
        Result.ShowActionButtons = false;
        Result.ShowSectionRibbon = false;
        Result.ShowTitleIcon = false;

        wchar_t Buffer[1024] = {};
        swprintf_s(Buffer,
            L"학력: %s\n"
            L"재산 계층: %s\n\n"
            L"음식: %d\n"
            L"의료: %d\n"
            L"오락: %d\n"
            L"신앙: %d\n"
            L"주거: %d\n"
            L"직업: %d\n"
            L"자유: %d\n"
            L"치안: %d\n"
            L"종합: %d\n\n"
            L"%s: %s (%s)\n"
            L"%s: %s (%s)\n"
            L"%s: %s (%s)\n"
            L"%s: %s (%s)",
            GetCitizenEducationDisplayName(IdentityProfile.EducationLevel),
            GetCitizenWealthDisplayName(IdentityProfile.WealthLevel),
            ToPercent(Satisfaction.Food),
            ToPercent(Satisfaction.Health),
            ToPercent(Satisfaction.Fun),
            ToPercent(Satisfaction.Faith),
            ToPercent(Satisfaction.Housing),
            ToPercent(Satisfaction.Job),
            ToPercent(Satisfaction.Freedom),
            ToPercent(Satisfaction.Security),
            ToPercent(Satisfaction.Overall),
            GetPoliticalAxisDisplayName(EPoliticalAxis::Economy),
            GetPoliticalFactionDisplayName(
                EPoliticalAxis::Economy,
                PoliticalProfile.Economy.Stance),
            GetPoliticalSupportDisplayName(
                PoliticalProfile.Economy.Support),
            GetPoliticalAxisDisplayName(EPoliticalAxis::ReligionMilitarism),
            GetPoliticalFactionDisplayName(
                EPoliticalAxis::ReligionMilitarism,
                PoliticalProfile.ReligionMilitarism.Stance),
            GetPoliticalSupportDisplayName(
                PoliticalProfile.ReligionMilitarism.Support),
            GetPoliticalAxisDisplayName(EPoliticalAxis::EnvironmentIndustry),
            GetPoliticalFactionDisplayName(
                EPoliticalAxis::EnvironmentIndustry,
                PoliticalProfile.EnvironmentIndustry.Stance),
            GetPoliticalSupportDisplayName(
                PoliticalProfile.EnvironmentIndustry.Support),
            GetPoliticalAxisDisplayName(EPoliticalAxis::IntellectualConservative),
            GetPoliticalFactionDisplayName(
                EPoliticalAxis::IntellectualConservative,
                PoliticalProfile.IntellectualConservative.Stance),
            GetPoliticalSupportDisplayName(
                PoliticalProfile.IntellectualConservative.Support));

        Result.BodyText = Buffer;
        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName)
    {
        if (!World || CitizenName.empty())
            return FCitizenInfoSnapshot();

        auto Citizen = World->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

        if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
            return FCitizenInfoSnapshot();

        return BuildCitizenSnapshot(
            CitizenName,
            Citizen->GetSatisfaction(),
            Citizen->GetIdentityProfile(),
            Citizen->GetPoliticalProfile());
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

        switch (Result.SelectedTabIndex)
        {
        case 1:
            Result.BodyText = BuildStatisticsBody(BuildingSnapshot);
            break;
        case 2:
            Result.BodyText = BuildUpgradesBody(BuildingSnapshot);
            break;
        case 3:
            Result.BodyText = BuildEfficiencyBody(BuildingSnapshot);
            break;
        case 4:
            Result.BodyText = BuildInformationBody(BuildingSnapshot);
            break;
        case 0:
        default:
            Result.BodyText = BuildOverviewBody(BuildingSnapshot);
            break;
        }

        return Result;
    }
}
