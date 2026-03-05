#pragma once

#include "UI/WidgetContainer.h"
#include "../Map/PlacementAreaObject.h"
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
    struct FCatalogEntryData
    {
        std::string Id;
        std::string DisplayName;
        std::string CategoryName;
        std::string IconTexturePath;
        bool Residential = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        int HousingSatisfactionCap = 100;
        int JobSatisfactionCap = 100;
        int FoodSatisfactionCap = 100;
        int FunSatisfactionCap = 100;
        int Capacity = 0;
        EPlacementTemplateType TemplateType =
            EPlacementTemplateType::Diamond3x3SingleMarker;
        EPlacementBuildingKind BuildingKind =
            EPlacementBuildingKind::BuildingB;
    };
    static bool GetCatalogEntryDataById(
        const std::string& EntryId,
        FCatalogEntryData& OutData);

private:
    struct FBuildCatalogEntry
    {
        std::string Id;
        std::wstring DisplayName;
        std::wstring CategoryName;
        std::wstring DetailText;
        bool Residential = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        int HousingSatisfactionCap = 100;
        int JobSatisfactionCap = 100;
        int FoodSatisfactionCap = 100;
        int FunSatisfactionCap = 100;
        int Capacity = 0;
        int CategoryIndex = 0;
        int CategoryLocalIndex = 0;
        EPlacementTemplateType TemplateType;
        EPlacementBuildingKind BuildingKind;
    };

private:
    std::weak_ptr<class CButton> mBuildButton;
    std::weak_ptr<class CButton> mYearbookButton;
    std::weak_ptr<class CTextBlock> mYearbookButtonText;
    std::weak_ptr<class CTextBlock> mNpcCountText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mDateText;
    std::weak_ptr<class CTextBlock> mDayProgressText;
    std::weak_ptr<class CProgressBar> mDayProgressBar;
    std::weak_ptr<class CImage> mYearbookPanel;
    std::weak_ptr<class CTextBlock> mYearbookTitleText;
    std::weak_ptr<class CTextBlock> mYearbookBodyText;
    std::weak_ptr<class CImage> mMenuBackground;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mPageText;
    std::weak_ptr<class CTextBlock> mDetailTitleText;
    std::weak_ptr<class CTextBlock> mDetailBodyText;
    std::weak_ptr<class CButton> mPrevPageButton;
    std::weak_ptr<class CButton> mNextPageButton;
    std::weak_ptr<class CImage> mTopHudSpeedPanel;
    std::weak_ptr<class CImage> mTopHudTimeBarBack;
    std::weak_ptr<class CImage> mTopHudTimeBarFill;
    std::weak_ptr<class CImage> mTopHudStatusBar;
    std::weak_ptr<class CImage> mTopHudStatusMoneyIcon;
    std::weak_ptr<class CImage> mTopHudStatusNpcIcon;
    std::weak_ptr<class CImage> mTopHudStatusSupportIcon;
    std::weak_ptr<class CTextBlock> mTopHudDateText;
    std::weak_ptr<class CTextBlock> mTopHudClickText;
    std::weak_ptr<class CTextBlock> mTopHudStatusNpcText;
    std::weak_ptr<class CTextBlock> mTopHudStatusSupportText;
    std::vector<std::weak_ptr<class CButton>> mTopHudSpeedButtons;
    std::vector<std::weak_ptr<class CButton>> mTopHudMenuButtons;
    std::vector<std::weak_ptr<class CButton>> mCategoryButtons;
    std::vector<std::weak_ptr<class CButton>> mBuildingButtons;
    std::vector<std::weak_ptr<class CTextBlock>> mBuildingButtonTexts;
    std::vector<int> mVisibleEntryIndices;
    float mTopHudMonthProgress = 0.f;
    bool mMenuOpen = false;
    bool mYearbookOpen = false;
    int mSelectedCategoryIndex = 0;
    int mPreviewEntryIndex = -1;
    int mCurrentPage = 0;
    float mPanelWidth = 656.f;
    float mPanelHeight = 544.f;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();

private:
    void RefreshLayout();
    void RefreshNpcCountText();
    void RefreshEconomyStatus();
    void RefreshYearbookStatus();
    void ApplyMenuOpenState();
    void ApplyYearbookOpenState();
    void RefreshCategoryButtons();
    void RefreshBuildingButtons();
    void SelectCategory(int CategoryIndex);
    void MovePage(int DeltaPage);
    void PreviewSlot(int SlotIndex);
    void StartPlacementBySlot(int SlotIndex);
    void RefreshDetailPanel();
    std::vector<int> CollectCategoryEntryIndices() const;
    static const std::vector<FBuildCatalogEntry>& GetCatalog();

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
    void OnTopHudAnyButtonClick();
};
