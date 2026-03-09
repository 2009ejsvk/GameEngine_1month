#pragma once

#include "../Politics/PoliticalTypes.h"
#include "UI/WidgetContainer.h"
#include <string>
#include <vector>

enum class EEdictUiCategory
{
    Colonial = 0,
    WorldWars,
    ColdWar,
    Modern
};

class CEdictWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CEdictWidget();

public:
    virtual ~CEdictWidget();

private:
    std::weak_ptr<class CImage>     mMenuBackground;
    std::weak_ptr<class CImage>     mMenuTitleRibbon;
    std::weak_ptr<class CImage>     mMenuGridFrame;
    std::weak_ptr<class CImage>     mMenuDetailFrame;
    std::weak_ptr<class CImage>     mDetailInfoPanel;
    std::weak_ptr<class CImage>     mTaxInfoPanel;
    std::weak_ptr<class CImage>     mDetailCostIcon;
    std::weak_ptr<class CImage>     mScrollTrack;
    std::weak_ptr<class CImage>     mScrollThumb;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mPageText;
    std::weak_ptr<class CTextBlock> mDetailTitleText;
    std::weak_ptr<class CTextBlock> mDetailCostText;
    std::weak_ptr<class CTextBlock> mDetailBodyText;
    std::weak_ptr<class CTextBlock> mDetailInfoText;
    std::weak_ptr<class CTextBlock> mFeedbackText;
    std::weak_ptr<class CTextBlock> mRequirementText;
    std::weak_ptr<class CTextBlock> mTaxPolicyTitleText;
    std::weak_ptr<class CTextBlock> mTaxPolicySummaryText;
    std::weak_ptr<class CButton>    mCloseButton;
    std::weak_ptr<class CButton>    mPrevPageButton;
    std::weak_ptr<class CButton>    mNextPageButton;
    std::weak_ptr<class CButton>    mApplyButton;
    std::weak_ptr<class CTextBlock> mApplyButtonText;
    std::vector<std::weak_ptr<class CButton>>    mCategoryButtons;
    std::vector<std::weak_ptr<class CImage>>     mCategoryButtonIcons;
    std::vector<std::weak_ptr<class CButton>>    mEdictButtons;
    std::vector<std::weak_ptr<class CButton>>    mTaxDecreaseButtons;
    std::vector<std::weak_ptr<class CButton>>    mTaxIncreaseButtons;
    std::vector<std::weak_ptr<class CImage>>     mEdictButtonIcons;
    std::vector<std::weak_ptr<class CImage>>     mEdictButtonStarLefts;
    std::vector<std::weak_ptr<class CImage>>     mEdictButtonStarRights;
    std::vector<std::weak_ptr<class CImage>>     mEdictButtonChecks;
    std::vector<std::weak_ptr<class CTextBlock>> mEdictButtonTexts;
    std::vector<std::weak_ptr<class CTextBlock>> mTaxPolicyRowTexts;
    std::vector<int> mVisibleEntryIndices;
    bool  mOpen = false;
    EEdictUiCategory mSelectedCategory = EEdictUiCategory::Colonial;
    int   mPreviewEntryIndex = -1;
    int   mSelectedEntryIndex = -1;
    int   mCurrentPage = 0;
    float mPanelWidth = 1120.f;
    float mPanelHeight = 760.f;
    std::wstring mFeedbackMessage;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    void ToggleOpen();
    void SetOpen(bool Open);
    bool IsOpen() const
    {
        return mOpen;
    }
    bool IsMouseOverOpenPanel(const FVector2& MousePos) const;

private:
    void RefreshLayout();
    void RefreshData();
    void ApplyOpenState();
    void RefreshCategoryButtons();
    void RefreshEdictButtons();
    void RefreshTaxPolicyControls();
    void SelectCategory(EEdictUiCategory Category);
    void MovePage(int DeltaPage);
    void PreviewSlot(int SlotIndex);
    void ActivateSlot(int SlotIndex);
    void AdjustTaxPolicy(ETaxPolicyType Type, int DeltaPercent);
    void RefreshDetailPanel();
    std::vector<int> CollectCategoryEntryIndices() const;

private:
    void OnPrevPageClick();
    void OnNextPageClick();
    void OnCloseButtonClick();
    void OnApplyButtonClick();
    void OnConsumptionTaxDownClick();
    void OnConsumptionTaxUpClick();
    void OnIncomeTaxDownClick();
    void OnIncomeTaxUpClick();
    void OnPropertyTaxDownClick();
    void OnPropertyTaxUpClick();
};
