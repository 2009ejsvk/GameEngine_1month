#pragma once

#include "UI/Button.h"
#include "TropicoUiTheme.h"
#include <string>

namespace TropicoUiAssets
{
    constexpr const TCHAR* GMainMenuPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GModernPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\CenterPopUp\\T_center_popUp.png");
    constexpr const TCHAR* GMenuTitleRibbonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_title.png");
    constexpr const TCHAR* GMenuGridFrameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameContent.png");
    constexpr const TCHAR* GMenuDetailFrameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameDescription.png");
    constexpr const TCHAR* GDetailInfoPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\SmallPopup\\T_small_popUp.png");
    constexpr const TCHAR* GCategoryTabTextureSelected = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_standard.png");
    constexpr const TCHAR* GCategoryTabTextureHidden = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_hidden.png");
    constexpr const TCHAR* GSlotCardTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg.png");
    constexpr const TCHAR* GSlotCardHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_hover.png");
    constexpr const TCHAR* GSlotCardSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_selected.png");
    constexpr const TCHAR* GSlotCardDisabledTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_disabled.png");
    constexpr const TCHAR* GBigTextButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_standard.png");
    constexpr const TCHAR* GBigTextButtonHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_hover.png");
    constexpr const TCHAR* GBigTextButtonSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_selected.png");
    constexpr const TCHAR* GBigTextButtonDisabledTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_deactivated.png");
    constexpr const TCHAR* GRoundButtonTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_standard.png");
    constexpr const TCHAR* GRoundButtonHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_hover.png");
    constexpr const TCHAR* GRoundButtonSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_selected.png");
    constexpr const TCHAR* GScrollTrackTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar_bg.png");
    constexpr const TCHAR* GScrollThumbTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar.png");
    constexpr const TCHAR* GDropdownArrowTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow.png");
    constexpr const TCHAR* GDropdownArrowHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow_hover.png");
}

namespace TropicoUiStyle
{
    inline void ConfigureDefaultButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            TropicoUiTheme::GButtonDefaultNormalTint);
        Button->SetTint(EButtonState::Hovered,
            TropicoUiTheme::GButtonDefaultHoverTint);
        Button->SetTint(EButtonState::Click,
            TropicoUiTheme::GButtonDefaultClickTint);
        Button->SetTint(EButtonState::Disable,
            TropicoUiTheme::GButtonDefaultDisableTint);
    }

    inline void ConfigureHighlightedButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            TropicoUiTheme::GButtonHighlightNormalTint);
        Button->SetTint(EButtonState::Hovered,
            TropicoUiTheme::GButtonHighlightHoverTint);
        Button->SetTint(EButtonState::Click,
            TropicoUiTheme::GButtonHighlightClickTint);
        Button->SetTint(EButtonState::Disable,
            TropicoUiTheme::GButtonHighlightDisableTint);
    }

    inline void ConfigureIconSlotButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            TropicoUiTheme::GButtonIconSlotNormalTint);
        Button->SetTint(EButtonState::Hovered,
            TropicoUiTheme::GButtonIconSlotHoverTint);
        Button->SetTint(EButtonState::Click,
            TropicoUiTheme::GButtonIconSlotClickTint);
        Button->SetTint(EButtonState::Disable,
            TropicoUiTheme::GButtonIconSlotDisableTint);
    }

    inline void ConfigureCategoryTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                TropicoUiTheme::GButtonCategorySelectedNormalTint);
            Button->SetTint(EButtonState::Hovered,
                TropicoUiTheme::GButtonCategorySelectedHoverTint);
            Button->SetTint(EButtonState::Click,
                TropicoUiTheme::GButtonCategorySelectedClickTint);
            Button->SetTint(EButtonState::Disable,
                TropicoUiTheme::GButtonCategorySelectedDisableTint);
            return;
        }

        Button->SetTint(EButtonState::Normal,
            TropicoUiTheme::GButtonCategoryNormalTint);
        Button->SetTint(EButtonState::Hovered,
            TropicoUiTheme::GButtonCategoryHoverTint);
        Button->SetTint(EButtonState::Click,
            TropicoUiTheme::GButtonCategoryClickTint);
        Button->SetTint(EButtonState::Disable,
            TropicoUiTheme::GButtonCategoryDisableTint);
    }

    inline void ApplyTextureToAllButtonStates(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKey,
        const TCHAR* TextureFile)
    {
        if (!Button || !TextureFile)
            return;

        if (!Button->SetTexture(
            EButtonState::Normal,
            TextureKey,
            TextureFile))
        {
            return;
        }

        Button->SetTexture(EButtonState::Hovered, TextureKey);
        Button->SetTexture(EButtonState::Click, TextureKey);
        Button->SetTexture(EButtonState::Disable, TextureKey);
    }

    inline void ApplyButtonTextureSet(
        const std::shared_ptr<CButton>& Button,
        const std::string& TextureKeyBase,
        const TCHAR* NormalTexture,
        const TCHAR* HoverTexture = nullptr,
        const TCHAR* ClickTexture = nullptr,
        const TCHAR* DisableTexture = nullptr)
    {
        if (!Button || !NormalTexture)
            return;

        const TCHAR* ResolvedHover =
            HoverTexture ? HoverTexture : NormalTexture;
        const TCHAR* ResolvedClick =
            ClickTexture ? ClickTexture : ResolvedHover;
        const TCHAR* ResolvedDisable =
            DisableTexture ? DisableTexture : NormalTexture;

        if (!Button->SetTexture(
            EButtonState::Normal,
            TextureKeyBase + "_normal",
            NormalTexture))
        {
            return;
        }

        Button->SetTexture(
            EButtonState::Hovered,
            TextureKeyBase + "_hover",
            ResolvedHover);
        Button->SetTexture(
            EButtonState::Click,
            TextureKeyBase + "_click",
            ResolvedClick);
        Button->SetTexture(
            EButtonState::Disable,
            TextureKeyBase + "_disable",
            ResolvedDisable);
    }
}
