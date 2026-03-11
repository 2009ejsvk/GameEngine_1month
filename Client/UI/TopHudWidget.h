#pragma once

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
    std::weak_ptr<class CImage>     mGameOverDim;
    std::weak_ptr<class CImage>     mGameOverPanel;
    std::weak_ptr<class CTextBlock> mDateText;
    std::weak_ptr<class CTextBlock> mBudgetLabelText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mNpcLabelText;
    std::weak_ptr<class CTextBlock> mNpcText;
    std::weak_ptr<class CTextBlock> mSupportLabelText;
    std::weak_ptr<class CTextBlock> mSupportText;
    std::weak_ptr<class CTextBlock> mElectionText;
    std::weak_ptr<class CTextBlock> mTaxPolicyText;
    std::weak_ptr<class CTextBlock> mEventText;
    std::weak_ptr<class CTextBlock> mGameOverTitleText;
    std::weak_ptr<class CTextBlock> mGameOverBodyText;
    std::vector<std::weak_ptr<class CButton>> mSpeedButtons;
    std::vector<std::weak_ptr<class CImage>> mSpeedButtonIcons;
    std::vector<std::weak_ptr<class CButton>> mMenuButtons;
    std::vector<std::weak_ptr<class CImage>> mMenuButtonIcons;
    std::vector<std::weak_ptr<class CTextBlock>> mMenuButtonTexts;
    float mMonthProgress = 0.f;
    bool mGameLost = false;
    bool mGameOverMenusClosed = false;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void RefreshFromState();
    void CloseMenus(
        bool CloseBuildMenu,
        bool CloseAlmanac,
        bool CloseEdicts);
    void OnConstructionButtonClick();
    void OnEdictsButtonClick();
    void OnAlmanacButtonClick();
    void OnAnyButtonClick();
};
