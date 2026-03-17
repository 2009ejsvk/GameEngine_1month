#pragma once

#include "TopHudWidgetState.h"
#include "UI/WidgetContainer.h"
#include <vector>

class FTopHudRenderer;

class CTopHudWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class FTopHudRenderer;

protected:
    CTopHudWidget();

public:
    virtual ~CTopHudWidget();

private:
    std::weak_ptr<class CImage>     mSpeedPanel;
    std::weak_ptr<class CImage>     mTimeBarBack;
    std::weak_ptr<class CImage>     mTimeBarFill;
    std::weak_ptr<class CImage>     mStatusBar;
    std::weak_ptr<class CImage>     mStatusMoneyIcon;
    std::weak_ptr<class CImage>     mStatusNpcIcon;
    std::weak_ptr<class CImage>     mStatusSupportIcon;
    std::weak_ptr<class CImage>     mStatusResearchIcon;
    std::weak_ptr<class CImage>     mGameOverDim;
    std::weak_ptr<class CImage>     mGameOverPanel;
    std::weak_ptr<class CImage>     mEraTransitionDim;
    std::weak_ptr<class CImage>     mEraTransitionPanel;
    std::weak_ptr<class CTextBlock> mDateText;
    std::weak_ptr<class CTextBlock> mBudgetLabelText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mNpcLabelText;
    std::weak_ptr<class CTextBlock> mNpcText;
    std::weak_ptr<class CTextBlock> mSupportLabelText;
    std::weak_ptr<class CTextBlock> mSupportText;
    std::weak_ptr<class CTextBlock> mResearchLabelText;
    std::weak_ptr<class CTextBlock> mResearchText;
    std::weak_ptr<class CTextBlock> mElectionText;
    std::weak_ptr<class CTextBlock> mTaxPolicyText;
    std::weak_ptr<class CTextBlock> mEventText;
    std::weak_ptr<class CTextBlock> mGameOverTitleText;
    std::weak_ptr<class CTextBlock> mGameOverBodyText;
    std::weak_ptr<class CTextBlock> mEraTransitionTitleText;
    std::weak_ptr<class CTextBlock> mEraTransitionBodyText;
    std::weak_ptr<class CTextBlock> mEraTransitionConfirmButtonText;
    std::weak_ptr<class CTextBlock> mEraTransitionCancelButtonText;
    std::vector<std::weak_ptr<class CButton>> mSpeedButtons;
    std::vector<std::weak_ptr<class CImage>> mSpeedButtonIcons;
    std::vector<std::weak_ptr<class CButton>> mMenuButtons;
    std::vector<std::weak_ptr<class CImage>> mMenuButtonIcons;
    std::vector<std::weak_ptr<class CTextBlock>> mMenuButtonTexts;
    std::weak_ptr<class CButton>    mEraTransitionConfirmButton;
    std::weak_ptr<class CButton>    mEraTransitionCancelButton;
    FTopHudWidgetState mState;

    // Compatibility aliases for existing widget code while callers migrate.
    float& mMonthProgress = mState.MonthProgress;
    bool& mGameLost = mState.GameLost;
    bool& mGameOverMenusClosed = mState.GameOverMenusClosed;
    bool& mManualEraTransitionPopupOpen = mState.ManualEraTransitionPopupOpen;
    bool& mConstitutionPopupActive = mState.ConstitutionPopupActive;
    bool& mEraTransitionPopupOpen = mState.EraTransitionPopupOpen;
    EConstitutionOptionId& mConstitutionConfirmOptionId =
        mState.ConstitutionConfirmOptionId;
    EConstitutionOptionId& mConstitutionCancelOptionId =
        mState.ConstitutionCancelOptionId;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    const FTopHudWidgetState& GetState() const
    {
        return mState;
    }
    FTopHudWidgetState& GetMutableState()
    {
        return mState;
    }

private:
    void RefreshFromState();
    void CloseMenus(
        bool CloseBuildMenu,
        bool CloseAlmanac,
        bool CloseEdicts,
        bool CloseTrade);
    void OnConstructionButtonClick();
    void OnEraTransitionButtonClick();
    void OnEdictsButtonClick();
    void OnAlmanacButtonClick();
    void OnTradeButtonClick();
    void OnEraTransitionConfirmButtonClick();
    void OnEraTransitionCancelButtonClick();
    void OnSpeedStateButtonClick();
    void OnSpeedMultiplierButtonClick();
    void OnAnyButtonClick();
};
