#include "CitizenInfoWidget.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementController.h"
#include "../ObjectNames.h"
#include "../World/MainWorldAccess.h"
#include "TropicoUiStyle.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;

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

    void SetPanelTextStyle(
        const std::shared_ptr<CTextBlock>& Text,
        float FontSize,
        const FVector4& Color,
        ETextAlignH AlignH = ETextAlignH::Left,
        ETextAlignV AlignV = ETextAlignV::Middle,
        bool Shadow = false)
    {
        if (!Text)
            return;

        Text->SetFontSize(FontSize);
        Text->SetAlignH(AlignH);
        Text->SetAlignV(AlignV);
        Text->SetTextColor(Color);
        Text->EnableShadow(Shadow);

        if (Shadow)
        {
            Text->SetShadowOffset(1.f, 1.f);
            Text->SetShadowTextColor(245, 235, 205, 180);
        }
    }
}

CCitizenInfoWidget::CCitizenInfoWidget()
{
}

CCitizenInfoWidget::~CCitizenInfoWidget()
{
}

bool CCitizenInfoWidget::Init()
{
    CWidgetContainer::Init();

    auto PanelImage = CreateWidget<CImage>("CitizenInfo_Panel", 6).lock();

    if (PanelImage)
    {
        PanelImage->SetTexture("CitizenInfoPanelTexture", GModernPanelTexture);
        PanelImage->SetTint(1.f, 1.f, 1.f, 0.98f);
        mPanelImage = PanelImage;
    }

    auto InnerFrame = CreateWidget<CImage>("CitizenInfo_InnerFrame", 7).lock();

    if (InnerFrame)
    {
        InnerFrame->SetTexture(
            "CitizenInfoInnerFrameTexture",
            GMenuDetailFrameTexture);
        InnerFrame->SetTint(1.f, 1.f, 1.f, 0.98f);
        mInnerFrame = InnerFrame;
    }

    auto TitleRibbon = CreateWidget<CImage>("CitizenInfo_TitleRibbon", 8).lock();

    if (TitleRibbon)
    {
        TitleRibbon->SetTexture(
            "CitizenInfoTitleRibbonTexture",
            GMenuTitleRibbonTexture);
        TitleRibbon->SetTint(1.f, 1.f, 1.f, 1.f);
        mTitleRibbon = TitleRibbon;
    }

    auto SectionRibbon =
        CreateWidget<CImage>("CitizenInfo_SectionRibbon", 8).lock();

    if (SectionRibbon)
    {
        SectionRibbon->SetTexture(
            "CitizenInfoSectionRibbonTexture",
            GMenuTitleRibbonTexture);
        SectionRibbon->SetTint(1.f, 1.f, 1.f, 0.96f);
        mSectionRibbon = SectionRibbon;
    }

    auto ScrollTrack = CreateWidget<CImage>("CitizenInfo_ScrollTrack", 8).lock();

    if (ScrollTrack)
    {
        ScrollTrack->SetTexture(
            "CitizenInfoScrollTrackTexture",
            GScrollTrackTexture);
        ScrollTrack->SetTint(1.f, 1.f, 1.f, 0.92f);
        mScrollTrack = ScrollTrack;
    }

    auto ScrollThumb = CreateWidget<CImage>("CitizenInfo_ScrollThumb", 9).lock();

    if (ScrollThumb)
    {
        ScrollThumb->SetTexture(
            "CitizenInfoScrollThumbTexture",
            GScrollThumbTexture);
        ScrollThumb->SetTint(1.f, 1.f, 1.f, 0.98f);
        mScrollThumb = ScrollThumb;
    }

    auto TitleIcon = CreateWidget<CImage>("CitizenInfo_TitleIcon", 9).lock();

    if (TitleIcon)
    {
        TitleIcon->SetTint(1.f, 1.f, 1.f, 1.f);
        mTitleIcon = TitleIcon;
    }

    auto TitleText = CreateWidget<CTextBlock>("CitizenInfo_TitleText", 9).lock();

    if (TitleText)
    {
        SetPanelTextStyle(
            TitleText,
            26.f,
            FVector4(0.37f, 0.26f, 0.10f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            true);
        mTitleText = TitleText;
    }

    auto SubtitleText =
        CreateWidget<CTextBlock>("CitizenInfo_SubtitleText", 9).lock();

    if (SubtitleText)
    {
        SetPanelTextStyle(
            SubtitleText,
            15.f,
            FVector4(0.35f, 0.30f, 0.22f, 1.f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            false);
        mSubtitleText = SubtitleText;
    }

    auto PageTitleText =
        CreateWidget<CTextBlock>("CitizenInfo_PageTitleText", 9).lock();

    if (PageTitleText)
    {
        SetPanelTextStyle(
            PageTitleText,
            21.f,
            FVector4(0.37f, 0.26f, 0.10f, 1.f),
            ETextAlignH::Center,
            ETextAlignV::Middle,
            true);
        mPageTitleText = PageTitleText;
    }

    auto BodyText = CreateWidget<CTextBlock>("CitizenInfo_BodyText", 9).lock();

    if (BodyText)
    {
        SetPanelTextStyle(
            BodyText,
            18.f,
            FVector4(0.22f, 0.22f, 0.22f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Top,
            false);
        mBodyText = BodyText;
    }

    auto BudgetText =
        CreateWidget<CTextBlock>("CitizenInfo_BudgetText", 9).lock();

    if (BudgetText)
    {
        SetPanelTextStyle(
            BudgetText,
            16.f,
            FVector4(0.24f, 0.24f, 0.24f, 1.f),
            ETextAlignH::Left,
            ETextAlignV::Middle,
            false);
        mBudgetText = BudgetText;
    }

    auto CloseButton = CreateWidget<CButton>("CitizenInfo_CloseButton", 9).lock();

    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "CitizenInfoClose",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click, this,
            &CCitizenInfoWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_CloseText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            SetPanelTextStyle(
                CloseText,
                20.f,
                FVector4(0.38f, 0.25f, 0.08f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    static const wchar_t* GTabLabels[GBuildingTabCount] =
    {
        L"기본",
        L"통계",
        L"업글",
        L"효율",
        L"정보"
    };

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = CreateWidget<CButton>(
            "CitizenInfo_Tab_" + std::to_string(Index + 1), 8).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "CitizenInfoTabTexture_" + std::to_string(Index),
            GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, false);
        Button->SetEventCallback(
            EButtonEventState::Click,
            [this, Index]()
            {
                SelectBuildingTab(
                    static_cast<EBuildingInfoTab>(Index));
            });

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_TabLabel_" + std::to_string(Index + 1),
            mWorld);

        if (Label)
        {
            Label->SetText(GTabLabels[Index]);
            SetPanelTextStyle(
                Label,
                14.f,
                FVector4(0.28f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
            mTabButtonTexts[static_cast<size_t>(Index)] = Label;
        }

        mTabButtons[static_cast<size_t>(Index)] = Button;
    }

    void (CCitizenInfoWidget::*BudgetCallbacks[GBudgetLevelCount])() =
    {
        &CCitizenInfoWidget::OnBudgetLevel1Click,
        &CCitizenInfoWidget::OnBudgetLevel2Click,
        &CCitizenInfoWidget::OnBudgetLevel3Click,
        &CCitizenInfoWidget::OnBudgetLevel4Click,
        &CCitizenInfoWidget::OnBudgetLevel5Click
    };

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = CreateWidget<CButton>(
            "CitizenInfo_BudgetButton_" + std::to_string(Index + 1),
            9).lock();

        if (!Button)
            continue;

        ApplyButtonTextureSet(
            Button,
            "CitizenInfoBudgetButtonTexture_" + std::to_string(Index),
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);
        Button->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click, this, BudgetCallbacks[Index]);

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenInfo_BudgetLabel_" + std::to_string(Index + 1),
            mWorld);

        if (Label)
        {
            wchar_t Buffer[8] = {};
            swprintf_s(Buffer, L"%d", Index + 1);
            Label->SetText(Buffer);
            SetPanelTextStyle(
                Label,
                16.f,
                FVector4(0.30f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
            mBudgetButtonTexts[static_cast<size_t>(Index)] = Label;
        }

        mBudgetButtons[static_cast<size_t>(Index)] = Button;
    }

    auto CreateActionButton =
        [this](
            const std::string& Name,
            const wchar_t* LabelText,
            void (CCitizenInfoWidget::*Callback)())
        -> std::shared_ptr<CButton>
    {
        auto Button = CreateWidget<CButton>(Name, 9).lock();

        if (!Button)
            return std::shared_ptr<CButton>();

        ApplyButtonTextureSet(
            Button,
            Name + "_Texture",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);
        ConfigureDefaultButtonStyle(Button);

        if (Callback)
        {
            Button->SetEventCallback<CCitizenInfoWidget>(
                EButtonEventState::Click, this, Callback);
        }

        auto Label = CWidget::CreateStaticWidget<CTextBlock>(
            Name + "_Label", mWorld);

        if (Label)
        {
            Label->SetText(LabelText);
            SetPanelTextStyle(
                Label,
                18.f,
                FVector4(0.29f, 0.22f, 0.12f, 1.f),
                ETextAlignH::Center,
                ETextAlignV::Middle,
                true);
            Button->SetChild(Label);
        }

        return Button;
    };

    mDemolishButton =
        CreateActionButton(
            "CitizenInfo_DemolishButton",
            L"철거",
            &CCitizenInfoWidget::OnDemolishButtonClick);
    mMoveButton =
        CreateActionButton(
            "CitizenInfo_MoveButton",
            L"이동",
            &CCitizenInfoWidget::OnMoveButtonClick);
    mCloneButton =
        CreateActionButton(
            "CitizenInfo_CloneButton",
            L"복제",
            &CCitizenInfoWidget::OnCloneButtonClick);

    if (auto CloneButton = mCloneButton.lock())
    {
        CloneButton->ButtonEnable(false);
        CloneButton->SetOpacityAll(0.72f);
    }

    SetTitle(L"-");
    SetSubtitle(L"");
    SetBodyText(L"");
    SetBudgetControlsVisible(false);
    SetActionButtonsVisible(false);
    SetTabButtonsVisible(false);
    RefreshBuildingTabState();
    SetPanelScreenPos(FVector2(0.f, 0.f));
    SetEnable(false);

    return true;
}

void CCitizenInfoWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    (void)DeltaTime;

    if (!GetEnable())
        return;

    if (!mTrackedCitizenName.empty())
    {
        auto World = mWorld.lock();

        if (!World)
            return;

        auto Citizen = World->FindObject<CBuildingMarkerOrb>(
            mTrackedCitizenName).lock();

        if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
        {
            mTrackedCitizenName.clear();
            SetEnable(false);
            return;
        }

        SetCitizenSatisfaction(
            Citizen->GetSatisfaction(),
            Citizen->GetIdentityProfile(),
            Citizen->GetPoliticalProfile());
        return;
    }

    if (!mTrackedBuildingName.empty())
        RefreshBuildingInfo();
}

void CCitizenInfoWidget::Render()
{
    CWidgetContainer::Render();
}

void CCitizenInfoWidget::OpenCitizen(
    const std::string& CitizenName,
    const FNpcSatisfaction& Satisfaction,
    const FVector2& ScreenPos)
{
    SetPanelScreenPos(ScreenPos);
    mPanelMode = EPanelMode::Citizen;
    mTrackedCitizenName = CitizenName;
    mTrackedBuildingName.clear();
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    RefreshModeVisibility();
    SetTitle(Utf8ToWide(CitizenName));
    SetSubtitle(L"시민 정보");

    FNpcPoliticalProfile PoliticalProfile;
    FCitizenIdentityProfile IdentityProfile;
    auto World = mWorld.lock();

    if (World)
    {
        auto Citizen = World->FindObject<CBuildingMarkerOrb>(
            CitizenName).lock();

        if (Citizen && Citizen->GetAlive() && Citizen->GetEnable())
        {
            PoliticalProfile = Citizen->GetPoliticalProfile();
            IdentityProfile = Citizen->GetIdentityProfile();
        }
    }

    SetCitizenSatisfaction(
        Satisfaction,
        IdentityProfile,
        PoliticalProfile);
    SetEnable(true);
}

void CCitizenInfoWidget::OpenBuilding(
    const std::string& BuildingObjectName,
    const std::string& BuildingDisplayName,
    const std::string& CategoryName,
    bool IsResidential,
    int Capacity,
    const FVector2& ScreenPos)
{
    (void)BuildingDisplayName;
    (void)CategoryName;
    (void)IsResidential;
    (void)Capacity;

    SetPanelScreenPos(ScreenPos);
    mPanelMode = EPanelMode::Building;
    mTrackedCitizenName.clear();
    mTrackedBuildingName = BuildingObjectName;
    mSelectedBuildingTab = EBuildingInfoTab::Overview;
    RefreshModeVisibility();
    RefreshBuildingInfo();
    SetEnable(true);
}

void CCitizenInfoWidget::SetTitle(const std::wstring& Title)
{
    auto TitleText = mTitleText.lock();

    if (TitleText)
        TitleText->SetText(Title.c_str());
}

void CCitizenInfoWidget::SetSubtitle(const std::wstring& Subtitle)
{
    auto SubtitleText = mSubtitleText.lock();

    if (SubtitleText)
    {
        SubtitleText->SetText(Subtitle.c_str());
        SubtitleText->SetEnable(!Subtitle.empty());
    }
}

void CCitizenInfoWidget::SetCitizenSatisfaction(
    const FNpcSatisfaction& Satisfaction,
    const FCitizenIdentityProfile& IdentityProfile,
    const FNpcPoliticalProfile& PoliticalProfile)
{
    auto ToPercent = [](float Value)
    {
        const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
        return static_cast<int>(roundf(Clamped));
    };

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

    SetBodyText(Buffer);
}

void CCitizenInfoWidget::SetBodyText(const std::wstring& Text)
{
    auto BodyText = mBodyText.lock();

    if (BodyText)
        BodyText->SetText(Text.c_str());
}

void CCitizenInfoWidget::SetPanelScreenPos(const FVector2& ScreenPos)
{
    (void)ScreenPos;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mPanelWidth = (std::min)(
        410.f,
        (std::max)(320.f, static_cast<float>(Resolution.Width) * 0.24f));
    mPanelTop = 58.f;
    mPanelHeight = (std::max)(
        420.f,
        static_cast<float>(Resolution.Height) - mPanelTop - 10.f);

    const float PanelLeft =
        static_cast<float>(Resolution.Width) - mPanelWidth - 10.f;

    SetPos(PanelLeft, 0.f);
    SetSize(mPanelWidth, mPanelTop + mPanelHeight);
    RefreshPanelLayout();
}

void CCitizenInfoWidget::RefreshPanelLayout()
{
    const float OuterLeft = 0.f;
    const float OuterTop = mPanelTop;
    const float PanelWidth = mPanelWidth;
    const float PanelHeight = mPanelHeight;
    const float InnerLeft = 18.f;
    const float InnerTop = OuterTop + 16.f;
    const float InnerWidth = PanelWidth - 36.f;
    const float InnerHeight = PanelHeight - 28.f;

    if (auto PanelImage = mPanelImage.lock())
    {
        PanelImage->SetPos(OuterLeft, OuterTop);
        PanelImage->SetSize(PanelWidth, PanelHeight);
    }

    if (auto InnerFrame = mInnerFrame.lock())
    {
        InnerFrame->SetPos(InnerLeft, InnerTop);
        InnerFrame->SetSize(InnerWidth, InnerHeight);
    }

    if (auto ScrollTrack = mScrollTrack.lock())
    {
        ScrollTrack->SetPos(8.f, OuterTop + 74.f);
        ScrollTrack->SetSize(15.f, PanelHeight - 126.f);
    }

    if (auto ScrollThumb = mScrollThumb.lock())
    {
        ScrollThumb->SetPos(8.5f, OuterTop + 88.f);
        ScrollThumb->SetSize(14.f, 94.f);
    }

    const float TabWidth = 56.f;
    const float TabGap = 10.f;
    const float TotalTabsWidth =
        static_cast<float>(GBuildingTabCount) * TabWidth +
        static_cast<float>(GBuildingTabCount - 1) * TabGap;
    const float TabStartX = (std::max)(
        4.f,
        (PanelWidth - TotalTabsWidth) * 0.5f);

    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = mTabButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            TabStartX + static_cast<float>(Index) * (TabWidth + TabGap),
            0.f);
        Button->SetSize(TabWidth, 56.f);
    }

    if (auto TitleRibbon = mTitleRibbon.lock())
    {
        TitleRibbon->SetPos(40.f, OuterTop + 18.f);
        TitleRibbon->SetSize(PanelWidth - 96.f, 44.f);
    }

    if (auto CloseButton = mCloseButton.lock())
    {
        CloseButton->SetPos(PanelWidth - 38.f, OuterTop + 4.f);
        CloseButton->SetSize(34.f, 34.f);
    }

    const bool TitleIconVisible =
        mPanelMode == EPanelMode::Building &&
        !mTitleIcon.expired() &&
        mTitleIcon.lock()->GetEnable();
    const float TitleLeft = TitleIconVisible ? 78.f : 54.f;

    if (auto TitleIcon = mTitleIcon.lock())
    {
        TitleIcon->SetPos(53.f, OuterTop + 27.f);
        TitleIcon->SetSize(22.f, 22.f);
    }

    if (auto TitleText = mTitleText.lock())
    {
        TitleText->SetPos(TitleLeft, OuterTop + 18.f);
        TitleText->SetSize(PanelWidth - TitleLeft - 62.f, 44.f);
    }

    if (auto SubtitleText = mSubtitleText.lock())
    {
        SubtitleText->SetPos(46.f, OuterTop + 67.f);
        SubtitleText->SetSize(PanelWidth - 92.f, 22.f);
    }

    const bool ShowSectionRibbon =
        mPanelMode == EPanelMode::Building &&
        mSelectedBuildingTab != EBuildingInfoTab::Overview;

    if (auto SectionRibbon = mSectionRibbon.lock())
    {
        SectionRibbon->SetPos(42.f, OuterTop + 96.f);
        SectionRibbon->SetSize(PanelWidth - 84.f, 34.f);
        SectionRibbon->SetEnable(ShowSectionRibbon);
    }

    if (auto PageTitleText = mPageTitleText.lock())
    {
        PageTitleText->SetPos(42.f, OuterTop + 96.f);
        PageTitleText->SetSize(PanelWidth - 84.f, 34.f);
        PageTitleText->SetEnable(ShowSectionRibbon);
    }

    if (auto BudgetText = mBudgetText.lock())
    {
        BudgetText->SetPos(44.f, OuterTop + 102.f);
        BudgetText->SetSize(PanelWidth - 88.f, 24.f);
    }

    const float BudgetButtonTop = OuterTop + 132.f;
    const float BudgetGap = 8.f;
    const float BudgetButtonWidth =
        (PanelWidth - 88.f - BudgetGap * 4.f) / 5.f;

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = mBudgetButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetPos(
            44.f + static_cast<float>(Index) *
                (BudgetButtonWidth + BudgetGap),
            BudgetButtonTop);
        Button->SetSize(BudgetButtonWidth, 30.f);
    }

    const bool ShowActions =
        mPanelMode == EPanelMode::Building &&
        mSelectedBuildingTab == EBuildingInfoTab::Overview;
    const float ActionTop = OuterTop + PanelHeight - 52.f;

    if (auto DemolishButton = mDemolishButton.lock())
    {
        DemolishButton->SetPos(42.f, ActionTop);
        DemolishButton->SetSize(124.f, 34.f);
    }

    if (auto MoveButton = mMoveButton.lock())
    {
        MoveButton->SetPos(PanelWidth - 154.f, ActionTop);
        MoveButton->SetSize(50.f, 34.f);
    }

    if (auto CloneButton = mCloneButton.lock())
    {
        CloneButton->SetPos(PanelWidth - 98.f, ActionTop);
        CloneButton->SetSize(50.f, 34.f);
    }

    const float BodyTop =
        ShowSectionRibbon ? (OuterTop + 142.f) :
        (ShowActions ? (OuterTop + 172.f) : (OuterTop + 102.f));
    const float BodyBottom =
        ShowActions ? (ActionTop - 18.f) :
        (OuterTop + PanelHeight - 28.f);

    if (auto BodyText = mBodyText.lock())
    {
        BodyText->SetPos(44.f, BodyTop);
        BodyText->SetSize(
            PanelWidth - 88.f,
            (std::max)(80.f, BodyBottom - BodyTop));
    }
}

void CCitizenInfoWidget::RefreshModeVisibility()
{
    const bool BuildingMode = mPanelMode == EPanelMode::Building;
    const bool ShowOverviewControls =
        BuildingMode &&
        mSelectedBuildingTab == EBuildingInfoTab::Overview;

    UpdateBuildingTitleIcon(nullptr);
    SetTabButtonsVisible(BuildingMode);
    SetBudgetControlsVisible(ShowOverviewControls);
    SetActionButtonsVisible(ShowOverviewControls);

    if (auto SectionRibbon = mSectionRibbon.lock())
    {
        SectionRibbon->SetEnable(
            BuildingMode &&
            mSelectedBuildingTab != EBuildingInfoTab::Overview);
    }

    if (auto PageTitleText = mPageTitleText.lock())
    {
        PageTitleText->SetEnable(
            BuildingMode &&
            mSelectedBuildingTab != EBuildingInfoTab::Overview);
    }

    RefreshPanelLayout();
}

void CCitizenInfoWidget::RefreshBuildingInfo()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    FBuildingUiSnapshot Snapshot;

    if (!BuildBuildingUiSnapshot(World, mTrackedBuildingName, Snapshot))
    {
        mTrackedBuildingName.clear();
        SetEnable(false);
        return;
    }

    UpdateBuildingTitleIcon(Snapshot.CatalogEntry);
    SetTitle(Snapshot.DisplayName.empty() ?
        Snapshot.ObjectName :
        Snapshot.DisplayName);

    std::wstring Subtitle;

    if (Snapshot.CatalogEntry)
    {
        Subtitle =
            std::wstring(GetBuildingEraDisplayName(
                Snapshot.CatalogEntry->UnlockEra)) +
            L"  |  " +
            Snapshot.CategoryName;
    }
    else
    {
        Subtitle = Snapshot.CategoryName;
    }

    SetSubtitle(Subtitle);

    static const wchar_t* GPageTitles[GBuildingTabCount] =
    {
        L"",
        L"통계",
        L"업그레이드",
        L"효율",
        L"정보"
    };

    if (auto PageTitleText = mPageTitleText.lock())
    {
        PageTitleText->SetText(
            GPageTitles[static_cast<int>(mSelectedBuildingTab)]);
    }

    auto BudgetText = mBudgetText.lock();

    if (BudgetText)
    {
        const long long TotalMonthlyCost =
            static_cast<long long>(Snapshot.MonthlyWageCost) +
            static_cast<long long>(Snapshot.MonthlyUpkeepCost);
        std::wstring BudgetSummary =
            L"예산 단계 " +
            std::to_wstring(Snapshot.BudgetLevel) +
            L"  |  " +
            FormatMultiplier(Snapshot.BudgetScale) +
            L"  |  월 비용 " +
            FormatMoney(TotalMonthlyCost);
        BudgetText->SetText(BudgetSummary.c_str());
    }

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = mBudgetButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        const bool Selected = Snapshot.BudgetLevel == Index + 1;
        ApplyButtonTextureSet(
            Button,
            "CitizenInfoBudgetRefresh_" + std::to_string(Index),
            Selected ?
                GBigTextButtonSelectedTexture :
                GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonSelectedTexture,
            GBigTextButtonDisabledTexture);

        auto Label = mBudgetButtonTexts[static_cast<size_t>(Index)].lock();

        if (Label)
        {
            Label->SetTextColor(
                Selected ?
                    FVector4(0.36f, 0.22f, 0.08f, 1.f) :
                    FVector4(0.30f, 0.22f, 0.12f, 1.f));
        }
    }

    std::wstring Body;

    switch (mSelectedBuildingTab)
    {
    case EBuildingInfoTab::Statistics:
        Body = BuildStatisticsBody(Snapshot);
        break;
    case EBuildingInfoTab::Upgrades:
        Body = BuildUpgradesBody(Snapshot);
        break;
    case EBuildingInfoTab::Efficiency:
        Body = BuildEfficiencyBody(Snapshot);
        break;
    case EBuildingInfoTab::Information:
        Body = BuildInformationBody(Snapshot);
        break;
    case EBuildingInfoTab::Overview:
    default:
        Body = BuildOverviewBody(Snapshot);
        break;
    }

    SetBodyText(Body);
    RefreshBuildingTabState();
    RefreshPanelLayout();
}

void CCitizenInfoWidget::RefreshBuildingTabState()
{
    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = mTabButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        const bool Selected =
            static_cast<int>(mSelectedBuildingTab) == Index;
        ApplyButtonTextureSet(
            Button,
            "CitizenInfoTabRefresh_" + std::to_string(Index),
            Selected ?
                GCategoryTabTextureSelected :
                GCategoryTabTextureHidden,
            GCategoryTabTextureSelected,
            GCategoryTabTextureSelected,
            GCategoryTabTextureHidden);
        ConfigureCategoryTabButtonStyle(Button, Selected);

        auto Label = mTabButtonTexts[static_cast<size_t>(Index)].lock();

        if (Label)
        {
            Label->SetTextColor(
                Selected ?
                    FVector4(0.27f, 0.17f, 0.06f, 1.f) :
                    FVector4(0.22f, 0.20f, 0.17f, 1.f));
        }
    }
}

void CCitizenInfoWidget::UpdateBuildingTitleIcon(
    const FBuildingCatalogEntry* CatalogEntry)
{
    auto TitleIcon = mTitleIcon.lock();

    if (!TitleIcon)
        return;

    if (mPanelMode != EPanelMode::Building || !CatalogEntry)
    {
        TitleIcon->SetEnable(false);
        return;
    }

    const wchar_t* IconPath = GetCatalogEntryIconPath(
        CatalogEntry->Category,
        CatalogEntry->CategoryLocalIndex);

    if (!IconPath ||
        !TitleIcon->SetTexture(
            "CitizenInfoTitleIcon_" + CatalogEntry->Id,
            IconPath))
    {
        TitleIcon->SetEnable(false);
        return;
    }

    TitleIcon->SetEnable(true);
}

void CCitizenInfoWidget::SetBudgetControlsVisible(bool Visible)
{
    if (auto BudgetText = mBudgetText.lock())
        BudgetText->SetEnable(Visible);

    for (int Index = 0; Index < GBudgetLevelCount; ++Index)
    {
        auto Button = mBudgetButtons[static_cast<size_t>(Index)].lock();

        if (!Button)
            continue;

        Button->SetEnable(Visible);
        Button->ButtonEnable(Visible);
    }
}

void CCitizenInfoWidget::SetActionButtonsVisible(bool Visible)
{
    if (auto DemolishButton = mDemolishButton.lock())
        DemolishButton->SetEnable(Visible);

    if (auto MoveButton = mMoveButton.lock())
        MoveButton->SetEnable(Visible);

    if (auto CloneButton = mCloneButton.lock())
        CloneButton->SetEnable(Visible);
}

void CCitizenInfoWidget::SetTabButtonsVisible(bool Visible)
{
    for (int Index = 0; Index < GBuildingTabCount; ++Index)
    {
        auto Button = mTabButtons[static_cast<size_t>(Index)].lock();

        if (Button)
            Button->SetEnable(Visible);
    }
}

void CCitizenInfoWidget::SelectBuildingTab(EBuildingInfoTab Tab)
{
    if (mSelectedBuildingTab == Tab)
        return;

    mSelectedBuildingTab = Tab;
    RefreshModeVisibility();

    if (!mTrackedBuildingName.empty())
        RefreshBuildingInfo();
}

void CCitizenInfoWidget::SetBuildingBudgetLevel(int Level)
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return;

    Building->SetBudgetLevel(Level);
    RefreshBuildingInfo();
}

void CCitizenInfoWidget::OnCloseButtonClick()
{
    mTrackedCitizenName.clear();
    mTrackedBuildingName.clear();
    SetEnable(false);
}

void CCitizenInfoWidget::OnDemolishButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementController =
        World->FindObject<CPlacementController>(
            GPlacementControllerName).lock();

    if (!PlacementController)
        return;

    if (!PlacementController->DemolishBuildingByName(mTrackedBuildingName))
        return;

    mTrackedBuildingName.clear();
    SetEnable(false);
}

void CCitizenInfoWidget::OnMoveButtonClick()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto PlacementController =
        World->FindObject<CPlacementController>(
            GPlacementControllerName).lock();

    if (!PlacementController)
        return;

    if (!PlacementController->BeginMoveExistingBuilding(mTrackedBuildingName))
        return;

    SetEnable(false);
}

void CCitizenInfoWidget::OnCloneButtonClick()
{
}

void CCitizenInfoWidget::OnBudgetLevel1Click()
{
    SetBuildingBudgetLevel(1);
}

void CCitizenInfoWidget::OnBudgetLevel2Click()
{
    SetBuildingBudgetLevel(2);
}

void CCitizenInfoWidget::OnBudgetLevel3Click()
{
    SetBuildingBudgetLevel(3);
}

void CCitizenInfoWidget::OnBudgetLevel4Click()
{
    SetBuildingBudgetLevel(4);
}

void CCitizenInfoWidget::OnBudgetLevel5Click()
{
    SetBuildingBudgetLevel(5);
}
