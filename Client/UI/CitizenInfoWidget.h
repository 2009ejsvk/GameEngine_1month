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
    void Open(
        const std::string& CitizenName,
        const FNpcSatisfaction& Satisfaction,
        const FVector2& ScreenPos);

private:
    void SetCitizenName(const std::string& CitizenName);
    void SetSatisfaction(const FNpcSatisfaction& Satisfaction);
    void SetPanelScreenPos(const FVector2& ScreenPos);
};
