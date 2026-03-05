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

private:
    struct FBuildCatalogEntry
    {
        std::string Id;
        std::wstring DisplayName;
        std::wstring CategoryName;
        bool Residential = false;
        bool FoodProvider = false;
        bool EntertainmentProvider = false;
        int HousingSatisfactionCap = 100;
        int JobSatisfactionCap = 100;
        int FoodSatisfactionCap = 100;
        int FunSatisfactionCap = 100;
        int Capacity = 0;
        int CategoryIndex = 0;
        EPlacementTemplateType TemplateType;
        EPlacementBuildingKind BuildingKind;
    };

private:
    std::weak_ptr<class CButton> mBuildButton;
    std::weak_ptr<class CButton> mDemolishButton;
    std::weak_ptr<class CTextBlock> mDemolishButtonText;
    std::weak_ptr<class CTextBlock> mNpcCountText;
    std::weak_ptr<class CImage> mMenuBackground;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mPageText;
    std::weak_ptr<class CButton> mPrevPageButton;
    std::weak_ptr<class CButton> mNextPageButton;
    std::vector<std::weak_ptr<class CButton>> mCategoryButtons;
    std::vector<std::weak_ptr<class CButton>> mBuildingButtons;
    std::vector<std::weak_ptr<class CTextBlock>> mBuildingButtonTexts;
    std::vector<int> mVisibleEntryIndices;
    bool mMenuOpen = false;
    int mSelectedCategoryIndex = 0;
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
    void ApplyMenuOpenState();
    void RefreshCategoryButtons();
    void RefreshBuildingButtons();
    void SelectCategory(int CategoryIndex);
    void MovePage(int DeltaPage);
    void StartPlacementBySlot(int SlotIndex);
    void SyncDemolitionButtonState();
    std::vector<int> CollectCategoryEntryIndices() const;
    const std::vector<FBuildCatalogEntry>& GetCatalog() const;

private:
    void OnBuildButtonClick();
    void OnDemolishButtonClick();
    void OnCategoryInfrastructureClick();
    void OnCategoryFoodResourceClick();
    void OnCategoryIndustryClick();
    void OnCategoryHousingClick();
    void OnCategoryEntertainmentClick();
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
};
