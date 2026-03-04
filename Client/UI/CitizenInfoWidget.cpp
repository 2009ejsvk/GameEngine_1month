#include "CitizenInfoWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
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

    void ConfigureBudgetButtonStyle(
        const std::shared_ptr<CButton>& Button, bool Selected)
    {
        if (!Button)
            return;

        if (Selected)
        {
            Button->SetTint(EButtonState::Normal,
                FVector4(0.10f, 0.32f, 0.52f, 0.95f));
            Button->SetTint(EButtonState::Hovered,
                FVector4(0.16f, 0.40f, 0.62f, 0.98f));
            Button->SetTint(EButtonState::Click,
                FVector4(0.08f, 0.24f, 0.40f, 0.98f));
            Button->SetTint(EButtonState::Disable,
                FVector4(0.08f, 0.24f, 0.40f, 0.70f));
            return;
        }

        Button->SetTint(EButtonState::Normal,
            FVector4(0.20f, 0.22f, 0.26f, 0.92f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.26f, 0.30f, 0.35f, 0.95f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.14f, 0.16f, 0.20f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.10f, 0.12f, 0.70f));
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

    auto BudgetText = CreateWidget<CTextBlock>(
        "CitizenBudgetText", 1).lock();

    if (BudgetText)
    {
        BudgetText->SetText(TEXT("예산 단계: 3"));
        BudgetText->SetFontSize(18.f);
        BudgetText->SetAlignH(ETextAlignH::Left);
        BudgetText->SetAlignV(ETextAlignV::Middle);
        BudgetText->SetTextColor(220, 220, 220, 255);
        mBudgetText = BudgetText;
    }

    mBudgetButtons.resize(5);
    mBudgetButtonTexts.resize(5);

    void (CCitizenInfoWidget::* BudgetCallbacks[5])() =
    {
        &CCitizenInfoWidget::OnBudgetLevel1Click,
        &CCitizenInfoWidget::OnBudgetLevel2Click,
        &CCitizenInfoWidget::OnBudgetLevel3Click,
        &CCitizenInfoWidget::OnBudgetLevel4Click,
        &CCitizenInfoWidget::OnBudgetLevel5Click
    };

    for (int i = 0; i < 5; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "CitizenBudgetButton_" + std::to_string(i + 1), 1).lock();

        if (!Button)
            continue;

        ConfigureBudgetButtonStyle(Button, i == 2);
        Button->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click, this, BudgetCallbacks[i]);

        auto Text = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenBudgetButtonText_" + std::to_string(i + 1), mWorld);

        if (Text)
        {
            wchar_t Label[8] = {};
            swprintf_s(Label, L"%d", i + 1);
            Text->SetText(Label);
            Text->SetFontSize(16.f);
            Text->SetAlignH(ETextAlignH::Center);
            Text->SetAlignV(ETextAlignV::Middle);
            Text->SetTextColor(245, 245, 245, 255);
            Button->SetChild(Text);
        }

        mBudgetButtons[i] = Button;
        mBudgetButtonTexts[i] = Text;
    }

    SetBudgetControlsVisible(false);

    SetEnable(false);

    return true;
}

void CCitizenInfoWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    (void)DeltaTime;

    if (!GetEnable())
        return;

    if (!mTrackedCitizenName.empty())
    {
        auto World = mWorld.lock();

        if (!World)
            return;

        auto Citizen = World->FindObject<CBuildingMarkerOrb>(
            mTrackedCitizenName).lock();

        if (!Citizen || !Citizen->GetAlive() || !Citizen->GetEnable())
        {
            mTrackedCitizenName.clear();
            SetEnable(false);
            return;
        }

        SetCitizenSatisfaction(Citizen->GetSatisfaction());
        return;
    }

    if (!mTrackedBuildingName.empty())
    {
        RefreshBuildingInfo();
    }
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
    mTrackedCitizenName = CitizenName;
    mTrackedBuildingName.clear();
    SetBudgetControlsVisible(false);
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
    mTrackedCitizenName.clear();
    mTrackedBuildingName = BuildingObjectName;
    SetBudgetControlsVisible(true);

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
    RefreshBuildingInfo();
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

    auto BudgetText = mBudgetText.lock();
    if (BudgetText)
    {
        BudgetText->SetPos(20.f, 60.f);
        BudgetText->SetSize(mPanelWidth - 40.f, 24.f);
    }

    const float BudgetButtonsTop = 90.f;
    const float BudgetGap = 8.f;
    const float BudgetButtonWidth =
        (mPanelWidth - 40.f - BudgetGap * 4.f) / 5.f;

    for (int i = 0; i < static_cast<int>(mBudgetButtons.size()); ++i)
    {
        auto Button = mBudgetButtons[i].lock();

        if (!Button)
            continue;

        Button->SetPos(
            20.f + (BudgetButtonWidth + BudgetGap) * static_cast<float>(i),
            BudgetButtonsTop);
        Button->SetSize(BudgetButtonWidth, 30.f);
    }

    auto BodyText = mBodyText.lock();
    if (BodyText)
    {
        BodyText->SetPos(20.f, 132.f);
        BodyText->SetSize(mPanelWidth - 40.f, mPanelHeight - 152.f);
    }
}

void CCitizenInfoWidget::SetBudgetControlsVisible(bool Visible)
{
    auto BudgetText = mBudgetText.lock();

    if (BudgetText)
        BudgetText->SetEnable(Visible);

    for (int i = 0; i < static_cast<int>(mBudgetButtons.size()); ++i)
    {
        auto Button = mBudgetButtons[i].lock();

        if (!Button)
            continue;

        Button->SetEnable(Visible);
        Button->ButtonEnable(Visible);
    }
}

void CCitizenInfoWidget::RefreshBuildingInfo()
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
    {
        mTrackedBuildingName.clear();
        SetEnable(false);
        return;
    }

    const std::wstring WideObjectName = Utf8ToWide(
        Building->GetName());
    const std::wstring WideDisplayName = Utf8ToWide(
        Building->GetBuildingDisplayName());
    const std::wstring WideCategoryName = Utf8ToWide(
        Building->GetBuildingCategoryName());

    const int SafeCapacity = (std::max)(0, Building->GetCapacity());
    const int HousingCap = Building->GetHousingSatisfactionCap();
    const int JobCap = Building->GetJobSatisfactionCap();
    const int FoodCap = Building->GetFoodSatisfactionCap();
    const int FunCap = Building->GetFunSatisfactionCap();
    const int BudgetLevel = Building->GetBudgetLevel();
    const float BudgetScale = Building->GetBudgetSatisfactionScale();

    auto BudgetText = mBudgetText.lock();
    if (BudgetText)
    {
        wchar_t BudgetBuffer[128] = {};
        swprintf_s(BudgetBuffer,
            L"예산 단계: %d (효율 x%.2f)",
            BudgetLevel, BudgetScale);
        BudgetText->SetText(BudgetBuffer);
    }

    for (int i = 0; i < static_cast<int>(mBudgetButtons.size()); ++i)
    {
        auto Button = mBudgetButtons[i].lock();

        if (!Button)
            continue;

        ConfigureBudgetButtonStyle(Button, BudgetLevel == i + 1);
        Button->ButtonEnable(true);
    }

    wchar_t Body[1024] = {};

    if (Building->IsResidential())
    {
        swprintf_s(Body,
            L"카테고리: %s\n건물 종류: %s\n거주 가능 인원: %d명\n\n"
            L"주거 만족도 상한: %d\n직업 만족도 상한: %d\n"
            L"음식 만족도 상한: %d\n유흥 만족도 상한: %d",
            WideCategoryName.c_str(),
            WideDisplayName.c_str(),
            SafeCapacity,
            HousingCap, JobCap, FoodCap, FunCap);
    }
    else
    {
        swprintf_s(Body,
            L"카테고리: %s\n건물 종류: %s\n근무 가능 인원: %d명\n\n"
            L"주거 만족도 상한: %d\n직업 만족도 상한: %d\n"
            L"음식 만족도 상한: %d\n유흥 만족도 상한: %d",
            WideCategoryName.c_str(),
            WideDisplayName.c_str(),
            SafeCapacity,
            HousingCap, JobCap, FoodCap, FunCap);
    }

    SetTitle(L"Building: " + WideObjectName);
    SetBodyText(Body);
}

void CCitizenInfoWidget::SetBuildingBudgetLevel(int Level)
{
    if (mTrackedBuildingName.empty())
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto Building = World->FindObject<CPlacementAreaObject>(
        mTrackedBuildingName).lock();

    if (!Building || !Building->GetAlive() || !Building->GetEnable())
        return;

    Building->SetBudgetLevel(Level);
    RefreshBuildingInfo();
}

void CCitizenInfoWidget::OnBudgetLevel1Click()
{
    SetBuildingBudgetLevel(1);
}

void CCitizenInfoWidget::OnBudgetLevel2Click()
{
    SetBuildingBudgetLevel(2);
}

void CCitizenInfoWidget::OnBudgetLevel3Click()
{
    SetBuildingBudgetLevel(3);
}

void CCitizenInfoWidget::OnBudgetLevel4Click()
{
    SetBuildingBudgetLevel(4);
}

void CCitizenInfoWidget::OnBudgetLevel5Click()
{
    SetBuildingBudgetLevel(5);
}
