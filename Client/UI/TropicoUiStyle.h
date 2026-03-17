#pragma once

#include "UI/Button.h"
#include "TropicoUiAssetCatalog.h"
#include "TropicoUiTheme.h"
#include <string>

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
