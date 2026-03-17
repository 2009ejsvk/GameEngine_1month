#pragma once

#include "ResultWidgetState.h"
#include "UI/WidgetContainer.h"

class CResultWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CResultWidget();

public:
    virtual ~CResultWidget();

private:
    std::weak_ptr<class CImage> mDimBackground;
    std::weak_ptr<class CImage> mPanelBackground;
    std::weak_ptr<class CImage> mTitleRibbon;
    std::weak_ptr<class CImage> mIconBadge;
    std::weak_ptr<class CTextBlock> mIconText;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mSummaryText;
    std::weak_ptr<class CTextBlock> mPrimaryDetailText;
    std::weak_ptr<class CTextBlock> mSecondaryDetailText;
    std::weak_ptr<class CTextBlock> mTertiaryDetailText;
    std::weak_ptr<class CTextBlock> mQuaternaryDetailText;
    std::weak_ptr<class CButton> mRestartButton;
    std::weak_ptr<class CTextBlock> mRestartButtonText;
    FResultWidgetState mState;

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;
    const FResultWidgetState& GetState() const
    {
        return mState;
    }
    FResultWidgetState& GetMutableState()
    {
        return mState;
    }

private:
    void RefreshFromState();
    void SetPopupVisible(bool Visible);
    void OnRestartClick();
};
