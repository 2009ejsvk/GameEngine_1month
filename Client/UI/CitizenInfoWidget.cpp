#include "CitizenInfoWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    std::wstring Utf8ToWide(const std::string& Text)
    {
        if (Text.empty())
            return std::wstring();

        const int RequiredCount = MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(), -1, nullptr, 0);

        if (RequiredCount <= 1)
        {
            return std::wstring(Text.begin(), Text.end());
        }

        std::wstring WideText;
        WideText.resize(RequiredCount - 1);
        MultiByteToWideChar(
            CP_UTF8, 0, Text.c_str(),
            static_cast<int>(Text.size()),
            &WideText[0], RequiredCount - 1);
        return WideText;
    }
}

CCitizenInfoWidget::CCitizenInfoWidget()
{
}

CCitizenInfoWidget::~CCitizenInfoWidget()
{
}

bool CCitizenInfoWidget::Init()
{
    CWidgetContainer::Init();

    auto PanelImage = CreateWidget<CImage>("CitizenPanel").lock();

    if (PanelImage)
    {
        PanelImage->SetPos(0.f, 0.f);
        PanelImage->SetSize(mPanelWidth, mPanelHeight);
        PanelImage->SetTint(0.08f, 0.08f, 0.08f, 0.88f);
        mPanelImage = PanelImage;
    }

    auto TitleText = CreateWidget<CTextBlock>("CitizenTitle", 1).lock();

    if (TitleText)
    {
        TitleText->SetPos(20.f, 18.f);
        TitleText->SetSize(mPanelWidth - 40.f, 34.f);
        TitleText->SetText(TEXT("Citizen: -"));
        TitleText->SetFontSize(24.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(255, 255, 255, 255);
        mTitleText = TitleText;
    }

    auto BodyText = CreateWidget<CTextBlock>("CitizenBody", 1).lock();

    if (BodyText)
    {
        BodyText->SetPos(20.f, 72.f);
        BodyText->SetSize(mPanelWidth - 40.f, mPanelHeight - 92.f);
        BodyText->SetText(TEXT(
            "Food: -\nHealth: -\nFun: -\nFaith: -\nHousing: -\n"
            "Job: -\nFreedom: -\nSecurity: -\nOverall: -"));
        BodyText->SetFontSize(18.f);
        BodyText->SetAlignH(ETextAlignH::Left);
        BodyText->SetAlignV(ETextAlignV::Top);
        BodyText->SetTextColor(220, 220, 220, 255);
        mBodyText = BodyText;
    }

    SetEnable(false);

    return true;
}

void CCitizenInfoWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
}

void CCitizenInfoWidget::Render()
{
    CWidgetContainer::Render();
}

void CCitizenInfoWidget::OpenCitizen(
    const std::string& CitizenName,
    const FNpcSatisfaction& Satisfaction,
    const FVector2& ScreenPos)
{
    (void)ScreenPos;
    SetPanelScreenPos(FVector2(0.f, 0.f));
    std::wstring WideName(CitizenName.begin(), CitizenName.end());
    SetTitle(L"Citizen: " + WideName);
    SetCitizenSatisfaction(Satisfaction);
    SetEnable(true);
}

void CCitizenInfoWidget::OpenBuilding(
    const std::string& BuildingObjectName,
    const std::string& BuildingDisplayName,
    const std::string& CategoryName,
    bool IsResidential,
    int Capacity,
    const FVector2& ScreenPos)
{
    (void)ScreenPos;
    SetPanelScreenPos(FVector2(0.f, 0.f));

    const std::wstring WideObjectName = Utf8ToWide(
        BuildingObjectName);
    const std::wstring WideDisplayName = BuildingDisplayName.empty() ?
        WideObjectName :
        Utf8ToWide(BuildingDisplayName);
    const std::wstring WideCategoryName = CategoryName.empty() ?
        L"미분류" :
        Utf8ToWide(CategoryName);
    const int SafeCapacity = (std::max)(0, Capacity);

    wchar_t Body[512] = {};

    if (IsResidential)
    {
        swprintf_s(Body,
            L"카테고리: %s\n건물 종류: %s\n거주 가능 인원: %d명",
            WideCategoryName.c_str(),
            WideDisplayName.c_str(),
            SafeCapacity);
    }
    else
    {
        swprintf_s(Body,
            L"카테고리: %s\n건물 종류: %s\n근무 가능 인원: %d명",
            WideCategoryName.c_str(),
            WideDisplayName.c_str(),
            SafeCapacity);
    }

    SetTitle(L"Building: " + WideObjectName);
    SetBodyText(Body);
    SetEnable(true);
}

void CCitizenInfoWidget::SetTitle(const std::wstring& Title)
{
    auto TitleText = mTitleText.lock();

    if (TitleText)
        TitleText->SetText(Title.c_str());
}

void CCitizenInfoWidget::SetCitizenSatisfaction(
    const FNpcSatisfaction& Satisfaction)
{
    auto ToPercent = [](float Value)
    {
        const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
        return (int)roundf(Clamped);
    };

    wchar_t Text[512] = {};

    swprintf_s(Text,
        L"Food: %d\nHealth: %d\nFun: %d\nFaith: %d\nHousing: %d\n"
        L"Job: %d\nFreedom: %d\nSecurity: %d\nOverall: %d",
        ToPercent(Satisfaction.Food),
        ToPercent(Satisfaction.Health),
        ToPercent(Satisfaction.Fun),
        ToPercent(Satisfaction.Faith),
        ToPercent(Satisfaction.Housing),
        ToPercent(Satisfaction.Job),
        ToPercent(Satisfaction.Freedom),
        ToPercent(Satisfaction.Security),
        ToPercent(Satisfaction.Overall));

    SetBodyText(Text);
}

void CCitizenInfoWidget::SetBodyText(const std::wstring& Text)
{
    auto BodyText = mBodyText.lock();

    if (BodyText)
        BodyText->SetText(Text.c_str());
}

void CCitizenInfoWidget::SetPanelScreenPos(const FVector2& ScreenPos)
{
    (void)ScreenPos;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mPanelWidth = (float)Resolution.Width / 3.f;
    mPanelHeight = (float)Resolution.Height;

    SetPos((float)Resolution.Width - mPanelWidth, 0.f);

    auto PanelImage = mPanelImage.lock();
    if (PanelImage)
    {
        PanelImage->SetPos(0.f, 0.f);
        PanelImage->SetSize(mPanelWidth, mPanelHeight);
    }

    auto TitleText = mTitleText.lock();
    if (TitleText)
    {
        TitleText->SetPos(20.f, 18.f);
        TitleText->SetSize(mPanelWidth - 40.f, 34.f);
    }

    auto BodyText = mBodyText.lock();
    if (BodyText)
    {
        BodyText->SetPos(20.f, 72.f);
        BodyText->SetSize(mPanelWidth - 40.f, mPanelHeight - 92.f);
    }
}
