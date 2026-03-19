#pragma once

#include "UI/WidgetContainer.h"
#include "../Politics/PoliticalTypes.h"
#include <string>
#include <vector>

class CTaskWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CTaskWidget();

public:
    virtual ~CTaskWidget();

private:
    using WButton = std::weak_ptr<class CButton>;
    using WImage = std::weak_ptr<class CImage>;
    using WText = std::weak_ptr<class CTextBlock>;

    struct FIssuerTabWidgets
    {
        WButton Button;
        WImage Frame;
        WImage Icon;
    };

    struct FDemandRowWidgets
    {
        WButton Button;
        WImage IconFrame;
        WImage Icon;
        WText Title;
        WText Subtitle;
        WText Counter;
    };

private:
    WImage mPanelBackground;
    WText mTitleText;
    WText mSubtitleText;
    WButton mCloseButton;
    std::vector<FIssuerTabWidgets> mIssuerTabs;
    std::vector<FDemandRowWidgets> mDemandRows;
    WText mEmptyText;
    WText mDetailTitleText;
    WText mDetailMetaText;
    WText mDetailBodyText;
    WText mObjectiveHeaderText;
    WText mObjectiveText;
    WText mRewardHeaderText;
    WText mRewardText;
    WText mPenaltyHeaderText;
    WText mPenaltyText;
    WImage mPortraitCard;
    WImage mPortraitImage;
    WImage mPortraitNameBackdrop;
    WText mPortraitNameText;
    WButton mPrimaryButton;
    WText mPrimaryButtonText;
    WButton mSecondaryButton;
    WText mSecondaryButtonText;
    WText mFeedbackText;

    bool mOpen = false;
    bool mHasSelectedDemand = false;
    bool mSelectedDemandAccepted = false;
    int mSelectedDemandIndex = 0;
    float mPanelWidth = 1180.f;
    float mPanelHeight = 760.f;
    int mLastResolutionWidth = 0;
    int mLastResolutionHeight = 0;
    std::wstring mFeedbackMessage;

public:
    virtual bool Init() override;
    virtual void Update(float DeltaTime) override;
    void ToggleOpen();
    void SetOpen(bool Open);
    void OpenEraTransitionTask();
    void OpenForDemand(
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex);
    bool IsOpen() const
    {
        return mOpen;
    }

private:
    void RefreshLayout();
    void RefreshFromState();
    void OnCloseButtonClick();
    void OnIssuerTabClick(int Index);
    void OnDemandRowClick(int Index);
    void OnPrimaryButtonClick();
    void OnSecondaryButtonClick();
};
