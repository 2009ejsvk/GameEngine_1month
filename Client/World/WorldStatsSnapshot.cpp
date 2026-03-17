#include "WorldStatsSnapshot.h"
#include "../Building/BuildingCatalog.h"
#include "../GameConstants.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../StringUtils.h"
#include "World/World.h"
#include <algorithm>
#include <cwctype>
#include <map>

namespace
{
    using StringUtils::Utf8ToWide;

    struct FHouseholdSnapshotInfo
    {
        ECitizenWealthLevel WealthLevel = ECitizenWealthLevel::Poor;
        std::string HomeName;
        int MemberCount = 0;
    };

    template <typename TMap>
    std::vector<std::pair<std::wstring, int>> BuildTopList(
        const TMap& Source,
        size_t MaxCount)
    {
        std::vector<std::pair<std::wstring, int>> Result;
        Result.reserve(Source.size());

        for (auto Iter = Source.begin(); Iter != Source.end(); ++Iter)
        {
            if (Iter->second <= 0)
                continue;

            Result.push_back(*Iter);
        }

        std::sort(
            Result.begin(),
            Result.end(),
            [](const std::pair<std::wstring, int>& A,
                const std::pair<std::wstring, int>& B)
            {
                if (A.second != B.second)
                    return A.second > B.second;

                return A.first < B.first;
            });

        if (Result.size() > MaxCount)
            Result.resize(MaxCount);

        return Result;
    }

    bool IsFaithBuildingName(const std::wstring& BuildingName)
    {
        return BuildingName == L"예배당" ||
            BuildingName == L"교회" ||
            BuildingName == L"성당";
    }

    int ResolveResidentialWealthTier(const FBuildingCatalogEntry& Entry)
    {
        const std::wstring& DetailText = Entry.DetailText;

        if (DetailText.find(L"더럽게 부유") != std::wstring::npos)
            return 4;
        if (DetailText.find(L"부유") != std::wstring::npos)
            return 3;
        if (DetailText.find(L"유복") != std::wstring::npos)
            return 2;
        if (DetailText.find(L"가난") != std::wstring::npos)
            return 1;
        if (DetailText.find(L"파산") != std::wstring::npos)
            return 0;

        switch (Entry.HousingClass)
        {
        case EBuildingHousingClass::Collective:
            return 1;
        case EBuildingHousingClass::Standard:
            return 2;
        case EBuildingHousingClass::Elite:
            return 3;
        default:
            return -1;
        }
    }

    int ResolveSatisfactionTier(double Value)
    {
        if (Value < 20.0)
            return 0;
        if (Value < 40.0)
            return 1;
        if (Value < 55.0)
            return 2;
        if (Value < 75.0)
            return 3;
        return 4;
    }

    int ResolveResidentialHouseholdCapacity(
        const CPlacementAreaObject& Building,
        const FBuildingCatalogEntry& Entry)
    {
        const int HouseholdCapacity = (std::max)(0, Entry.HouseholdCapacity);

        if (HouseholdCapacity > 0)
            return HouseholdCapacity;

        return (std::max)(0, Building.GetCapacity());
    }

    bool IsTourismAnalyticsVenue(const FBuildingCatalogEntry& Entry)
    {
        return Entry.Category == EBuildingCategory::Tourism ||
            Entry.Category == EBuildingCategory::LuxuryEntertainment ||
            HasTouristPreference(Entry.PrimaryTouristPreference);
    }

    std::shared_ptr<CPlacementAreaObject> ResolveActiveBuildingByName(
        const std::map<std::string, std::shared_ptr<CPlacementAreaObject>>&
            ActiveBuildingsByName,
        const std::string& Name)
    {
        if (Name.empty())
            return nullptr;

        const auto Iter = ActiveBuildingsByName.find(Name);
        return Iter != ActiveBuildingsByName.end() ? Iter->second : nullptr;
    }

    std::vector<std::wstring> SplitLines(const std::wstring& Text)
    {
        std::vector<std::wstring> Lines;
        std::wstring Current;

        for (wchar_t Ch : Text)
        {
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

        if (!Current.empty())
            Lines.push_back(Current);

        return Lines;
    }

    std::wstring TrimCopy(const std::wstring& Text)
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
        return Text.size() >= PrefixLength &&
            Text.compare(0, PrefixLength, Prefix) == 0;
    }

    int ExtractPowerValueMW(
        const std::wstring& DetailText,
        const wchar_t* Prefix)
    {
        if (!Prefix)
            return 0;

        const std::vector<std::wstring> Lines = SplitLines(DetailText);

        for (const std::wstring& RawLine : Lines)
        {
            const std::wstring Line = TrimCopy(RawLine);
            if (!StartsWith(Line, Prefix))
                continue;

            size_t Pos = wcslen(Prefix);
            while (Pos < Line.size() &&
                !iswdigit(Line[Pos]) &&
                Line[Pos] != L'+' &&
                Line[Pos] != L'-')
            {
                ++Pos;
            }

            bool Negative = false;
            if (Pos < Line.size() &&
                (Line[Pos] == L'+' || Line[Pos] == L'-'))
            {
                Negative = Line[Pos] == L'-';
                ++Pos;
            }

            std::wstring Digits;
            while (Pos < Line.size())
            {
                const wchar_t Ch = Line[Pos];
                if (iswdigit(Ch))
                    Digits.push_back(Ch);
                else if (Ch != L',')
                    break;
                ++Pos;
            }

            if (Digits.empty())
                return 0;

            const int Value = _wtoi(Digits.c_str());
            return Negative ? -Value : Value;
        }

        return 0;
    }
}

namespace WorldStats
{
    FWorldStatsSnapshot BuildSnapshot(const std::shared_ptr<CWorld>& World)
    {
        FWorldStatsSnapshot Snapshot;

        if (!World)
            return Snapshot;

        for (int ResourceIndex = 0;
            ResourceIndex < GResourceTypeCount;
            ++ResourceIndex)
        {
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].Type =
                static_cast<EResourceType>(ResourceIndex);
        }
        for (int CategoryIndex = 0;
            CategoryIndex < GBuildingCategoryCount;
            ++CategoryIndex)
        {
            Snapshot.BuildingCategories[static_cast<size_t>(CategoryIndex)].
                Category = static_cast<EBuildingCategory>(CategoryIndex);
        }

        std::map<std::wstring, int> BuildingCounts;
        std::map<std::wstring, int> ResourceCounts;
        std::array<std::map<std::wstring, int>, GBuildingCategoryCount>
            BuildingCountsByCategory;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ResourceCountsByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ProducerCountsByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            WarehouseCountsByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ConsumerCountsByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            HarborCountsByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ResourceShortageByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ResourceReservedByType;
        std::array<std::map<std::wstring, int>, GResourceTypeCount>
            ResourceOverflowByType;
        std::map<std::string, std::shared_ptr<CPlacementAreaObject>>
            ActiveBuildingsByName;
        std::map<std::string, int> ResidentialCapacityByName;
        std::map<std::string, int> ResidentialOccupancyByName;
        std::map<std::string, int> ResidentialWealthTierByName;
        std::map<std::string, int> WorkCapacityByName;
        std::map<std::string, int> WorkOccupancyByName;
        std::map<std::string, int> WorkEducationByName;
        std::vector<std::weak_ptr<CPlacementAreaObject>> BuildingList;

        if (World->FindObjectListByType<CPlacementAreaObject>(BuildingList))
        {
            for (size_t i = 0; i < BuildingList.size(); ++i)
            {
                auto Building = BuildingList[i].lock();

                if (!Building ||
                    !Building->GetAlive() ||
                    !Building->GetEnable() ||
                    !Building->HasPlacedArea())
                {
                    continue;
                }

                ++Snapshot.TotalBuildingCount;
                Snapshot.MonthlyWageCost += Building->GetMonthlyWageCost();
                Snapshot.MonthlyUpkeepCost += Building->GetMonthlyUpkeepCost();
                Snapshot.TotalResourceStock += Building->GetResourceStock();
                ActiveBuildingsByName[Building->GetName()] = Building;

                if (Building->IsFoodProvider())
                    ++Snapshot.FoodProviderCount;

                if (Building->IsEntertainmentProvider())
                    ++Snapshot.EntertainmentBuildingCount;

                if (Building->IsHarbor())
                    ++Snapshot.HarborCount;

                const std::wstring BuildingName =
                    Utf8ToWide(Building->GetBuildingDisplayName());

                if (IsFaithBuildingName(BuildingName))
                    ++Snapshot.FaithBuildingCount;

                ++BuildingCounts[BuildingName];

                if (Building->GetResourceStock() > 0)
                    ResourceCounts[BuildingName] += Building->GetResourceStock();

                for (int ResourceIndex = 1;
                    ResourceIndex < GResourceTypeCount;
                    ++ResourceIndex)
                {
                    const EResourceType ResourceType =
                        static_cast<EResourceType>(ResourceIndex);
                    const int Stock = Building->GetResourceStock(ResourceType);
                    const int AvailableStock =
                        Building->GetAvailableResourceStock(ResourceType);
                    const int ReservedPickup =
                        Building->GetReservedResourcePickupAmount(
                            ResourceType);
                    const int ReservedIncoming =
                        Building->GetReservedIncomingResourceAmount(
                            ResourceType);
                    const int IncomingCapacity =
                        Building->GetAvailableIncomingCapacity(ResourceType);
                    const int CoveredStock = Stock + ReservedIncoming;
                    const bool Produces =
                        Building->GetProducedResourceType() == ResourceType;
                    bool Consumes =
                        Building->GetVisitConsumptionResourceType() ==
                        ResourceType &&
                        Building->GetProducedResourceType() != ResourceType;

                    for (int SlotIndex = 0;
                         SlotIndex < Building->GetProductionInputCount();
                         ++SlotIndex)
                    {
                        if (Building->GetProductionInputType(SlotIndex) ==
                            ResourceType)
                        {
                            Consumes = true;
                            break;
                        }
                    }
                    bool AssignedWarehouseSlot = false;

                    if (Building->IsWarehouse())
                    {
                        for (int SlotIndex = 0;
                            SlotIndex < Building->GetWarehouseSlotCount();
                            ++SlotIndex)
                        {
                            if (Building->GetWarehouseSlotType(SlotIndex) ==
                                ResourceType)
                            {
                                AssignedWarehouseSlot = true;
                                break;
                            }
                        }
                    }

                    const bool HasActiveStorage =
                        Produces ||
                        Stock > 0 ||
                        ReservedPickup > 0 ||
                        ReservedIncoming > 0 ||
                        IncomingCapacity > 0 ||
                        AssignedWarehouseSlot;
                    const bool TracksResource = HasActiveStorage || Consumes;

                    if (!TracksResource)
                        continue;

                    auto& ResourceSnapshot =
                        Snapshot.ResourceTypes[
                            static_cast<size_t>(ResourceIndex)];

                    ResourceSnapshot.TotalStock += Stock;
                    ResourceSnapshot.AvailableStock += AvailableStock;
                    ResourceSnapshot.ReservedPickup += ReservedPickup;
                    ResourceSnapshot.ReservedIncoming += ReservedIncoming;
                    ResourceSnapshot.AvailableIncomingCapacity +=
                        IncomingCapacity;

                    if (Consumes)
                    {
                        ++ResourceSnapshot.ConsumerBuildingCount;
                        const int ShortageAmount = (std::max)(
                            0,
                            GameConstants::Orb::
                                TeamsterConsumerTargetStock -
                                CoveredStock);
                        ResourceSnapshot.ShortagePressure += ShortageAmount;
                        ResourceSnapshot.ConsumerCoveredStock += CoveredStock;

                        if (CoveredStock > 0)
                        {
                            ConsumerCountsByType[
                                static_cast<size_t>(ResourceIndex)][
                                    BuildingName] += CoveredStock;
                        }

                        if (ShortageAmount > 0)
                        {
                            ResourceShortageByType[
                                static_cast<size_t>(ResourceIndex)][
                                    BuildingName] += ShortageAmount;
                        }
                    }
                    if (Produces)
                    {
                        ++ResourceSnapshot.ProducerBuildingCount;
                        ResourceSnapshot.ProducerAvailableStock +=
                            AvailableStock;

                        if (AvailableStock > 0)
                        {
                            ProducerCountsByType[
                                static_cast<size_t>(ResourceIndex)][
                                    BuildingName] += AvailableStock;
                        }
                    }

                    if (HasActiveStorage)
                    {
                        ResourceSnapshot.Capacity +=
                            Building->GetResourceTypeCapacity(ResourceType);
                        ++ResourceSnapshot.StorageBuildingCount;

                        if (Building->IsHarbor())
                        {
                            ++ResourceSnapshot.HarborBuildingCount;
                            ResourceSnapshot.HarborExportableStock +=
                                Building->GetAvailableExportableResourceStock();
                            ResourceSnapshot.HarborReservedPickup +=
                                Building->GetReservedExportPickupAmount();

                            if (Stock > 0)
                            {
                                HarborCountsByType[
                                    static_cast<size_t>(ResourceIndex)][
                                        BuildingName] += Stock;
                            }
                        }
                    }

                    if (Building->IsWarehouse())
                    {
                        ++ResourceSnapshot.WarehouseBuildingCount;
                        ResourceSnapshot.WarehouseBufferedStock += Stock;

                        if (Stock > 0)
                        {
                            WarehouseCountsByType[
                                static_cast<size_t>(ResourceIndex)][
                                    BuildingName] += Stock;
                        }
                    }

                    if (Stock > 0)
                    {
                        ResourceCountsByType[
                            static_cast<size_t>(ResourceIndex)][BuildingName] +=
                            Stock;

                        const int Capacity =
                            Building->GetResourceTypeCapacity(ResourceType);
                        const int FillPercent =
                            Capacity > 0 ?
                                static_cast<int>(round(
                                    static_cast<double>(Stock) /
                                    static_cast<double>(Capacity) *
                                    100.0)) :
                                0;

                        if (FillPercent >= 75)
                        {
                            auto& OverflowMap =
                                ResourceOverflowByType[
                                    static_cast<size_t>(ResourceIndex)];
                            auto OverflowIter = OverflowMap.find(BuildingName);

                            if (OverflowIter == OverflowMap.end())
                            {
                                OverflowMap.emplace(
                                    BuildingName,
                                    FillPercent);
                            }
                            else
                            {
                                OverflowIter->second =
                                    (std::max)(
                                        OverflowIter->second,
                                        FillPercent);
                            }
                        }
                    }

                    if (ReservedPickup > 0)
                    {
                        ResourceReservedByType[
                            static_cast<size_t>(ResourceIndex)][BuildingName] +=
                                ReservedPickup;
                    }
                }

                const FBuildingCatalogEntry* Entry =
                    FindBuildingCatalogEntry(Building->GetBuildingId());

                if (!Entry)
                    continue;

                const int CategoryIndex = static_cast<int>(Entry->Category);

                if (CategoryIndex >= 0 &&
                    CategoryIndex < GBuildingCategoryCount)
                {
                    ++Snapshot.BuildingCategoryCount[CategoryIndex];
                    ++Snapshot.BuildingCategories[
                        static_cast<size_t>(CategoryIndex)].Count;
                    ++BuildingCountsByCategory[
                        static_cast<size_t>(CategoryIndex)][BuildingName];
                }

                if (Entry->Category == EBuildingCategory::Tourism)
                    ++Snapshot.TourismBuildingCount;

                if (Building->IsEntertainmentProvider() &&
                    IsTourismAnalyticsVenue(*Entry))
                {
                    Snapshot.TourismVisitCapacity +=
                        (std::max)(
                            0,
                            Building->GetServiceVisitCapacity(
                                EBuildingServiceType::Fun));
                    Snapshot.TourismVisitOccupancy +=
                        (std::max)(
                            0,
                            Building->GetActiveServiceVisitorCount(
                                EBuildingServiceType::Fun));
                }

                Snapshot.TotalProducedPowerMW +=
                    (std::max)(0, Building->GetProducedPowerMW());
                Snapshot.TotalRequiredPowerMW +=
                    (std::max)(0, Building->GetRequiredPowerMW());

                const bool AffectsFreedom =
                    std::any_of(
                        Entry->PoliticalSignals.begin(),
                        Entry->PoliticalSignals.end(),
                        [](const FPoliticalSignalDef& Signal)
                        {
                            return Signal.Axis ==
                                EPoliticalAxis::IntellectualConservative;
                        });

                if (AffectsFreedom)
                    ++Snapshot.FreedomInfluenceBuildingCount;

                const bool AffectsSecurity =
                    Entry->DetailText.find(L"치안") != std::wstring::npos ||
                    Entry->DetailText.find(L"범죄") != std::wstring::npos;

                if (AffectsSecurity)
                    ++Snapshot.SecurityInfluenceBuildingCount;

                const bool IsWorkBuilding =
                    !Building->IsResidential() &&
                    !Building->IsHarbor() &&
                    (!Building->IsEntertainmentProvider() ||
                        Building->IsFoodProvider());

                if (Entry->Residential)
                {
                    const int HouseholdCapacity =
                        ResolveResidentialHouseholdCapacity(*Building, *Entry);
                    Snapshot.ResidentialCapacity += HouseholdCapacity;
                    ResidentialCapacityByName[Building->GetName()] =
                        HouseholdCapacity;
                    ResidentialWealthTierByName[Building->GetName()] =
                        ResolveResidentialWealthTier(*Entry);
                }

                if (IsWorkBuilding)
                {
                    Snapshot.JobCapacity += Building->GetCapacity();
                    WorkCapacityByName[Building->GetName()] =
                        (std::max)(0, Building->GetCapacity());
                    WorkEducationByName[Building->GetName()] =
                        static_cast<int>(Entry->RequiredEducationLevel);
                }
            }
        }

        for (int ResourceIndex = 1;
            ResourceIndex < GResourceTypeCount;
            ++ResourceIndex)
        {
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopStockBuildings =
                    BuildTopList(
                        ResourceCountsByType[
                            static_cast<size_t>(ResourceIndex)],
                        8);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopProducerBuildings =
                    BuildTopList(
                        ProducerCountsByType[
                            static_cast<size_t>(ResourceIndex)],
                        4);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopWarehouseBuildings =
                    BuildTopList(
                        WarehouseCountsByType[
                            static_cast<size_t>(ResourceIndex)],
                        4);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopConsumerBuildings =
                    BuildTopList(
                        ConsumerCountsByType[
                            static_cast<size_t>(ResourceIndex)],
                        4);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopHarborBuildings =
                    BuildTopList(
                        HarborCountsByType[
                            static_cast<size_t>(ResourceIndex)],
                        4);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopShortageBuildings =
                    BuildTopList(
                        ResourceShortageByType[
                            static_cast<size_t>(ResourceIndex)],
                        6);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopReservedBuildings =
                    BuildTopList(
                        ResourceReservedByType[
                            static_cast<size_t>(ResourceIndex)],
                        6);
            Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)].
                TopOverflowBuildings =
                    BuildTopList(
                        ResourceOverflowByType[
                            static_cast<size_t>(ResourceIndex)],
                        6);
        }
        for (int CategoryIndex = 0;
            CategoryIndex < GBuildingCategoryCount;
            ++CategoryIndex)
        {
            Snapshot.BuildingCategories[static_cast<size_t>(CategoryIndex)].
                TopBuildings =
                    BuildTopList(
                        BuildingCountsByCategory[
                            static_cast<size_t>(CategoryIndex)],
                        8);
        }

        Snapshot.TopBuildings = BuildTopList(BuildingCounts, 8);
        Snapshot.TopResourceBuildings = BuildTopList(ResourceCounts, 8);

        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        {
            std::map<int, FHouseholdSnapshotInfo> HouseholdsById;
            int ResidentialOverlaySampleCount = 0;

            for (size_t i = 0; i < OrbList.size(); ++i)
            {
                auto Orb = OrbList[i].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                const FNpcSatisfaction& Satisfaction = Orb->GetSatisfaction();
                const FNpcPoliticalProfile& Political =
                    Orb->GetPoliticalProfile();

                ++Snapshot.ActiveCitizenCount;
                Snapshot.AverageFood += Satisfaction.Food;
                Snapshot.AverageHealth += Satisfaction.Health;
                Snapshot.AverageFun += Satisfaction.Fun;
                Snapshot.AverageFaith += Satisfaction.Faith;
                Snapshot.AverageHousing += Satisfaction.Housing;
                Snapshot.AverageJob += Satisfaction.Job;
                Snapshot.AverageFreedom += Satisfaction.Freedom;
                Snapshot.AverageSecurity += Satisfaction.Security;
                Snapshot.AverageOverall += Satisfaction.Overall;
                const int OverallSatisfactionTier =
                    ResolveSatisfactionTier(Satisfaction.Overall);
                if (OverallSatisfactionTier >= 0 &&
                    OverallSatisfactionTier < 5)
                {
                    ++Snapshot.OverallSatisfactionCitizenCount[
                        OverallSatisfactionTier];
                }
                const int WealthIndex = static_cast<int>(
                    Orb->GetIdentityProfile().WealthLevel);
                if (WealthIndex >= 0 &&
                    WealthIndex < GCitizenWealthLevelCount)
                    ++Snapshot.CitizenWealthCount[WealthIndex];
                const int EducationIndex = static_cast<int>(
                    Orb->GetIdentityProfile().EducationLevel);

                if (EducationIndex >= 0 && EducationIndex < 3)
                    ++Snapshot.EducationCount[EducationIndex];

                const FCitizenIdentityProfile& Identity =
                    Orb->GetIdentityProfile();
                const std::shared_ptr<CPlacementAreaObject> HomeBuilding =
                    ResolveActiveBuildingByName(
                        ActiveBuildingsByName,
                        Orb->GetHomeBuilding());
                if (HomeBuilding)
                {
                    Snapshot.AverageResidentialFreedom +=
                        HomeBuilding->GetLocalFreedomScore();
                    Snapshot.AverageResidentialSecurity +=
                        HomeBuilding->GetLocalSecurityScore();
                    Snapshot.AverageResidentialPollution +=
                        HomeBuilding->GetLocalPollutionExposure();
                    ++ResidentialOverlaySampleCount;
                }
                if (Identity.IsTourist)
                {
                    ++Snapshot.ActiveTouristCount;

                    const int PreferenceIndex =
                        static_cast<int>(Identity.TouristProfile);
                    if (PreferenceIndex >= 0 &&
                        PreferenceIndex < GTouristPreferenceCount)
                    {
                        ++Snapshot.TouristProfileCount[PreferenceIndex];
                    }

                    const std::string& FunBuildingName =
                        !Orb->GetFunVisitBuilding().empty() ?
                            Orb->GetFunVisitBuilding() :
                            Orb->GetFunBuilding();
                    if (!FunBuildingName.empty())
                    {
                        const auto BuildingIter =
                            ActiveBuildingsByName.find(FunBuildingName);
                        if (BuildingIter != ActiveBuildingsByName.end() &&
                            BuildingIter->second)
                        {
                            const FBuildingCatalogEntry* const FunEntry =
                                FindBuildingCatalogEntry(
                                    BuildingIter->second->GetBuildingId());
                            if (FunEntry &&
                                FunEntry->PrimaryTouristPreference ==
                                    Identity.TouristProfile)
                            {
                                ++Snapshot.TouristPreferenceMatchedCount;
                            }
                        }
                    }
                }
                int HouseholdId = Identity.HouseholdId;

                if (HouseholdId <= 0)
                    HouseholdId = -static_cast<int>(i) - 1;

                auto& Household = HouseholdsById[HouseholdId];
                Household.WealthLevel = Identity.WealthLevel;
                ++Household.MemberCount;

                if (Household.HomeName.empty() &&
                    !Orb->GetHomeBuilding().empty())
                {
                    Household.HomeName = Orb->GetHomeBuilding();
                }

                if (Orb->GetHomeBuilding().empty())
                {
                    ++Snapshot.HomelessCount;
                }

                if (Orb->GetWorkBuilding().empty())
                {
                    ++Snapshot.UnemployedCount;
                    if (EducationIndex >= 0 && EducationIndex < 3)
                    {
                        ++Snapshot.UnemployedEducationCount[EducationIndex];
                    }
                }
                else
                {
                    ++Snapshot.AssignedJobCount;
                    ++WorkOccupancyByName[Orb->GetWorkBuilding()];
                }

                int NeutralAxisCount = 0;

                for (int AxisIndex = 0;
                    AxisIndex < static_cast<int>(EPoliticalAxis::Count);
                    ++AxisIndex)
                {
                    const EPoliticalAxis Axis =
                        static_cast<EPoliticalAxis>(AxisIndex);
                    const int StanceIndex =
                        static_cast<int>(Political.Get(Axis).Stance);

                    if (StanceIndex >= 0 && StanceIndex < 3)
                    {
                        ++Snapshot.PoliticalCount[AxisIndex][StanceIndex];

                        if (StanceIndex ==
                            static_cast<int>(EPoliticalStance::Neutral))
                        {
                            ++NeutralAxisCount;
                        }
                    }
                }

                if (NeutralAxisCount > 0)
                    ++Snapshot.AnyNeutralAxisCitizenCount;

                if (NeutralAxisCount ==
                    static_cast<int>(EPoliticalAxis::Count))
                {
                    ++Snapshot.FullyNeutralCitizenCount;
                }
            }

            Snapshot.ActiveHouseholdCount =
                static_cast<int>(HouseholdsById.size());

            for (auto HouseholdIt = HouseholdsById.begin();
                HouseholdIt != HouseholdsById.end();
                ++HouseholdIt)
            {
                const FHouseholdSnapshotInfo& Household = HouseholdIt->second;
                const int WealthIndex =
                    static_cast<int>(Household.WealthLevel);

                if (Household.HomeName.empty())
                {
                    ++Snapshot.HomelessHouseholdCount;

                    if (WealthIndex >= 0 &&
                        WealthIndex < GCitizenWealthLevelCount)
                        ++Snapshot.HomelessWealthCount[WealthIndex];

                    continue;
                }

                ++Snapshot.AssignedHomeCount;
                ++ResidentialOccupancyByName[Household.HomeName];
            }

            if (ResidentialOverlaySampleCount > 0)
            {
                const double Denominator =
                    static_cast<double>(ResidentialOverlaySampleCount);
                Snapshot.AverageResidentialFreedom /= Denominator;
                Snapshot.AverageResidentialSecurity /= Denominator;
                Snapshot.AverageResidentialPollution /= Denominator;
            }
        }

        if (Snapshot.ActiveCitizenCount > 0)
        {
            const double Denominator =
                static_cast<double>(Snapshot.ActiveCitizenCount);
            Snapshot.AverageFood /= Denominator;
            Snapshot.AverageHealth /= Denominator;
            Snapshot.AverageFun /= Denominator;
            Snapshot.AverageFaith /= Denominator;
            Snapshot.AverageHousing /= Denominator;
            Snapshot.AverageJob /= Denominator;
            Snapshot.AverageFreedom /= Denominator;
            Snapshot.AverageSecurity /= Denominator;
            Snapshot.AverageOverall /= Denominator;
        }

        for (auto Iter = ResidentialCapacityByName.begin();
            Iter != ResidentialCapacityByName.end();
            ++Iter)
        {
            const int Capacity = (std::max)(0, Iter->second);

            if (Capacity <= 0)
                continue;

            const auto OccupancyIter =
                ResidentialOccupancyByName.find(Iter->first);
            const int Occupied =
                OccupancyIter != ResidentialOccupancyByName.end() ?
                (std::max)(0, OccupancyIter->second) :
                0;

            if (Occupied < Capacity)
            {
                ++Snapshot.ResidentialVacancyBuildingCount;
                const auto WealthTierIter =
                    ResidentialWealthTierByName.find(Iter->first);
                const int WealthTier =
                    WealthTierIter != ResidentialWealthTierByName.end() ?
                    WealthTierIter->second :
                    -1;

                if (WealthTier >= 0 && WealthTier < 5)
                {
                    ++Snapshot.ResidentialVacancyWealthCount[WealthTier];
                }
            }
        }

        for (auto Iter = WorkCapacityByName.begin();
            Iter != WorkCapacityByName.end();
            ++Iter)
        {
            const int Capacity = (std::max)(0, Iter->second);

            if (Capacity <= 0)
                continue;

            const auto OccupancyIter =
                WorkOccupancyByName.find(Iter->first);
            const int Occupied =
                OccupancyIter != WorkOccupancyByName.end() ?
                (std::max)(0, OccupancyIter->second) :
                0;

            if (Occupied < Capacity)
            {
                ++Snapshot.WorkVacancyBuildingCount;
                const auto EducationIter =
                    WorkEducationByName.find(Iter->first);
                const int EducationIndex =
                    EducationIter != WorkEducationByName.end() ?
                    EducationIter->second :
                    -1;
                const int VacancyCount =
                    (std::max)(0, Capacity - Occupied);

                if (EducationIndex >= 0 && EducationIndex < 3)
                {
                    Snapshot.WorkVacancyEducationCount[EducationIndex] +=
                        VacancyCount;
                }
            }
        }

        return Snapshot;
    }
}
