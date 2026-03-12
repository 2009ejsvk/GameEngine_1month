#pragma once

#include "UI/WidgetContainer.h"
#include "../Building/BuildingTypes.h"
#include <vector>

class CTradeWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CTradeWidget();

public:
    virtual ~CTradeWidget();

private:
    using WButton = std::weak_ptr<class CButton>;
    using WImage = std::weak_ptr<class CImage>;
    using WText = std::weak_ptr<class CTextBlock>;

    struct FProposalRowWidgets
    {
        WButton Button;
        WText Direction;
        WText Partner;
        WText Resource;
        WText Margin;
    };

    struct FDetailRowWidgets
    {
        WText Label;
        WText Value;
    };

private:
    WImage mPanelBackground;
    WImage mTitleRibbon;
    WImage mListFrame;
    WImage mDetailFrame;
    WText mTitleText;
    WText mCountdownText;
    WButton mCloseButton;
    std::vector<WButton> mPageButtons;
    std::vector<WText> mPageButtonTexts;
    std::vector<WText> mModifierSectionTitles;
    WButton mFilterButton;
    WText mFilterButtonText;
    std::vector<WButton> mSortButtons;
    std::vector<WText> mSortButtonTexts;
    std::vector<FProposalRowWidgets> mProposalRows;
    WText mDetailTitleText;
    std::vector<FDetailRowWidgets> mDetailRows;
    WText mAmountTitleText;
    std::vector<WButton> mAmountButtons;
    std::vector<WText> mAmountButtonTexts;
    WButton mActionButton;
    WText mActionButtonText;
    WButton mCompletionAutoOpenButton;
    WText mCompletionAutoOpenButtonText;
    WText mFeedbackText;

    bool mOpen = false;
    int mSelectedPageIndex = 0;
    int mSelectedFilterIndex = 0;
    int mSelectedSortIndex = 3;
    bool mSortDescending = true;
    int mSelectedProposalIndex = 0;
    int mSelectedPriceIndex = 0;
    int mSelectedActiveRouteIndex = 0;
    int mSelectedCompletedRouteIndex = 0;
    int mSelectedAmountIndex = 0;
    bool mAutoOpenCompletionPage = true;
    int mLastSeenCompletionNotificationVersion = 0;
    float mPanelWidth = 1180.f;
    float mPanelHeight = 770.f;
    int mLastResolutionWidth = 0;
    int mLastResolutionHeight = 0;
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
    void RefreshFromState();
    void RefreshLayout();
    void OnCloseButtonClick();
    void OnPageButtonClick(int PageIndex);
    void OnFilterButtonClick();
    void OnSortButtonClick(int SortIndex);
    void OnProposalButtonClick(int RowIndex);
    void OnAmountButtonClick(int AmountIndex);
    void OnActionButtonClick();
    void OnCompletionAutoOpenButtonClick();
};
