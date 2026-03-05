#include "BuildMenuWidget.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Player/MainCamera.h"
#include "../World/MainWorld.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>
#include <string>

namespace
{
    constexpr int CategoryCount = 5;
    constexpr int SlotsPerPage = 12;
    constexpr int SlotColumnCount = 4;
    constexpr int SlotRowCount = 3;
    constexpr const TCHAR* GBuildMenuPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GYearbookPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GEmptySlotTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_standard.png");

    const wchar_t* CategoryLabels[CategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"유흥"
    };

    const TCHAR* const GInfrastructureIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_constructionOffice.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_teamsters.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_dock.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_storageQuay.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_DLC_warehouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_electricSubstation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_powerPlant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_parkDeck.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_busGarage.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_busStop.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_telefericStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_NuclearPowerPlant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_metroStation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_airport.png"),
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
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_mine.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_oilWell.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_fishFarm.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_oilRig.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_hydrophobicPlantation.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_factoryRanch.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_automatedMine.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_juicer.png")
    };

    const TCHAR* const GIndustryIcons[] =
    {
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
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_DLC_smartFurnitureStudio.png")
    };

    const TCHAR* const GHousingIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_bunkhouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_countryHouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_house.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_mansion.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_apartment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_tenment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\ICO_Flophouse.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_Conventillo.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_cabin.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_beachVilla.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_modernApartment.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_modernMansion.png")
    };

    const TCHAR* const GEntertainmentIcons[] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_tavern.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColonial\\T_ICO_Colonial_circus.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_cabaret.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsWorldWars\\T_ICO_WorldWar_botanicalGarden.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_funFairPier.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_gourmetRestaurant.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_arcade.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_fastFoodJoint.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsColdWar\\T_ICO_ColdWar_movieTheatre.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_aquaPark.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_rollerCoaster.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\BuildingIcons\\BuildingsModernTimes\\T_ICO_ModernTimes_stadium.png")
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

    const TCHAR* GetCatalogEntryIconPath(int CategoryIndex, int CategoryLocalIndex)
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
        default:
            break;
        }

        return nullptr;
    }

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

    void ConfigureDefaultButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.20f, 0.22f, 0.26f, 0.92f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.26f, 0.30f, 0.35f, 0.95f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.14f, 0.16f, 0.20f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.10f, 0.12f, 0.70f));
    }

    void ConfigureHighlightedButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.10f, 0.32f, 0.52f, 0.95f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.16f, 0.40f, 0.62f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.08f, 0.24f, 0.40f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.08f, 0.24f, 0.40f, 0.70f));
    }

    void ConfigureIconSlotButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.f, 1.f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.80f, 0.80f, 0.80f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.35f, 0.35f, 0.35f, 0.75f));
    }

    void ApplyTextureToAllButtonStates(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKey,
        const TCHAR* TextureFile)
    {
        if (!Button || !TextureFile)
            return;

        if (!Button->SetTexture(EButtonState::Normal, TextureKey, TextureFile))
            return;

        Button->SetTexture(EButtonState::Hovered, TextureKey);
        Button->SetTexture(EButtonState::Click, TextureKey);
        Button->SetTexture(EButtonState::Disable, TextureKey);
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
}

CBuildMenuWidget::CBuildMenuWidget()
{
}

CBuildMenuWidget::~CBuildMenuWidget()
{
}

bool CBuildMenuWidget::Init()
{
    CWidgetContainer::Init();

    mVisibleEntryIndices.assign(SlotsPerPage, -1);

    auto BuildButton = CreateWidget<CButton>("BuildMenu_BuildButton", 10).lock();

    if (BuildButton)
    {
        ConfigureHighlightedButtonStyle(BuildButton);
        BuildButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnBuildButtonClick);

        auto BuildButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_BuildButtonText", mWorld);

        if (BuildButtonText)
        {
            BuildButtonText->SetText(TEXT("건축"));
            BuildButtonText->SetFontSize(24.f);
            BuildButtonText->SetAlignH(ETextAlignH::Center);
            BuildButtonText->SetAlignV(ETextAlignV::Middle);
            BuildButtonText->SetTextColor(255, 255, 255, 255);
            BuildButtonText->EnableShadow(true);
            BuildButtonText->SetShadowOffset(1.f, 1.f);
            BuildButtonText->SetShadowTextColor(40, 40, 40, 255);
            BuildButton->SetChild(BuildButtonText);
        }

        mBuildButton = BuildButton;
    }

    auto YearbookButton =
        CreateWidget<CButton>("BuildMenu_YearbookButton", 10).lock();

    if (YearbookButton)
    {
        ConfigureDefaultButtonStyle(YearbookButton);
        YearbookButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnYearbookButtonClick);

        auto YearbookButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_YearbookButtonText", mWorld);

        if (YearbookButtonText)
        {
            YearbookButtonText->SetText(TEXT("연감"));
            YearbookButtonText->SetFontSize(24.f);
            YearbookButtonText->SetAlignH(ETextAlignH::Center);
            YearbookButtonText->SetAlignV(ETextAlignV::Middle);
            YearbookButtonText->SetTextColor(255, 255, 255, 255);
            YearbookButtonText->EnableShadow(true);
            YearbookButtonText->SetShadowOffset(1.f, 1.f);
            YearbookButtonText->SetShadowTextColor(40, 40, 40, 255);
            YearbookButton->SetChild(YearbookButtonText);
            mYearbookButtonText = YearbookButtonText;
        }

        mYearbookButton = YearbookButton;
    }

    auto DemolishButton =
        CreateWidget<CButton>("BuildMenu_DemolishButton", 10).lock();

    if (DemolishButton)
    {
        ConfigureDefaultButtonStyle(DemolishButton);
        DemolishButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnDemolishButtonClick);

        auto DemolishButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_DemolishButtonText", mWorld);

        if (DemolishButtonText)
        {
            DemolishButtonText->SetText(TEXT("철거 OFF"));
            DemolishButtonText->SetFontSize(24.f);
            DemolishButtonText->SetAlignH(ETextAlignH::Center);
            DemolishButtonText->SetAlignV(ETextAlignV::Middle);
            DemolishButtonText->SetTextColor(255, 255, 255, 255);
            DemolishButtonText->EnableShadow(true);
            DemolishButtonText->SetShadowOffset(1.f, 1.f);
            DemolishButtonText->SetShadowTextColor(40, 40, 40, 255);
            DemolishButton->SetChild(DemolishButtonText);
            mDemolishButtonText = DemolishButtonText;
        }

        mDemolishButton = DemolishButton;
    }

    auto NpcCountText = CreateWidget<CTextBlock>("BuildMenu_NpcCount", 11).lock();

    if (NpcCountText)
    {
        NpcCountText->SetText(TEXT("NPC: 0"));
        NpcCountText->SetFontSize(20.f);
        NpcCountText->SetAlignH(ETextAlignH::Left);
        NpcCountText->SetAlignV(ETextAlignV::Middle);
        NpcCountText->SetTextColor(245, 245, 245, 255);
        NpcCountText->EnableShadow(true);
        NpcCountText->SetShadowOffset(1.f, 1.f);
        NpcCountText->SetShadowTextColor(20, 20, 20, 255);
        mNpcCountText = NpcCountText;
    }

    auto BudgetText = CreateWidget<CTextBlock>(
        "BuildMenu_NationalBudget", 11).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("국가 예산: $0"));
        BudgetText->SetFontSize(20.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(245, 245, 210, 255);
        BudgetText->EnableShadow(true);
        BudgetText->SetShadowOffset(1.f, 1.f);
        BudgetText->SetShadowTextColor(20, 20, 20, 255);
        mBudgetText = BudgetText;
    }

    auto DateText = CreateWidget<CTextBlock>(
        "BuildMenu_SimulationDate", 11).lock();

    if (DateText)
    {
        DateText->SetText(TEXT("날짜: 2000-01-01"));
        DateText->SetFontSize(20.f);
        DateText->SetAlignH(ETextAlignH::Left);
        DateText->SetAlignV(ETextAlignV::Middle);
        DateText->SetTextColor(220, 235, 255, 255);
        DateText->EnableShadow(true);
        DateText->SetShadowOffset(1.f, 1.f);
        DateText->SetShadowTextColor(20, 20, 20, 255);
        mDateText = DateText;
    }

    auto DayProgressText = CreateWidget<CTextBlock>(
        "BuildMenu_DayProgressText", 11).lock();

    if (DayProgressText)
    {
        DayProgressText->SetText(TEXT("월 진행: 0%"));
        DayProgressText->SetFontSize(16.f);
        DayProgressText->SetAlignH(ETextAlignH::Left);
        DayProgressText->SetAlignV(ETextAlignV::Middle);
        DayProgressText->SetTextColor(220, 220, 220, 255);
        DayProgressText->EnableShadow(true);
        DayProgressText->SetShadowOffset(1.f, 1.f);
        DayProgressText->SetShadowTextColor(20, 20, 20, 255);
        mDayProgressText = DayProgressText;
    }

    auto DayProgressBar = CreateWidget<CProgressBar>(
        "BuildMenu_DayProgressBar", 10).lock();

    if (DayProgressBar)
    {
        DayProgressBar->SetTint(
            EProgressBarImageType::Back,
            FVector4(0.08f, 0.08f, 0.10f, 0.85f));
        DayProgressBar->SetTint(
            EProgressBarImageType::Fill,
            FVector4(0.18f, 0.62f, 0.34f, 0.95f));
        DayProgressBar->SetPercent(0.f);
        DayProgressBar->SetBarDir(EProgressBarDir::RightToLeft);
        mDayProgressBar = DayProgressBar;
    }

    auto YearbookPanel = CreateWidget<CImage>("BuildMenu_YearbookPanel", 12).lock();

    if (YearbookPanel)
    {
        YearbookPanel->SetTexture("BuildMenuYearbookBackground",
            GYearbookPanelTexture);
        YearbookPanel->SetTint(1.f, 1.f, 1.f, 1.f);
        mYearbookPanel = YearbookPanel;
    }

    auto YearbookTitleText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookTitle", 13).lock();

    if (YearbookTitleText)
    {
        YearbookTitleText->SetText(TEXT("연감"));
        YearbookTitleText->SetFontSize(24.f);
        YearbookTitleText->SetAlignH(ETextAlignH::Left);
        YearbookTitleText->SetAlignV(ETextAlignV::Middle);
        YearbookTitleText->SetTextColor(245, 245, 245, 255);
        YearbookTitleText->EnableShadow(true);
        YearbookTitleText->SetShadowOffset(1.f, 1.f);
        YearbookTitleText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookTitleText = YearbookTitleText;
    }

    auto YearbookBodyText =
        CreateWidget<CTextBlock>("BuildMenu_YearbookBody", 13).lock();

    if (YearbookBodyText)
    {
        YearbookBodyText->SetText(
            TEXT("종합 만족도: -\n"
                "음식: -\n"
                "보건: -\n"
                "유흥: -\n"
                "신앙: -\n"
                "주거: -\n"
                "직업: -\n"
                "자유: -\n"
                "치안: -\n"
                "무주택자 수: 0명\n"
                "실업자 수: 0명"));
        YearbookBodyText->SetFontSize(14.f);
        YearbookBodyText->SetAlignH(ETextAlignH::Left);
        YearbookBodyText->SetAlignV(ETextAlignV::Top);
        YearbookBodyText->SetTextColor(225, 225, 225, 255);
        YearbookBodyText->EnableShadow(true);
        YearbookBodyText->SetShadowOffset(1.f, 1.f);
        YearbookBodyText->SetShadowTextColor(20, 20, 20, 255);
        mYearbookBodyText = YearbookBodyText;
    }

    auto MenuBackground = CreateWidget<CImage>("BuildMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTexture("BuildMenuBackground",
            GBuildMenuPanelTexture);
        MenuBackground->SetTint(1.f, 1.f, 1.f, 1.f);
        mMenuBackground = MenuBackground;
    }

    auto TitleText = CreateWidget<CTextBlock>("BuildMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(TEXT("건물 선택"));
        TitleText->SetFontSize(28.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(245, 245, 245, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(20, 20, 20, 255);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("BuildMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(18.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(220, 220, 220, 255);
        mPageText = PageText;
    }

    auto PrevPageButton = CreateWidget<CButton>("BuildMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ConfigureDefaultButtonStyle(PrevPageButton);
        PrevPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnPrevPageClick);

        auto PrevText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_PrevText", mWorld);

        if (PrevText)
        {
            PrevText->SetText(TEXT("<"));
            PrevText->SetFontSize(20.f);
            PrevText->SetAlignH(ETextAlignH::Center);
            PrevText->SetAlignV(ETextAlignV::Middle);
            PrevText->SetTextColor(255, 255, 255, 255);
            PrevPageButton->SetChild(PrevText);
        }

        mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton = CreateWidget<CButton>("BuildMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ConfigureDefaultButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnNextPageClick);

        auto NextText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_NextText", mWorld);

        if (NextText)
        {
            NextText->SetText(TEXT(">"));
            NextText->SetFontSize(20.f);
            NextText->SetAlignH(ETextAlignH::Center);
            NextText->SetAlignV(ETextAlignV::Middle);
            NextText->SetTextColor(255, 255, 255, 255);
            NextPageButton->SetChild(NextText);
        }

        mNextPageButton = NextPageButton;
    }

    mCategoryButtons.resize(CategoryCount);
    void (CBuildMenuWidget::* CategoryCallbacks[CategoryCount])() =
    {
        &CBuildMenuWidget::OnCategoryInfrastructureClick,
        &CBuildMenuWidget::OnCategoryFoodResourceClick,
        &CBuildMenuWidget::OnCategoryIndustryClick,
        &CBuildMenuWidget::OnCategoryHousingClick,
        &CBuildMenuWidget::OnCategoryEntertainmentClick
    };

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Category_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureDefaultButtonStyle(Button);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, CategoryCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_CategoryText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(CategoryLabels[i]);
            ButtonText->SetFontSize(16.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Middle);
            ButtonText->SetTextColor(245, 245, 245, 255);
            Button->SetChild(ButtonText);
        }

        mCategoryButtons[i] = Button;
    }

    mBuildingButtons.resize(SlotsPerPage);
    mBuildingButtonTexts.resize(SlotsPerPage);

    void (CBuildMenuWidget::* SlotCallbacks[SlotsPerPage])() =
    {
        &CBuildMenuWidget::OnSlot0Click,
        &CBuildMenuWidget::OnSlot1Click,
        &CBuildMenuWidget::OnSlot2Click,
        &CBuildMenuWidget::OnSlot3Click,
        &CBuildMenuWidget::OnSlot4Click,
        &CBuildMenuWidget::OnSlot5Click,
        &CBuildMenuWidget::OnSlot6Click,
        &CBuildMenuWidget::OnSlot7Click,
        &CBuildMenuWidget::OnSlot8Click,
        &CBuildMenuWidget::OnSlot9Click,
        &CBuildMenuWidget::OnSlot10Click,
        &CBuildMenuWidget::OnSlot11Click
    };

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureIconSlotButtonStyle(Button);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, SlotCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(TEXT("-"));
            ButtonText->SetFontSize(14.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Bottom);
            ButtonText->SetTextColor(240, 240, 240, 255);
            ButtonText->EnableShadow(true);
            ButtonText->SetShadowOffset(1.f, 1.f);
            ButtonText->SetShadowTextColor(16, 16, 16, 220);
            Button->SetChild(ButtonText);
        }

        mBuildingButtons[i] = Button;
        mBuildingButtonTexts[i] = ButtonText;
    }

    mMenuOpen = false;
    mYearbookOpen = false;
    SelectCategory(0);
    ApplyMenuOpenState();
    ApplyYearbookOpenState();
    SyncDemolitionButtonState();
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();
    RefreshLayout();

    return true;
}

void CBuildMenuWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    SyncDemolitionButtonState();
    RefreshNpcCountText();
    RefreshEconomyStatus();
    RefreshYearbookStatus();
    RefreshLayout();
}

void CBuildMenuWidget::Render()
{
    CWidgetContainer::Render();
}

void CBuildMenuWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float AvailableWidth = (std::max)(320.f, ScreenWidth - 80.f);
    const float AvailableHeight = (std::max)(320.f, ScreenHeight - 180.f);
    const float Scale =
        (std::min)(1.f,
            (std::min)(AvailableWidth / mPanelWidth,
                AvailableHeight / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f;
    const float HorizontalMargin = 24.f * Scale;
    const float ContentWidth = PanelWidth - HorizontalMargin * 2.f;
    const float HeaderTopPadding = 14.f * Scale;
    const float HeaderHeight = 36.f * Scale;

    auto BuildButton = mBuildButton.lock();
    auto YearbookButton = mYearbookButton.lock();
    auto DemolishButton = mDemolishButton.lock();

    const float BuildButtonWidth = 220.f;
    const float YearbookButtonWidth = 140.f;
    const float BottomButtonGap = 14.f;
    const float BottomButtonHeight = 58.f;
    const float BottomButtonTop =
        ScreenWidth * 0.5f -
        (BuildButtonWidth + BottomButtonGap + YearbookButtonWidth) * 0.5f;

    if (BuildButton)
    {
        BuildButton->SetPivot(0.f, 1.f);
        BuildButton->SetPos(BottomButtonTop, ScreenHeight - 20.f);
        BuildButton->SetSize(BuildButtonWidth, BottomButtonHeight);
    }

    if (YearbookButton)
    {
        YearbookButton->SetPivot(0.f, 1.f);
        YearbookButton->SetPos(
            BottomButtonTop + BuildButtonWidth + BottomButtonGap,
            ScreenHeight - 20.f);
        YearbookButton->SetSize(YearbookButtonWidth, BottomButtonHeight);
    }

    if (DemolishButton)
    {
        DemolishButton->SetPivot(0.f, 0.f);
        DemolishButton->SetPos(
            PanelLeft + PanelWidth * 0.5f - 72.f * Scale,
            PanelTop + HeaderTopPadding);
        DemolishButton->SetSize(144.f * Scale, HeaderHeight);
    }

    auto NpcCountText = mNpcCountText.lock();
    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (NpcCountText)
    {
        NpcCountText->SetPivot(0.f, 1.f);
        NpcCountText->SetPos(20.f, ScreenHeight - 20.f);
        NpcCountText->SetSize(380.f, 28.f);
    }

    if (BudgetText)
    {
        BudgetText->SetPivot(0.f, 1.f);
        BudgetText->SetPos(20.f, ScreenHeight - 48.f);
        BudgetText->SetSize(420.f, 28.f);
    }

    if (DateText)
    {
        DateText->SetPivot(0.f, 1.f);
        DateText->SetPos(20.f, ScreenHeight - 76.f);
        DateText->SetSize(420.f, 28.f);
    }

    if (DayProgressText)
    {
        DayProgressText->SetPivot(0.f, 1.f);
        DayProgressText->SetPos(20.f, ScreenHeight - 104.f);
        DayProgressText->SetSize(420.f, 24.f);
    }

    if (DayProgressBar)
    {
        DayProgressBar->SetPivot(0.f, 1.f);
        DayProgressBar->SetPos(20.f, ScreenHeight - 124.f);
        DayProgressBar->SetSize(300.f, 14.f);
    }

    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();

    const float YearbookPanelWidth = PanelWidth;
    const float YearbookPanelHeight = PanelHeight;
    const float YearbookLeft = PanelLeft;
    const float YearbookTop = PanelTop;
    const float YearbookBodyTop =
        YearbookTop + HeaderTopPadding + HeaderHeight + 14.f * Scale;
    const float YearbookBodyHeight = (std::max)(
        40.f, YearbookPanelHeight - (YearbookBodyTop - YearbookTop) - 16.f * Scale);

    if (YearbookPanel)
    {
        YearbookPanel->SetPos(YearbookLeft, YearbookTop);
        YearbookPanel->SetSize(YearbookPanelWidth, YearbookPanelHeight);
    }

    if (YearbookTitleText)
    {
        YearbookTitleText->SetPos(
            YearbookLeft + HorizontalMargin,
            YearbookTop + HeaderTopPadding);
        YearbookTitleText->SetSize(ContentWidth, HeaderHeight);
    }

    if (YearbookBodyText)
    {
        YearbookBodyText->SetPos(
            YearbookLeft + HorizontalMargin, YearbookBodyTop);
        YearbookBodyText->SetSize(ContentWidth, YearbookBodyHeight);
    }

    auto MenuBackground = mMenuBackground.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(PanelLeft, PanelTop);
        MenuBackground->SetSize(PanelWidth, PanelHeight);
    }

    auto TitleText = mTitleText.lock();

    if (TitleText)
    {
        TitleText->SetPos(
            PanelLeft + HorizontalMargin,
            PanelTop + HeaderTopPadding);
        TitleText->SetSize(280.f * Scale, HeaderHeight);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto PageText = mPageText.lock();

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 136.f * Scale,
            PanelTop + HeaderTopPadding);
        PrevPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    if (PageText)
    {
        PageText->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 96.f * Scale,
            PanelTop + HeaderTopPadding);
        PageText->SetSize(56.f * Scale, HeaderHeight);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(
            PanelLeft + PanelWidth - HorizontalMargin - 36.f * Scale,
            PanelTop + HeaderTopPadding);
        NextPageButton->SetSize(36.f * Scale, HeaderHeight);
    }

    const float CategoryTop =
        PanelTop + HeaderTopPadding + HeaderHeight + 14.f * Scale;
    const float CategoryGap = 12.f * Scale;
    const float CategoryWidth =
        (ContentWidth - CategoryGap * (CategoryCount - 1)) /
        static_cast<float>(CategoryCount);
    const float CategoryHeight = 40.f * Scale;

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            PanelLeft + HorizontalMargin +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, CategoryHeight);
    }

    const float SlotGapX = 12.f * Scale;
    const float SlotGapY = 12.f * Scale;
    const float SlotTop = CategoryTop + CategoryHeight + 12.f * Scale;
    const float SlotWidth =
        (ContentWidth - SlotGapX * (SlotColumnCount - 1)) /
        static_cast<float>(SlotColumnCount);
    const float SlotHeight =
        (PanelHeight - (SlotTop - PanelTop) -
            SlotGapY * (SlotRowCount - 1) - 16.f * Scale) /
        static_cast<float>(SlotRowCount);

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int Row = i / SlotColumnCount;
        const int Col = i % SlotColumnCount;

        auto SlotButton = mBuildingButtons[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            PanelLeft + HorizontalMargin +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);
    }
}

void CBuildMenuWidget::ApplyMenuOpenState()
{
    auto DemolishButton = mDemolishButton.lock();
    auto MenuBackground = mMenuBackground.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (DemolishButton)
        DemolishButton->SetEnable(mMenuOpen);
    if (MenuBackground)
        MenuBackground->SetEnable(mMenuOpen);
    if (TitleText)
        TitleText->SetEnable(mMenuOpen);
    if (PageText)
        PageText->SetEnable(mMenuOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(mMenuOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(mMenuOpen);

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }

    for (size_t i = 0; i < mBuildingButtons.size(); ++i)
    {
        auto Button = mBuildingButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }
}

void CBuildMenuWidget::ApplyYearbookOpenState()
{
    auto YearbookPanel = mYearbookPanel.lock();
    auto YearbookTitleText = mYearbookTitleText.lock();
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto YearbookButton = mYearbookButton.lock();
    auto YearbookButtonText = mYearbookButtonText.lock();

    if (YearbookPanel)
        YearbookPanel->SetEnable(mYearbookOpen);
    if (YearbookTitleText)
        YearbookTitleText->SetEnable(mYearbookOpen);
    if (YearbookBodyText)
        YearbookBodyText->SetEnable(mYearbookOpen);

    if (YearbookButton)
    {
        if (mYearbookOpen)
            ConfigureHighlightedButtonStyle(YearbookButton);
        else
            ConfigureDefaultButtonStyle(YearbookButton);
    }

    if (YearbookButtonText)
    {
        if (mYearbookOpen)
            YearbookButtonText->SetText(TEXT("연감 ON"));
        else
            YearbookButtonText->SetText(TEXT("연감"));
    }
}

void CBuildMenuWidget::RefreshNpcCountText()
{
    auto NpcCountText = mNpcCountText.lock();
    auto World = mWorld.lock();

    if (!NpcCountText || !World)
        return;

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

    wchar_t CountBuffer[64] = {};
    swprintf_s(CountBuffer, L"NPC: %d", NpcCount);
    NpcCountText->SetText(CountBuffer);
}

void CBuildMenuWidget::RefreshEconomyStatus()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (!MainWorld)
        return;

    auto BudgetText = mBudgetText.lock();
    auto DateText = mDateText.lock();
    auto DayProgressText = mDayProgressText.lock();
    auto DayProgressBar = mDayProgressBar.lock();

    if (BudgetText)
    {
        const std::wstring BudgetLabel =
            L"국가 예산: " +
            FormatCurrency(MainWorld->GetNationalBudget());
        BudgetText->SetText(BudgetLabel.c_str());
    }

    const int Year = MainWorld->GetSimulationYear();
    const int Month = MainWorld->GetSimulationMonth();
    const int Day = MainWorld->GetSimulationDay();
    const int MonthDays = MainWorld->GetSimulationMonthDayCount();
    const float MonthProgress = MainWorld->GetSimulationMonthProgress();

    if (DateText)
    {
        wchar_t DateBuffer[64] = {};
        swprintf_s(DateBuffer, L"날짜: %04d-%02d-%02d", Year, Month, Day);
        DateText->SetText(DateBuffer);
    }

    if (DayProgressBar)
        DayProgressBar->SetPercent(MonthProgress);

    if (DayProgressText)
    {
        wchar_t ProgressBuffer[96] = {};
        swprintf_s(
            ProgressBuffer,
            L"월 진행: %d%%  |  %d / %d일",
            static_cast<int>(roundf(MonthProgress * 100.f)),
            Day,
            MonthDays);
        DayProgressText->SetText(ProgressBuffer);
    }
}

void CBuildMenuWidget::RefreshYearbookStatus()
{
    auto YearbookBodyText = mYearbookBodyText.lock();
    auto World = mWorld.lock();

    if (!YearbookBodyText || !World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int ActiveNpcCount = 0;
    int HomelessCount = 0;
    int UnemployedCount = 0;
    int PoliticalCount[static_cast<int>(EPoliticalAxis::Count)][3] = {};
    double FoodSum = 0.0;
    double HealthSum = 0.0;
    double FunSum = 0.0;
    double FaithSum = 0.0;
    double HousingSum = 0.0;
    double JobSum = 0.0;
    double FreedomSum = 0.0;
    double SecuritySum = 0.0;
    double OverallSatisfactionSum = 0.0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            const FNpcSatisfaction& Satisfaction = Orb->GetSatisfaction();
            const FNpcPoliticalProfile& Political = Orb->GetPoliticalProfile();
            ++ActiveNpcCount;
            FoodSum += static_cast<double>(Satisfaction.Food);
            HealthSum += static_cast<double>(Satisfaction.Health);
            FunSum += static_cast<double>(Satisfaction.Fun);
            FaithSum += static_cast<double>(Satisfaction.Faith);
            HousingSum += static_cast<double>(Satisfaction.Housing);
            JobSum += static_cast<double>(Satisfaction.Job);
            FreedomSum += static_cast<double>(Satisfaction.Freedom);
            SecuritySum += static_cast<double>(Satisfaction.Security);
            OverallSatisfactionSum += static_cast<double>(Satisfaction.Overall);

            if (Orb->GetHomeBuilding().empty())
                ++HomelessCount;

            if (Orb->GetWorkBuilding().empty())
                ++UnemployedCount;

            for (int AxisIndex = 0;
                AxisIndex < static_cast<int>(EPoliticalAxis::Count);
                ++AxisIndex)
            {
                const EPoliticalAxis Axis =
                    static_cast<EPoliticalAxis>(AxisIndex);
                const int StanceIndex =
                    static_cast<int>(Political.Get(Axis).Stance);

                if (StanceIndex >= 0 && StanceIndex < 3)
                    ++PoliticalCount[AxisIndex][StanceIndex];
            }
        }
    }

    const double AverageSatisfaction =
        ActiveNpcCount > 0 ?
        OverallSatisfactionSum / static_cast<double>(ActiveNpcCount) :
        0.0;
    const double AverageFood =
        ActiveNpcCount > 0 ? FoodSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageHealth =
        ActiveNpcCount > 0 ? HealthSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFun =
        ActiveNpcCount > 0 ? FunSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFaith =
        ActiveNpcCount > 0 ? FaithSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageHousing =
        ActiveNpcCount > 0 ? HousingSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageJob =
        ActiveNpcCount > 0 ? JobSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageFreedom =
        ActiveNpcCount > 0 ? FreedomSum / static_cast<double>(ActiveNpcCount) : 0.0;
    const double AverageSecurity =
        ActiveNpcCount > 0 ? SecuritySum / static_cast<double>(ActiveNpcCount) : 0.0;

    std::wstring Body;

    if (ActiveNpcCount > 0)
    {
        wchar_t Buffer[256] = {};
        swprintf_s(Buffer, L"종합 만족도: %.1f / 100\n", AverageSatisfaction);
        Body += Buffer;
        swprintf_s(Buffer, L"음식: %.1f\n", AverageFood); Body += Buffer;
        swprintf_s(Buffer, L"보건: %.1f\n", AverageHealth); Body += Buffer;
        swprintf_s(Buffer, L"유흥: %.1f\n", AverageFun); Body += Buffer;
        swprintf_s(Buffer, L"신앙: %.1f\n", AverageFaith); Body += Buffer;
        swprintf_s(Buffer, L"주거: %.1f\n", AverageHousing); Body += Buffer;
        swprintf_s(Buffer, L"직업: %.1f\n", AverageJob); Body += Buffer;
        swprintf_s(Buffer, L"자유: %.1f\n", AverageFreedom); Body += Buffer;
        swprintf_s(Buffer, L"치안: %.1f\n", AverageSecurity); Body += Buffer;
        swprintf_s(Buffer, L"무주택자 수: %d명\n", HomelessCount); Body += Buffer;
        swprintf_s(Buffer, L"실업자 수: %d명\n", UnemployedCount); Body += Buffer;
    }
    else
    {
        Body +=
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
            L"실업자 수: 0명\n";
    }

    Body += L"\n정치 성향 인원\n";

    for (int AxisIndex = 0;
        AxisIndex < static_cast<int>(EPoliticalAxis::Count);
        ++AxisIndex)
    {
        const EPoliticalAxis Axis =
            static_cast<EPoliticalAxis>(AxisIndex);
        wchar_t LineBuffer[384] = {};
        swprintf_s(
            LineBuffer,
            L"%s: %s %d명 / %s %d명 / %s %d명\n",
            GetPoliticalAxisDisplayName(Axis),
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Left),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Left)],
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Neutral),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Neutral)],
            GetPoliticalFactionDisplayName(Axis, EPoliticalStance::Right),
            PoliticalCount[AxisIndex][static_cast<int>(EPoliticalStance::Right)]);
        Body += LineBuffer;
    }

    YearbookBodyText->SetText(Body.c_str());
}

void CBuildMenuWidget::RefreshCategoryButtons()
{
    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        if (i == mSelectedCategoryIndex)
            ConfigureHighlightedButtonStyle(CategoryButton);
        else
            ConfigureDefaultButtonStyle(CategoryButton);
    }
}

std::vector<int> CBuildMenuWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Catalog = GetCatalog();

    for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
    {
        if (Catalog[i].CategoryIndex == mSelectedCategoryIndex)
            Result.push_back(i);
    }

    return Result;
}

void CBuildMenuWidget::RefreshBuildingButtons()
{
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + SlotsPerPage - 1) / SlotsPerPage);

    if (mCurrentPage < 0)
        mCurrentPage = 0;
    else if (mCurrentPage >= PageCount)
        mCurrentPage = PageCount - 1;

    mVisibleEntryIndices.assign(SlotsPerPage, -1);
    const int BeginIndex = mCurrentPage * SlotsPerPage;

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        auto Button = mBuildingButtons[i].lock();
        auto ButtonText = mBuildingButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            const int EntryIndex = CategoryEntries[CategoryListIndex];
            const auto& Entry = GetCatalog()[EntryIndex];
            const TCHAR* IconPath = GetCatalogEntryIconPath(
                Entry.CategoryIndex, Entry.CategoryLocalIndex);
            const std::string TextureKey = "BuildMenuSlotIcon_" +
                std::to_string(Entry.CategoryIndex) + "_" +
                std::to_string(Entry.CategoryLocalIndex);

            mVisibleEntryIndices[i] = EntryIndex;
            Button->ButtonEnable(true);
            ButtonText->SetText(Entry.DisplayName.c_str());

            if (IconPath)
            {
                ApplyTextureToAllButtonStates(Button, TextureKey, IconPath);
            }
            else
            {
                ApplyTextureToAllButtonStates(
                    Button,
                    "BuildMenuSlotEmptyTexture",
                    GEmptySlotTexture);
            }
        }
        else
        {
            Button->ButtonEnable(false);
            ButtonText->SetText(TEXT("-"));
            ApplyTextureToAllButtonStates(
                Button,
                "BuildMenuSlotEmptyTexture",
                GEmptySlotTexture);
        }
    }

    auto PageText = mPageText.lock();

    if (PageText)
    {
        wchar_t PageBuffer[64] = {};
        swprintf_s(PageBuffer, L"%d / %d", mCurrentPage + 1, PageCount);
        PageText->SetText(PageBuffer);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(mCurrentPage > 0);
    if (NextPageButton)
        NextPageButton->ButtonEnable(mCurrentPage < PageCount - 1);
}

void CBuildMenuWidget::SelectCategory(int CategoryIndex)
{
    if (CategoryIndex < 0)
        CategoryIndex = 0;
    else if (CategoryIndex >= CategoryCount)
        CategoryIndex = CategoryCount - 1;

    mSelectedCategoryIndex = CategoryIndex;
    mCurrentPage = 0;
    RefreshCategoryButtons();
    RefreshBuildingButtons();
}

void CBuildMenuWidget::MovePage(int DeltaPage)
{
    mCurrentPage += DeltaPage;
    RefreshBuildingButtons();
}

void CBuildMenuWidget::StartPlacementBySlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    MainCamera->SetDemolitionMode(false);
    SyncDemolitionButtonState();

    const FBuildCatalogEntry& Entry = Catalog[EntryIndex];

    const bool Started = MainCamera->BeginBuildPlacement(
        Entry.Id,
        WideToUtf8(Entry.DisplayName),
        WideToUtf8(Entry.CategoryName),
        Entry.Residential,
        Entry.Capacity,
        Entry.FoodProvider,
        Entry.EntertainmentProvider,
        Entry.HousingSatisfactionCap,
        Entry.JobSatisfactionCap,
        Entry.FoodSatisfactionCap,
        Entry.FunSatisfactionCap,
        Entry.TemplateType,
        Entry.BuildingKind);

    if (!Started)
        return;

    mMenuOpen = false;
    ApplyMenuOpenState();
}

void CBuildMenuWidget::SyncDemolitionButtonState()
{
    auto DemolishButton = mDemolishButton.lock();
    auto DemolishButtonText = mDemolishButtonText.lock();
    auto World = mWorld.lock();

    if (!DemolishButton || !DemolishButtonText || !World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    const bool Enabled = MainCamera->IsDemolitionMode();

    if (Enabled)
    {
        ConfigureHighlightedButtonStyle(DemolishButton);
        DemolishButtonText->SetText(TEXT("철거 ON"));
    }
    else
    {
        ConfigureDefaultButtonStyle(DemolishButton);
        DemolishButtonText->SetText(TEXT("철거 OFF"));
    }
}

const std::vector<CBuildMenuWidget::FBuildCatalogEntry>&
    CBuildMenuWidget::GetCatalog() const
{
    static std::vector<FBuildCatalogEntry> Catalog = []()
    {
        static const wchar_t* InfrastructureNames[] =
        {
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
            L"공장식 목장",
            L"자동화 광산"
        };

        static const wchar_t* IndustryNames[] =
        {
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
            L"경기장"
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
            80  // 경기장
        };

        std::vector<FBuildCatalogEntry> Entries;

        auto AppendCategory = [&](
            int CategoryIndex,
            const wchar_t* const* Names,
            int NameCount,
            bool Residential,
            bool FoodProvider,
            bool EntertainmentProvider)
        {
            for (int i = 0; i < NameCount; ++i)
            {
                FBuildCatalogEntry Entry;
                Entry.Id = "build_" + std::to_string(CategoryIndex + 1) +
                    "_" + std::to_string(i + 1);
                Entry.DisplayName = Names[i];
                Entry.CategoryName = CategoryLabels[CategoryIndex];
                Entry.Residential = Residential;
                Entry.FoodProvider = FoodProvider;
                Entry.EntertainmentProvider = EntertainmentProvider;
                Entry.CategoryIndex = CategoryIndex;
                Entry.CategoryLocalIndex = i;
                Entry.TemplateType = static_cast<EPlacementTemplateType>(i % 4);
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

                if (CategoryIndex == 0)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        85, 45 + (i % 7) * 5 + (i / 7) * 3);
                }
                else if (CategoryIndex == 1)
                {
                    Entry.FoodSatisfactionCap = (std::min)(
                        82, 35 + (i % 7) * 6 + (i / 7) * 4);
                    Entry.JobSatisfactionCap = (std::min)(
                        70, 40 + (i % 5) * 5 + (i / 5) * 2);
                }
                else if (CategoryIndex == 2)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        90, 50 + (i % 8) * 5 + (i / 8) * 4);
                }
                else if (CategoryIndex == 3)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(HousingCaps) / sizeof(HousingCaps[0])))
                    {
                        Entry.HousingSatisfactionCap = HousingCaps[i];
                    }
                }
                else if (CategoryIndex == 4)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(EntertainmentFunCaps) /
                        sizeof(EntertainmentFunCaps[0])))
                    {
                        Entry.FunSatisfactionCap =
                            EntertainmentFunCaps[i];
                    }
                }

                if (CategoryIndex == 4 &&
                    (Entry.DisplayName == L"레스토랑" ||
                        Entry.DisplayName == L"패스트푸드 체인점"))
                {
                    Entry.FoodProvider = true;

                    if (Entry.DisplayName == L"레스토랑")
                        Entry.FoodSatisfactionCap = 65;
                    else
                        Entry.FoodSatisfactionCap = 55;
                }

                Entries.push_back(Entry);
            }
        };

        AppendCategory(
            0, InfrastructureNames,
            static_cast<int>(sizeof(InfrastructureNames) /
                sizeof(InfrastructureNames[0])),
            false,
            false,
            false);
        AppendCategory(
            1, FoodResourceNames,
            static_cast<int>(sizeof(FoodResourceNames) /
                sizeof(FoodResourceNames[0])),
            false,
            true,
            false);
        AppendCategory(
            2, IndustryNames,
            static_cast<int>(sizeof(IndustryNames) /
                sizeof(IndustryNames[0])),
            false,
            false,
            false);
        AppendCategory(
            3, HousingNames,
            static_cast<int>(sizeof(HousingNames) /
                sizeof(HousingNames[0])),
            true,
            false,
            false);
        AppendCategory(
            4, EntertainmentNames,
            static_cast<int>(sizeof(EntertainmentNames) /
                sizeof(EntertainmentNames[0])),
            false,
            false,
            true);

        return Entries;
    }();

    return Catalog;
}

void CBuildMenuWidget::OnBuildButtonClick()
{
    const bool NextOpen = !mMenuOpen;
    mMenuOpen = NextOpen;

    if (NextOpen)
        mYearbookOpen = false;

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnYearbookButtonClick()
{
    const bool NextOpen = !mYearbookOpen;
    mYearbookOpen = NextOpen;

    if (NextOpen)
        mMenuOpen = false;

    ApplyMenuOpenState();
    ApplyYearbookOpenState();
}

void CBuildMenuWidget::OnDemolishButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    const bool NextMode = !MainCamera->IsDemolitionMode();
    MainCamera->SetDemolitionMode(NextMode);
    SyncDemolitionButtonState();

    if (NextMode)
    {
        mMenuOpen = false;
        ApplyMenuOpenState();
    }
}

void CBuildMenuWidget::OnCategoryInfrastructureClick()
{
    SelectCategory(0);
}

void CBuildMenuWidget::OnCategoryFoodResourceClick()
{
    SelectCategory(1);
}

void CBuildMenuWidget::OnCategoryIndustryClick()
{
    SelectCategory(2);
}

void CBuildMenuWidget::OnCategoryHousingClick()
{
    SelectCategory(3);
}

void CBuildMenuWidget::OnCategoryEntertainmentClick()
{
    SelectCategory(4);
}

void CBuildMenuWidget::OnPrevPageClick()
{
    MovePage(-1);
}

void CBuildMenuWidget::OnNextPageClick()
{
    MovePage(1);
}

void CBuildMenuWidget::OnSlot0Click()
{
    StartPlacementBySlot(0);
}

void CBuildMenuWidget::OnSlot1Click()
{
    StartPlacementBySlot(1);
}

void CBuildMenuWidget::OnSlot2Click()
{
    StartPlacementBySlot(2);
}

void CBuildMenuWidget::OnSlot3Click()
{
    StartPlacementBySlot(3);
}

void CBuildMenuWidget::OnSlot4Click()
{
    StartPlacementBySlot(4);
}

void CBuildMenuWidget::OnSlot5Click()
{
    StartPlacementBySlot(5);
}

void CBuildMenuWidget::OnSlot6Click()
{
    StartPlacementBySlot(6);
}

void CBuildMenuWidget::OnSlot7Click()
{
    StartPlacementBySlot(7);
}

void CBuildMenuWidget::OnSlot8Click()
{
    StartPlacementBySlot(8);
}

void CBuildMenuWidget::OnSlot9Click()
{
    StartPlacementBySlot(9);
}

void CBuildMenuWidget::OnSlot10Click()
{
    StartPlacementBySlot(10);
}

void CBuildMenuWidget::OnSlot11Click()
{
    StartPlacementBySlot(11);
}
