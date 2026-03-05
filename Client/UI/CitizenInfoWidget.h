#pragma once

#include "UI/WidgetContainer.h"
#include <string>
#include <vector>

struct FNpcSatisfaction;
struct FNpcPoliticalProfile;

class CCitizenInfoWidget :
    public CWidgetContainer
{
    friend class CWorldUIManager;

protected:
    CCitizenInfoWidget();

public:
    virtual ~CCitizenInfoWidget();

private:
    std::weak_ptr<class CImage> mPanelImage;
    std::weak_ptr<class CTextBlock> mTitleText;
    std::weak_ptr<class CTextBlock> mBodyText;
    std::string mTrackedCitizenName;
    std::string mTrackedBuildingName;
    std::weak_ptr<class CTextBlock> mBudgetText;
    std::vector<std::weak_ptr<class CButton>> mBudgetButtons;
    std::vector<std::weak_ptr<class CTextBlock>> mBudgetButtonTexts;
    float mPanelWidth = 260.f;
    float mPanelHeight = 120.f;

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
    void SetTitle(const std::wstring& Title);
    void SetCitizenSatisfaction(
        const FNpcSatisfaction& Satisfaction,
        const FNpcPoliticalProfile& PoliticalProfile);
    void SetBodyText(const std::wstring& Text);
    void SetPanelScreenPos(const FVector2& ScreenPos);
    void SetBudgetControlsVisible(bool Visible);
    void RefreshBuildingInfo();
    void SetBuildingBudgetLevel(int Level);
    void OnBudgetLevel1Click();
    void OnBudgetLevel2Click();
    void OnBudgetLevel3Click();
    void OnBudgetLevel4Click();
    void OnBudgetLevel5Click();
};
