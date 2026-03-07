#include "BuildingCatalog.h"
#include <Windows.h>
#include <algorithm>
#include <vector>
#include <string>

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

    bool IsCollectiveHousingName(const std::wstring& Name)
    {
        return Name == L"판잣집" ||
            Name == L"합숙소" ||
            Name == L"간이 숙박소" ||
            Name == L"서민 아파트" ||
            Name == L"공동주택";
    }

    bool IsEliteHousingName(const std::wstring& Name)
    {
        return Name == L"시골 주택" ||
            Name == L"저택" ||
            Name == L"단독주택" ||
            Name == L"현대식 저택" ||
            Name == L"보안 저택";
    }

    bool IsEliteLeisureName(const std::wstring& Name)
    {
        return Name == L"카지노" ||
            Name == L"칵테일 바" ||
            Name == L"골프 코스" ||
            Name == L"고급 레스토랑" ||
            Name == L"나이트클럽" ||
            Name == L"요트 클럽";
    }

    bool IsCulturalVenueName(const std::wstring& Name)
    {
        return Name == L"극장" ||
            Name == L"오페라 극장" ||
            Name == L"현대 미술관";
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
            if (IsCollectiveHousingName(Name))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Right, 4.0f, EPoliticalScope::Resident);
            }
            else if (IsEliteHousingName(Name))
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
            if (IsEliteLeisureName(Name))
            {
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::Economy,
                    EPoliticalStance::Left, 2.5f);
                AddPoliticalSignal(
                    Entry, EPoliticalAxis::IntellectualConservative,
                    EPoliticalStance::Right, 1.5f);
            }
            else if (IsCulturalVenueName(Name))
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
        static const wchar_t* InfrastructureNames[] =
        {
            L"도로",
            L"철거",
            L"항구",
            L"건설 사무소",
            L"운송업자 사무소",
            L"선창",
            L"화물창고",
            L"창고",
            L"변전소",
            L"발전소",
            L"대형 주차장",
            L"버스차고지",
            L"버스 정류장",
            L"터널 연결",
            L"원자력 발전소",
            L"지하철역",
            L"공중 케이블 기지",
            L"풍력 터빈",
            L"해상 풍력 터빈",
            L"태양광 발전소"
        };
        static const wchar_t* InfrastructureDetails[] =
        {
            LR"(건설 비용: 15
등장 시기: 식민지 시대, 세계대전 시대(다리)
건물을 도로로 연결하여 상품 운송과 이동을 가능하게 합니다.)",
            LR"(건설 비용: 무료
등장 시기: 식민지 시대
건물, 장식물, 다리, 도로를 철거합니다.)",
            LR"(건설 비용: 9600
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
비고: 도로, 해안, 최소 1개 필요
효과: 수입/수출, 이민/이주 처리, 주변 치안 감소
운영 모드:
- 일반 통제
- 철두철미한 검사 (범죄 -15, 효율 -20%))",
            LR"(건설 비용: 2000
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
크기: 3x2
비고: 도로, 최소 1개 필요
운영 모드:
- 일반 통제
- 안전 규정 무시 (건설 속도 +20%, 사망 위험 10%)
업그레이드:
- 더 많은 일손 (1200, 일자리 +4))",
            LR"(건설 비용: 2000
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
크기: 3x2
비고: 도로, 최소 1개 필요
운영 모드:
- 안전 하중
- 느슨한 하중 제한 (적하량 +50%, 최대 10% 화물 손실)
업그레이드:
- 교대 근무 (세계대전 시대, 2500, 일자리 +6))",
            LR"(건설 비용: 300
등장 시기: 식민지 시대
크기: 2x5
비고: 해안
시민/관광객의 보트 승하선을 지원합니다.)",
            LR"(건설 비용: 1700
등장 시기: 식민지 시대
필요 인력: 갑판원 업그레이드 시 6 (무학력자)
크기: 2x5
비고: 해안, 도로
운영 모드:
- 안전 하중
- 느슨한 속도 제한 (화물선 속도 +50%, 최대 5% 화물 손실)
업그레이드:
- 갑판원 (1200, 일자리 +6))",
            LR"(건설 비용: 4000
등장 시기: 식민지 시대
필요 인력: 없음
크기: 3x3
비고: 도로
효과: 최대 3종류 상품 대량 보관 및 물류 지침 설정
업그레이드:
- 고층 진열대 (냉전, 5000, 슬롯당 보관량 +5000)
- 무작위 창고 보관 (현대, 15000, 슬롯 보관량 +50%))",
            LR"(설계도 비용: 1000
건설 비용: 2000
등장 시기: 세계대전 시대
필요 인력: 없음
필요 전력: 10MW
크기: 1x1
효과: 전력망 반경 확장, 공해 배출)",
            LR"(설계도 비용: 6000
건설 비용: 12000
등장 시기: 세계대전 시대
필요 인력: 6 (고졸)
생산 전력: 360MW
크기: 4x6
비고: 도로
업그레이드:
- 중앙 냉난방 (냉전, 4000)
- 오염 필터 (냉전, 5000)
- 석유 가열로 (냉전, 6000))",
            LR"(설계도 비용: 1000
건설 비용: 1200
등장 시기: 세계대전 시대
필요 인력: 1 (무학력자)
크기: 2x2
비고: 도로
효과: 부유한 시민/관광객 자동차 이용, 주변 치안 감소
업그레이드:
- 카메라 감시 (현대, 800))",
            LR"(설계도 비용: 1000
건설 비용: 1500
등장 시기: 세계대전 시대
필요 인력: 4 (무학력자)
크기: 3x2
비고: 도로
운영 모드:
- 승차권
- 무료 승차
업그레이드:
- 전기 버스 (현대, 1500, 40MW 필요))",
            LR"(건설 비용: 0
등장 시기: 세계대전 시대
버스 노선의 지점을 설정합니다.
버스차고지 건설 시 사용 가능합니다.)",
            LR"(건설 비용: 4000
등장 시기: 세계대전 시대
두 개의 터널을 연결해 고저차 지형을 우회하는 지름길 도로망을 만듭니다.
기본 비용은 연결 길이에 따라 증가합니다.)",
            LR"(설계도 비용: 16000
건설 비용: 32000
등장 시기: 냉전 시대
필요 인력: 4 (대졸)
발전량: 800MW
크기: 8x6
비고: 도로
운영 모드:
- 전기 생산 집중
- 우라늄 농축 (연구 필요)
업그레이드:
- 보관 용기 (냉전, 4000)
- 납 차폐 (냉전, 6000))",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 냉전 시대
필요 인력: 2 (무학력자)
필요 전력: 20MW
크기: 1x1
효과: 같은 섬 지하철역 간 승객 이동
업그레이드:
- 조명 추가 (현대, 500))",
            LR"(설계도 비용: 2250
건설 비용: 9000
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
필요 전력: 15MW
크기: 2x2
운영 모드:
- 승차권
- 무료 승차
업그레이드:
- 오디오 여행 가이드 (현대, 1500))",
            LR"(설계도 비용: 1250
건설 비용: 2500
등장 시기: 현대 시대
필요 인력: 없음
크기: 2x2
생산 전력: 70MW(최고효율)
운영 모드:
- 자연풍 발전
- 분당 64회전 (연구 필요))",
            LR"(설계도 비용: 5000
건설 비용: 9500
등장 시기: 현대 시대
필요 인력: 없음
크기: 8x8
생산 전력: 82MW(최고효율)
운영 모드:
- 공해 풍력 발전
- 스모그 팬 (전력 45MW 소모, 공해 감소))",
            LR"(설계도 비용: 9000
건설 비용: 18000
등장 시기: 현대 시대
필요 인력: 3 (고졸)
생산 전력: 450MW
크기: 7x7
운영 모드:
- 화창한 오후
- 태양광 과부하 (연구 필요)
업그레이드:
- 수프라 서페이스 3000메가 (현대, 5000))"
        };

        static const wchar_t* FoodResourceNames[] =
        {
            L"코코넛 수확기",
            L"벌목소",
            L"어선 선착장",
            L"대규모 농장",
            L"퇴비 살포기",
            L"목장",
            L"광산",
            L"정유시설",
            L"양식장",
            L"석유 시추 장치",
            L"대규모 수경 농장",
            L"공장식 목장"
        };
        static const wchar_t* FoodResourceDetails[] =
        {
            LR"(설계도 비용: 없음(기본 해금)
건설 비용: 800
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
크기: 3x2
비고: 도로
효과: 근처 야자수에서 코코넛 수확
업그레이드:
- 코코넛 절단기 (식민지, 300, 직업 품질 +5, 효율 +15%)
- 치료 (냉전, 1500, 코코넛 재생 시간 -50%))",
            LR"(설계도 비용: 없음
건설 비용: 875
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
크기: 3x2
비고: 도로
효과: 나무 벌목, 적은 공해 배출
운영 모드:
- 식수 병행 (묘목 재식재)
- 숲 벌목 (묘목 재식재 없음)
업그레이드:
- 양묘장 (세계대전, 1500, 성장 시간 -50%)
- 전기톱 (냉전, 1200, 효율 +20%, 직업 품질 +15))",
            LR"(설계도 비용: 없음
건설 비용: 1200
등장 시기: 식민지 시대
필요 인력: 4 (무학력자)
크기: 5x6
비고: 해안, 도로
효과: 근처 어장에서 물고기 생산
운영 모드:
- 선조의 방식
- 유망어업 (세계대전, 어장 1개당 물고기 1.75 생산)
- 지역시장 공급 (냉전, 산출량 -25%, 고급 레스토랑 효율 보너스)
업그레이드:
- 형망 어업 (식민지, 750, 조개류 추가 생산))",
            LR"(설계도 비용: 없음
건설 비용: 1500
등장 시기: 식민지 시대
필요 인력: 8 (무학력자)
크기: 최대 6x9
비고: 도로
효과: 선택 작물 재배
운영 모드:
- 단일 재배 (최대 효율, 비옥도 감소)
- 다양한 재배 (효율 -40%, 비옥도 유지, 인접 보너스)
업그레이드(요약):
- 공통: 녹색 폐기물 분쇄기, 현대화(대규모 수경 농장 전환)
- 작물별: 옥수수/바나나/파인애플/설탕/코코아/담배/커피/목화/고무 전용 업그레이드)",
            LR"(설계도 비용: 없음
건설 비용: 300
등장 시기: 식민지 시대
필요 인력: 3 (무학력자)
크기: 2x1
비고: 도로
효과: 범위 내 대규모 농장/목장 토양 악화 대응
운영 모드:
- 광역 살포
- 집중 살포 (범위 -30%, 효율 +200%)
업그레이드:
- 코 죔쇠 (식민지, 300, 직업 품질 +15)
- 더 높이 쌓기 (식민지, 600, 퇴비 재고 +2000))",
            LR"(설계도 비용: 없음
건설 비용: 900
등장 시기: 식민지 시대
필요 인력: 4 (무학력자)
크기: 4x4
효과: 가축 자원 생산(우유/고기/양모/생가죽/가죽)
운영 모드:
- 인도적인 착취
- 방목 금지 (비옥도 유지, 효율 -15%)
업그레이드(요약):
- 공통: 가축병 예방접종
- 가축별: 소/양/악어/돼지/염소/라마 전용 업그레이드)",
            LR"(설계도 비용: 없음
건설 비용: 1200
등장 시기: 식민지 시대
필요 인력: 5 (무학력자)
크기: 3x4
효과: 석탄/철/황금/니켈/알루미늄/우라늄 채굴, 공해 배출
운영 모드:
- 기본
- 수익 추구 절차
업그레이드(요약):
- 공통: 동력 드릴(15MW 필요, 효율 +25%), 카나리아(화재 위험 감소)
- 자원별: 선탄장/배소실/유해폐기물 폐기장 등)",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 세계대전 시대
필요 인력: 4 (고졸)
크기: 2x2
효과: 석유 추출, 공해 배출
운영 모드:
- 기본
업그레이드:
- 수리학적파쇄 시추공 (냉전, 3500, 효율 +20%, 공해 +50%)
- 코일 튜브 시추기 (냉전, 2500, 매장지 소모량 감소))",
            LR"(설계도 비용: 4000
건설 비용: 8000
등장 시기: 냉전 시대
필요 인력: 4 (고졸)
필요 전력: 10MW
크기: 3x4
효과: 물고기/조개류 양식
운영 모드:
- 기본
업그레이드:
- 대용량 수조 (냉전, 2000, 생산량 +10%, 외부 재고 +1920)
- 조수 시뮬레이터 (4000, 생산량 +25%, 전력 +25MW)
- 해수 파이프라인 (냉전, 750, 가뭄/화재 면역))",
            LR"(설계도 비용: 10000
건설 비용: 20000
등장 시기: 냉전 시대
필요 인력: 6 (고졸)
크기: 3x2
비고: 해양
효과: 해양 석유 매장지에서 석유 채굴)",
            LR"(설계도 비용: 2000
건설 비용: 4000
등장 시기: 현대 시대
필요 인력: 4 (고졸)
필요 전력: 30MW
크기: 3x5
효과: 지역 작물 상태와 무관하게 선택 작물 생산)",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 현대 시대
필요 인력: 4 (무학력자)
필요 전력: 25MW
크기: 4x4
효과: 주변 가축 상태와 독립 운영, 사료 필요(옥수수/악어는 생선)
비고: 적은 공해 배출)"
        };

        static const wchar_t* IndustryNames[] =
        {
            L"자동화 광산",
            L"제재소",
            L"럼주 증류소",
            L"무두질 공장",
            L"통조림 공장",
            L"유제품 공장",
            L"시가 공장",
            L"조선소",
            L"제철소",
            L"방직소",
            L"무기 공장",
            L"초콜릿 공장",
            L"가구 공장",
            L"보석 세공소",
            L"플라스틱 공장",
            L"자동차 공장",
            L"전자 제품 공장",
            L"패션 회사",
            L"제약 회사",
            L"주스 공장"
        };
        static const wchar_t* IndustryDetails[] =
        {
            LR"(설계도 비용: 4500
건설 비용: 9000
등장 시기: 현대 시대
필요 인력: 3 (무학력자)
필요 전력: 50MW
크기: 3x4
효과: 석탄/철/금/니켈/알루미늄/우라늄 자원을 고속 추출
비고: 공해 배출)",
            LR"(설계도 비용: 없음
건설 비용: 5600
등장 시기: 식민지 시대
필요 인력: 4 (무학력자)
크기: 3x4
운영 모드:
- 효율적인 하차
- 신속한 하차 (통나무 소모 +30%, 효율 +30%)
업그레이드:
- 전기톱 (세계대전, 5000, 효율 +30%, 전력 10MW 필요)
- 손가락 보험 (현대, 유지비 +$10, 직업 품질 +10))",
            LR"(설계도 비용: 없음
건설 비용: 9600
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
크기: 4x2
운영 모드:
- 수출용 진품 혼합물
- 지역시장 공급 (산출량 -25%, 나이트클럽/경기장 효율 보너스)
업그레이드:
- 당밀 증류 (식민지, 5000, 설탕 소모 -25%)
- 전기 증류 (세계대전, 5000, 효율 +20%, 전력 30MW 필요))",
            LR"(설계도 비용: 없음
건설 비용: 5000
등장 시기: 데이터 미기재
필요 인력: 5 (무학력자)
크기: 3x3
운영 모드:
- 신중한 측정
- 다다익선 (효율 +50%, 공해 +100%, 유지비 +100%)
업그레이드:
- 크롬 무두질 (세계대전, 5000, 생가죽 소모 -25%)
- 향기 스프레이 (냉전, 직업 품질 +10))",
            LR"(설계도 비용: 3350
건설 비용: 6500
등장 시기: 세계대전 시대
필요 인력: 8 (무학력자)
필요 전력: 80MW
크기: 3x3
효과: 파인애플/물고기를 통조림으로 가공, 공해 배출)",
            LR"(설계도 비용: 5350
건설 비용: 10500
등장 시기: 세계대전 시대
필요 인력: 4 (무학력자)
크기: 4x4
효과: 우유를 치즈로 가공, 적은 공해 배출)",
            LR"(설계도 비용: 6750
건설 비용: 13600
등장 시기: 세계대전 시대
필요 인력: 5 (고졸)
크기: 3x5
운영 모드:
- 말고, 말고 또 말고
- 지역시장 공급 (산출량 -25%, 요트 클럽/은행 효율 보너스)
업그레이드:
- 자동 생산 라인 (세계대전, 6000, 일자리 -2, 효율 +20%)
- 온도 조절기 (냉전, 6000, 투입량 -20%, 전력 +5MW)
- 자동 절단기 (냉전, 5000, 효율 +10%, 전력 +30MW))",
            LR"(설계도 비용: 2375
건설 비용: 9500
등장 시기: 세계대전 시대
필요 인력: 5 (고졸)
크기: 3x3
비고: 해안
효과: 널빤지를 가공해 보트 생산, 공해 배출
업그레이드:
- 중형 크레인 (세계대전, 12000, 효율 +20%, 직업 품질 +10)
- 용접 설비 (냉전, 내부 재고 확장, 강철/알루미늄 사용 가능, 전력 5MW 필요))",
            LR"(설계도 비용: 4750
건설 비용: 9500
등장 시기: 세계대전 시대
필요 인력: 8 (고졸)
크기: 4x2
운영 모드:
- 표준 공정
- 노정 가스 재순환 (효율 -10%, 공해 -20%)
- 열간 압연 (효율 +20%, 유지비 +20%)
업그레이드:
- 전기 아크 가열로 (현대, 석탄 소모 -50%, 전력 80MW 필요))",
            LR"(설계도 비용: 2625
건설 비용: 10500
등장 시기: 세계대전 시대
필요 인력: 10 (무학력자)
크기: 5x3
운영 모드:
- 재래식 염료
- 아크릴 염료 (효율 +15%, 공해 +50%)
- 유기농 염료 (현대, 연구 필요, 공해 -50%, 효율 -20%)
업그레이드:
- 작업장 확장 (세계대전, 4500, 일자리 +2)
- 고급 원단 롤러장 (세계대전, 3800, 효율 +10%, 전력 50MW 필요)
- 통합 직물 공급기 (냉전, 양모/목화 동시 투입 시 산출량 +15%))",
            LR"(설계도 비용: 4500
건설 비용: 18000
등장 시기: 세계대전 시대
필요 인력: 8 (고졸)
크기: 5x4
효과: 강철/니켈을 가공해 무기 생산, 공해 배출)",
            LR"(설계도 비용: 3250
건설 비용: 6500
등장 시기: 냉전 시대
필요 인력: 5 (고졸)
크기: 2x3
효과: 코코아/설탕을 가공해 초콜릿 생산, 적은 공해 배출)",
            LR"(설계도 비용: 4000
건설 비용: 8000
등장 시기: 냉전 시대
필요 인력: 5 (무학력자)
필요 전력: 25MW
크기: 2x3
효과: 널빤지를 가공해 가구 생산, 공해 배출
업그레이드:
- 조립 공정 (냉전, 전력 +25MW, 효율 +100%)
- 주형틀 (내부 플라스틱 재고 +8320, 전력 +35MW)
- CAD 소프트웨어 (현대, 고졸 필요, 널빤지 소모 -25%))",
            LR"(설계도 비용: 4750
건설 비용: 9500
등장 시기: 냉전 시대
필요 인력: 4 (고졸)
크기: 3x2
효과: 황금을 가공해 보석류 생산, 적은 공해 배출)",
            LR"(설계도 비용: 5250
건설 비용: 10500
등장 시기: 냉전 시대
필요 인력: 5 (무학력자)
필요 전력: 25MW
크기: 4x3
효과: 석유를 가공해 플라스틱 생산, 많은 공해 배출
운영 모드:
- 표준 제조
- 고속 제조 (소모량 +100%, 생산량 +75%)
업그레이드:
- 안전 폐기 장치 (냉전, 8000, 공해 -50%, 유지비 +200%)
- 바이오폴리머 제조 시설 (현대, 4500, 옥수수 재고 추가, 옥수수 생산 허용))",
            LR"(설계도 비용: 5500
건설 비용: 22000
등장 시기: 냉전 시대
필요 인력: 10 (고졸)
필요 전력: 50MW
크기: 6x3
효과: 강철/고무를 가공해 차량 생산, 공해 배출
운영 모드:
- 자동차 회사
- 회사 자동차 (전 직원 차량 이용, 유지비/직업 품질 증가)
업그레이드:
- 충돌 시험용 더미 (냉전, 1000, 직업 품질 +5)
- 완전 자동화 조립 (현대, 5000, 일자리 -2, 효율 +30%, 전력 +15MW)
- 향상된 공해 테스트 소프트웨어 (현대, 500, 환경주의자 우호도 +5))",
            LR"(설계도 비용: 12500
건설 비용: 25000
등장 시기: 현대 시대
필요 인력: 8 (고졸)
필요 전력: 60MW
크기: 4x3
효과: 황금/플라스틱을 가공해 전자 제품 생산, 공해 배출
운영 모드:
- 조립 공정
- 노동력 착취 (연구 필요, 생산량 +20%, 직업 품질 -50%, 사망 위험 증가)
업그레이드:
- 조립 로봇 (현대, 생산량 +50%, 전력 +100MW))",
            LR"(설계도 비용: 9000
건설 비용: 18000
등장 시기: 현대 시대
필요 인력: 4 (무학력자)
필요 전력: 25MW
크기: 4x2
효과: 옷감/가죽을 가공해 의류 생산, 적은 공해 배출
운영 모드:
- 바쁜 쿠튀르
- 오트 쿠튀르 (옷감+가죽 동시 소모, 임금/직업 품질 증가)
업그레이드:
- 공장 직판점 (현대, 15000, 주변 주거 품질 보너스, 생산량 -1))",
            LR"(설계도 비용: 14000
건설 비용: 28000
등장 시기: 현대 시대
필요 인력: 6 (고졸)
필요 전력: 80MW
크기: 4x3
효과: 석유를 가공해 의약품 생산, 공해 배출
운영 모드:
- 감쪽같은 복제품
- 강력한 플라시보 약 (연구 필요, 설탕 소모 생산)
- 지역시장 공급 (산출량 -25%, 나이트클럽/경기장 효율 보너스))",
            LR"(설계도 비용: 6000
건설 비용: 12000
등장 시기: 현대 시대
필요 인력: 5 (무학력자)
필요 전력: 30MW
크기: 4x3
효과: 바나나/파인애플을 가공해 주스 생산, 적은 공해 배출)"
        };

        static const wchar_t* HousingNames[] =
        {
            L"판잣집",
            L"시골 주택",
            L"합숙소",
            L"저택",
            L"단독주택",
            L"간이 숙박소",
            L"아파트",
            L"서민 아파트",
            L"공동주택",
            L"현대식 아파트",
            L"현대식 저택",
            L"보안 저택"
        };
        static const wchar_t* HousingDetails[] =
        {
            LR"(설계도 비용: 없음
건설 비용: 없음(건설불가)
등장 시기: 식민지 시대
수용 가구: 1
주거 품질: 10
요구 재산: 파산
크기: 1x1
효과: 주거지를 찾지 못한 시민이 자발적으로 건설
비고: 건축 UI 미노출, 적은 공해, 주변 치안 감소)",
            LR"(설계도 비용: 없음
건설 비용: 850
등장 시기: 식민지 시대
수용 가구: 2
주거 품질: 48
요구 재산: 유복
크기: 2x1
운영 모드:
- 판잣집보단 낫다
- 작은 에덴 (유지비 +50%, 주거 품질 +12)
업그레이드:
- 배전 (세계대전)
- 현대화 (세계대전, 단독주택으로 업그레이드))",
            LR"(설계도 비용: 없음
건설 비용: 1200
등장 시기: 식민지 시대
수용 가구: 6
주거 품질: 32
요구 재산: 가난
크기: 2x2
운영 모드:
- 보통 점유율
- 더 높이 쌓기 (가구수 +2, 주거 품질 -8, 파산 시민 입주 가능))",
            LR"(설계도 비용: 없음
건설 비용: 1200
등장 시기: 식민지 시대
수용 가구: 1
주거 품질: 68
요구 재산: 부유
크기: 3x2
업그레이드:
- 배전 (세계대전)
- 현대화 (현대, 전력 필요, 현대식 저택으로 업그레이드))",
            LR"(설계도 비용: 1000
건설 비용: 1450
등장 시기: 세계대전 시대
수용 가구: 3
주거 품질: 64
요구 재산: 유복
크기: 2x1
운영 모드:
- 보통 관리
- 과시하기 (연구 필요, 유지비 +10, 주변 미관 +5))",
            LR"(설계도 비용: 1000
건설 비용: 1800
등장 시기: 세계대전 시대
수용 가구: 9
주거 품질: 38
요구 재산: 가난
크기: 3x1
효과: 고밀도 저가 주거)",
            LR"(설계도 비용: 1500
건설 비용: 3000
등장 시기: 세계대전 시대
수용 가구: 10
주거 품질: 52
요구 재산: 유복
크기: 3x2
업그레이드:
- 현대화 (현대, 현대식 아파트로 업그레이드))",
            LR"(설계도 비용: 1950
건설 비용: 3900
등장 시기: 냉전 시대
수용 가구: 20
주거 품질: 36(시간 경과로 악화)
요구 재산: 가난
크기: 3x3)",
            LR"(설계도 비용: 1750
건설 비용: 3500
등장 시기: 냉전 시대
수용 가구: 16
주거 품질: 40
요구 재산: 가난
크기: 3x2
효과: 적은 공해, 주변 치안 감소)",
            LR"(설계도 비용: 4325
건설 비용: 8650
등장 시기: 현대 시대
필요 전력: 50MW
수용 가구: 16
주거 품질: 80
요구 재산: 유복
크기: 3x2)",
            LR"(설계도 비용: 1025
건설 비용: 2050
등장 시기: 현대 시대
필요 전력: 18MW
수용 가구: 3
주거 품질: 90
요구 재산: 부유
크기: 3x2)",
            LR"(설계도 비용: 1025
건설 비용: 2050
등장 시기: 현대 시대
필요 전력: 18MW
수용 가구: 2
주거 품질: 90
요구 재산: 부유
크기: 3x2)"
        };

        static const wchar_t* EntertainmentNames[] =
        {
            L"주점",
            L"서커스",
            L"버스커",
            L"식물원",
            L"유원지 부두",
            L"레스토랑",
            L"오락실",
            L"패스트푸드 체인점",
            L"영화관",
            L"아쿠아 파크",
            L"롤러코스터",
            L"경기장",
            L"극장",
            L"오페라 극장",
            L"카바레",
            L"카지노",
            L"칵테일 바",
            L"골프 코스",
            L"고급 레스토랑",
            L"나이트클럽",
            L"스노클 베이",
            L"해변 휴양지",
            L"행글라이딩",
            L"현대 미술관",
            L"요트 클럽"
        };
        static const wchar_t* EntertainmentDetails[] =
        {
            LR"(설계도 비용: 없음
건설 비용: 1200
등장 시기: 식민지 시대
필요 인력: 3 (무학력자)
직업 품질: 40
서비스 품질: 32
수용 인원: 12
재산 요구치: 가난
선호 관광객: 문화
효과: 시민/관광객 유흥 제공, 주변 치안 감소
운영 모드:
- 물처럼 흐르는 슬픔
- 음료 무한 제공 (직업 품질 +20, 효율 +25%))",
            LR"(설계도 비용: 2400
건설 비용: 4800
등장 시기: 식민지 시대
필요 인력: 6 (무학력자)
직업 품질: 45
서비스 품질: 45
수용 인원: 36
재산 요구치: 가난
선호 관광객: 아동
운영 모드:
- 광대와 팝콘
- 야수와 피 (연구 필요)
- 마법사와 마법 (연구 필요)
업그레이드:
- 화재 방지 시스템 (냉전))",
            LR"(설계도 비용: 800
건설 비용: 400
등장 시기: 식민지 시대
필요 인력: 1 (무학력자)
직업 품질: 45
서비스 품질: 20
수용 인원: 6
재산 요구치: 파산
크기: 1x1
비고: 도로)",
            LR"(설계도 비용: 1050
건설 비용: 4200
등장 시기: 세계대전 시대
필요 인력: 2 (무학력자)
직업 품질: 45
서비스 품질: 30
수용 인원: 16
재산 요구치: 파산
선호 관광객: 배낭여행
효과: 주변 공해 감소)",
            LR"(설계도 비용: 3675
건설 비용: 7350
등장 시기: 세계대전 시대
필요 인력: 6 (무학력자)
필요 전력: 20MW
직업 품질: 45
서비스 품질: 60
수용 인원: 24
재산 요구치: 가난
선호 관광객: 아동)",
            LR"(설계도 비용: 1300
건설 비용: 2600
등장 시기: 세계대전 시대
필요 인력: 4 (무학력자)
직업 품질: 45
서비스 품질: 65
수용 인원: 12
재산 요구치: 유복
효과: 유흥 제공 + 방문 시민 음식 만족도 증가)",
            LR"(설계도 비용: 600
건설 비용: 2400
등장 시기: 냉전 시대
필요 인력: 3 (무학력자)
필요 전력: 10MW
직업 품질: 50
서비스 품질: 50
수용 인원: 12
재산 요구치: 유복
선호 관광객: 아동)",
            LR"(설계도 비용: 575
건설 비용: 2300
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
직업 품질: 40
서비스 품질: 45
수용 인원: 20
재산 요구치: 가난
선호 관광객: 휴양, 아동
효과: 유흥 제공 + 방문 시민 음식 만족도 증가)",
            LR"(설계도 비용: 2360
건설 비용: 9440
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
필요 전력: 10MW
직업 품질: 60
서비스 품질: 70
수용 인원: 24
재산 요구치: 가난
선호 관광객: 휴양)",
            LR"(설계도 비용: 4500
건설 비용: 9000
등장 시기: 현대 시대
필요 인력: 3 (무학력자)
필요 전력: 15MW
직업 품질: 50
서비스 품질: 70
수용 인원: 18
재산 요구치: 유복
선호 관광객: 휴양, 아동)",
            LR"(설계도 비용: 4700
건설 비용: 9400
등장 시기: 현대 시대
필요 인력: 3 (무학력자)
필요 전력: 35MW
직업 품질: 50
서비스 품질: 70
수용 인원: 18
재산 요구치: 가난
선호 관광객: 스릴중독, 아동)",
            LR"(설계도 비용: 10500
건설 비용: 21000
등장 시기: 현대 시대
필요 인력: 12 (무학력자)
필요 전력: 60MW
직업 품질: 50
서비스 품질: 80
수용 인원: 60
재산 요구치: 유복)",
            LR"(설계도 비용: 1325
건설 비용: 5300
등장 시기: 식민지 시대
필요 인력: 미기재
직업 품질: 50
서비스 품질: 60
수용 인원: 24
재산 요구치: 유복
선호 관광객: 문화)",
            LR"(설계도 비용: 2470
건설 비용: 9400
등장 시기: 식민지 시대
필요 인력: (고졸) - 인원 미기재
직업 품질: 60
서비스 품질: 75
수용 인원: 16
재산 요구치: 부유
선호 관광객: 문화)",
            LR"(설계도 비용: 1250
건설 비용: 5000
등장 시기: 세계대전 시대
필요 인력: 미기재
직업 품질: 50
서비스 품질: 65
수용 인원: 16
재산 요구치: 유복
선호 관광객: 문화
운영 모드:
- 무용 법
- 풍자극 (연구 필요)
- 정치적 행동 (연구 필요)
업그레이드:
- 재생 장비 (냉전, 5000, 효율 +10%, 전력 +25MW))",
            LR"(설계도 비용: 2300
건설 비용: 9200
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 60
서비스 품질: 70
수용 인원: 24
재산 요구치: 유복
선호 관광객: 스릴중독)",
            LR"(설계도 비용: 850
건설 비용: 3400
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 60
서비스 품질: 70
수용 인원: 12
재산 요구치: 유복
선호 관광객: 휴양
운영 모드:
- 트로피코의 자유
- 상류층 (연구 필요)
- 해피 아워 (연구 필요)
업그레이드:
- 단방향 거울 (냉전, 2500, 첩보 학교 효율 보너스))",
            LR"(설계도 비용: 3000
건설 비용: 12000
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 45
서비스 품질: 60
수용 인원: 20
재산 요구치: 유복
선호 관광객: 휴양)",
            LR"(설계도 비용: 4000
건설 비용: 16000
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 65
서비스 품질: 80
수용 인원: 12
재산 요구치: 부유
선호 관광객: 문화
효과: 유흥 제공 + 방문 시민 음식 만족도 증가)",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 50
서비스 품질: 65
수용 인원: 20
재산 요구치: 유복
선호 관광객: 스릴중독
효과: 주변 치안 감소)",
            LR"(설계도 비용: 2450
건설 비용: 9800
등장 시기: 냉전 시대
필요 인력: 미기재
직업 품질: 60
서비스 품질: 65
수용 인원: 12
재산 요구치: 유복
선호 관광객: 배낭여행, 스릴중독
비고: 어장
운영 모드:
- 암초 보존
- 사냥을 허가한다 (연구 필요)
- 물고기 유인 (연구 필요))",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 현대 시대
필요 인력: 미기재
직업 품질: 55
서비스 품질: 65
수용 인원: 20
재산 요구치: 유복
선호 관광객: 휴양
운영 모드:
- 문 열기
- 칵테일 타임 (연구 필요)
업그레이드:
- 자동 해변 청소기 (1500, 서비스 품질 +10))",
            LR"(설계도 비용: 2000
건설 비용: 4000
등장 시기: 현대 시대
필요 인력: 미기재
직업 품질: 50
서비스 품질: 75
수용 인원: 12
재산 요구치: 유복
선호 관광객: 스릴중독
운영 모드:
- 바람처럼 비행
- 추가 관광 (연구 필요)
- 추가 연료
업그레이드:
- 고출력 엔진 (3000, 요금 +1, 서비스 품질 +5))",
            LR"(설계도 비용: 8750
건설 비용: 17500
등장 시기: 현대 시대
필요 인력: 미기재
직업 품질: 75
서비스 품질: 70
수용 인원: 20
재산 요구치: 유복
선호 관광객: 문화)",
            LR"(설계도 비용: 5900
건설 비용: 미기재(원문 누락)
등장 시기: 현대 시대
필요 인력: 미기재(원문 표기 오류: 11800)
직업 품질: 75
서비스 품질: 95
수용 인원: 8
재산 요구치: 부유
운영 모드:
- 안락한 일몰
- 어마어마한 중요도 (연구 필요)
- 만지지 말고 눈으로만 (연구 필요)
업그레이드:
- 개인 선창 (5000, 더럽게 부유한 관광객 출국 지원))"
        };

        static const wchar_t* MediaEducationNames[] =
        {
            L"신문사",
            L"도서관",
            L"고등학교",
            L"대학교",
            L"라디오 방송국",
            L"대통령 박물관",
            L"영묘",
            L"TV 방송국",
            L"격려용 광고판",
            L"격려용 동상",
            L"연구소",
            L"우주 개발 프로그램",
            L"핵 개발 프로그램"
        };
        static const wchar_t* MediaEducationDetails[] =
        {
            LR"(설계도 비용: 없음
건설 비용: 1800
등장 시기: 식민지 시대
필요 인력: 4 (고졸)
효과: 주변 자유 증가, 주변 거주 시민 대상 선전물 출판
운영 모드:
- 개방적인 사고
- 독립 신문 (식민지 시대만 가능)
- 군식구
- 광고 전단
- 붉은 별 (세계대전, 연구 필요)
- 거래와 수익 (세계대전, 연구 필요)
- 말 (세계대전, 연구 필요)
- 총잡이 (세계대전, 연구 필요)
- 대지와 바람 (냉전, 연구 필요)
- 차세대 (냉전, 연구 필요)
- 보호자 (현대, 연구 필요)
- 생각합시다! (현대, 연구 필요))",
            LR"(설계도 비용: 없음
건설 비용: 3600
등장 시기: 식민지 시대
필요 인력: 3 (고졸)
효과: 통치 수단에 쓸 지식 생성
운영 모드:
- 입증된 도서 목록
- 공익 서비스 지원 (효율 -25%, 학교 계열 효율 보너스)
업그레이드:
- 학과 분류 (세계대전, 2000, 효율 +25%)
- 전자 책장 (현대, 일자리 -1, 지식 생산 +50%))",
            LR"(설계도 비용: 3500
건설 비용: 7000
등장 시기: 세계대전 시대
필요 인력: 4 (고졸)
효과: 무학력 시민에게 고등학교 교육 제공
운영 모드:
- 일반 교육
- 군사 교육 (연구 필요)
- 종교 교육 (연구 필요)
업그레이드:
- 상호 소통 교육 (현대 시대))",
            LR"(설계도 비용: 6000
건설 비용: 12000
등장 시기: 세계대전 시대
필요 인력: 3 (대졸))",
            LR"(설계도 비용: 1200
건설 비용: 2400
등장 시기: 세계대전 시대
필요 인력: 2 (고졸))",
            LR"(설계도 비용: 3750
건설 비용: 7500
등장 시기: 미기재
필요 인력: 4 (고졸)
비고: 고유)",
            LR"(설계도 비용: 14000
건설 비용: 28000
등장 시기: 미기재
필요 인력: 4 (고졸)
비고: 고유)",
            LR"(설계도 비용: 4100
건설 비용: 8200
등장 시기: 냉전 시대
필요 인력: 4 (고졸))",
            LR"(설계도 비용: 1000
건설 비용: 2000
등장 시기: 냉전 시대
필요 인력: 없음
비고: 도로
운영 모드:
- 냉전 시대 기본 운영
- 현대 시대 (연구 필요))",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 냉전 시대
필요 인력: 없음)",
            LR"(설계도 비용: 8000
건설 비용: 16000
등장 시기: 냉전 시대
필요 인력: 5 (대졸))",
            LR"(설계도 비용: 21000
건설 비용: 42000
등장 시기: 냉전 시대
필요 인력: 10 (대졸))",
            LR"(설계도 비용: 17500
건설 비용: 35000
등장 시기: 냉전 시대
필요 인력: 6 (대졸))"
        };

        static const wchar_t* TourismNames[] =
        {
            L"여객선 터미널",
            L"통나무집",
            L"바닷가 별장",
            L"카바나 마을",
            L"모텔",
            L"호텔",
            L"전망대",
            L"기념품 매장",
            L"여행사",
            L"고대 유적",
            L"원주민 마을",
            L"공항",
            L"저가 호텔",
            L"가족 휴양지",
            L"고급 호텔",
            L"초고층 호텔",
            L"유람선"
        };
        static const wchar_t* TourismDetails[] =
        {
            LR"(설계도 비용: 4500
건설 비용: 9000
등장 시기: 냉전 시대
필요 인력: 6 (무학력자)
직업 품질: 55
관광객 재산: 유복, 부유
크기: 10x4
비고: 해안
운영 모드:
- 저가 여객선
- 호화 여객선 (연구 필요)
업그레이드:
- 화환 공급기 (냉전, 350, 도착 관광객 종합 만족도 +5))",
            LR"(설계도 비용: 1000
건설 비용: 700
등장 시기: 냉전 시대
필요 인력: 미기재
서비스 품질: 40
수용 가구: 1
필요 재산: 유복 이상
크기: 1x1
선호 관광객: 미기재)",
            LR"(설계도 비용: 1000
건설 비용: 1100
등장 시기: 냉전 시대
필요 인력: 1 (고졸)
직업 품질: 50
서비스 품질: 57
수용 가구: 2
필요 재산: 부유, 더럽게 부유
크기: 2x1
선호 관광객: 미기재)",
            LR"(설계도 비용: 1000
건설 비용: 1900
등장 시기: 냉전 시대
필요 인력: 2 (무학력자)
직업 품질: 45
서비스 품질: 46
수용 가구: 6
필요 재산: 유복 이상
크기: 5x2
선호 관광객: 미기재)",
            LR"(설계도 비용: 1075
건설 비용: 2150
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
직업 품질: 40
서비스 품질: 32
수용 가구: 8
필요 재산: 유복 이상
크기: 3x2
선호 관광객: 미기재)",
            LR"(설계도 비용: 2200
건설 비용: 4400
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
직업 품질: 45
서비스 품질: 50
수용 가구: 12
필요 재산: 유복 이상
크기: 3x2
선호 관광객: 휴양
운영 모드:
- 호텔 트로피카나
- 호텔 카마로테 (숙박 슬롯 +4, 주거 품질 -10)
- 지역 사업체 공급 (방 슬롯 -2, 주거 품질 -5, 사무소 효율 보너스)
업그레이드:
- 냉난방 (냉전, 700, 효율 +20%, 전력 20MW 필요)
- 엘리베이터 (냉전, 850, 효율 +25%, 전력 30MW 필요))",
            LR"(설계도 비용: 1000
건설 비용: 500
등장 시기: 냉전 시대
필요 인력: 미기재
서비스 품질: 45
수용 가구: 4
필요 재산: 유복 이상
크기: 2x1
선호 관광객: 미기재)",
            LR"(설계도 비용: 1000
건설 비용: 1320
등장 시기: 냉전 시대
필요 인력: 2 (무학력자)
직업 품질: 45
서비스 품질: 55
수용 가구: 12
필요 재산: 유복 이상
크기: 2x1
선호 관광객: 미기재)",
            LR"(설계도 비용: 1200
건설 비용: 2400
등장 시기: 냉전 시대
필요 인력: 3 (무학력자)
직업 품질: 45
서비스 품질: 60
수용 가구: 24
필요 재산: 유복 이상
크기: 2x1
선호 관광객: 미기재)",
            LR"(설계도 비용: 1250
건설 비용: 2500
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
직업 품질: 50
서비스 품질: 75
수용 가구: 24
필요 재산: 유복 이상
크기: 5x5
선호 관광객: 문화, 배낭여행
비고: 유적지
운영 모드:
- 거의 온전히 보존
- 테마 공원 개발
- 고고학 발굴지
- 비공식 고물 처리장 (연구 필요))",
            LR"(설계도 비용: 3300
건설 비용: 6600
등장 시기: 냉전 시대
필요 인력: 4 (무학력자)
직업 품질: 50
서비스 품질: 60
수용 가구: 16
필요 재산: 유복 이상
크기: 3x2
선호 관광객: 문화)",
            LR"(설계도 비용: 9000
건설 비용: 18000
등장 시기: 냉전 시대
필요 인력: 6 (고졸)
필요 전력: 15MW
직업 품질: 65
서비스 품질: 미기재
수용 가구: 미기재
필요 재산: 부유, 더럽게 부유
크기: 16x5
운영 모드:
- 저가 항공기 (현대)
- 염가 항공기 (현대, 연구 필요)
- 호화 항공기 (현대, 연구 필요)
업그레이드:
- 면세점 (현대, 도착 성인 관광객당 +$25)
- 기내식 (현대, 도착 관광객 종합 만족도 +10)
- 지하철역 (현대, 추가 전력 10MW))",
            LR"(설계도 비용: 3125
건설 비용: 6250
등장 시기: 현대 시대
필요 인력: 6 (무학력자)
필요 전력: 15MW
직업 품질: 35
서비스 품질: 50
수용 가구: 18
필요 재산: 유복 이상
크기: 4x4
선호 관광객: 미기재)",
            LR"(설계도 비용: 1675
건설 비용: 3350
등장 시기: 현대 시대
필요 인력: 4 (무학력자)
필요 전력: 20MW
직업 품질: 40
서비스 품질: 70
수용 가구: 8
필요 재산: 유복 이상
크기: 4x3
선호 관광객: 미기재)",
            LR"(설계도 비용: 6000
건설 비용: 12000
등장 시기: 현대 시대
필요 인력: 4 (무학력자)
필요 전력: 40MW
직업 품질: 50
서비스 품질: 80
수용 가구: 6
필요 재산: 더럽게 부유
크기: 4x3
선호 관광객: 미기재)",
            LR"(설계도 비용: 3525
건설 비용: 7050
등장 시기: 현대 시대
필요 인력: 6 (무학력자)
필요 전력: 20MW
직업 품질: 45
서비스 품질: 70
수용 가구: 16
필요 재산: 부유, 더럽게 부유
크기: 3x2
운영 모드:
- 모든 층의 경치를 끝내주게
- 지역 사업체 공급 (현대, 연구 필요)
업그레이드:
- 행동 층 (현대)
- 복지 층 (현대))",
            LR"(설계도 비용: 2500
건설 비용: 5000
등장 시기: 현대 시대
필요 인력: 4 (무학력자)
직업 품질: 55
서비스 품질: 70
수용 가구: 12
필요 재산: 부유, 더럽게 부유
크기: 7x2
비고: 해양)"
        };

        static const wchar_t* PublicServiceNames[] =
        {
            L"예배당",
            L"식료품점",
            L"교회",
            L"진료소",
            L"소방서",
            L"성당",
            L"병원",
            L"정신병원",
            L"쓰레기장",
            L"쇼핑몰",
            L"폐기물 처리장",
            L"중독 치료소",
            L"미용 농원"
        };
        static const wchar_t* PublicServiceDetails[] =
        {
            LR"(설계도 비용: 없음
건설 비용: 미기재
등장 시기: 식민지 시대
필요 인력: 미기재
효과: 방문 시민에게 신앙 제공(재산 요건 없음)
운영 모드:
- 전통적 설교
- 먼저 돕고 나중에 설교하라)",
            LR"(설계도 비용: 없음
건설 비용: 미기재
등장 시기: 식민지 시대
필요 인력: 미기재
요구 재산: 가난)",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 세계대전 시대
필요 인력: 4명 (고졸)
효과: 방문 시민에게 신앙 제공(재산 요건 없음), 고유
운영 모드:
- 여유로운 공간
- 좌석 공유 (연구 필요, 노동자당 방문객 슬롯 +2, 서비스 품질 -15)
업그레이드:
- 폼푸스 벨 (세계대전, 1500))",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 세계대전 시대
필요 인력: 3 (대졸)
효과: 방문 시민에게 보건 제공(가난 이상)
운영 모드:
- 국가 공인 의사
- 돌팔이 (연구 필요)
업그레이드:
- 최신식 진단 (냉전, 노동자당 방문객 슬롯 +2)
- 마취 (세계대전, 의료 품질 +10))",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 세계대전 시대
필요 인력: 4명 (고졸)
효과: 범위 내 화재 진압 및 화재 시 생존율 증가
운영 모드:
- 불끄기
- 나무 위의 고양이 (냉전, 연구 필요)
- 건물 조사 (냉전, 연구 필요)
- 대피 훈련
업그레이드:
- 소방 헬리콥터 (냉전, 15000)
- 고압 물 분사기 노즐 (냉전, 750))",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 냉전 시대
필요 인력: 4명 (대졸)
효과: 방문 시민에게 신앙 제공(재산 요건 없음)
운영 모드:
- 철석같은 믿음
- 프레지덴테의 고해 (연구 필요)
- 오래된 성서 (연구 필요)
업그레이드:
- 집요한 고백 (냉전, 치안 +15))",
            LR"(설계도 비용: 미기재
건설 비용: 미기재
등장 시기: 냉전 시대
필요 인력: 4명 (대졸)
필요 전력: 35MW
효과: 방문 시민에게 보건 제공(유복 이상)
운영 모드:
- 병원 진료
- 외래 진료 (연구 필요)
- 치료법 연구 (연구 필요)
업그레이드:
- 입원시설 (냉전, 6000, 노동자당 방문객 슬롯 +2))",
            LR"(설계도 비용: 1875
건설 비용: 7500
등장 시기: 냉전 시대
필요 인력: 6명 (대졸)
효과: 보호 시설 수용자에게 특수 요법, 수용자는 정치적 관점 상실
운영 모드:
- 반복 속의 휴식
- 파란 약의 길
- 빨간 약의 길
업그레이드:
- 저항실 (냉전, 2500, 효율 +20%, 전력 5MW 필요))",
            LR"(설계도 비용: 250
건설 비용: 1000
등장 시기: 냉전 시대
필요 인력: 3명 (무학력)
효과: 건물 자체는 공해 배출, 범위 내 다른 건물 공해 감소
운영 모드:
- 모든 쓰레기는 평등하다
- 산업폐기물 처리장
- 민간 쓰레기 수거
업그레이드:
- 전신 방호복 (냉전, 1000, 직업 품질 +10))",
            LR"(설계도 비용: 3625
건설 비용: 7250
등장 시기: 현대 시대
필요 인력: 8명 (무학력)
요구 재산: 유복
효과: 식료품 판매, 소비재 판매로 가구 만족도 증가
운영 모드:
- 셀프 서비스
- 포장 서비스
업그레이드:
- 호화 아울렛 (현대, 20000)
- 냉난방 (현대, 15000, 직업 품질 +20, 추가 전력 40MW))",
            LR"(설계도 비용: 4000
건설 비용: 8000
등장 시기: 현대 시대
필요 인력: 4명 (고졸)
효과: 범위 내 다른 건물 공해 감소
운영 모드:
- 깔끔한 소각
- 쓰레기 소각
업그레이드:
- 쓰레기더미 뒤지기 (현대, 6000))",
            LR"(설계도 비용: 4250
건설 비용: 8500
등장 시기: 현대 시대
필요 인력: 5명 (대졸)
효과: 시민 보건 + 관광객 유흥 제공
선호 관광객: 유명인
재산 요구치: 부유 이상
운영 모드:
- 적절한 해독 요법
- 급속 해독 요법
- 후원 해독 요법
- 위장 해독 요법)",
            LR"(설계도 비용: 4500
건설 비용: 9000
등장 시기: 현대 시대
필요 인력: 5명 (고졸)
효과: 시민 신앙 + 관광객 유흥 제공
선호 관광객: 유명인
재산 요구치: 부유 이상
운영 모드:
- 건강&스파
- 건강&메스
- 샴페인&메스
업그레이드:
- 냉동 지방 분해 장치 (현대, 3000, 서비스 품질 +10, 추가 전력 5MW))"
        };

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

        auto AppendCategory = [&](
            EBuildingCategory Category,
            const wchar_t* const* Names,
            int NameCount,
            bool Residential,
            bool FoodProvider,
            bool EntertainmentProvider,
            const wchar_t* const* Details = nullptr)
        {
            const int CategoryIndex = static_cast<int>(Category);

            for (int i = 0; i < NameCount; ++i)
            {
                FBuildingCatalogEntry Entry;
                Entry.Id = "build_" + std::to_string(CategoryIndex + 1) +
                    "_" + std::to_string(i + 1);
                Entry.DisplayName = Names[i];
                Entry.CategoryName = CategoryLabels[CategoryIndex];
                Entry.DetailText =
                    (Details && Details[i]) ?
                    Details[i] :
                    L"세부 데이터 준비 중";
                Entry.Residential = Residential;
                Entry.FoodProvider = FoodProvider;
                Entry.EntertainmentProvider = EntertainmentProvider;
                Entry.Category = Category;
                Entry.CategoryLocalIndex = i;
                Entry.TemplateType =
                    ResolveTemplateTypeByBuildingId(Entry.Id);
                if (Entry.DisplayName == L"항구")
                    Entry.BuildingKind = EPlacementBuildingKind::Harbor;
                else if (Entry.DisplayName == L"운송업자 사무소")
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

                if (Category == EBuildingCategory::Infrastructure)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        85, 45 + (i % 7) * 5 + (i / 7) * 3);
                }
                else if (Category == EBuildingCategory::FoodResource)
                {
                    Entry.FoodSatisfactionCap = (std::min)(
                        82, 35 + (i % 7) * 6 + (i / 7) * 4);
                    Entry.JobSatisfactionCap = (std::min)(
                        70, 40 + (i % 5) * 5 + (i / 5) * 2);
                }
                else if (Category == EBuildingCategory::Industry)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        90, 50 + (i % 8) * 5 + (i / 8) * 4);
                }
                else if (Category == EBuildingCategory::Housing)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(HousingCaps) / sizeof(HousingCaps[0])))
                    {
                        Entry.HousingSatisfactionCap = HousingCaps[i];
                    }
                }
                else if (Category == EBuildingCategory::Entertainment)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(EntertainmentFunCaps) /
                        sizeof(EntertainmentFunCaps[0])))
                    {
                        Entry.FunSatisfactionCap =
                            EntertainmentFunCaps[i];
                    }
                }
                else if (Category == EBuildingCategory::PublicService)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        88, 48 + (i % 6) * 5 + (i / 6) * 4);
                }

                if (Category == EBuildingCategory::Entertainment &&
                    (Entry.DisplayName == L"레스토랑" ||
                        Entry.DisplayName == L"패스트푸드 체인점" ||
                        Entry.DisplayName == L"고급 레스토랑"))
                {
                    Entry.FoodProvider = true;

                    if (Entry.DisplayName == L"레스토랑")
                        Entry.FoodSatisfactionCap = 65;
                    else if (Entry.DisplayName == L"고급 레스토랑")
                        Entry.FoodSatisfactionCap = 75;
                    else
                        Entry.FoodSatisfactionCap = 55;
                }
                else if (Category == EBuildingCategory::PublicService &&
                    (Entry.DisplayName == L"식료품점" ||
                        Entry.DisplayName == L"쇼핑몰"))
                {
                    Entry.FoodProvider = true;
                    Entry.FoodSatisfactionCap =
                        (Entry.DisplayName == L"쇼핑몰") ? 70 : 55;
                }

                if (Category == EBuildingCategory::PublicService &&
                    (Entry.DisplayName == L"중독 치료소" ||
                        Entry.DisplayName == L"미용 농원"))
                {
                    Entry.EntertainmentProvider = true;
                    Entry.FunSatisfactionCap = 68;
                }

                AssignPoliticalSignals(Entry);

                Entries.push_back(Entry);
            }
        };

        AppendCategory(
            EBuildingCategory::Infrastructure, InfrastructureNames,
            static_cast<int>(sizeof(InfrastructureNames) /
                sizeof(InfrastructureNames[0])),
            false,
            false,
            false,
            InfrastructureDetails);
        AppendCategory(
            EBuildingCategory::FoodResource, FoodResourceNames,
            static_cast<int>(sizeof(FoodResourceNames) /
                sizeof(FoodResourceNames[0])),
            false,
            true,
            false,
            FoodResourceDetails);
        AppendCategory(
            EBuildingCategory::Industry, IndustryNames,
            static_cast<int>(sizeof(IndustryNames) /
                sizeof(IndustryNames[0])),
            false,
            false,
            false,
            IndustryDetails);
        AppendCategory(
            EBuildingCategory::Housing, HousingNames,
            static_cast<int>(sizeof(HousingNames) /
                sizeof(HousingNames[0])),
            true,
            false,
            false,
            HousingDetails);
        AppendCategory(
            EBuildingCategory::Entertainment, EntertainmentNames,
            static_cast<int>(sizeof(EntertainmentNames) /
                sizeof(EntertainmentNames[0])),
            false,
            false,
            true,
            EntertainmentDetails);
        AppendCategory(
            EBuildingCategory::MediaEducation, MediaEducationNames,
            static_cast<int>(sizeof(MediaEducationNames) /
                sizeof(MediaEducationNames[0])),
            false,
            false,
            false,
            MediaEducationDetails);
        AppendCategory(
            EBuildingCategory::Tourism, TourismNames,
            static_cast<int>(sizeof(TourismNames) /
                sizeof(TourismNames[0])),
            false,
            false,
            false,
            TourismDetails);
        AppendCategory(
            EBuildingCategory::PublicService, PublicServiceNames,
            static_cast<int>(sizeof(PublicServiceNames) /
                sizeof(PublicServiceNames[0])),
            false,
            false,
            false,
            PublicServiceDetails);

        // ── UI 동작 플래그 후처리 ─────────────────────────────────────────
        // DisplayName 기반 동작 분기를 외부 레이어(BuildMenuWidget 등)가
        // 직접 비교하지 않도록 카탈로그 구성 시점에 플래그로 굳힌다.
        for (auto& Entry : Entries)
        {
            if (Entry.DisplayName == L"철거")
                Entry.IsDemolish = true;

            if (Entry.DisplayName == L"판잣집")
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
