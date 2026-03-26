#pragma once

#include "EdictConstants.h"
#include "EdictWidgetState.h"
#include "../Politics/PoliticalTypes.h"
#include "UI/WidgetContainer.h"
#include <vector>

class FEdictRenderer;

class CEdictWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class FEdictRenderer;

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
    FEdictWidgetState mState;

    // Compatibility aliases for existing renderer/layout code.
    std::vector<int>& mVisibleEntryIndices = mState.VisibleEntryIndices;
    bool mAutoPaused = false;
    bool& mOpen = mState.Open;
    EEdictUiCategory& mSelectedCategory = mState.SelectedCategory;
    int& mPreviewEntryIndex = mState.PreviewEntryIndex;
    int& mSelectedEntryIndex = mState.SelectedEntryIndex;
    int& mCurrentPage = mState.CurrentPage;
    int& mPageCount = mState.PageCount;
    float& mPanelWidth = mState.PanelWidth;
    float& mPanelHeight = mState.PanelHeight;
    bool& mShowTaxPolicyPanel = mState.ShowTaxPolicyPanel;
    std::wstring& mFeedbackMessage = mState.FeedbackMessage;
    int& mLastLayoutWidth = mState.LastLayoutWidth;
    int& mLastLayoutHeight = mState.LastLayoutHeight;
    int& mLastClickedEntryIndex = mState.LastClickedEntryIndex;
    float& mDoubleClickTimer = mState.DoubleClickTimer;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    const FEdictWidgetState& GetState() const
    {
        return mState;
    }
    FEdictWidgetState& GetMutableState()
    {
        return mState;
    }
    void ToggleOpen();
    void SetOpen(bool Open);
    bool IsOpen() const
    {
        return mOpen;
    }
    bool IsMouseOverOpenPanel(const FVector2& MousePos) const;

private:
    void RefreshFromState();
    void SelectCategory(EEdictUiCategory Category);
    void MovePage(int DeltaPage);
    void PreviewSlot(int SlotIndex);
    void ActivateSlot(int SlotIndex);
    void SelectOrApplySlot(int SlotIndex);
    void AdjustTaxPolicy(ETaxPolicyType Type, int DeltaPercent);

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
