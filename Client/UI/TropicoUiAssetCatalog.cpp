#include "TropicoUiAssetCatalog.h"
#include "../RuntimeConfigRegistry.h"
#include "../StringUtils.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <string>

namespace TropicoUiAssets
{
    const FAssetPathRef GMainMenuPanelTexture(EAssetId::MainMenuPanelTexture);
    const FAssetPathRef GModernPanelTexture(EAssetId::ModernPanelTexture);
    const FAssetPathRef GMenuTitleRibbonTexture(EAssetId::MenuTitleRibbonTexture);
    const FAssetPathRef GMenuGridFrameTexture(EAssetId::MenuGridFrameTexture);
    const FAssetPathRef GMenuDetailFrameTexture(EAssetId::MenuDetailFrameTexture);
    const FAssetPathRef GDetailInfoPanelTexture(EAssetId::DetailInfoPanelTexture);
    const FAssetPathRef GCategoryTabTextureSelected(EAssetId::CategoryTabTextureSelected);
    const FAssetPathRef GCategoryTabTextureHidden(EAssetId::CategoryTabTextureHidden);
    const FAssetPathRef GSlotCardTexture(EAssetId::SlotCardTexture);
    const FAssetPathRef GSlotCardHoverTexture(EAssetId::SlotCardHoverTexture);
    const FAssetPathRef GSlotCardSelectedTexture(EAssetId::SlotCardSelectedTexture);
    const FAssetPathRef GSlotCardDisabledTexture(EAssetId::SlotCardDisabledTexture);
    const FAssetPathRef GBigTextButtonTexture(EAssetId::BigTextButtonTexture);
    const FAssetPathRef GBigTextButtonHoverTexture(EAssetId::BigTextButtonHoverTexture);
    const FAssetPathRef GBigTextButtonSelectedTexture(EAssetId::BigTextButtonSelectedTexture);
    const FAssetPathRef GBigTextButtonDisabledTexture(EAssetId::BigTextButtonDisabledTexture);
    const FAssetPathRef GRoundButtonTexture(EAssetId::RoundButtonTexture);
    const FAssetPathRef GRoundButtonHoverTexture(EAssetId::RoundButtonHoverTexture);
    const FAssetPathRef GRoundButtonSelectedTexture(EAssetId::RoundButtonSelectedTexture);
    const FAssetPathRef GScrollTrackTexture(EAssetId::ScrollTrackTexture);
    const FAssetPathRef GScrollThumbTexture(EAssetId::ScrollThumbTexture);
    const FAssetPathRef GDropdownArrowTexture(EAssetId::DropdownArrowTexture);
    const FAssetPathRef GDropdownArrowHoverTexture(EAssetId::DropdownArrowHoverTexture);

    const TCHAR* FAssetPathRef::Get() const
    {
        return ResolveAssetPath(mId);
    }

    FAssetPathRef::operator const TCHAR*() const
    {
        return Get();
    }
}

namespace
{
    using StringUtils::Utf8ToWide;

    constexpr const wchar_t* GConfigId = L"UI.Assets";
    constexpr size_t GAssetCount =
        static_cast<size_t>(TropicoUiAssets::EAssetId::Count);

    struct FAssetBinding
    {
        const char* Section;
        const char* Key;
        const wchar_t* DefaultValue;
    };

    std::array<FAssetBinding, GAssetCount> GBindings =
    {{
        { "panel", "main_menu_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\5_MainMenu\\CenterPopUp\\T_center_popUp.png" },
        { "panel", "modern_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\CenterPopUp\\T_center_popUp.png" },
        { "panel", "title_ribbon_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_title.png" },
        { "panel", "grid_frame_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameContent.png" },
        { "panel", "detail_frame_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\CenterPopUp\\T_center_popUp_frameDescription.png" },
        { "panel", "detail_info_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\4_Modern\\SmallPopup\\T_small_popUp.png" },
        { "category_tab", "selected_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_standard.png" },
        { "category_tab", "hidden_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\Tabs\\T_bookmark_hidden.png" },
        { "slot_card", "normal_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg.png" },
        { "slot_card", "hover_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_hover.png" },
        { "slot_card", "selected_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_selected.png" },
        { "slot_card", "disabled_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\CardButton\\T_construction_card_bg_disabled.png" },
        { "text_button_big", "normal_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_standard.png" },
        { "text_button_big", "hover_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_hover.png" },
        { "text_button_big", "selected_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_selected.png" },
        { "text_button_big", "disabled_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\TextButton\\T_Text_bttn_big_deactivated.png" },
        { "round_button", "normal_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_standard.png" },
        { "round_button", "hover_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_hover.png" },
        { "round_button", "selected_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Buttons\\RoundButton\\T_round_button_selected.png" },
        { "scroll", "track_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar_bg.png" },
        { "scroll", "thumb_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Buttons\\Sliderthumb\\T_scrollbar.png" },
        { "dropdown", "arrow_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow.png" },
        { "dropdown", "arrow_hover_texture", L"TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Indicators\\T_dropDownArrow_hover.png" }
    }};

    std::array<std::wstring, GAssetCount> GValues = {};

    void TrimString(std::string& Value)
    {
        Value.erase(
            Value.begin(),
            std::find_if(
                Value.begin(),
                Value.end(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }));
        Value.erase(
            std::find_if(
                Value.rbegin(),
                Value.rend(),
                [](unsigned char Character)
                {
                    return !std::isspace(Character);
                }).base(),
            Value.end());
    }

    std::string ToLowerCopy(std::string Value)
    {
        std::transform(
            Value.begin(),
            Value.end(),
            Value.begin(),
            [](unsigned char Character)
            {
                return static_cast<char>(std::tolower(Character));
            });
        return Value;
    }

    bool TryParseSectionHeader(
        const std::string& Line,
        std::string& OutSection)
    {
        if (Line.size() < 3 || Line.front() != '[' || Line.back() != ']')
            return false;

        OutSection = Line.substr(1, Line.size() - 2);
        TrimString(OutSection);
        OutSection = ToLowerCopy(OutSection);
        return !OutSection.empty();
    }

    bool TrySplitSectionAndKey(
        const std::string& CurrentSection,
        const std::string& RawKey,
        std::string& OutSection,
        std::string& OutKey)
    {
        OutSection = CurrentSection;
        OutKey = RawKey;

        const size_t DotPos = RawKey.find('.');

        if (DotPos != std::string::npos)
        {
            OutSection = RawKey.substr(0, DotPos);
            OutKey = RawKey.substr(DotPos + 1);
        }

        TrimString(OutSection);
        TrimString(OutKey);
        OutSection = ToLowerCopy(OutSection);
        OutKey = ToLowerCopy(OutKey);
        return !OutSection.empty() && !OutKey.empty();
    }

    void ResetToDefaults()
    {
        for (size_t Index = 0; Index < GBindings.size(); ++Index)
            GValues[Index] = GBindings[Index].DefaultValue;
    }

    bool ApplyAssetValue(
        const std::string& Section,
        const std::string& Key,
        const std::wstring& Value)
    {
        for (size_t Index = 0; Index < GBindings.size(); ++Index)
        {
            if (Section == GBindings[Index].Section &&
                Key == GBindings[Index].Key)
            {
                GValues[Index] = Value;
                return true;
            }
        }

        return false;
    }

    bool LoadFile(const std::wstring& Path)
    {
        std::ifstream File(Path);

        if (!File.is_open())
            return false;

        std::string CurrentSection;
        std::string Line;

        while (std::getline(File, Line))
        {
            if (!Line.empty() && Line.back() == '\r')
                Line.pop_back();

            std::string TrimmedLine = Line;
            TrimString(TrimmedLine);

            if (TrimmedLine.empty() ||
                TrimmedLine[0] == '#' ||
                TrimmedLine[0] == ';')
            {
                continue;
            }

            std::string ParsedSection;

            if (TryParseSectionHeader(TrimmedLine, ParsedSection))
            {
                CurrentSection = ParsedSection;
                continue;
            }

            const size_t EqualsPos = TrimmedLine.find('=');

            if (EqualsPos == std::string::npos)
                continue;

            std::string RawKey = TrimmedLine.substr(0, EqualsPos);
            std::string RawValue = TrimmedLine.substr(EqualsPos + 1);
            TrimString(RawKey);
            TrimString(RawValue);

            if (RawKey.empty() || RawValue.empty())
                continue;

            std::string Section;
            std::string Key;

            if (!TrySplitSectionAndKey(CurrentSection, RawKey, Section, Key))
                continue;

            ApplyAssetValue(Section, Key, Utf8ToWide(RawValue));
        }

        return true;
    }
}

namespace TropicoUiAssets
{
    void RegisterRuntimeConfig()
    {
        RuntimeConfigRegistry::RegisterSource(
            {
                GConfigId,
                RuntimeConfigRegistry::BuildExeRelativePath(L"UIAssets.ini"),
                0.5f,
                &ResetToDefaults,
                &LoadFile,
                nullptr
            });
    }

    bool ReloadIfChanged(float DeltaTime)
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::PollSource(GConfigId, DeltaTime);
    }

    unsigned long long GetRuntimeConfigGeneration()
    {
        RegisterRuntimeConfig();
        return RuntimeConfigRegistry::GetSourceGeneration(GConfigId);
    }

    const TCHAR* ResolveAssetPath(EAssetId Id)
    {
        const size_t Index = static_cast<size_t>(Id);

        if (Index >= GValues.size())
            return TEXT("");

        return GValues[Index].c_str();
    }
}
