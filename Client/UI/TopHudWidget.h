#pragma once

#include "UI/WidgetContainer.h"
#include <vector>

class CTopHudWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

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
    std::weak_ptr<class CTextBlock> mElectionText;
    std::weak_ptr<class CTextBlock> mTaxPolicyText;
    std::weak_ptr<class CTextBlock> mEventText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mNpcText;
    std::weak_ptr<class CTextBlock> mSupportText;
    std::weak_ptr<class CTextBlock> mGameOverTitleText;
    std::weak_ptr<class CTextBlock> mGameOverBodyText;
    std::vector<std::weak_ptr<class CButton>> mSpeedButtons;
    std::vector<std::weak_ptr<class CButton>> mMenuButtons;
    float mMonthProgress = 0.f;
    bool mGameOverMenusClosed = false;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void RefreshData();
    void RefreshLayout();
    void CloseMenus(
        bool CloseBuildMenu,
        bool CloseAlmanac,
        bool CloseEdicts);
    void OnConstructionButtonClick();
    void OnEdictsButtonClick();
    void OnAlmanacButtonClick();
    void OnAnyButtonClick();
};
