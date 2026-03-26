#pragma once

#include "EventWidgetState.h"
#include "UI/WidgetContainer.h"

class CEventWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CEventWidget();

public:
    virtual ~CEventWidget();

private:
    std::weak_ptr<class CImage> mPanelBackground;
    std::weak_ptr<class CImage> mTitleRibbon;
    std::weak_ptr<class CImage> mIconBadge;
    std::weak_ptr<class CImage> mBodyPanel;
    std::weak_ptr<class CTextBlock> mIconText;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mBodyText;
    std::weak_ptr<class CTextBlock> mAcceptConsequenceText;
    std::weak_ptr<class CTextBlock> mRejectConsequenceText;
    std::weak_ptr<class CButton> mAcceptButton;
    std::weak_ptr<class CButton> mRejectButton;
    std::weak_ptr<class CTextBlock> mAcceptButtonText;
    std::weak_ptr<class CTextBlock> mRejectButtonText;
    std::weak_ptr<class CButton> mCornerCloseButton;
    FEventWidgetState mState;
    bool mAutoPaused = false;
    bool mPopupWasVisible = false;

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;
    const FEventWidgetState& GetState() const
    {
        return mState;
    }
    FEventWidgetState& GetMutableState()
    {
        return mState;
    }

private:
    bool TryRedirectToTaskWidget();
    void RefreshFromState();
    void SetPopupVisible(bool Visible);
    void OnCornerCloseClick();
    void OnAcceptClick();
    void OnRejectClick();
};
