#pragma once

#include "AlmanacRendererFormatting.h"
#include "AlmanacWidget.h"
#include "TropicoUiStyle.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TextBlock.h"
#include <cmath>
#include <memory>
#include <string>

namespace
{
    void ConfigureTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(30.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(96, 73, 32, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(246, 234, 192, 170);
    }

    void ConfigureSectionText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(18.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(121, 92, 42, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(247, 241, 220, 140);
    }

    void ConfigureBodyLabelText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(76, 70, 60, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 248, 238, 96);
    }

    void ConfigureBodyValueText(
        const std::shared_ptr<CTextBlock>& Text,
        unsigned char r = 78,
        unsigned char g = 68,
        unsigned char b = 54)
    {
        if (!Text)
            return;

        Text->SetFontSize(17.f);
        Text->SetAlignH(ETextAlignH::Right);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(r, g, b, 255);
        Text->EnableShadow(true);
        Text->SetShadowOffset(1.f, 1.f);
        Text->SetShadowTextColor(250, 248, 238, 92);
    }

    void ConfigureNoticeText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(13.f);
        Text->SetAlignH(ETextAlignH::Left);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(120, 108, 91, 255);
    }

    void ConfigureAuxiliaryText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(13.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(136, 123, 98, 255);
    }

    void ConfigureCardTitleText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Top);
        Text->SetTextColor(103, 79, 39, 255);
    }

    void ConfigureCardValueText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(24.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(63, 59, 51, 255);
    }

    void ConfigureCardDetailText(const std::shared_ptr<CTextBlock>& Text)
    {
        if (!Text)
            return;

        Text->SetFontSize(15.f);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
        Text->SetTextColor(125, 112, 92, 255);
    }

    void ConfigureFrameImage(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName,
        const TCHAR* TextureFile)
    {
        if (!Image || !TextureFile)
            return;

        Image->SetTexture(TextureName, TextureFile);
        Image->SetTint(1.f, 1.f, 1.f, 1.f);
    }

    void ApplySelectableBackground(
        const std::shared_ptr<CImage>& Image,
        bool Highlight,
        bool CardStyle = false)
    {
        if (!Image)
            return;

        const TCHAR* TextureFile = nullptr;

        if (CardStyle)
            TextureFile = Highlight ? GCardTextureSelected : GCardTexture;
        else
            TextureFile = Highlight ? GRowTextureSelected : GRowTexture;

        Image->SetTexture(
            Image->GetName() + (Highlight ? "_sel" : "_base"),
            TextureFile);
        Image->SetTint(
            Highlight ? FVector4(1.f, 0.98f, 0.88f, 1.f) :
                FVector4(1.f, 1.f, 1.f, 0.94f));
    }

    void ConfigureRowBackground(
        const std::shared_ptr<CImage>& Image,
        const std::string& TextureName)
    {
        ConfigureFrameImage(Image, TextureName, GRowTexture);
        ApplySelectableBackground(Image, false);
    }

    void ConfigureMetricBar(const std::shared_ptr<CProgressBar>& Bar)
    {
        if (!Bar)
            return;

        Bar->SetTexture(
            EProgressBarImageType::Back,
            "Almanac_BarBack",
            GBarBackTexture);
        Bar->SetTexture(
            EProgressBarImageType::Fill,
            "Almanac_BarFill",
            GBarFillTexture);
        Bar->SetTint(EProgressBarImageType::Back,
            FVector4(0.92f, 0.86f, 0.66f, 0.45f));
        Bar->SetTint(EProgressBarImageType::Fill,
            FVector4(0.90f, 0.72f, 0.18f, 0.95f));
        Bar->SetBarDir(EProgressBarDir::RightToLeft);
        Bar->SetPercent(0.f);
    }

    void ConfigureSatisfactionRowButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            TropicoUiStyle::ApplyButtonTextureSet(
                Button,
                Button->GetName() + "_selected",
                GBigTextButtonSelectedTexture,
                GBigTextButtonSelectedTexture,
                GBigTextButtonSelectedTexture,
                GBigTextButtonDisabledTexture);
            Button->SetTint(EButtonState::Normal,
                FVector4(1.02f, 0.98f, 0.70f, 0.98f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.04f, 1.00f, 0.78f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.98f, 0.94f, 0.62f, 0.98f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.64f, 0.62f, 0.46f, 0.72f));
            return;
        }

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_normal",
            GBigTextButtonTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonHoverTexture,
            GBigTextButtonDisabledTexture);
        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.02f, 1.01f, 0.98f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.96f, 0.94f, 0.90f, 0.96f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.62f, 0.62f, 0.62f, 0.70f));
    }

    void ConfigurePoliticsFactionButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            TropicoUiStyle::ApplyButtonTextureSet(
                Button,
                Button->GetName() + "_selected",
                GCardTextureSelected,
                GCardTextureSelected,
                GCardTextureSelected,
                GCardTexture);
            Button->SetTint(EButtonState::Normal,
                FVector4(1.04f, 0.98f, 0.72f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.05f, 1.00f, 0.80f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.98f, 0.92f, 0.62f, 0.98f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.68f, 0.64f, 0.48f, 0.72f));
            return;
        }

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_normal",
            GCardTexture,
            GCardTextureSelected,
            GCardTextureSelected,
            GCardTexture);
        Button->SetTint(EButtonState::Normal,
            FVector4(1.f, 1.f, 1.f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(1.02f, 1.00f, 0.96f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.96f, 0.94f, 0.88f, 0.96f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.62f, 0.62f, 0.62f, 0.70f));
    }

    void ConfigureSatisfactionRowBar(const std::shared_ptr<CProgressBar>& Bar)
    {
        ConfigureMetricBar(Bar);

        if (!Bar)
            return;

        Bar->SetTint(EProgressBarImageType::Back,
            FVector4(0.82f, 0.76f, 0.58f, 0.14f));
        Bar->SetTint(EProgressBarImageType::Fill,
            FVector4(0.92f, 0.78f, 0.24f, 0.18f));
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

    void ConfigureTabButtonStyle(
        const std::shared_ptr<CButton>& Button,
        bool Selected)
    {
        if (!Button)
            return;

        ApplyTextureToAllButtonStates(
            Button,
            Selected ? Button->GetName() + "_selected" :
                Button->GetName() + "_normal",
            GTabTexture);

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(1.08f, 1.02f, 0.84f, 1.f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(1.10f, 1.06f, 0.90f, 1.f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.94f, 0.86f, 0.62f, 1.f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.60f, 0.60f, 0.60f, 0.70f));
            return;
        }

        Button->SetTint(EButtonState::Normal,
            FVector4(0.84f, 0.90f, 0.97f, 0.96f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.94f, 0.98f, 1.f, 1.f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.76f, 0.84f, 0.92f, 1.f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.50f, 0.50f, 0.50f, 0.70f));
    }

    void ConfigureCloseButtonStyle(const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        TropicoUiStyle::ApplyButtonTextureSet(
            Button,
            Button->GetName() + "_close",
            GRoundButtonTexture,
            GRoundButtonHoverTexture,
            GRoundButtonSelectedTexture,
            GRoundButtonTexture);
        TropicoUiStyle::ConfigureIconSlotButtonStyle(Button);
    }

    void SetCardTint(const CAlmanacWidget::FCardWidgets& Card, bool Highlight)
    {
        auto Background = Card.Background.lock();

        if (!Background)
            return;

        ApplySelectableBackground(Background, Highlight, true);
    }

    void SetCardData(
        const CAlmanacWidget::FCardWidgets& Card,
        const std::wstring& Title,
        const std::wstring& Value,
        const std::wstring& Detail,
        bool Highlight = false)
    {
        SetCardTint(Card, Highlight);

        auto TitleText = Card.Title.lock();
        auto ValueText = Card.Value.lock();
        auto DetailText = Card.Detail.lock();

        if (TitleText)
            TitleText->SetText(Title.c_str());
        if (ValueText)
            ValueText->SetText(Value.c_str());
        if (DetailText)
            DetailText->SetText(Detail.c_str());
    }

    void SetMetricRowBackground(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        bool Highlight)
    {
        auto Background = Row.Background.lock();

        if (!Background)
            return;

        ApplySelectableBackground(Background, Highlight);
    }

    void SetMetricRowData(
        const CAlmanacWidget::FMetricRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Highlight = false)
    {
        SetMetricRowBackground(Row, Highlight);

        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (LabelText)
            LabelText->SetText(Label.c_str());

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Highlight ? 92 : 78,
                Highlight ? 54 : 68,
                Highlight ? 12 : 54,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Clamp01(Percent));
            Bar->SetTint(EProgressBarImageType::Fill, FillTint);
        }
    }

    void SetSatisfactionRowData(
        const CAlmanacWidget::FSatisfactionRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        const FVector4& FillTint,
        bool Selected)
    {
        auto Button = Row.Button.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();
        auto Icon = Row.Icon.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Selected);
            Button->ButtonEnable(true);
        }

        if (Icon)
        {
            Icon->SetTint(
                Selected ?
                    FVector4(1.06f, 1.02f, 0.96f, 1.f) :
                    FVector4(0.98f, 0.98f, 0.98f, 1.f));
        }

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Selected ? 84 : 78,
                Selected ? 64 : 68,
                Selected ? 22 : 54,
                255);
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Selected ? 96 : 78,
                Selected ? 62 : 68,
                Selected ? 8 : 54,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Clamp01(Percent));
            Bar->SetTint(
                EProgressBarImageType::Fill,
                Selected ?
                    FVector4(0.98f, 0.80f, 0.12f, 0.98f) :
                    FillTint);
            Bar->SetEnable(false);
        }
    }

    void SetPoliticsFactionTileData(
        const CAlmanacWidget::FPoliticsFactionTileWidgets& Tile,
        const TCHAR* IconTexture,
        const FVector4& IconTint,
        const std::wstring& Label,
        int Count,
        int Favor,
        bool Selected)
    {
        auto Button = Tile.Button.lock();
        auto Icon = Tile.Icon.lock();
        auto LabelText = Tile.Label.lock();
        auto CountIcon = Tile.CountIcon.lock();
        auto CountValue = Tile.CountValue.lock();
        auto FavorIcon = Tile.FavorIcon.lock();
        auto FavorValue = Tile.FavorValue.lock();

        if (Button)
        {
            ConfigurePoliticsFactionButtonStyle(Button, Selected);
            Button->ButtonEnable(true);
        }

        if (Icon && IconTexture)
        {
            Icon->SetTexture(Icon->GetName() + "_texture", IconTexture);
            Icon->SetTint(IconTint);
        }

        if (CountIcon)
            CountIcon->SetTint(0.88f, 0.70f, 0.18f, 0.96f);

        if (FavorIcon)
            FavorIcon->SetTint(0.92f, 0.68f, 0.16f, 0.96f);

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Selected ? 86 : 74,
                Selected ? 68 : 70,
                Selected ? 18 : 58,
                255);
        }

        if (CountValue)
        {
            CountValue->SetText(std::to_wstring(Count).c_str());
            CountValue->SetTextColor(
                Selected ? 96 : 104,
                Selected ? 70 : 95,
                Selected ? 18 : 64,
                255);
        }

        if (FavorValue)
        {
            FavorValue->SetText(std::to_wstring(Favor).c_str());
            FavorValue->SetTextColor(
                Selected ? 102 : 121,
                Selected ? 66 : 92,
                Selected ? 6 : 28,
                255);
        }
    }

    void SetForeignPowerRowData(
        const CAlmanacWidget::FSatisfactionRowWidgets& Row,
        const TCHAR* IconTexture,
        const std::wstring& Label,
        const std::wstring& Value,
        float Percent,
        bool Selected,
        bool Enabled = true)
    {
        auto Button = Row.Button.lock();
        auto Icon = Row.Icon.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();
        auto Bar = Row.Bar.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Selected);
            Button->ButtonEnable(Enabled);
        }

        if (Icon && IconTexture)
        {
            Icon->SetTexture(Icon->GetName() + "_texture", IconTexture);
            Icon->SetTint(
                Enabled ? 1.f : 0.45f,
                Enabled ? 1.f : 0.45f,
                Enabled ? 1.f : 0.45f,
                Enabled ? 1.f : 0.65f);
        }

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            LabelText->SetTextColor(
                Enabled ? (Selected ? 84 : 78) : 112,
                Enabled ? (Selected ? 64 : 68) : 112,
                Enabled ? (Selected ? 22 : 54) : 112,
                255);
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Enabled ? (Selected ? 96 : 74) : 112,
                Enabled ? (Selected ? 62 : 96) : 112,
                Enabled ? (Selected ? 8 : 128) : 112,
                255);
        }

        if (Bar)
        {
            Bar->SetPercent(Enabled ? Clamp01(Percent) : 0.f);
            Bar->SetTint(
                EProgressBarImageType::Back,
                FVector4(0.78f, 0.80f, 0.74f, 0.24f));
            Bar->SetTint(
                EProgressBarImageType::Fill,
                Enabled ?
                    FVector4(0.32f, 0.60f, 0.90f, 0.96f) :
                    FVector4(0.48f, 0.48f, 0.48f, 0.55f));
            Bar->SetEnable(false);
        }
    }

    void SetDetailRowData(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        const std::wstring& Label,
        const std::wstring& Value,
        bool Highlight = false,
        const FVector4& ValueColor = FVector4(0.31f, 0.27f, 0.21f, 1.f))
    {
        auto Button = Row.Button.lock();
        auto Background = Row.Background.lock();
        auto LabelText = Row.Label.lock();
        auto ValueText = Row.Value.lock();

        if (Button)
        {
            ConfigureSatisfactionRowButtonStyle(Button, Highlight);
            Button->ButtonEnable(true);
        }
        else if (Background)
        {
            ApplySelectableBackground(Background, Highlight);
        }

        if (LabelText)
        {
            LabelText->SetText(Label.c_str());
            if (Button)
            {
                LabelText->SetTextColor(
                    Highlight ? 84 : (Value.empty() ? 92 : 76),
                    Highlight ? 64 : (Value.empty() ? 84 : 70),
                    Highlight ? 22 : (Value.empty() ? 66 : 60),
                    255);
            }
        }

        if (ValueText)
        {
            ValueText->SetText(Value.c_str());
            ValueText->SetTextColor(
                Button && Highlight ? 96 :
                    static_cast<unsigned char>(ValueColor.x * 255.f),
                Button && Highlight ? 62 :
                    static_cast<unsigned char>(ValueColor.y * 255.f),
                Button && Highlight ? 8 :
                    static_cast<unsigned char>(ValueColor.z * 255.f),
                255);
        }
    }

    void SetLineSegment(
        const std::shared_ptr<CImage>& Segment,
        float X0,
        float Y0,
        float X1,
        float Y1,
        float Thickness,
        const FVector4& Tint)
    {
        if (!Segment)
            return;

        const float DX = X1 - X0;
        const float DY = Y1 - Y0;
        const float Length = std::sqrt(DX * DX + DY * DY);

        if (Length <= 0.5f)
        {
            Segment->SetEnable(false);
            return;
        }

        Segment->SetEnable(true);
        Segment->SetPivot(0.f, 0.5f);
        Segment->SetPos(X0, Y0);
        Segment->SetSize(Length, Thickness);
        Segment->SetAngle(static_cast<float>(
            std::atan2(DY, DX) * 180.0 / 3.14159265358979323846));
        Segment->SetTint(Tint);
    }
}
