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
    std::weak_ptr<class CTextBlock> mDateText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CTextBlock> mNpcText;
    std::weak_ptr<class CTextBlock> mSupportText;
    std::vector<std::weak_ptr<class CButton>> mSpeedButtons;
    std::vector<std::weak_ptr<class CButton>> mMenuButtons;
    float mMonthProgress = 0.f;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void RefreshData();
    void RefreshLayout();
    void OnConstructionButtonClick();
    void OnAlmanacButtonClick();
    void OnAnyButtonClick();
};
