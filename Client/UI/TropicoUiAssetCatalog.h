#pragma once

#include <tchar.h>

namespace TropicoUiAssets
{
    enum class EAssetId
    {
        MainMenuPanelTexture,
        ModernPanelTexture,
        MenuTitleRibbonTexture,
        MenuGridFrameTexture,
        MenuDetailFrameTexture,
        DetailInfoPanelTexture,
        CategoryTabTextureSelected,
        CategoryTabTextureHidden,
        SlotCardTexture,
        SlotCardHoverTexture,
        SlotCardSelectedTexture,
        SlotCardDisabledTexture,
        BigTextButtonTexture,
        BigTextButtonHoverTexture,
        BigTextButtonSelectedTexture,
        BigTextButtonDisabledTexture,
        RoundButtonTexture,
        RoundButtonHoverTexture,
        RoundButtonSelectedTexture,
        ScrollTrackTexture,
        ScrollThumbTexture,
        DropdownArrowTexture,
        DropdownArrowHoverTexture,
        Count
    };

    class FAssetPathRef
    {
    public:
        constexpr explicit FAssetPathRef(EAssetId InId)
            : mId(InId)
        {
        }

        operator const TCHAR*() const;
        const TCHAR* Get() const;

    private:
        EAssetId mId;
    };

    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);
    unsigned long long GetRuntimeConfigGeneration();
    const TCHAR* ResolveAssetPath(EAssetId Id);

    extern const FAssetPathRef GMainMenuPanelTexture;
    extern const FAssetPathRef GModernPanelTexture;
    extern const FAssetPathRef GMenuTitleRibbonTexture;
    extern const FAssetPathRef GMenuGridFrameTexture;
    extern const FAssetPathRef GMenuDetailFrameTexture;
    extern const FAssetPathRef GDetailInfoPanelTexture;
    extern const FAssetPathRef GCategoryTabTextureSelected;
    extern const FAssetPathRef GCategoryTabTextureHidden;
    extern const FAssetPathRef GSlotCardTexture;
    extern const FAssetPathRef GSlotCardHoverTexture;
    extern const FAssetPathRef GSlotCardSelectedTexture;
    extern const FAssetPathRef GSlotCardDisabledTexture;
    extern const FAssetPathRef GBigTextButtonTexture;
    extern const FAssetPathRef GBigTextButtonHoverTexture;
    extern const FAssetPathRef GBigTextButtonSelectedTexture;
    extern const FAssetPathRef GBigTextButtonDisabledTexture;
    extern const FAssetPathRef GRoundButtonTexture;
    extern const FAssetPathRef GRoundButtonHoverTexture;
    extern const FAssetPathRef GRoundButtonSelectedTexture;
    extern const FAssetPathRef GScrollTrackTexture;
    extern const FAssetPathRef GScrollThumbTexture;
    extern const FAssetPathRef GDropdownArrowTexture;
    extern const FAssetPathRef GDropdownArrowHoverTexture;
}
