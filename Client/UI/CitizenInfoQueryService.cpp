#include "CitizenInfoQueryService.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorldAccess.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cwchar>
#include <vector>

namespace
{
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

    void PushUnique(std::vector<std::string>& Names, const std::string& Name)
    {
        if (Name.empty())
            return;

        if (std::find(Names.begin(), Names.end(), Name) == Names.end())
            Names.push_back(Name);
    }

    class CWorldCitizenInfoQuerySource final :
        public CitizenInfoDataProvider::ICitizenInfoQuerySource
    {
    public:
        explicit CWorldCitizenInfoQuerySource(
            const std::shared_ptr<CWorld>& World)
            : mWorld(World)
            , mMainWorldAccess(
                std::dynamic_pointer_cast<IMainWorldBuildMenuAccess>(World))
        {
        }

    public:
        bool TryGetBuildingRecord(
            const std::string& BuildingName,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const override
        {
            auto Building = FindValidBuilding(BuildingName);

            if (!Building)
                return false;

            OutRecord =
                CitizenInfoDataProvider::FCitizenInfoBuildingRecord();
            OutRecord.Valid = true;
            OutRecord.ObjectName = Utf8ToWide(Building->GetName());
            OutRecord.DisplayName =
                Utf8ToWide(Building->GetBuildingDisplayName());
            OutRecord.CategoryName =
                Utf8ToWide(Building->GetBuildingCategoryName());
            OutRecord.BuildingId = Building->GetBuildingId();
            OutRecord.Residential = Building->IsResidential();
            OutRecord.WorkProvider =
                !OutRecord.Residential &&
                Building->GetCapacity() > 0;
            OutRecord.FoodProvider = Building->IsFoodProvider();
            OutRecord.EntertainmentProvider =
                Building->IsEntertainmentProvider();
            OutRecord.Harbor = Building->IsHarbor();
            OutRecord.Warehouse = Building->IsWarehouse();
            OutRecord.IsRoad = Building->IsRoad();
            OutRecord.CanGenerateWorkOutput =
                Building->CanGenerateWorkOutput();
            OutRecord.Capacity = (std::max)(0, Building->GetCapacity());
            OutRecord.BudgetLevel = Building->GetBudgetLevel();
            OutRecord.BudgetScale = Building->GetBudgetSatisfactionScale();
            OutRecord.AccessibilityScore =
                Building->GetAccessibilityScore();
            OutRecord.HousingCap = Building->GetHousingSatisfactionCap();
            OutRecord.JobCap = Building->GetEffectiveJobSatisfactionCap();
            OutRecord.FoodCap = Building->GetFoodSatisfactionCap();
            OutRecord.FunCap = Building->GetFunSatisfactionCap();
            OutRecord.ResourceStock = Building->GetResourceStock();
            OutRecord.ExportableStock =
                Building->GetExportableResourceStock();
            OutRecord.MaxResourceStock = Building->GetMaxResourceStock();
            OutRecord.HarborShipProgressPercent =
                Building->GetHarborShipProgressPercent();
            OutRecord.RequiredEducationLevel =
                Building->GetRequiredEducationLevel();
            OutRecord.UsesResourceStock =
                OutRecord.ResourceStock > 0 ||
                OutRecord.CanGenerateWorkOutput ||
                OutRecord.FoodProvider ||
                OutRecord.Harbor ||
                OutRecord.Warehouse;
            OutRecord.DaysInMonth = mMainWorldAccess ?
                (std::max)(1, mMainWorldAccess->GetSimulationMonthDayCount()) :
                30;
            OutRecord.MonthlyWageCost = Building->GetMonthlyWageCost();
            OutRecord.MonthlyUpkeepCost = Building->GetMonthlyUpkeepCost();
            OutRecord.DailyWageCost =
                Building->GetDailyWageCost(OutRecord.DaysInMonth);
            OutRecord.DailyUpkeepCost =
                Building->GetDailyUpkeepCost(OutRecord.DaysInMonth);

            if (OutRecord.Warehouse)
            {
                for (int SlotIndex = 0;
                    SlotIndex < Building->GetWarehouseSlotCount();
                    ++SlotIndex)
                {
                    CitizenInfoDataProvider::FWarehouseSlotRecord SlotRecord;
                    SlotRecord.Type =
                        Building->GetWarehouseSlotType(SlotIndex);
                    SlotRecord.Stock =
                        SlotRecord.Type == EResourceType::None ?
                            0 :
                            Building->GetResourceStock(SlotRecord.Type);
                    SlotRecord.Capacity =
                        SlotRecord.Type == EResourceType::None ?
                            0 :
                            Building->GetResourceTypeCapacity(SlotRecord.Type);
                    OutRecord.WarehouseSlots.push_back(SlotRecord);
                }
            }

            PopulatePowerTotals(OutRecord);
            PopulateCitizenAssignments(BuildingName, *Building, OutRecord);
            return true;
        }

        bool TryGetCitizenRecord(
            const std::string& CitizenName,
            CitizenInfoDataProvider::FCitizenInfoCitizenRecord& OutRecord)
            const override
        {
            auto Citizen = FindValidCitizen(CitizenName);

            if (!Citizen)
                return false;

            OutRecord =
                CitizenInfoDataProvider::FCitizenInfoCitizenRecord();
            OutRecord.Valid = true;
            OutRecord.Name = Citizen->GetName();
            OutRecord.Satisfaction = Citizen->GetSatisfaction();
            OutRecord.IdentityProfile = Citizen->GetIdentityProfile();
            OutRecord.PoliticalProfile = Citizen->GetPoliticalProfile();
            OutRecord.State = Citizen->GetCitizenState();
            OutRecord.HomeBuildingName = Citizen->GetHomeBuilding();
            OutRecord.WorkBuildingName = Citizen->GetWorkBuilding();
            OutRecord.FoodBuildingName = Citizen->GetFoodBuilding();
            OutRecord.FoodVisitBuildingName = Citizen->GetFoodVisitBuilding();
            OutRecord.FunBuildingName = Citizen->GetFunBuilding();
            return true;
        }

        std::wstring ResolveBuildingDisplayName(
            const std::string& BuildingName) const override
        {
            if (BuildingName.empty())
                return L"-";

            auto Building = FindValidBuilding(BuildingName);

            if (!Building)
                return Utf8ToWide(BuildingName);

            const std::wstring DisplayName =
                Utf8ToWide(Building->GetBuildingDisplayName());
            return DisplayName.empty() ?
                Utf8ToWide(BuildingName) :
                DisplayName;
        }

    private:
        std::shared_ptr<CPlacementAreaObject> FindValidBuilding(
            const std::string& BuildingName) const
        {
            if (!mWorld || BuildingName.empty())
                return nullptr;

            auto Building =
                mWorld->FindObject<CPlacementAreaObject>(BuildingName).lock();

            if (!Building || !Building->GetAlive() || !Building->GetEnable())
                return nullptr;

            return Building;
        }

        std::shared_ptr<CBuildingMarkerOrb> FindValidCitizen(
            const std::string& CitizenName) const
        {
            if (!mWorld || CitizenName.empty())
                return nullptr;

            auto Citizen =
                mWorld->FindObject<CBuildingMarkerOrb>(CitizenName).lock();

            if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
                return nullptr;

            return Citizen;
        }

        void PopulatePowerTotals(
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!mWorld)
                return;

            std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

            if (!mWorld->FindObjectListByType<CPlacementAreaObject>(BuildingList))
                return;

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

                OutRecord.TotalProducedPowerMW +=
                    (std::max)(
                        0,
                        (std::max)(
                            ExtractPowerValueMW(
                                Entry->DetailText,
                                L"생산 전력:"),
                            ExtractPowerValueMW(
                                Entry->DetailText,
                                L"발전량:")));
                OutRecord.TotalRequiredPowerMW +=
                    (std::max)(
                        0,
                        ExtractPowerValueMW(
                            Entry->DetailText,
                            L"필요 전력:"));
            }
        }

        void PopulateCitizenAssignments(
            const std::string& BuildingName,
            CPlacementAreaObject& Building,
            CitizenInfoDataProvider::FCitizenInfoBuildingRecord& OutRecord)
            const
        {
            if (!mWorld)
                return;

            std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

            if (!mWorld->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
                return;

            for (size_t Index = 0; Index < OrbList.size(); ++Index)
            {
                auto Orb = OrbList[Index].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                const std::string OrbName = Orb->GetName();

                if (Orb->GetHomeBuilding() == BuildingName)
                    PushUnique(OutRecord.Residents, OrbName);

                if (OutRecord.WorkProvider &&
                    Orb->GetWorkBuilding() == BuildingName)
                {
                    PushUnique(OutRecord.AssignedEmployees, OrbName);

                    if (Orb->GetCitizenState() == ECitizenState::AtWork)
                    {
                        PushUnique(
                            OutRecord.WorkingEmployees,
                            OrbName);
                    }
                }

                if (OutRecord.EntertainmentProvider &&
                    Orb->GetFunBuilding() == BuildingName)
                {
                    PushUnique(OutRecord.AssignedVisitors, OrbName);

                    if (Orb->GetCitizenState() == ECitizenState::AtFun)
                        PushUnique(OutRecord.ArrivedVisitors, OrbName);
                }

                if (!OutRecord.FoodProvider)
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

                    PushUnique(OutRecord.AssignedVisitors, OrbName);
                    PushUnique(OutRecord.ArrivedVisitors, OrbName);
                    continue;
                }

                if (OrbState != ECitizenState::GoingToFood ||
                    Orb->GetFoodBuilding() != BuildingName)
                {
                    continue;
                }

                FVector3 MarkerPos = FVector3::Zero;

                if (!Building.GetClosestMarkerWorldPos(
                    Orb->GetWorldPos(),
                    MarkerPos))
                {
                    continue;
                }

                FVector3 OrbPos = Orb->GetWorldPos();
                OrbPos.z = MarkerPos.z;

                const float NearDistance =
                    (std::max)(8.f, Orb->GetArrivalDistance() * 2.f);

                if (OrbPos.Distance(MarkerPos) > NearDistance)
                    continue;

                PushUnique(OutRecord.AssignedVisitors, OrbName);
                PushUnique(OutRecord.IncomingVisitors, OrbName);
            }

            std::sort(OutRecord.Residents.begin(), OutRecord.Residents.end());
            std::sort(
                OutRecord.AssignedEmployees.begin(),
                OutRecord.AssignedEmployees.end());
            std::sort(
                OutRecord.WorkingEmployees.begin(),
                OutRecord.WorkingEmployees.end());
            std::sort(
                OutRecord.AssignedVisitors.begin(),
                OutRecord.AssignedVisitors.end());
            std::sort(
                OutRecord.ArrivedVisitors.begin(),
                OutRecord.ArrivedVisitors.end());
            std::sort(
                OutRecord.IncomingVisitors.begin(),
                OutRecord.IncomingVisitors.end());
        }

    private:
        std::shared_ptr<CWorld> mWorld;
        std::shared_ptr<IMainWorldBuildMenuAccess> mMainWorldAccess;
    };
}

namespace CitizenInfoQueryService
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldCitizenInfoQuerySource>(World);
    }
}
