#include "CitizenInfoWidget.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Map/PlacementAreaObject.h"
#include "../World/MainWorld.h"
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

    auto CloseButton = CreateWidget<CButton>("CitizenCloseButton", 1).lock();

    if (CloseButton)
    {
        ConfigureBudgetButtonStyle(CloseButton, false);
        CloseButton->SetEventCallback<CCitizenInfoWidget>(
            EButtonEventState::Click, this,
            &CCitizenInfoWidget::OnCloseButtonClick);

        auto CloseText = CWidget::CreateStaticWidget<CTextBlock>(
            "CitizenCloseButtonText", mWorld);

        if (CloseText)
        {
            CloseText->SetText(TEXT("X"));
            CloseText->SetFontSize(16.f);
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(245, 245, 245, 255);
            CloseButton->SetChild(CloseText);
        }

        mCloseButton = CloseButton;
    }

    auto BodyText = CreateWidget<CTextBlock>("CitizenBody", 1).lock();

    if (BodyText)
    {
        BodyText->SetPos(20.f, 72.f);
        BodyText->SetSize(mPanelWidth - 40.f, mPanelHeight - 92.f);
        BodyText->SetText(TEXT(
            "Food: -\nHealth: -\nFun: -\nFaith: -\nHousing: -\n"
            "Job: -\nFreedom: -\nSecurity: -\nOverall: -\n\n"
            "경제: - (-)\n신념: - (-)\n산업: - (-)\n사회: - (-)"));
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

        SetCitizenSatisfaction(
            Citizen->GetSatisfaction(),
            Citizen->GetPoliticalProfile());
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

    FNpcPoliticalProfile PoliticalProfile;
    auto World = mWorld.lock();

    if (World)
    {
        auto Citizen = World->FindObject<CBuildingMarkerOrb>(
            CitizenName).lock();

        if (Citizen && Citizen->GetAlive() && Citizen->GetEnable())
            PoliticalProfile = Citizen->GetPoliticalProfile();
    }

    SetCitizenSatisfaction(Satisfaction, PoliticalProfile);
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
    const FNpcSatisfaction& Satisfaction,
    const FNpcPoliticalProfile& PoliticalProfile)
{
    auto ToPercent = [](float Value)
    {
        const float Clamped = (std::max)(0.f, (std::min)(100.f, Value));
        return (int)roundf(Clamped);
    };

    wchar_t Text[1024] = {};

    swprintf_s(Text,
        L"Food: %d\nHealth: %d\nFun: %d\nFaith: %d\nHousing: %d\n"
        L"Job: %d\nFreedom: %d\nSecurity: %d\nOverall: %d\n\n"
        L"%s: %s (%s)\n"
        L"%s: %s (%s)\n"
        L"%s: %s (%s)\n"
        L"%s: %s (%s)",
        ToPercent(Satisfaction.Food),
        ToPercent(Satisfaction.Health),
        ToPercent(Satisfaction.Fun),
        ToPercent(Satisfaction.Faith),
        ToPercent(Satisfaction.Housing),
        ToPercent(Satisfaction.Job),
        ToPercent(Satisfaction.Freedom),
        ToPercent(Satisfaction.Security),
        ToPercent(Satisfaction.Overall),
        GetPoliticalAxisDisplayName(EPoliticalAxis::Economy),
        GetPoliticalFactionDisplayName(
            EPoliticalAxis::Economy,
            PoliticalProfile.Economy.Stance),
        GetPoliticalSupportDisplayName(
            PoliticalProfile.Economy.Support),
        GetPoliticalAxisDisplayName(EPoliticalAxis::ReligionMilitarism),
        GetPoliticalFactionDisplayName(
            EPoliticalAxis::ReligionMilitarism,
            PoliticalProfile.ReligionMilitarism.Stance),
        GetPoliticalSupportDisplayName(
            PoliticalProfile.ReligionMilitarism.Support),
        GetPoliticalAxisDisplayName(EPoliticalAxis::EnvironmentIndustry),
        GetPoliticalFactionDisplayName(
            EPoliticalAxis::EnvironmentIndustry,
            PoliticalProfile.EnvironmentIndustry.Stance),
        GetPoliticalSupportDisplayName(
            PoliticalProfile.EnvironmentIndustry.Support),
        GetPoliticalAxisDisplayName(EPoliticalAxis::IntellectualConservative),
        GetPoliticalFactionDisplayName(
            EPoliticalAxis::IntellectualConservative,
            PoliticalProfile.IntellectualConservative.Stance),
        GetPoliticalSupportDisplayName(
            PoliticalProfile.IntellectualConservative.Support));

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
    SetSize(mPanelWidth, mPanelHeight);

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
        TitleText->SetSize(mPanelWidth - 80.f, 34.f);
    }

    auto CloseButton = mCloseButton.lock();
    if (CloseButton)
    {
        CloseButton->SetPos(mPanelWidth - 52.f, 14.f);
        CloseButton->SetSize(32.f, 32.f);
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
    }

    std::wstring Body = L"카테고리: " + WideCategoryName +
        L"\n건물 종류: " + WideDisplayName + L"\n";

    if (Building->IsResidential())
        Body += L"거주 가능 인원: " + std::to_wstring(SafeCapacity) + L"명";
    else
        Body += L"근무 가능 인원: " + std::to_wstring(SafeCapacity) + L"명";

    const bool FoodProvider = Building->IsFoodProvider();
    const bool EntertainmentProvider =
        Building->IsEntertainmentProvider();
    const bool WorkProvider = !Building->IsResidential() &&
        (!EntertainmentProvider || FoodProvider);
    const bool UsesResourceStock =
        (WorkProvider && !Building->IsTransportOffice()) ||
        FoodProvider;
    int DaysInMonth = 30;

    auto MainWorld = std::dynamic_pointer_cast<CMainWorld>(World);

    if (MainWorld)
        DaysInMonth = (std::max)(1, MainWorld->GetSimulationMonthDayCount());

    const int MonthlyWageCost = Building->GetMonthlyWageCost();
    const int MonthlyUpkeepCost = Building->GetMonthlyUpkeepCost();
    const int DailyWageCost = Building->GetDailyWageCost(DaysInMonth);
    const int DailyUpkeepCost = Building->GetDailyUpkeepCost(DaysInMonth);

    if (UsesResourceStock)
    {
        Body += L"\nResource stock: " +
            std::to_wstring(Building->GetResourceStock()) +
            L"/" + std::to_wstring(Building->GetMaxResourceStock());
    }

    if (Building->IsHarbor())
    {
        const int ShipProgressPercent = static_cast<int>(roundf(
            Building->GetHarborShipProgressPercent() * 100.f));
        Body += L"\n선박 도착 진행: " +
            std::to_wstring(ShipProgressPercent) + L"%";
    }

    Body += L"\n월급(월): $" + std::to_wstring(MonthlyWageCost);
    Body += L"\n유지비(월): $" + std::to_wstring(MonthlyUpkeepCost);
    Body += L"\n월급(일): $" + std::to_wstring(DailyWageCost);
    Body += L"\n유지비(일): $" + std::to_wstring(DailyUpkeepCost);

    auto SummarizeNames = [&](const std::vector<std::string>& Names)
        -> std::wstring
    {
        if (Names.empty())
            return L"-";

        constexpr size_t GMaxShownNames = 8;
        const size_t CountToShow = (std::min)(Names.size(), GMaxShownNames);
        std::wstring Summary;

        for (size_t i = 0; i < CountToShow; ++i)
        {
            if (i > 0)
                Summary += L", ";

            Summary += Utf8ToWide(Names[i]);
        }

        if (Names.size() > GMaxShownNames)
        {
            Summary += L" (+";
            Summary += std::to_wstring(Names.size() - GMaxShownNames);
            Summary += L" more)";
        }

        return Summary;
    };

    std::vector<std::string> AssignedEmployees;
    std::vector<std::string> WorkingEmployees;
    std::vector<std::string> ArrivedGuests;
    std::vector<std::string> IncomingGuests;
    std::vector<std::string> ListedGuests;
    std::vector<std::string> ConsumingGuests;
    std::vector<std::string> WaitingGuests;
    const std::string BuildingName = Building->GetName();

    if (WorkProvider || FoodProvider)
    {
        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
        {
            for (size_t i = 0; i < OrbList.size(); ++i)
            {
                auto Orb = OrbList[i].lock();

                if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                    continue;

                const std::string& OrbName = Orb->GetName();

                if (WorkProvider && Orb->GetWorkBuilding() == BuildingName)
                {
                    AssignedEmployees.push_back(OrbName);

                    if (Orb->GetCitizenState() == ECitizenState::AtWork)
                        WorkingEmployees.push_back(OrbName);
                }

                if (FoodProvider)
                {
                    const ECitizenState OrbState = Orb->GetCitizenState();

                    if (OrbState == ECitizenState::AtFood)
                    {
                        const std::string& VisitFoodBuilding =
                            Orb->GetFoodVisitBuilding();
                        const bool IsVisitBuildingMatched =
                            VisitFoodBuilding == BuildingName ||
                            (VisitFoodBuilding.empty() &&
                                Orb->GetFoodBuilding() == BuildingName);

                        if (IsVisitBuildingMatched)
                        {
                            ArrivedGuests.push_back(OrbName);
                            ListedGuests.push_back(OrbName);

                            if (Orb->IsConsumingFoodThisVisit())
                                ConsumingGuests.push_back(OrbName);
                            else
                                WaitingGuests.push_back(OrbName);
                        }
                    }
                    else if (OrbState == ECitizenState::GoingToFood &&
                        Orb->GetFoodBuilding() == BuildingName)
                    {
                        FVector3 MarkerPos = FVector3::Zero;

                        if (Building->GetClosestMarkerWorldPos(
                            Orb->GetWorldPos(), MarkerPos))
                        {
                            FVector3 OrbPos = Orb->GetWorldPos();
                            OrbPos.z = MarkerPos.z;

                            const float NearDistance =
                                (std::max)(8.f,
                                    Orb->GetArrivalDistance() * 2.f);

                            if (OrbPos.Distance(MarkerPos) <= NearDistance)
                            {
                                IncomingGuests.push_back(OrbName);
                                ListedGuests.push_back(OrbName);
                            }
                        }
                    }
                }
            }
        }

        std::sort(AssignedEmployees.begin(), AssignedEmployees.end());
        std::sort(WorkingEmployees.begin(), WorkingEmployees.end());
        std::sort(ArrivedGuests.begin(), ArrivedGuests.end());
        std::sort(IncomingGuests.begin(), IncomingGuests.end());
        std::sort(ListedGuests.begin(), ListedGuests.end());
        std::sort(ConsumingGuests.begin(), ConsumingGuests.end());
        std::sort(WaitingGuests.begin(), WaitingGuests.end());
    }

    if (WorkProvider)
    {
        Body += L"\nEmployees: " + std::to_wstring(AssignedEmployees.size()) +
            L"/" + std::to_wstring((std::max)(0, SafeCapacity));
        Body += L"\nAssigned: " + SummarizeNames(AssignedEmployees);
        Body += L"\nWorking now: " + SummarizeNames(WorkingEmployees);
    }

    if (FoodProvider)
    {
        Body += L"\nGuests at building: " + std::to_wstring(ArrivedGuests.size());
        Body += L"\nIncoming(near): " + SummarizeNames(IncomingGuests);
        Body += L"\nGuest list: " + SummarizeNames(ListedGuests);
        Body += L"\nConsuming food: " + SummarizeNames(ConsumingGuests);
        Body += L"\nWaiting(no stock): " + SummarizeNames(WaitingGuests);
    }

    std::vector<std::wstring> CapLines;

    if (Building->IsResidential())
    {
        CapLines.push_back(
            L"주거 만족도 상한: " + std::to_wstring(HousingCap));
    }
    else
    {
        if (WorkProvider)
        {
            CapLines.push_back(
                L"직업 만족도 상한: " + std::to_wstring(JobCap));
        }

        if (FoodProvider)
        {
            CapLines.push_back(
                L"음식 만족도 상한: " + std::to_wstring(FoodCap));
        }

        if (EntertainmentProvider)
        {
            CapLines.push_back(
                L"유흥 만족도 상한: " + std::to_wstring(FunCap));
        }
    }

    if (!CapLines.empty())
    {
        Body += L"\n\n";

        for (size_t i = 0; i < CapLines.size(); ++i)
        {
            if (i > 0)
                Body += L"\n";

            Body += CapLines[i];
        }
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

void CCitizenInfoWidget::OnCloseButtonClick()
{
    mTrackedCitizenName.clear();
    mTrackedBuildingName.clear();
    SetEnable(false);
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
