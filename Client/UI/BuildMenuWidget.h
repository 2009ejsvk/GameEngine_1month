#pragma once

#include "UI/WidgetContainer.h"
#include "../Map/PlacementAreaObject.h"
#include "../Building/BuildingCatalog.h"
#include <string>
#include <vector>

class CBuildMenuWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CBuildMenuWidget();

public:
    virtual ~CBuildMenuWidget();

private:
    std::weak_ptr<class CButton>    mBuildButton;
    std::weak_ptr<class CButton>    mYearbookButton;
    std::weak_ptr<class CTextBlock> mYearbookButtonText;
    std::weak_ptr<class CTextBlock> mNpcCountText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mDateText;
    std::weak_ptr<class CTextBlock> mDayProgressText;
    std::weak_ptr<class CProgressBar> mDayProgressBar;
    std::weak_ptr<class CImage>     mYearbookPanel;
    std::weak_ptr<class CTextBlock> mYearbookTitleText;
    std::weak_ptr<class CTextBlock> mYearbookBodyText;
    std::weak_ptr<class CImage>     mMenuBackground;
    std::weak_ptr<class CImage>     mMenuTitleRibbon;
    std::weak_ptr<class CImage>     mMenuGridFrame;
    std::weak_ptr<class CImage>     mMenuDetailFrame;
    std::weak_ptr<class CImage>     mDetailInfoPanel;
    std::weak_ptr<class CImage>     mDetailBlueprintIcon;
    std::weak_ptr<class CImage>     mDetailConstructionIcon;
    std::weak_ptr<class CImage>     mScrollTrack;
    std::weak_ptr<class CImage>     mScrollThumb;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mPageText;
    std::weak_ptr<class CTextBlock> mDetailTitleText;
    std::weak_ptr<class CTextBlock> mDetailBlueprintCostText;
    std::weak_ptr<class CTextBlock> mDetailConstructionCostText;
    std::weak_ptr<class CTextBlock> mDetailInfoText;
    std::weak_ptr<class CTextBlock> mDetailBodyText;
    std::weak_ptr<class CButton>    mMenuCloseButton;
    std::weak_ptr<class CButton>    mYearbookCloseButton;
    std::weak_ptr<class CButton>    mPrevPageButton;
    std::weak_ptr<class CButton>    mNextPageButton;
    std::vector<std::weak_ptr<class CButton>>    mCategoryButtons;
    std::vector<std::weak_ptr<class CImage>>     mCategoryButtonIcons;
    std::vector<std::weak_ptr<class CButton>>    mBuildingButtons;
    std::vector<std::weak_ptr<class CImage>>     mBuildingButtonIcons;
    std::vector<std::weak_ptr<class CTextBlock>> mBuildingButtonTexts;
    std::vector<int> mVisibleEntryIndices;
    bool  mMenuOpen               = false;
    bool  mYearbookOpen           = false;
    EBuildingCategory mSelectedCategory = EBuildingCategory::Infrastructure;
    int   mPreviewEntryIndex      = -1;
    int   mCurrentPage            = 0;
    float mPanelWidth             = 1100.f;
    float mPanelHeight            = 760.f;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
    void ToggleBuildMenu();
    void ToggleAlmanac();
    void SetBuildMenuOpen(bool Open);
    void SetAlmanacOpen(bool Open);
    bool IsBuildMenuOpen() const
    {
        return mMenuOpen;
    }
    bool IsAlmanacOpen() const
    {
        return mYearbookOpen;
    }

private:
    void RefreshLayout();
    void RefreshNpcCountText();
    void RefreshEconomyStatus();
    void RefreshYearbookStatus();
    void ApplyMenuOpenState();
    void ApplyYearbookOpenState();
    void RefreshCategoryButtons();
    void RefreshBuildingButtons();
    void SelectCategory(EBuildingCategory Category);
    void MovePage(int DeltaPage);
    void PreviewSlot(int SlotIndex);
    void StartPlacementBySlot(int SlotIndex);
    void RefreshDetailPanel();
    std::vector<int> CollectCategoryEntryIndices() const;

private:
    void OnBuildButtonClick();
    void OnYearbookButtonClick();
    void OnCategoryInfrastructureClick();
    void OnCategoryFoodResourceClick();
    void OnCategoryIndustryClick();
    void OnCategoryHousingClick();
    void OnCategoryEntertainmentClick();
    void OnCategoryMediaEducationClick();
    void OnCategoryTourismClick();
    void OnCategoryPublicServiceClick();
    void OnMenuCloseButtonClick();
    void OnYearbookCloseButtonClick();
    void OnPrevPageClick();
    void OnNextPageClick();
    void OnSlot0Click();
    void OnSlot1Click();
    void OnSlot2Click();
    void OnSlot3Click();
    void OnSlot4Click();
    void OnSlot5Click();
    void OnSlot6Click();
    void OnSlot7Click();
    void OnSlot8Click();
    void OnSlot9Click();
    void OnSlot10Click();
    void OnSlot11Click();
    void OnSlot12Click();
    void OnSlot13Click();
    void OnSlot14Click();
    void OnSlot0Hovered();
    void OnSlot1Hovered();
    void OnSlot2Hovered();
    void OnSlot3Hovered();
    void OnSlot4Hovered();
    void OnSlot5Hovered();
    void OnSlot6Hovered();
    void OnSlot7Hovered();
    void OnSlot8Hovered();
    void OnSlot9Hovered();
    void OnSlot10Hovered();
    void OnSlot11Hovered();
    void OnSlot12Hovered();
    void OnSlot13Hovered();
    void OnSlot14Hovered();
};
