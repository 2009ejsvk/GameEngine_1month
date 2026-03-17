#pragma once

#include "TopHudWidgetState.h"
#include "UI/WidgetContainer.h"
#include <string>
#include <vector>

class FTopHudRenderer;
namespace ConstitutionSystem
{
#ifdef _DEBUG
    struct FDebugValidationStep;
#endif
}

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
    std::weak_ptr<class CTextBlock> mPopupRightButtonText;
    std::weak_ptr<class CTextBlock> mPopupLeftButtonText;
    std::weak_ptr<class CTextBlock> mConstitutionRightButtonText;
    std::weak_ptr<class CTextBlock> mConstitutionLeftButtonText;
    std::vector<std::weak_ptr<class CButton>> mSpeedButtons;
    std::vector<std::weak_ptr<class CImage>> mSpeedButtonIcons;
    std::vector<std::weak_ptr<class CButton>> mMenuButtons;
    std::vector<std::weak_ptr<class CImage>> mMenuButtonIcons;
    std::vector<std::weak_ptr<class CTextBlock>> mMenuButtonTexts;
    std::weak_ptr<class CButton>    mPopupRightButton;
    std::weak_ptr<class CButton>    mPopupLeftButton;
    std::weak_ptr<class CButton>    mConstitutionRightButton;
    std::weak_ptr<class CButton>    mConstitutionLeftButton;
    FTopHudWidgetState mState;
#ifdef _DEBUG
    int mDebugPopupRightHandlerEntryCount = 0;
    EConstitutionOptionId mDebugLastPopupRightHandlerOptionId =
        EConstitutionOptionId::None;
#endif

    // Compatibility aliases for existing widget code while callers migrate.
    float& mMonthProgress = mState.MonthProgress;
    bool& mGameLost = mState.GameLost;
    bool& mGameOverMenusClosed = mState.GameOverMenusClosed;
    bool& mManualEraTransitionPopupOpen = mState.ManualEraTransitionPopupOpen;
    bool& mConstitutionPopupActive = mState.ConstitutionPopupActive;
    bool& mEraTransitionPopupOpen = mState.EraTransitionPopupOpen;
    EConstitutionOptionId& mConstitutionLeftOptionId =
        mState.ConstitutionLeftOptionId;
    EConstitutionOptionId& mConstitutionRightOptionId =
        mState.ConstitutionRightOptionId;

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
#ifdef _DEBUG
    bool DebugValidateCurrentConstitutionRightButton(
        const ConstitutionSystem::FDebugValidationStep& Step,
        std::string& OutMessage);
#endif

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
    void OnPopupRightButtonClick();
    void OnPopupLeftButtonClick();
    void OnSpeedStateButtonClick();
    void OnSpeedMultiplierButtonClick();
    void OnAnyButtonClick();
};
