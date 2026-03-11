#include "CitizenInfoDataProvider.h"
#include "CitizenInfoConstants.h"
#include "CitizenInfoQueryService.h"
#include "UIStrings.h"
#include "../Building/BuildingCatalog.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <vector>

namespace
{
    using CitizenInfoConstants::GBuildingTabCount;
    using CitizenInfoConstants::GCitizenTabCount;

    struct FBuildingUiSnapshot
    {
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
        ECitizenEducationLevel RequiredEducationLevel =
            ECitizenEducationLevel::Uneducated;
        bool Residential = false;
        bool WorkProvider = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        bool UsesResourceStock = false;
        bool Harbor = false;
        bool Warehouse = false;
        bool IsRoad = false;
        bool CanGenerateWorkOutput = false;
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

    const std::wstring& Ui(const wchar_t* Key)
    {
        return UIStrings::Get(Key);
    }

    const wchar_t* UiText(const wchar_t* Key)
    {
        return UIStrings::Get(Key).c_str();
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

    std::wstring FormatMegawattValue(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.megawatt");
    }

    std::wstring FormatSignedMegawattValue(int Value)
    {
        return std::wstring(Value > 0 ? L"+" : L"") +
            FormatMegawattValue(Value);
    }

    std::wstring FormatDayCount(int Value)
    {
        return std::to_wstring(Value) +
            UIStrings::Get(L"citizen_info.unit.day_suffix");
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
        if (Value == UIStrings::Get(L"citizen.wealth.well_off"))
            return UIStrings::Get(L"citizen_info.wealth_profile.well_off");
        if (Value == UIStrings::Get(L"citizen.wealth.rich"))
            return UIStrings::Get(L"citizen_info.wealth_profile.rich");
        if (Value == UIStrings::Get(L"citizen.wealth.poor"))
            return UIStrings::Get(L"citizen_info.wealth_profile.poor");

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
                    UIStrings::Get(L"citizen_info.section.main_effect") + L": " +
                    Trim(Line.substr(wcslen(L"효과:"))));
                continue;
            }

            if (StartsWith(Line, L"비고:"))
            {
                Result.push_back(
                    UIStrings::Get(L"citizen_info.section.note") + L": " +
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
            return UiText(L"citizen_info.building.housing_class.collective");
        case EBuildingHousingClass::Standard:
            return UiText(L"citizen_info.building.housing_class.standard");
        case EBuildingHousingClass::Elite:
            return UiText(L"citizen_info.building.housing_class.elite");
        default:
            return UiText(L"citizen_info.building.housing_class.default");
        }
    }

    const wchar_t* GetLeisureClassDisplayName(
        EBuildingLeisureClass LeisureClass)
    {
        switch (LeisureClass)
        {
        case EBuildingLeisureClass::Cultural:
            return UiText(L"citizen_info.building.leisure_class.cultural");
        case EBuildingLeisureClass::Luxury:
            return UiText(L"citizen_info.building.leisure_class.luxury");
        case EBuildingLeisureClass::General:
            return UiText(L"citizen_info.building.leisure_class.general");
        default:
            return UiText(L"citizen_info.building.leisure_class.unknown");
        }
    }

    std::wstring ResolveRoleSummary(const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.Residential)
        {
            std::wstring Summary = UIStrings::Format(
                L"citizen_info.building.role.residential",
                {
                    Snapshot.DisplayName,
                    std::to_wstring((std::max)(0, Snapshot.Capacity))
                });

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += UIStrings::Format(
                    L"citizen_info.building.role.residential_class_suffix",
                    {
                        GetHousingClassDisplayName(
                            Snapshot.CatalogEntry->HousingClass)
                    });
            }

            return Summary;
        }

        if (Snapshot.Harbor)
            return UIStrings::Get(L"citizen_info.building.role.harbor");

        if (Snapshot.EntertainmentProvider && Snapshot.FoodProvider)
            return UIStrings::Get(
                L"citizen_info.building.role.food_entertainment");

        if (Snapshot.EntertainmentProvider)
        {
            std::wstring Summary =
                UIStrings::Get(L"citizen_info.building.role.entertainment");

            if (Snapshot.CatalogEntry)
            {
                Summary += L" ";
                Summary += UIStrings::Format(
                    L"citizen_info.building.role.entertainment_class_suffix",
                    {
                        GetLeisureClassDisplayName(
                            Snapshot.CatalogEntry->LeisureClass)
                    });
            }

            return Summary;
        }

        if (Snapshot.FoodProvider)
            return UIStrings::Get(L"citizen_info.building.role.food");

        if (Snapshot.CanGenerateWorkOutput)
            return UIStrings::Get(L"citizen_info.building.role.work_output");

        return UIStrings::Get(L"citizen_info.building.role.support");
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

    void AppendKeyValueByKey(
        std::wstring& Body,
        const wchar_t* LabelKey,
        const std::wstring& Value)
    {
        if (Value.empty())
            return;

        AppendLine(
            Body,
            UIStrings::Get(LabelKey) + L": " + Value);
    }

    bool BuildBuildingUiSnapshot(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName,
        FBuildingUiSnapshot& OutSnapshot)
    {
        if (!QuerySource || BuildingName.empty())
            return false;

        CitizenInfoDataProvider::FCitizenInfoBuildingRecord BuildingRecord;

        if (!QuerySource->TryGetBuildingRecord(BuildingName, BuildingRecord) ||
            !BuildingRecord.Valid)
        {
            return false;
        }

        OutSnapshot = FBuildingUiSnapshot();
        OutSnapshot.CatalogEntry =
            FindBuildingCatalogEntry(BuildingRecord.BuildingId);
        OutSnapshot.ObjectName = BuildingRecord.ObjectName;
        OutSnapshot.DisplayName = BuildingRecord.DisplayName;
        OutSnapshot.CategoryName = BuildingRecord.CategoryName;
        OutSnapshot.DetailText = OutSnapshot.CatalogEntry ?
            OutSnapshot.CatalogEntry->DetailText :
            std::wstring();
        OutSnapshot.RequiredEducationLevel =
            BuildingRecord.RequiredEducationLevel;
        OutSnapshot.Residential = BuildingRecord.Residential;
        OutSnapshot.WorkProvider = BuildingRecord.WorkProvider;
        OutSnapshot.FoodProvider = BuildingRecord.FoodProvider;
        OutSnapshot.EntertainmentProvider =
            BuildingRecord.EntertainmentProvider;
        OutSnapshot.UsesResourceStock = BuildingRecord.UsesResourceStock;
        OutSnapshot.Harbor = BuildingRecord.Harbor;
        OutSnapshot.Warehouse = BuildingRecord.Warehouse;
        OutSnapshot.IsRoad = BuildingRecord.IsRoad;
        OutSnapshot.CanGenerateWorkOutput =
            BuildingRecord.CanGenerateWorkOutput;
        OutSnapshot.Capacity = BuildingRecord.Capacity;
        OutSnapshot.BudgetLevel = BuildingRecord.BudgetLevel;
        OutSnapshot.DaysInMonth = BuildingRecord.DaysInMonth;
        OutSnapshot.MonthlyWageCost = BuildingRecord.MonthlyWageCost;
        OutSnapshot.MonthlyUpkeepCost = BuildingRecord.MonthlyUpkeepCost;
        OutSnapshot.DailyWageCost = BuildingRecord.DailyWageCost;
        OutSnapshot.DailyUpkeepCost = BuildingRecord.DailyUpkeepCost;
        OutSnapshot.HousingCap = BuildingRecord.HousingCap;
        OutSnapshot.JobCap = BuildingRecord.JobCap;
        OutSnapshot.FoodCap = BuildingRecord.FoodCap;
        OutSnapshot.FunCap = BuildingRecord.FunCap;
        OutSnapshot.ResourceStock = BuildingRecord.ResourceStock;
        OutSnapshot.ExportableStock = BuildingRecord.ExportableStock;
        OutSnapshot.MaxResourceStock = BuildingRecord.MaxResourceStock;
        OutSnapshot.TotalProducedPowerMW =
            BuildingRecord.TotalProducedPowerMW;
        OutSnapshot.TotalRequiredPowerMW =
            BuildingRecord.TotalRequiredPowerMW;
        OutSnapshot.BudgetScale = BuildingRecord.BudgetScale;
        OutSnapshot.AccessibilityScore =
            BuildingRecord.AccessibilityScore;
        OutSnapshot.HarborShipProgressPercent =
            BuildingRecord.HarborShipProgressPercent;
        OutSnapshot.Residents = BuildingRecord.Residents;
        OutSnapshot.AssignedEmployees = BuildingRecord.AssignedEmployees;
        OutSnapshot.WorkingEmployees = BuildingRecord.WorkingEmployees;
        OutSnapshot.AssignedVisitors = BuildingRecord.AssignedVisitors;
        OutSnapshot.ArrivedVisitors = BuildingRecord.ArrivedVisitors;
        OutSnapshot.IncomingVisitors = BuildingRecord.IncomingVisitors;

        if (OutSnapshot.Warehouse)
        {
            for (size_t SlotIndex = 0;
                SlotIndex < BuildingRecord.WarehouseSlots.size();
                ++SlotIndex)
            {
                const CitizenInfoDataProvider::FWarehouseSlotRecord& SlotRecord =
                    BuildingRecord.WarehouseSlots[SlotIndex];
                std::wstring SlotValue;

                if (SlotRecord.Type == EResourceType::None)
                {
                    SlotValue =
                        UIStrings::Get(L"citizen_info.building.warehouse.empty");
                }
                else
                {
                    SlotValue =
                        std::wstring(GetResourceTypeDisplayName(
                            SlotRecord.Type)) +
                        L" " +
                        FormatInteger(SlotRecord.Stock) +
                        L" / " +
                        FormatInteger(SlotRecord.Capacity);
                }

                std::wstring SlotLine = UIStrings::Format(
                    L"citizen_info.building.warehouse.slot_line",
                    {
                        std::to_wstring(static_cast<int>(SlotIndex) + 1),
                        SlotValue
                    });
                OutSnapshot.WarehouseSlotLines.push_back(std::move(SlotLine));
            }
        }

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
        return true;
    }

    std::wstring BuildOverviewBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, Ui(L"citizen_info.section.operation_modes"));
            AppendLine(Body, L"  " + Snapshot.OperationModes.front());
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.residents_status") + L": " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.workers") + L": " +
                std::to_wstring(Snapshot.AssignedEmployees.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_education",
                GetCitizenEducationDisplayName(
                    Snapshot.RequiredEducationLevel));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.job_quality",
                Snapshot.JobQualityText);
        }

        if (Snapshot.Residential && Snapshot.CatalogEntry)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.housing_class",
                GetHousingClassDisplayName(
                    Snapshot.CatalogEntry->HousingClass));
        }

        if (Snapshot.ServiceCapacity > 0 ||
            !Snapshot.ServiceQualityText.empty() ||
            !Snapshot.WealthRequirementText.empty() ||
            !Snapshot.TouristPreferenceText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.service"));

            if (Snapshot.ServiceCapacity > 0)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.visitors") + L": " +
                    std::to_wstring(Snapshot.AssignedVisitors.size()) +
                    L" / " +
                    std::to_wstring(Snapshot.ServiceCapacity));
            }

            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.service_quality",
                Snapshot.ServiceQualityText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.wealth_requirement",
                Snapshot.WealthRequirementText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.tourist_preference",
                Snapshot.TouristPreferenceText);
        }

        if (!Snapshot.RequiredPowerText.empty() ||
            !Snapshot.ProducedPowerText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.power"));
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.required_power",
                Snapshot.RequiredPowerText);
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.produced_power",
                Snapshot.ProducedPowerText);
        }

        if (Snapshot.UsesResourceStock || Snapshot.Harbor)
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.storage_logistics"));

            if (Snapshot.UsesResourceStock)
            {
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.stock") + L": " +
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
                    Ui(L"citizen_info.label.exportable_stock") + L": " +
                    FormatInteger(Snapshot.ExportableStock));
                AppendLine(
                    Body,
                    Ui(L"citizen_info.label.ship_progress") + L": " +
                    std::to_wstring(static_cast<int>(roundf(
                        Snapshot.HarborShipProgressPercent * 100.f))) +
                    L"%");
            }
        }

        if (!Snapshot.EffectText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.main_effect"));
            AppendLine(Body, Snapshot.EffectText);
        }

        if (!Snapshot.NoteText.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.note"));
            AppendLine(Body, Snapshot.NoteText);
        }

        return Body.empty() ?
            UIStrings::Get(L"citizen_info.building.data_pending") :
            Body;
    }

    std::wstring BuildStatisticsBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_wage_cost") +
                L": " + FormatMoney(Snapshot.MonthlyWageCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_upkeep_cost") +
                L": " + FormatMoney(Snapshot.MonthlyUpkeepCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.monthly_total_cost") + L": " +
            FormatMoney(
                static_cast<long long>(Snapshot.MonthlyWageCost) +
                static_cast<long long>(Snapshot.MonthlyUpkeepCost)));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.daily_wage_cost") +
                L": " + FormatMoney(Snapshot.DailyWageCost));
        AppendLine(
            Body,
            Ui(L"citizen_info.label.daily_upkeep_cost") +
                L": " + FormatMoney(Snapshot.DailyUpkeepCost));

        if (Snapshot.UsesResourceStock)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.current_stock") + L": " +
                FormatInteger(Snapshot.ResourceStock) +
                L" / " +
                FormatInteger(Snapshot.MaxResourceStock));
        }

        if (Snapshot.Warehouse && !Snapshot.WarehouseSlotLines.empty())
        {
            AppendLine(Body, L"");
            AppendLine(Body, Ui(L"citizen_info.section.warehouse_slots"));

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
                Ui(L"citizen_info.label.ship_arrival_progress") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.HarborShipProgressPercent * 100.f))) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.exportable_total") + L": " +
                FormatInteger(Snapshot.ExportableStock));
        }

        if (Snapshot.Residential)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.resident_assignment") + L": " +
                std::to_wstring(Snapshot.Residents.size()) +
                L" / " +
                std::to_wstring(Snapshot.Capacity));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_resident") +
                    L": " + SummarizeNames(Snapshot.Residents));
        }

        if (Snapshot.WorkProvider)
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.assigned_workers") + L": " +
                std::to_wstring(Snapshot.AssignedEmployees.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.working_now") + L": " +
                std::to_wstring(Snapshot.WorkingEmployees.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_worker") + L": " +
                SummarizeNames(Snapshot.AssignedEmployees));
        }

        if (!Snapshot.AssignedVisitors.empty() ||
            !Snapshot.ArrivedVisitors.empty() ||
            !Snapshot.IncomingVisitors.empty())
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.assigned_visitors") + L": " +
                std::to_wstring(Snapshot.AssignedVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.on_site_visitors") + L": " +
                std::to_wstring(Snapshot.ArrivedVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.incoming_visitors") + L": " +
                std::to_wstring(Snapshot.IncomingVisitors.size()));
            AppendLine(
                Body,
                Ui(L"citizen_info.label.representative_visitor") + L": " +
                SummarizeNames(Snapshot.ArrivedVisitors));
        }

        return Body;
    }

    std::wstring BuildUpgradesBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body;

        if (!Snapshot.UpgradeHints.empty())
        {
            AppendLine(Body, Ui(L"citizen_info.section.registered_upgrades"));

            for (size_t Index = 0; Index < Snapshot.UpgradeHints.size(); ++Index)
                AppendLine(Body, L"- " + Snapshot.UpgradeHints[Index]);
        }
        else
        {
            AppendLine(Body, Ui(L"citizen_info.upgrades.none"));
        }

        if (!Snapshot.OperationModes.empty())
        {
            AppendLine(Body, L"");
            AppendLine(
                Body,
                Ui(L"citizen_info.section.operation_mode_candidates"));

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
            Ui(L"citizen_info.label.current_efficiency") + L": " +
            std::to_wstring(BudgetPercent) +
            L"%");
        AppendLine(
            Body,
            Ui(L"citizen_info.label.budget_scale") +
                L": " + FormatMultiplier(Snapshot.BudgetScale));

        if (!Snapshot.IsRoad)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.road_accessibility") + L": " +
                std::to_wstring(static_cast<int>(roundf(
                    Snapshot.AccessibilityScore * 100.f))) +
                L"%");
        }

        if (Snapshot.Residential)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.housing_fill_rate") + L": " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.housing_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.HousingCap));
        }
        else if (Snapshot.WorkProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.worker_fill_rate") + L": " +
                std::to_wstring((std::max)(0, CapacityFillPercent)) +
                L"%");
            AppendLine(
                Body,
                Ui(L"citizen_info.label.job_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.JobCap));
        }

        if (Snapshot.FoodProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.food_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.FoodCap));
        }

        if (Snapshot.EntertainmentProvider)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.fun_satisfaction_cap") + L": " +
                std::to_wstring(Snapshot.FunCap));
        }

        if (Snapshot.ServiceCapacity > 0)
        {
            AppendLine(
                Body,
                Ui(L"citizen_info.label.visitor_utilization") + L": " +
                std::to_wstring((std::max)(0, VisitorFillPercent)) +
                L"%");
        }

        if (!Snapshot.RequiredPowerText.empty())
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.power_demand",
                Snapshot.RequiredPowerText);

        return Body;
    }

    std::wstring BuildInformationBody(const FBuildingUiSnapshot& Snapshot)
    {
        std::wstring Body = ResolveRoleSummary(Snapshot);

        AppendLine(Body, L"");
        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.category",
            Snapshot.CategoryName);

        if (Snapshot.CatalogEntry)
        {
            AppendKeyValueByKey(
                Body,
                L"citizen_info.label.unlock_era",
                GetBuildingEraDisplayName(Snapshot.CatalogEntry->UnlockEra));

            if (Snapshot.CatalogEntry->HousingClass !=
                EBuildingHousingClass::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.housing_grade",
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass));
            }

            if (Snapshot.CatalogEntry->LeisureClass !=
                EBuildingLeisureClass::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.leisure_grade",
                    GetLeisureClassDisplayName(
                        Snapshot.CatalogEntry->LeisureClass));
            }

            if (Snapshot.CatalogEntry->PrimaryTouristPreference !=
                ETouristPreference::None)
            {
                AppendKeyValueByKey(
                    Body,
                    L"citizen_info.label.primary_tourist",
                    GetTouristPreferenceDisplayName(
                        Snapshot.CatalogEntry->PrimaryTouristPreference));
            }
        }

        AppendKeyValueByKey(
            Body,
            L"citizen_info.label.required_education",
            GetCitizenEducationDisplayName(
                Snapshot.RequiredEducationLevel));

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
            return UiText(L"citizen_info.wealth_profile.well_off");
        case ECitizenWealthLevel::Rich:
            return UiText(L"citizen_info.wealth_profile.rich");
        default:
            return UiText(L"citizen_info.wealth_profile.poor");
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
            return UiText(L"citizen_info.activity.work");
        case ECitizenState::GoingToFood:
        case ECitizenState::AtFood:
            return UiText(L"citizen_info.activity.food");
        case ECitizenState::GoingHome:
        case ECitizenState::AtHome:
        case ECitizenState::GoingToFun:
        case ECitizenState::AtFun:
            return UiText(L"citizen_info.activity.leisure");
        default:
            return UIStrings::Get(L"citizen_info.activity.move").c_str();
        }
    }

    std::wstring ResolveBuildingDisplayName(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const std::string& BuildingName)
    {
        if (BuildingName.empty())
            return L"-";

        if (!QuerySource)
            return Utf8ToWide(BuildingName);

        return QuerySource->ResolveBuildingDisplayName(BuildingName);
    }

    std::wstring ResolveCitizenLocationText(
        const std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>&
            QuerySource,
        const CitizenInfoDataProvider::FCitizenInfoCitizenRecord& Citizen)
    {
        if (!Citizen.Valid)
            return L"-";

        auto MakeInteriorText =
            [&](const std::string& BuildingName)
        {
            const std::wstring DisplayName =
                ResolveBuildingDisplayName(QuerySource, BuildingName);

            if (DisplayName.empty() || DisplayName == L"-")
                return std::wstring(L"-");

            return DisplayName +
                UIStrings::Get(L"citizen_info.location.interior_suffix");
        };

        switch (Citizen.State)
        {
        case ECitizenState::AtHome:
        case ECitizenState::GoingHome:
            return MakeInteriorText(Citizen.HomeBuildingName);
        case ECitizenState::AtWork:
        case ECitizenState::GoingToWork:
        case ECitizenState::GoingToTeamsterSource:
        case ECitizenState::GoingToTeamsterHarbor:
        case ECitizenState::GoingToTeamsterConsumerSource:
        case ECitizenState::GoingToTeamsterConsumerTarget:
        case ECitizenState::GoingToTeamsterOffice:
            return MakeInteriorText(Citizen.WorkBuildingName);
        case ECitizenState::AtFood:
        case ECitizenState::GoingToFood:
            return MakeInteriorText(Citizen.FoodBuildingName);
        case ECitizenState::AtFun:
        case ECitizenState::GoingToFun:
            return MakeInteriorText(Citizen.FunBuildingName);
        default:
            return UIStrings::Get(L"citizen_info.location.outside_tropico");
        }
    }

    const wchar_t* GetCitizenPoliticalIntensityDisplayName(
        EPoliticalAxis Axis,
        EPoliticalSupportLevel Support)
    {
        if (Support == EPoliticalSupportLevel::Strong)
        {
            return Axis == EPoliticalAxis::ReligionMilitarism ?
                UiText(L"citizen_info.politics.intensity.fervent") :
                UiText(L"citizen_info.politics.intensity.strong");
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
        const int ClampedTab =
            (std::max)(0, (std::min)(GCitizenTabCount - 1, TabIndex));

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

        {
            Result.Subtitle = UIStrings::Format(
                L"citizen_info.subtitle.citizen_identity_template",
                {
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant") :
                        std::wstring()
                });
        }

        if (ClampedTab == 0)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_overview",
                {
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.CommuteTimePenalty)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall))
                });
        }
        else if (ClampedTab == 1)
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_politics",
                {
                    std::to_wstring(ToPercent(Satisfaction.Food)),
                    std::to_wstring(ToPercent(Satisfaction.Health)),
                    std::to_wstring(ToPercent(Satisfaction.Fun)),
                    std::to_wstring(ToPercent(Satisfaction.Faith)),
                    std::to_wstring(ToPercent(Satisfaction.Housing)),
                    std::to_wstring(ToPercent(Satisfaction.Job)),
                    std::to_wstring(ToPercent(Satisfaction.Freedom)),
                    std::to_wstring(ToPercent(Satisfaction.Security)),
                    std::to_wstring(ToPercent(Satisfaction.Overall)),
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
                        PoliticalProfile.IntellectualConservative.Support)
                });
        }
        else
        {
            Result.PageTitle =
                CitizenInfoConstants::GetCitizenPageTitle(ClampedTab);
            const int Overall = ToPercent(Satisfaction.Overall);

            const wchar_t* SatisfactionDesc =
                Overall >= 75 ?
                    UiText(L"citizen_info.thoughts.satisfaction.very_high") :
                Overall >= 55 ?
                    UiText(L"citizen_info.thoughts.satisfaction.high") :
                Overall >= 35 ?
                    UiText(L"citizen_info.thoughts.satisfaction.low") :
                    UiText(L"citizen_info.thoughts.satisfaction.very_low");

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

            Result.BodyText = UIStrings::Format(
                L"citizen_info.body.citizen_thoughts",
                {
                    Result.Title,
                    GetCitizenWealthDisplayName(
                        IdentityProfile.WealthLevel),
                    GetCitizenEducationDisplayName(
                        IdentityProfile.EducationLevel),
                    IdentityProfile.IsImmigrant ?
                        Ui(L"citizen_info.fragment.immigrant_parenthetical") :
                        std::wstring(),
                    SatisfactionDesc,
                    DominantStance ? DominantStance : L"-"
                });
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        if (!QuerySource || CitizenName.empty())
            return FCitizenInfoSnapshot();

        FCitizenInfoCitizenRecord Citizen;

        if (!QuerySource->TryGetCitizenRecord(CitizenName, Citizen) ||
            !Citizen.Valid)
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result = BuildCitizenSnapshot(
            Citizen.Name,
            Citizen.Satisfaction,
            Citizen.IdentityProfile,
            Citizen.PoliticalProfile,
            SelectedCitizenTabIndex);

        if (Result.Valid && Result.SelectedTabIndex == 0)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(L"citizen_info.subtitle.tropican");
            Result.ShowCitizenProfileOverview = true;
            Result.ShowCitizenActionButtons = true;
            Result.ShowSectionDivider = true;
            Result.CitizenPortraitSlotCount = 11;
            Result.CitizenPortraitOccupiedSlot = 4;
            Result.CitizenPortraitVariant =
                static_cast<int>(Citizen.Name.size() % 4);
            Result.CitizenFooterText = L"TROPICO EXEC. 16FA-923";

            Result.OverviewMetricLabels[0] =
                Ui(L"citizen_info.metric.activity");
            Result.OverviewMetricValues[0] =
                GetCitizenActivityDisplayName(Citizen.State);
            Result.OverviewMetricLabels[1] =
                Ui(L"citizen_info.metric.location");
            Result.OverviewMetricValues[1] =
                ResolveCitizenLocationText(QuerySource, Citizen);
            Result.OverviewMetricLabels[2] =
                Ui(L"citizen_info.metric.age");
            Result.OverviewMetricValues[2] =
                Ui(L"citizen_info.value.age_30_adult");
            Result.OverviewMetricLabels[3] =
                Ui(L"citizen_info.metric.origin");
            Result.OverviewMetricValues[3] =
                Citizen.IdentityProfile.IsImmigrant ?
                    Ui(L"citizen_info.origin.france") :
                    Ui(L"citizen_info.origin.tropico");
            Result.OverviewMetricLabels[4] =
                Ui(L"citizen_info.metric.wealth");
            Result.OverviewMetricValues[4] =
                GetCitizenProfileWealthDisplayName(
                    Citizen.IdentityProfile.WealthLevel);
            Result.OverviewMetricLabels[5] =
                Ui(L"citizen_info.metric.education");
            Result.OverviewMetricValues[5] =
                GetCitizenEducationDisplayName(
                    Citizen.IdentityProfile.EducationLevel);
            Result.OverviewMetricLabels[6] =
                Ui(L"citizen_info.metric.job");
            Result.OverviewMetricValues[6] =
                ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.WorkBuildingName);
            Result.OverviewMetricLabels[7] =
                Ui(L"citizen_info.metric.home");
            Result.OverviewMetricValues[7] =
                ResolveBuildingDisplayName(
                    QuerySource,
                    Citizen.HomeBuildingName);
            Result.OverviewMetricAccentValues[6] =
                !Citizen.WorkBuildingName.empty();
            Result.OverviewMetricAccentValues[7] =
                !Citizen.HomeBuildingName.empty();

            Result.CitizenActionLabels[0] =
                Ui(L"citizen_info.action.bribe");
            Result.CitizenActionLabels[1] =
                Ui(L"citizen_info.action.kill");
            Result.CitizenActionLabels[2] =
                Ui(L"citizen_info.action.stage_accident");
            Result.CitizenActionLabels[3] =
                Ui(L"citizen_info.action.arrest");
            Result.CitizenActionLabels[4] =
                Ui(L"citizen_info.action.isolate");
            Result.CitizenActionLabels[5] =
                Ui(L"citizen_info.action.send_to_space");
        }
        else if (Result.Valid && Result.SelectedTabIndex == 1)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(L"citizen_info.subtitle.tropican");
            Result.ShowCitizenPoliticsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();

            Result.CitizenPoliticsSatisfactionLabels =
            {{
                Ui(L"almanac.satisfaction.overall"),
                Ui(L"almanac.satisfaction.food"),
                Ui(L"almanac.satisfaction.health"),
                Ui(L"almanac.satisfaction.fun"),
                Ui(L"almanac.satisfaction.faith"),
                Ui(L"almanac.satisfaction.housing"),
                Ui(L"almanac.satisfaction.job"),
                Ui(L"almanac.satisfaction.freedom"),
                Ui(L"almanac.satisfaction.security")
            }};
            Result.CitizenPoliticsSatisfactionRatios =
            {{
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Overall / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Food / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Health / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Fun / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Faith / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Housing / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Job / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Freedom / 100.f)),
                (std::max)(0.f, (std::min)(1.f, Citizen.Satisfaction.Security / 100.f))
            }};
            Result.CitizenPoliticsOpinionLines =
                BuildCitizenOpinionLines(Citizen.PoliticalProfile);
            Result.CitizenPoliticsSupportRatio =
                BuildCitizenSupportRatio(Citizen.Satisfaction);
        }
        else if (Result.Valid && Result.SelectedTabIndex == 2)
        {
            Result.Subtitle.clear();
            Result.BodyText.clear();
            Result.PageTitle.clear();
            Result.ShowSectionRibbon = false;
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                UIStrings::Get(L"citizen_info.subtitle.tropican");
            Result.ShowCitizenThoughtsOverview = true;
            Result.ShowCitizenProfileOverview = false;
            Result.ShowCitizenPoliticsOverview = false;
            Result.ShowCitizenActionButtons = false;
            Result.ShowSectionDivider = false;
            Result.CitizenFooterText.clear();
            Result.CitizenThoughtLines =
            {{
                Ui(L"citizen_info.thought.line1"),
                Ui(L"citizen_info.thought.line2"),
                Ui(L"citizen_info.thought.line3"),
                Ui(L"citizen_info.thought.line4"),
                Ui(L"citizen_info.thought.line5")
            }};
        }

        return Result;
    }

    FCitizenInfoSnapshot BuildTrackedCitizenSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& CitizenName,
        int SelectedCitizenTabIndex)
    {
        return BuildTrackedCitizenSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            CitizenName,
            SelectedCitizenTabIndex);
    }

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<ICitizenInfoQuerySource>& QuerySource,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex)
    {
        FBuildingUiSnapshot BuildingSnapshot;

        if (!BuildBuildingUiSnapshot(
            QuerySource,
            BuildingName,
            BuildingSnapshot))
        {
            return FCitizenInfoSnapshot();
        }

        FCitizenInfoSnapshot Result;
        Result.Valid = true;
        Result.Mode = EPanelMode::Building;
        Result.SelectedTabIndex =
            (std::max)(
                0,
                (std::min)(
                    GBuildingTabCount - 1,
                    SelectedBuildingTabIndex));
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

        Result.PageTitle =
            CitizenInfoConstants::GetBuildingPageTitle(
                Result.SelectedTabIndex);
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
                Ui(L"citizen_info.action.change_resource") :
                std::wstring();

        if (UseHydroponicFarmWorkOverview(BuildingSnapshot))
        {
            Result.ShowBuildingSubtitle = true;
            Result.BuildingSubtitleText =
                Ui(L"citizen_info.subtitle.cocoa");
        }

        const long long TotalMonthlyCost =
            static_cast<long long>(BuildingSnapshot.MonthlyWageCost) +
            static_cast<long long>(BuildingSnapshot.MonthlyUpkeepCost);
        Result.BudgetText = UIStrings::Format(
            L"citizen_info.budget.summary_template",
            {
                std::to_wstring(BuildingSnapshot.BudgetLevel),
                FormatMultiplier(BuildingSnapshot.BudgetScale),
                FormatMoney(TotalMonthlyCost)
            });

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

            Result.OverviewBudgetLabel =
                Ui(L"citizen_info.label.budget");
            Result.OverviewBudgetValue =
                FormatMoney(BuildingSnapshot.MonthlyUpkeepCost);
            Result.OverviewOccupancyLabel =
                Ui(L"citizen_info.label.residence");
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
                Ui(L"citizen_info.label.housing_quality"),
                Ui(L"citizen_info.label.required_wealth"),
                Ui(L"citizen_info.label.monthly_income"),
                Ui(L"citizen_info.label.electricity"),
                Ui(L"citizen_info.label.power_network"),
                Ui(L"citizen_info.label.power_grid_status")
            };
            Result.OverviewMetricValues =
            {
                std::to_wstring(HousingQuality),
                NormalizeWealthRequirementText(
                    BuildingSnapshot.WealthRequirementText),
                FormatMoney(MonthlyIncome),
                FormatMegawattValue(-RequiredPowerMW),
                L"#1",
                FormatSignedMegawattValue(PowerSurplusMW)
            };
        }

        if (Result.ShowBuildingWorkOverview)
        {
            if (UseHydroponicFarmWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel =
                    Ui(L"citizen_info.label.work_mode");
                Result.OverviewWorkModeValue =
                    Ui(L"citizen_info.work_mode.mostly_natural");
                Result.OverviewBudgetLabel =
                    Ui(L"citizen_info.label.budget");
                Result.OverviewBudgetValue = L"$95";
                Result.OverviewOccupancyLabel =
                    Ui(L"citizen_info.label.workers");
                Result.OverviewOccupancyValue = L"4 / 4";
                Result.OverviewResidentCount = 4;
                Result.OverviewResidentCapacity = 4;
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.job_quality"),
                    Ui(L"citizen_info.label.required_education"),
                    Ui(L"citizen_info.label.wage"),
                    Ui(L"citizen_info.label.efficiency"),
                    Ui(L"citizen_info.label.electricity"),
                    Ui(L"citizen_info.label.power_network"),
                    Ui(L"citizen_info.label.power_grid_status"),
                    Ui(L"citizen_info.label.storage"),
                    Ui(L"citizen_info.label.production"),
                    Ui(L"citizen_info.commodity.cocoa"),
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"65",
                    GetCitizenEducationDisplayName(
                        ECitizenEducationLevel::HighSchool),
                    L"$18",
                    L"145%",
                    FormatMegawattValue(-30),
                    L"#1",
                    FormatSignedMegawattValue(785),
                    L"",
                    L"",
                    L"0 / 2320",
                    L"",
                    L""
                };
            }
            else if (UseAquaParkWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel =
                    Ui(L"citizen_info.label.work_mode");
                Result.OverviewWorkModeValue =
                    Ui(L"citizen_info.work_mode.fresh_and_clean");
                Result.OverviewBudgetLabel =
                    Ui(L"citizen_info.label.budget");
                Result.OverviewBudgetValue = L"$57 / $75";
                Result.OverviewOccupancyLabel =
                    Ui(L"citizen_info.label.workers");
                Result.OverviewOccupancyValue = L"1 / 3";
                Result.OverviewResidentCount = 1;
                Result.OverviewResidentCapacity = 3;
                Result.ShowBuildingVisitorIcons = true;
                Result.OverviewVisitorCount = 9;
                Result.OverviewVisitorCapacity = 12;
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.job_quality"),
                    Ui(L"citizen_info.label.required_education"),
                    Ui(L"citizen_info.label.wage"),
                    Ui(L"citizen_info.label.efficiency"),
                    Ui(L"citizen_info.label.electricity"),
                    Ui(L"citizen_info.label.power_network"),
                    Ui(L"citizen_info.label.power_grid_status"),
                    Ui(L"citizen_info.label.visitors"),
                    Ui(L"citizen_info.label.service_quality"),
                    Ui(L"citizen_info.label.required_wealth"),
                    Ui(L"citizen_info.label.preferred_type"),
                    Ui(L"citizen_info.label.fee_income"),
                    Ui(L"citizen_info.label.tourist_only_checkbox"),
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"65",
                    GetCitizenEducationDisplayName(
                        ECitizenEducationLevel::Uneducated),
                    L"$9",
                    L"125%",
                    FormatMegawattValue(-15),
                    L"#1",
                    FormatSignedMegawattValue(785),
                    L"9 / 6 (18)",
                    L"88",
                    Ui(L"citizen_info.wealth_profile.well_off"),
                    GetTouristPreferenceDisplayName(
                        ETouristPreference::Relaxation),
                    L"$15 ($0)",
                    L" ",
                    L""
                };
            }
            else if (UseRestaurantWorkOverview(BuildingSnapshot))
            {
                Result.OverviewWorkModeLabel =
                    Ui(L"citizen_info.label.work_mode");
                Result.OverviewWorkModeValue =
                    Ui(L"citizen_info.work_mode.thousand_plus");
                Result.OverviewBudgetLabel =
                    Ui(L"citizen_info.label.budget");
                Result.OverviewBudgetValue = L"$64";
                Result.OverviewOccupancyLabel =
                    Ui(L"citizen_info.label.workers");
                Result.OverviewOccupancyValue = L"4 / 4";
                Result.OverviewResidentCount = 4;
                Result.OverviewResidentCapacity = 4;
                Result.ShowBuildingVisitorIcons = true;
                Result.OverviewVisitorCount = 12;
                Result.OverviewVisitorCapacity = 12;
                Result.OverviewMetricLabels =
                {
                    Ui(L"citizen_info.label.job_quality"),
                    Ui(L"citizen_info.label.required_education"),
                    Ui(L"citizen_info.label.wage"),
                    Ui(L"citizen_info.label.efficiency"),
                    Ui(L"citizen_info.label.electricity"),
                    Ui(L"citizen_info.label.power_network"),
                    Ui(L"citizen_info.label.power_grid_status"),
                    Ui(L"citizen_info.label.visitors"),
                    Ui(L"citizen_info.label.service_quality"),
                    Ui(L"citizen_info.label.required_wealth"),
                    Ui(L"citizen_info.label.fee_revenue"),
                    Ui(L"citizen_info.label.tourist_only_checkbox"),
                    L"",
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"55",
                    GetCitizenEducationDisplayName(
                        ECitizenEducationLevel::Uneducated),
                    L"$13",
                    L"125%",
                    FormatMegawattValue(-10),
                    L"#1",
                    FormatSignedMegawattValue(785),
                    L"12 / 12",
                    L"96",
                    Ui(L"citizen_info.wealth_profile.well_off"),
                    L"$18 ($0)",
                    L" ",
                    L"",
                    L""
                };
            }
            else
            {
                Result.OverviewWorkModeLabel =
                    Ui(L"citizen_info.label.work_mode");
                Result.OverviewWorkModeValue =
                    !BuildingSnapshot.OperationModes.empty() ?
                        BuildingSnapshot.OperationModes.front() :
                        Ui(L"citizen_info.work_mode.general_control");
                Result.OverviewBudgetLabel =
                    Ui(L"citizen_info.label.budget");
                Result.OverviewBudgetValue = L"$120";
                Result.OverviewOccupancyLabel =
                    Ui(L"citizen_info.label.workers");
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
                    Ui(L"citizen_info.label.job_quality"),
                    Ui(L"citizen_info.label.required_education"),
                    Ui(L"citizen_info.label.wage"),
                    Ui(L"citizen_info.label.efficiency"),
                    Ui(L"citizen_info.label.harbor"),
                    Ui(L"citizen_info.label.next_arrival_time"),
                    Ui(L"citizen_info.label.total_cost"),
                    Ui(L"citizen_info.label.expected_revenue"),
                    Ui(L"citizen_info.label.storage"),
                    Ui(L"citizen_info.commodity.sugar"),
                    Ui(L"citizen_info.commodity.corn"),
                    L""
                };
                Result.OverviewMetricValues =
                {
                    L"50",
                    GetCitizenEducationDisplayName(
                        BuildingSnapshot.RequiredEducationLevel),
                    L"$18",
                    L"135%",
                    L"",
                    FormatDayCount(20),
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
                    Ui(L"citizen_info.label.total_income"),
                    Ui(L"citizen_info.label.last_month_income"),
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
                    Ui(L"citizen_info.label.total_income"),
                    Ui(L"citizen_info.label.last_month_income"),
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
                Ui(L"citizen_info.note.housing_quality_efficiency");
            Result.OverviewMetricLabels =
            {
                Ui(L"citizen_info.label.efficiency"),
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
            Result.UpgradeCardTitle =
                Ui(L"citizen_info.upgrade.solar_window.title");
            Result.UpgradeCardDescription =
                Ui(L"citizen_info.upgrade.solar_window.description");
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
            Result.InformationTopText = UIStrings::Format(
                L"citizen_info.information.modern_apartment.top",
                {
                    NormalizeWealthRequirementText(
                        BuildingSnapshot.WealthRequirementText)
                });
            Result.InformationBottomText =
                Ui(L"citizen_info.information.modern_apartment.bottom");
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

    FCitizenInfoSnapshot BuildTrackedBuildingSnapshot(
        const std::shared_ptr<CWorld>& World,
        const std::string& BuildingName,
        int SelectedBuildingTabIndex)
    {
        return BuildTrackedBuildingSnapshot(
            CitizenInfoQueryService::CreateWorldQuerySource(World),
            BuildingName,
            SelectedBuildingTabIndex);
    }
}
