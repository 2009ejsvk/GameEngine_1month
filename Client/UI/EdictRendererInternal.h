#pragma once

#include "EdictDataProvider.h"
#include "EdictWidget.h"
#include "TropicoUiStyle.h"
#include "TropicoUiTheme.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include <Windows.h>
#include <algorithm>
#include <memory>
#include <string>


namespace
{
    using namespace TropicoUiAssets;
    using namespace TropicoUiStyle;
    using namespace TropicoUiTheme;

    constexpr int GEdictCategoryCount = 4;
    constexpr int GEdictSlotsPerPage = 14;
    constexpr int GEdictSlotColumnCount = 7;
    constexpr int GEdictSlotRowCount = 2;
    constexpr int GTaxPolicyRowCount = 3;
    constexpr bool GEnableTaxPolicyPanel = false;

    constexpr const TCHAR* GEdictMenuPanelTexture = GMainMenuPanelTexture;
    constexpr const TCHAR* GCostIconTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\CurrencyIcons\\T_ICO_money.png");
    constexpr const TCHAR* GStarEmptyTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_edicts_star_empty.png");
    constexpr const TCHAR* GStarFullTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_edicts_star_full.png");
    constexpr const TCHAR* GEdictActiveCheckTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_research_popUp_check.png");

    const wchar_t* GCategoryLabels[GEdictCategoryCount] =
    {
        L"식민지 시대",
        L"세계대전 시대",
        L"냉전 시대",
        L"현대 시대"
    };

    const TCHAR* const GCategoryTabIcons[GEdictCategoryCount] =
    {
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_general.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_interior.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_defense.png"),
        TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\EdictIcons\\EdictCategories\\T_ICO_education.png")
    };

    void ConfigureEdictSlotButtonVisual(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase,
        bool Focused,
        bool Active,
        bool CoolingDown,
        bool Available)
    {
        if (!Button)
            return;

        const TCHAR* NormalTexture = GSlotCardTexture;
        const TCHAR* HoverTexture = GSlotCardHoverTexture;
        const TCHAR* ClickTexture = GSlotCardSelectedTexture;
        const TCHAR* DisableTexture = GSlotCardDisabledTexture;
        FVector4 Tint = FVector4(1.f, 1.f, 1.f, 1.f);

        if (Active)
        {
            NormalTexture = GSlotCardSelectedTexture;
            HoverTexture = GSlotCardSelectedTexture;
            ClickTexture = GSlotCardSelectedTexture;
            Tint = GEdictSlotActiveTint;
        }
        else if (CoolingDown)
        {
            NormalTexture = GSlotCardDisabledTexture;
            HoverTexture = GSlotCardDisabledTexture;
            ClickTexture = GSlotCardDisabledTexture;
            Tint = GEdictSlotCoolingDownTint;
        }
        else if (!Available)
        {
            NormalTexture = GSlotCardDisabledTexture;
            HoverTexture = GSlotCardDisabledTexture;
            ClickTexture = GSlotCardDisabledTexture;
            Tint = GEdictSlotUnavailableTint;
        }
        else if (Focused)
        {
            NormalTexture = GSlotCardSelectedTexture;
            HoverTexture = GSlotCardSelectedTexture;
            ClickTexture = GSlotCardSelectedTexture;
            Tint = GEdictSlotFocusedTint;
        }

        ApplyButtonTextureSet(
            Button,
            TextureKeyBase,
            NormalTexture,
            HoverTexture,
            ClickTexture,
            DisableTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetTint(EButtonState::Normal, Tint);
        Button->SetTint(EButtonState::Hovered, Tint);
        Button->SetTint(EButtonState::Click, Tint);
        Button->SetTint(EButtonState::Disable, GOverlayDisabledTint);
    }

    void ConfigureEdictActionButtonVisual(
        const std::shared_ptr<CButton>& Button,
        const std::shared_ptr<CTextBlock>& ButtonText,
        EdictDataProvider::EEdictActionVisualMode Mode,
        const std::wstring& Label)
    {
        if (ButtonText)
            ButtonText->SetText(Label.c_str());

        if (!Button)
            return;

        const TCHAR* NormalTexture = GBigTextButtonTexture;
        const TCHAR* HoverTexture = GBigTextButtonHoverTexture;
        const TCHAR* ClickTexture = GBigTextButtonSelectedTexture;
        const TCHAR* DisableTexture = GBigTextButtonDisabledTexture;
        FVector4 NormalTint = FVector4(1.f, 1.f, 1.f, 0.96f);
        FVector4 HoverTint = FVector4(1.f, 1.f, 1.f, 1.f);
        FVector4 ClickTint = FVector4(0.86f, 0.86f, 0.86f, 1.f);
        FVector4 DisableTint = FVector4(0.55f, 0.55f, 0.55f, 0.80f);
        unsigned char TextR = 89;
        unsigned char TextG = 60;
        unsigned char TextB = 16;
        float FontSize = 18.f;

        switch (Mode)
        {
        case EdictDataProvider::EEdictActionVisualMode::Primary:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.16f, 1.04f, 0.72f, 1.f);
            HoverTint = FVector4(1.22f, 1.10f, 0.80f, 1.f);
            ClickTint = FVector4(1.02f, 0.92f, 0.64f, 1.f);
            DisableTint = FVector4(0.92f, 0.86f, 0.70f, 0.86f);
            TextR = 84;
            TextG = 54;
            TextB = 10;
            FontSize = 21.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::Active:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(0.96f, 1.02f, 1.08f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.88f, 0.94f, 1.00f, 1.f);
            DisableTint = FVector4(0.92f, 0.98f, 1.04f, 0.96f);
            TextR = 70;
            TextG = 78;
            TextB = 88;
            FontSize = 19.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::CoolingDown:
            NormalTexture = GBigTextButtonDisabledTexture;
            HoverTexture = GBigTextButtonDisabledTexture;
            ClickTexture = GBigTextButtonDisabledTexture;
            DisableTexture = GBigTextButtonDisabledTexture;
            NormalTint = FVector4(0.98f, 0.94f, 0.84f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.92f, 0.88f, 0.78f, 1.f);
            DisableTint = FVector4(0.94f, 0.90f, 0.82f, 0.96f);
            TextR = 110;
            TextG = 83;
            TextB = 34;
            FontSize = 19.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::Requirement:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.10f, 0.94f, 0.66f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(1.00f, 0.86f, 0.58f, 1.f);
            DisableTint = FVector4(1.04f, 0.90f, 0.62f, 0.96f);
            TextR = 128;
            TextG = 82;
            TextB = 12;
            FontSize = 20.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::BudgetShortage:
            NormalTexture = GBigTextButtonSelectedTexture;
            HoverTexture = GBigTextButtonSelectedTexture;
            ClickTexture = GBigTextButtonSelectedTexture;
            DisableTexture = GBigTextButtonSelectedTexture;
            NormalTint = FVector4(1.06f, 0.82f, 0.66f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.98f, 0.74f, 0.58f, 1.f);
            DisableTint = FVector4(1.00f, 0.78f, 0.62f, 0.96f);
            TextR = 145;
            TextG = 52;
            TextB = 28;
            FontSize = 20.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::Waiting:
            NormalTexture = GBigTextButtonDisabledTexture;
            HoverTexture = GBigTextButtonDisabledTexture;
            ClickTexture = GBigTextButtonDisabledTexture;
            DisableTexture = GBigTextButtonDisabledTexture;
            NormalTint = FVector4(0.95f, 0.95f, 0.92f, 1.f);
            HoverTint = NormalTint;
            ClickTint = FVector4(0.88f, 0.88f, 0.85f, 1.f);
            DisableTint = FVector4(0.92f, 0.92f, 0.89f, 0.95f);
            TextR = 92;
            TextG = 78;
            TextB = 52;
            FontSize = 18.f;
            break;
        case EdictDataProvider::EEdictActionVisualMode::Neutral:
        default:
            break;
        }

        ApplyButtonTextureSet(
            Button,
            "EdictMenu_ApplyButtonVisual",
            NormalTexture,
            HoverTexture,
            ClickTexture,
            DisableTexture);
        ConfigureIconSlotButtonStyle(Button);
        Button->SetTint(EButtonState::Normal, NormalTint);
        Button->SetTint(EButtonState::Hovered, HoverTint);
        Button->SetTint(EButtonState::Click, ClickTint);
        Button->SetTint(EButtonState::Disable, DisableTint);

        if (ButtonText)
        {
            ButtonText->SetTextColor(TextR, TextG, TextB, 255);
            ButtonText->SetFontSize(FontSize);
        }
    }
}
