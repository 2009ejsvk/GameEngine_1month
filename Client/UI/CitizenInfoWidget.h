#pragma once

#include "UI/WidgetContainer.h"
#include <array>
#include <string>

struct FNpcSatisfaction;
struct FNpcPoliticalProfile;
struct FCitizenIdentityProfile;
struct FBuildingCatalogEntry;

class FCitizenInfoRenderer;

class CCitizenInfoWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;
    friend class FCitizenInfoRenderer;

protected:
    CCitizenInfoWidget();

public:
    virtual ~CCitizenInfoWidget();

private:
    enum class EPanelMode
    {
        Citizen,
        Building
    };

    enum class EBuildingInfoTab
    {
        Overview = 0,
        Statistics,
        Upgrades,
        Efficiency,
        Information,
        Count
    };

    static constexpr int GBuildingTabCount =
        static_cast<int>(EBuildingInfoTab::Count);
    static constexpr int GBudgetLevelCount = 5;

    EPanelMode mPanelMode = EPanelMode::Citizen;
    EBuildingInfoTab mSelectedBuildingTab = EBuildingInfoTab::Overview;

    std::weak_ptr<class CImage> mPanelImage;
    std::weak_ptr<class CImage> mInnerFrame;
    std::weak_ptr<class CImage> mTitleRibbon;
    std::weak_ptr<class CImage> mSectionRibbon;
    std::weak_ptr<class CImage> mScrollTrack;
    std::weak_ptr<class CImage> mScrollThumb;
    std::weak_ptr<class CImage> mTitleIcon;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mSubtitleText;
    std::weak_ptr<class CTextBlock> mPageTitleText;
    std::weak_ptr<class CTextBlock> mBodyText;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::weak_ptr<class CButton> mCloseButton;
    std::weak_ptr<class CButton> mDemolishButton;
    std::weak_ptr<class CButton> mMoveButton;
    std::weak_ptr<class CButton> mCloneButton;
    std::array<std::weak_ptr<class CButton>, GBuildingTabCount> mTabButtons;
    std::array<std::weak_ptr<class CTextBlock>, GBuildingTabCount>
        mTabButtonTexts;
    std::array<std::weak_ptr<class CButton>, GBudgetLevelCount> mBudgetButtons;
    std::array<std::weak_ptr<class CTextBlock>, GBudgetLevelCount>
        mBudgetButtonTexts;
    std::string mTrackedCitizenName;
    std::string mTrackedBuildingName;
    float mPanelWidth = 360.f;
    float mPanelHeight = 720.f;
    float mPanelTop = 56.f;
    FVector2 mRequestedScreenPos = FVector2(0.f, 0.f);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();

public:
    void OpenCitizen(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FVector2& ScreenPos);
    void OpenBuilding(
        const std::string& BuildingObjectName,
        const std::string& BuildingDisplayName,
        const std::string& CategoryName,
        bool IsResidential,
        int Capacity,
        const FVector2& ScreenPos);

private:
    void RefreshFromState();
    void SelectBuildingTab(EBuildingInfoTab Tab);
    void SetBuildingBudgetLevel(int Level);
    void OnCloseButtonClick();
    void OnDemolishButtonClick();
    void OnMoveButtonClick();
    void OnCloneButtonClick();
    void OnBudgetLevel1Click();
    void OnBudgetLevel2Click();
    void OnBudgetLevel3Click();
    void OnBudgetLevel4Click();
    void OnBudgetLevel5Click();
};
