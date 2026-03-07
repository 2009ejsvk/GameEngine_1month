#pragma once

#include "../Politics/PoliticalTypes.h"
#include "UI/WidgetContainer.h"
#include <string>
#include <vector>

class CEdictWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CEdictWidget();

public:
    virtual ~CEdictWidget();

private:
    std::weak_ptr<class CImage>     mBackground;
    std::weak_ptr<class CImage>     mDetailFrame;
    std::weak_ptr<class CImage>     mDetailIcon;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mSubtitleText;
    std::weak_ptr<class CTextBlock> mDetailTitleText;
    std::weak_ptr<class CTextBlock> mDetailBodyText;
    std::weak_ptr<class CTextBlock> mFeedbackText;
    std::weak_ptr<class CButton>    mApplyButton;
    std::weak_ptr<class CTextBlock> mApplyButtonText;
    std::vector<EGovernmentEdictType> mEdictTypes;
    std::vector<std::weak_ptr<class CButton>> mEdictButtons;
    std::vector<std::weak_ptr<class CImage>> mEdictIcons;
    std::vector<std::weak_ptr<class CTextBlock>> mEdictLabels;
    std::vector<std::weak_ptr<class CTextBlock>> mEdictStatuses;
    bool mOpen = false;
    EGovernmentEdictType mSelectedEdict =
        EGovernmentEdictType::FoodForThePeople;
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
    void SelectEdictByIndex(int Index);
    void OnApplyButtonClick();
    void OnEdictButton0Click();
    void OnEdictButton1Click();
    void OnEdictButton2Click();
    void OnEdictButton3Click();
    void OnEdictButton4Click();
};
