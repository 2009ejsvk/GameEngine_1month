#pragma once

#include "../Politics/PoliticalTypes.h"
#include "UI/WidgetContainer.h"
#include <string>
#include <vector>

enum class EEdictUiCategory
{
    General = 0,
    Interior,
    Defense,
    Education
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
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mPageText;
    std::weak_ptr<class CTextBlock> mDetailTitleText;
    std::weak_ptr<class CTextBlock> mDetailBodyText;
    std::weak_ptr<class CTextBlock> mFeedbackText;
    std::weak_ptr<class CTextBlock> mTaxPolicyTitleText;
    std::weak_ptr<class CTextBlock> mTaxPolicySummaryText;
    std::weak_ptr<class CButton>    mCloseButton;
    std::weak_ptr<class CButton>    mPrevPageButton;
    std::weak_ptr<class CButton>    mNextPageButton;
    std::weak_ptr<class CButton>    mApplyButton;
    std::weak_ptr<class CTextBlock> mApplyButtonText;
    std::vector<std::weak_ptr<class CButton>>    mCategoryButtons;
    std::vector<std::weak_ptr<class CButton>>    mEdictButtons;
    std::vector<std::weak_ptr<class CButton>>    mTaxDecreaseButtons;
    std::vector<std::weak_ptr<class CButton>>    mTaxIncreaseButtons;
    std::vector<std::weak_ptr<class CTextBlock>> mEdictButtonTexts;
    std::vector<std::weak_ptr<class CTextBlock>> mTaxPolicyRowTexts;
    std::vector<int> mVisibleEntryIndices;
    bool  mOpen = false;
    EEdictUiCategory mSelectedCategory = EEdictUiCategory::General;
    int   mPreviewEntryIndex = -1;
    int   mSelectedEntryIndex = -1;
    int   mCurrentPage = 0;
    float mPanelWidth = 656.f;
    float mPanelHeight = 544.f;
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
    void OnCategoryGeneralClick();
    void OnCategoryInteriorClick();
    void OnCategoryDefenseClick();
    void OnCategoryEducationClick();
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
};
