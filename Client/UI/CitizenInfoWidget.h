#pragma once

#include "UI/WidgetContainer.h"
#include <string>

struct FNpcSatisfaction;

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
    void SetCitizenSatisfaction(const FNpcSatisfaction& Satisfaction);
    void SetBodyText(const std::wstring& Text);
    void SetPanelScreenPos(const FVector2& ScreenPos);
};
