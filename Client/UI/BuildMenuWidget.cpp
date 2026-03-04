#include "BuildMenuWidget.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/BuildingMarkerOrb.h"
#include "../Player/MainCamera.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "World/World.h"
#include <Windows.h>
#include <algorithm>
#include <cwchar>

namespace
{
    constexpr int CategoryCount = 5;
    constexpr int SlotsPerPage = 12;
    constexpr int SlotColumnCount = 4;
    constexpr int SlotRowCount = 3;

    const wchar_t* CategoryLabels[CategoryCount] =
    {
        L"교통 및 기반시설",
        L"음식 및 자원",
        L"산업",
        L"주거지",
        L"유흥"
    };

    std::string WideToUtf8(const std::wstring& Text)
    {
        if (Text.empty())
            return std::string();

        const int RequiredBytes = WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            nullptr, 0, nullptr, nullptr);

        if (RequiredBytes <= 0)
        {
            std::string Fallback;
            Fallback.reserve(Text.size());

            for (size_t i = 0; i < Text.size(); ++i)
            {
                const wchar_t Ch = Text[i];

                if (Ch >= 0 && Ch <= 0x7f)
                    Fallback.push_back(static_cast<char>(Ch));
                else
                    Fallback.push_back('?');
            }

            return Fallback;
        }

        std::string Utf8;
        Utf8.resize(RequiredBytes);
        WideCharToMultiByte(
            CP_UTF8, 0, Text.c_str(), static_cast<int>(Text.size()),
            &Utf8[0], RequiredBytes, nullptr, nullptr);
        return Utf8;
    }

    void ConfigureDefaultButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.20f, 0.22f, 0.26f, 0.92f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.26f, 0.30f, 0.35f, 0.95f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.14f, 0.16f, 0.20f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.10f, 0.10f, 0.12f, 0.70f));
    }

    void ConfigureHighlightedButtonStyle(
        const std::shared_ptr<CButton>& Button)
    {
        if (!Button)
            return;

        Button->SetTint(EButtonState::Normal,
            FVector4(0.10f, 0.32f, 0.52f, 0.95f));
        Button->SetTint(EButtonState::Hovered,
            FVector4(0.16f, 0.40f, 0.62f, 0.98f));
        Button->SetTint(EButtonState::Click,
            FVector4(0.08f, 0.24f, 0.40f, 0.98f));
        Button->SetTint(EButtonState::Disable,
            FVector4(0.08f, 0.24f, 0.40f, 0.70f));
    }
}

CBuildMenuWidget::CBuildMenuWidget()
{
}

CBuildMenuWidget::~CBuildMenuWidget()
{
}

bool CBuildMenuWidget::Init()
{
    CWidgetContainer::Init();

    mVisibleEntryIndices.assign(SlotsPerPage, -1);

    auto BuildButton = CreateWidget<CButton>("BuildMenu_BuildButton", 10).lock();

    if (BuildButton)
    {
        ConfigureHighlightedButtonStyle(BuildButton);
        BuildButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnBuildButtonClick);

        auto BuildButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_BuildButtonText", mWorld);

        if (BuildButtonText)
        {
            BuildButtonText->SetText(TEXT("건축"));
            BuildButtonText->SetFontSize(24.f);
            BuildButtonText->SetAlignH(ETextAlignH::Center);
            BuildButtonText->SetAlignV(ETextAlignV::Middle);
            BuildButtonText->SetTextColor(255, 255, 255, 255);
            BuildButtonText->EnableShadow(true);
            BuildButtonText->SetShadowOffset(1.f, 1.f);
            BuildButtonText->SetShadowTextColor(40, 40, 40, 255);
            BuildButton->SetChild(BuildButtonText);
        }

        mBuildButton = BuildButton;
    }

    auto DemolishButton =
        CreateWidget<CButton>("BuildMenu_DemolishButton", 10).lock();

    if (DemolishButton)
    {
        ConfigureDefaultButtonStyle(DemolishButton);
        DemolishButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnDemolishButtonClick);

        auto DemolishButtonText =
            CWidget::CreateStaticWidget<CTextBlock>(
                "BuildMenu_DemolishButtonText", mWorld);

        if (DemolishButtonText)
        {
            DemolishButtonText->SetText(TEXT("철거 OFF"));
            DemolishButtonText->SetFontSize(24.f);
            DemolishButtonText->SetAlignH(ETextAlignH::Center);
            DemolishButtonText->SetAlignV(ETextAlignV::Middle);
            DemolishButtonText->SetTextColor(255, 255, 255, 255);
            DemolishButtonText->EnableShadow(true);
            DemolishButtonText->SetShadowOffset(1.f, 1.f);
            DemolishButtonText->SetShadowTextColor(40, 40, 40, 255);
            DemolishButton->SetChild(DemolishButtonText);
            mDemolishButtonText = DemolishButtonText;
        }

        mDemolishButton = DemolishButton;
    }

    auto NpcCountText = CreateWidget<CTextBlock>("BuildMenu_NpcCount", 11).lock();

    if (NpcCountText)
    {
        NpcCountText->SetText(TEXT("NPC: 0"));
        NpcCountText->SetFontSize(20.f);
        NpcCountText->SetAlignH(ETextAlignH::Left);
        NpcCountText->SetAlignV(ETextAlignV::Middle);
        NpcCountText->SetTextColor(245, 245, 245, 255);
        NpcCountText->EnableShadow(true);
        NpcCountText->SetShadowOffset(1.f, 1.f);
        NpcCountText->SetShadowTextColor(20, 20, 20, 255);
        mNpcCountText = NpcCountText;
    }

    auto MenuBackground = CreateWidget<CImage>("BuildMenu_Background", 6).lock();

    if (MenuBackground)
    {
        MenuBackground->SetTint(0.07f, 0.08f, 0.10f, 0.92f);
        mMenuBackground = MenuBackground;
    }

    auto TitleText = CreateWidget<CTextBlock>("BuildMenu_Title", 7).lock();

    if (TitleText)
    {
        TitleText->SetText(TEXT("건물 선택"));
        TitleText->SetFontSize(28.f);
        TitleText->SetAlignH(ETextAlignH::Left);
        TitleText->SetAlignV(ETextAlignV::Middle);
        TitleText->SetTextColor(245, 245, 245, 255);
        TitleText->EnableShadow(true);
        TitleText->SetShadowOffset(1.f, 1.f);
        TitleText->SetShadowTextColor(20, 20, 20, 255);
        mTitleText = TitleText;
    }

    auto PageText = CreateWidget<CTextBlock>("BuildMenu_PageText", 7).lock();

    if (PageText)
    {
        PageText->SetText(TEXT("1 / 1"));
        PageText->SetFontSize(18.f);
        PageText->SetAlignH(ETextAlignH::Center);
        PageText->SetAlignV(ETextAlignV::Middle);
        PageText->SetTextColor(220, 220, 220, 255);
        mPageText = PageText;
    }

    auto PrevPageButton = CreateWidget<CButton>("BuildMenu_PrevPage", 7).lock();

    if (PrevPageButton)
    {
        ConfigureDefaultButtonStyle(PrevPageButton);
        PrevPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnPrevPageClick);

        auto PrevText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_PrevText", mWorld);

        if (PrevText)
        {
            PrevText->SetText(TEXT("<"));
            PrevText->SetFontSize(20.f);
            PrevText->SetAlignH(ETextAlignH::Center);
            PrevText->SetAlignV(ETextAlignV::Middle);
            PrevText->SetTextColor(255, 255, 255, 255);
            PrevPageButton->SetChild(PrevText);
        }

        mPrevPageButton = PrevPageButton;
    }

    auto NextPageButton = CreateWidget<CButton>("BuildMenu_NextPage", 7).lock();

    if (NextPageButton)
    {
        ConfigureDefaultButtonStyle(NextPageButton);
        NextPageButton->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this,
            &CBuildMenuWidget::OnNextPageClick);

        auto NextText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_NextText", mWorld);

        if (NextText)
        {
            NextText->SetText(TEXT(">"));
            NextText->SetFontSize(20.f);
            NextText->SetAlignH(ETextAlignH::Center);
            NextText->SetAlignV(ETextAlignV::Middle);
            NextText->SetTextColor(255, 255, 255, 255);
            NextPageButton->SetChild(NextText);
        }

        mNextPageButton = NextPageButton;
    }

    mCategoryButtons.resize(CategoryCount);
    void (CBuildMenuWidget::* CategoryCallbacks[CategoryCount])() =
    {
        &CBuildMenuWidget::OnCategoryInfrastructureClick,
        &CBuildMenuWidget::OnCategoryFoodResourceClick,
        &CBuildMenuWidget::OnCategoryIndustryClick,
        &CBuildMenuWidget::OnCategoryHousingClick,
        &CBuildMenuWidget::OnCategoryEntertainmentClick
    };

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Category_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureDefaultButtonStyle(Button);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, CategoryCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_CategoryText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(CategoryLabels[i]);
            ButtonText->SetFontSize(16.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Middle);
            ButtonText->SetTextColor(245, 245, 245, 255);
            Button->SetChild(ButtonText);
        }

        mCategoryButtons[i] = Button;
    }

    mBuildingButtons.resize(SlotsPerPage);
    mBuildingButtonTexts.resize(SlotsPerPage);

    void (CBuildMenuWidget::* SlotCallbacks[SlotsPerPage])() =
    {
        &CBuildMenuWidget::OnSlot0Click,
        &CBuildMenuWidget::OnSlot1Click,
        &CBuildMenuWidget::OnSlot2Click,
        &CBuildMenuWidget::OnSlot3Click,
        &CBuildMenuWidget::OnSlot4Click,
        &CBuildMenuWidget::OnSlot5Click,
        &CBuildMenuWidget::OnSlot6Click,
        &CBuildMenuWidget::OnSlot7Click,
        &CBuildMenuWidget::OnSlot8Click,
        &CBuildMenuWidget::OnSlot9Click,
        &CBuildMenuWidget::OnSlot10Click,
        &CBuildMenuWidget::OnSlot11Click
    };

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        auto Button = CreateWidget<CButton>(
            "BuildMenu_Slot_" + std::to_string(i + 1), 7).lock();

        if (!Button)
            continue;

        ConfigureDefaultButtonStyle(Button);
        Button->SetEventCallback<CBuildMenuWidget>(
            EButtonEventState::Click, this, SlotCallbacks[i]);

        auto ButtonText = CWidget::CreateStaticWidget<CTextBlock>(
            "BuildMenu_SlotText_" + std::to_string(i + 1), mWorld);

        if (ButtonText)
        {
            ButtonText->SetText(TEXT("-"));
            ButtonText->SetFontSize(18.f);
            ButtonText->SetAlignH(ETextAlignH::Center);
            ButtonText->SetAlignV(ETextAlignV::Middle);
            ButtonText->SetTextColor(240, 240, 240, 255);
            Button->SetChild(ButtonText);
        }

        mBuildingButtons[i] = Button;
        mBuildingButtonTexts[i] = ButtonText;
    }

    mMenuOpen = false;
    SelectCategory(0);
    ApplyMenuOpenState();
    SyncDemolitionButtonState();
    RefreshLayout();

    return true;
}

void CBuildMenuWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);
    SyncDemolitionButtonState();
    RefreshNpcCountText();
    RefreshLayout();
}

void CBuildMenuWidget::Render()
{
    CWidgetContainer::Render();
}

void CBuildMenuWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float PanelTop = ScreenHeight - mPanelHeight;
    const float HorizontalMargin = 24.f;
    const float ContentWidth = ScreenWidth - HorizontalMargin * 2.f;

    auto BuildButton = mBuildButton.lock();
    auto DemolishButton = mDemolishButton.lock();

    if (BuildButton)
    {
        BuildButton->SetPivot(0.5f, 1.f);
        BuildButton->SetPos(ScreenWidth * 0.5f, ScreenHeight - 20.f);
        BuildButton->SetSize(220.f, 58.f);
    }

    if (DemolishButton)
    {
        DemolishButton->SetPivot(0.f, 0.f);
        DemolishButton->SetPos(HorizontalMargin + 336.f, PanelTop + 12.f);
        DemolishButton->SetSize(140.f, 36.f);
    }

    auto NpcCountText = mNpcCountText.lock();

    if (NpcCountText)
    {
        NpcCountText->SetPivot(0.f, 1.f);
        NpcCountText->SetPos(20.f, ScreenHeight - 20.f);
        NpcCountText->SetSize(280.f, 28.f);
    }

    auto MenuBackground = mMenuBackground.lock();

    if (MenuBackground)
    {
        MenuBackground->SetPos(0.f, PanelTop);
        MenuBackground->SetSize(ScreenWidth, mPanelHeight);
    }

    auto TitleText = mTitleText.lock();

    if (TitleText)
    {
        TitleText->SetPos(HorizontalMargin, PanelTop + 14.f);
        TitleText->SetSize(320.f, 34.f);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();
    auto PageText = mPageText.lock();

    if (PrevPageButton)
    {
        PrevPageButton->SetPos(ScreenWidth - 176.f, PanelTop + 12.f);
        PrevPageButton->SetSize(40.f, 36.f);
    }

    if (PageText)
    {
        PageText->SetPos(ScreenWidth - 132.f, PanelTop + 12.f);
        PageText->SetSize(88.f, 36.f);
    }

    if (NextPageButton)
    {
        NextPageButton->SetPos(ScreenWidth - 40.f, PanelTop + 12.f);
        NextPageButton->SetSize(40.f, 36.f);
    }

    const float CategoryTop = PanelTop + 58.f;
    const float CategoryGap = 12.f;
    const float CategoryWidth =
        (ContentWidth - CategoryGap * (CategoryCount - 1)) /
        static_cast<float>(CategoryCount);

    for (int i = 0; i < CategoryCount; ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        CategoryButton->SetPos(
            HorizontalMargin +
            (CategoryWidth + CategoryGap) * static_cast<float>(i),
            CategoryTop);
        CategoryButton->SetSize(CategoryWidth, 40.f);
    }

    const float SlotGapX = 12.f;
    const float SlotGapY = 12.f;
    const float SlotTop = PanelTop + 112.f;
    const float SlotWidth =
        (ContentWidth - SlotGapX * (SlotColumnCount - 1)) /
        static_cast<float>(SlotColumnCount);
    const float SlotHeight =
        (mPanelHeight - (SlotTop - PanelTop) -
            SlotGapY * (SlotRowCount - 1) - 16.f) /
        static_cast<float>(SlotRowCount);

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int Row = i / SlotColumnCount;
        const int Col = i % SlotColumnCount;

        auto SlotButton = mBuildingButtons[i].lock();

        if (!SlotButton)
            continue;

        SlotButton->SetPos(
            HorizontalMargin +
            (SlotWidth + SlotGapX) * static_cast<float>(Col),
            SlotTop + (SlotHeight + SlotGapY) * static_cast<float>(Row));
        SlotButton->SetSize(SlotWidth, SlotHeight);
    }
}

void CBuildMenuWidget::ApplyMenuOpenState()
{
    auto DemolishButton = mDemolishButton.lock();
    auto MenuBackground = mMenuBackground.lock();
    auto TitleText = mTitleText.lock();
    auto PageText = mPageText.lock();
    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (DemolishButton)
        DemolishButton->SetEnable(mMenuOpen);
    if (MenuBackground)
        MenuBackground->SetEnable(mMenuOpen);
    if (TitleText)
        TitleText->SetEnable(mMenuOpen);
    if (PageText)
        PageText->SetEnable(mMenuOpen);
    if (PrevPageButton)
        PrevPageButton->SetEnable(mMenuOpen);
    if (NextPageButton)
        NextPageButton->SetEnable(mMenuOpen);

    for (size_t i = 0; i < mCategoryButtons.size(); ++i)
    {
        auto Button = mCategoryButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }

    for (size_t i = 0; i < mBuildingButtons.size(); ++i)
    {
        auto Button = mBuildingButtons[i].lock();

        if (Button)
            Button->SetEnable(mMenuOpen);
    }
}

void CBuildMenuWidget::RefreshNpcCountText()
{
    auto NpcCountText = mNpcCountText.lock();
    auto World = mWorld.lock();

    if (!NpcCountText || !World)
        return;

    std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;
    int NpcCount = 0;

    if (World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
    {
        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (Orb && Orb->GetAlive())
                ++NpcCount;
        }
    }

    wchar_t CountBuffer[64] = {};
    swprintf_s(CountBuffer, L"NPC: %d", NpcCount);
    NpcCountText->SetText(CountBuffer);
}

void CBuildMenuWidget::RefreshCategoryButtons()
{
    for (int i = 0; i < static_cast<int>(mCategoryButtons.size()); ++i)
    {
        auto CategoryButton = mCategoryButtons[i].lock();

        if (!CategoryButton)
            continue;

        if (i == mSelectedCategoryIndex)
            ConfigureHighlightedButtonStyle(CategoryButton);
        else
            ConfigureDefaultButtonStyle(CategoryButton);
    }
}

std::vector<int> CBuildMenuWidget::CollectCategoryEntryIndices() const
{
    std::vector<int> Result;
    const auto& Catalog = GetCatalog();

    for (int i = 0; i < static_cast<int>(Catalog.size()); ++i)
    {
        if (Catalog[i].CategoryIndex == mSelectedCategoryIndex)
            Result.push_back(i);
    }

    return Result;
}

void CBuildMenuWidget::RefreshBuildingButtons()
{
    const std::vector<int> CategoryEntries = CollectCategoryEntryIndices();
    const int EntryCount = static_cast<int>(CategoryEntries.size());
    const int PageCount = (std::max)(
        1, (EntryCount + SlotsPerPage - 1) / SlotsPerPage);

    if (mCurrentPage < 0)
        mCurrentPage = 0;
    else if (mCurrentPage >= PageCount)
        mCurrentPage = PageCount - 1;

    mVisibleEntryIndices.assign(SlotsPerPage, -1);
    const int BeginIndex = mCurrentPage * SlotsPerPage;

    for (int i = 0; i < SlotsPerPage; ++i)
    {
        const int CategoryListIndex = BeginIndex + i;
        auto Button = mBuildingButtons[i].lock();
        auto ButtonText = mBuildingButtonTexts[i].lock();

        if (!Button || !ButtonText)
            continue;

        if (CategoryListIndex >= 0 && CategoryListIndex < EntryCount)
        {
            const int EntryIndex = CategoryEntries[CategoryListIndex];
            const auto& Entry = GetCatalog()[EntryIndex];

            mVisibleEntryIndices[i] = EntryIndex;
            Button->ButtonEnable(true);
            ButtonText->SetText(Entry.DisplayName.c_str());
        }
        else
        {
            Button->ButtonEnable(false);
            ButtonText->SetText(TEXT("-"));
        }
    }

    auto PageText = mPageText.lock();

    if (PageText)
    {
        wchar_t PageBuffer[64] = {};
        swprintf_s(PageBuffer, L"%d / %d", mCurrentPage + 1, PageCount);
        PageText->SetText(PageBuffer);
    }

    auto PrevPageButton = mPrevPageButton.lock();
    auto NextPageButton = mNextPageButton.lock();

    if (PrevPageButton)
        PrevPageButton->ButtonEnable(mCurrentPage > 0);
    if (NextPageButton)
        NextPageButton->ButtonEnable(mCurrentPage < PageCount - 1);
}

void CBuildMenuWidget::SelectCategory(int CategoryIndex)
{
    if (CategoryIndex < 0)
        CategoryIndex = 0;
    else if (CategoryIndex >= CategoryCount)
        CategoryIndex = CategoryCount - 1;

    mSelectedCategoryIndex = CategoryIndex;
    mCurrentPage = 0;
    RefreshCategoryButtons();
    RefreshBuildingButtons();
}

void CBuildMenuWidget::MovePage(int DeltaPage)
{
    mCurrentPage += DeltaPage;
    RefreshBuildingButtons();
}

void CBuildMenuWidget::StartPlacementBySlot(int SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= static_cast<int>(mVisibleEntryIndices.size()))
        return;

    const int EntryIndex = mVisibleEntryIndices[SlotIndex];

    if (EntryIndex < 0)
        return;

    const auto& Catalog = GetCatalog();

    if (EntryIndex >= static_cast<int>(Catalog.size()))
        return;

    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    MainCamera->SetDemolitionMode(false);
    SyncDemolitionButtonState();

    const FBuildCatalogEntry& Entry = Catalog[EntryIndex];

    const bool Started = MainCamera->BeginBuildPlacement(
        Entry.Id,
        WideToUtf8(Entry.DisplayName),
        WideToUtf8(Entry.CategoryName),
        Entry.Residential,
        Entry.Capacity,
        Entry.FoodProvider,
        Entry.EntertainmentProvider,
        Entry.HousingSatisfactionCap,
        Entry.JobSatisfactionCap,
        Entry.FoodSatisfactionCap,
        Entry.FunSatisfactionCap,
        Entry.TemplateType,
        Entry.BuildingKind);

    if (!Started)
        return;

    mMenuOpen = false;
    ApplyMenuOpenState();
}

void CBuildMenuWidget::SyncDemolitionButtonState()
{
    auto DemolishButton = mDemolishButton.lock();
    auto DemolishButtonText = mDemolishButtonText.lock();
    auto World = mWorld.lock();

    if (!DemolishButton || !DemolishButtonText || !World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    const bool Enabled = MainCamera->IsDemolitionMode();

    if (Enabled)
    {
        ConfigureHighlightedButtonStyle(DemolishButton);
        DemolishButtonText->SetText(TEXT("철거 ON"));
    }
    else
    {
        ConfigureDefaultButtonStyle(DemolishButton);
        DemolishButtonText->SetText(TEXT("철거 OFF"));
    }
}

const std::vector<CBuildMenuWidget::FBuildCatalogEntry>&
    CBuildMenuWidget::GetCatalog() const
{
    static std::vector<FBuildCatalogEntry> Catalog = []()
    {
        static const wchar_t* InfrastructureNames[] =
        {
            L"건설 사무소",
            L"운송업자 사무소",
            L"선창",
            L"화물창고",
            L"창고",
            L"변전소",
            L"발전소",
            L"대형 주차장",
            L"버스차고지",
            L"버스 정류장",
            L"터널 연결",
            L"원자력 발전소",
            L"지하철역",
            L"공중 케이블 기지",
            L"풍력 터빈",
            L"해상 풍력 터빈",
            L"태양광 발전소"
        };

        static const wchar_t* FoodResourceNames[] =
        {
            L"코코넛 수확기",
            L"벌목소",
            L"어선 선착장",
            L"대규모 농장",
            L"퇴비 살포기",
            L"목장",
            L"광산",
            L"정유시설",
            L"양식장",
            L"석유 시추 장치",
            L"대규모 수경 농장",
            L"공장식 목장",
            L"자동화 광산"
        };

        static const wchar_t* IndustryNames[] =
        {
            L"제재소",
            L"럼주 증류소",
            L"무두질 공장",
            L"통조림 공장",
            L"유제품 공장",
            L"시가 공장",
            L"조선소",
            L"제철소",
            L"방직소",
            L"무기 공장",
            L"초콜릿 공장",
            L"가구 공장",
            L"보석 세공소",
            L"플라스틱 공장",
            L"자동차 공장",
            L"전자 제품 공장",
            L"패션 회사",
            L"제약 회사",
            L"주스 공장"
        };

        static const wchar_t* HousingNames[] =
        {
            L"판잣집",
            L"시골 주택",
            L"합숙소",
            L"저택",
            L"단독주택",
            L"간이 숙박소",
            L"아파트",
            L"서민 아파트",
            L"공동주택",
            L"현대식 아파트",
            L"현대식 저택",
            L"보안 저택"
        };

        static const wchar_t* EntertainmentNames[] =
        {
            L"주점",
            L"서커스",
            L"버스커",
            L"식물원",
            L"유원지 부두",
            L"레스토랑",
            L"오락실",
            L"패스트푸드 체인점",
            L"영화관",
            L"아쿠아 파크",
            L"롤러코스터",
            L"경기장"
        };

        static const int HousingCaps[] =
        {
            10, // 판잣집
            20, // 시골 주택
            30, // 합숙소
            75, // 저택
            50, // 단독주택
            25, // 간이 숙박소
            60, // 아파트
            40, // 서민 아파트
            45, // 공동주택
            70, // 현대식 아파트
            85, // 현대식 저택
            90  // 보안 저택
        };

        static const int EntertainmentFunCaps[] =
        {
            45, // 주점
            70, // 서커스
            40, // 버스커
            55, // 식물원
            68, // 유원지 부두
            60, // 레스토랑
            58, // 오락실
            52, // 패스트푸드 체인점
            65, // 영화관
            72, // 아쿠아 파크
            78, // 롤러코스터
            80  // 경기장
        };

        std::vector<FBuildCatalogEntry> Entries;

        auto AppendCategory = [&](
            int CategoryIndex,
            const wchar_t* const* Names,
            int NameCount,
            bool Residential,
            bool FoodProvider,
            bool EntertainmentProvider)
        {
            for (int i = 0; i < NameCount; ++i)
            {
                FBuildCatalogEntry Entry;
                Entry.Id = "build_" + std::to_string(CategoryIndex + 1) +
                    "_" + std::to_string(i + 1);
                Entry.DisplayName = Names[i];
                Entry.CategoryName = CategoryLabels[CategoryIndex];
                Entry.Residential = Residential;
                Entry.FoodProvider = FoodProvider;
                Entry.EntertainmentProvider = EntertainmentProvider;
                Entry.CategoryIndex = CategoryIndex;
                Entry.TemplateType = static_cast<EPlacementTemplateType>(i % 4);
                Entry.BuildingKind =
                    (Residential || (i % 2 == 0)) ?
                    EPlacementBuildingKind::BuildingA :
                    EPlacementBuildingKind::BuildingB;

                if (Residential)
                {
                    Entry.Capacity = 20 + (i % 5) * 8 + (i / 5) * 4;
                }
                else
                {
                    Entry.Capacity = 15 + (i % 6) * 6 + (i / 6) * 3;
                }

                Entry.HousingSatisfactionCap = 100;
                Entry.JobSatisfactionCap = 100;
                Entry.FoodSatisfactionCap = 100;
                Entry.FunSatisfactionCap = 100;

                if (CategoryIndex == 0)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        85, 45 + (i % 7) * 5 + (i / 7) * 3);
                }
                else if (CategoryIndex == 1)
                {
                    Entry.FoodSatisfactionCap = (std::min)(
                        82, 35 + (i % 7) * 6 + (i / 7) * 4);
                    Entry.JobSatisfactionCap = (std::min)(
                        70, 40 + (i % 5) * 5 + (i / 5) * 2);
                }
                else if (CategoryIndex == 2)
                {
                    Entry.JobSatisfactionCap = (std::min)(
                        90, 50 + (i % 8) * 5 + (i / 8) * 4);
                }
                else if (CategoryIndex == 3)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(HousingCaps) / sizeof(HousingCaps[0])))
                    {
                        Entry.HousingSatisfactionCap = HousingCaps[i];
                    }
                }
                else if (CategoryIndex == 4)
                {
                    if (i >= 0 && i < static_cast<int>(
                        sizeof(EntertainmentFunCaps) /
                        sizeof(EntertainmentFunCaps[0])))
                    {
                        Entry.FunSatisfactionCap =
                            EntertainmentFunCaps[i];
                    }
                }

                if (CategoryIndex == 4 &&
                    (Entry.DisplayName == L"레스토랑" ||
                        Entry.DisplayName == L"패스트푸드 체인점"))
                {
                    Entry.FoodProvider = true;

                    if (Entry.DisplayName == L"레스토랑")
                        Entry.FoodSatisfactionCap = 65;
                    else
                        Entry.FoodSatisfactionCap = 55;
                }

                Entries.push_back(Entry);
            }
        };

        AppendCategory(
            0, InfrastructureNames,
            static_cast<int>(sizeof(InfrastructureNames) /
                sizeof(InfrastructureNames[0])),
            false,
            false,
            false);
        AppendCategory(
            1, FoodResourceNames,
            static_cast<int>(sizeof(FoodResourceNames) /
                sizeof(FoodResourceNames[0])),
            false,
            true,
            false);
        AppendCategory(
            2, IndustryNames,
            static_cast<int>(sizeof(IndustryNames) /
                sizeof(IndustryNames[0])),
            false,
            false,
            false);
        AppendCategory(
            3, HousingNames,
            static_cast<int>(sizeof(HousingNames) /
                sizeof(HousingNames[0])),
            true,
            false,
            false);
        AppendCategory(
            4, EntertainmentNames,
            static_cast<int>(sizeof(EntertainmentNames) /
                sizeof(EntertainmentNames[0])),
            false,
            false,
            true);

        return Entries;
    }();

    return Catalog;
}

void CBuildMenuWidget::OnBuildButtonClick()
{
    mMenuOpen = !mMenuOpen;
    ApplyMenuOpenState();
}

void CBuildMenuWidget::OnDemolishButtonClick()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto MainCamera = World->FindObject<CMainCamera>("MainCamera").lock();

    if (!MainCamera)
        return;

    const bool NextMode = !MainCamera->IsDemolitionMode();
    MainCamera->SetDemolitionMode(NextMode);
    SyncDemolitionButtonState();

    if (NextMode)
    {
        mMenuOpen = false;
        ApplyMenuOpenState();
    }
}

void CBuildMenuWidget::OnCategoryInfrastructureClick()
{
    SelectCategory(0);
}

void CBuildMenuWidget::OnCategoryFoodResourceClick()
{
    SelectCategory(1);
}

void CBuildMenuWidget::OnCategoryIndustryClick()
{
    SelectCategory(2);
}

void CBuildMenuWidget::OnCategoryHousingClick()
{
    SelectCategory(3);
}

void CBuildMenuWidget::OnCategoryEntertainmentClick()
{
    SelectCategory(4);
}

void CBuildMenuWidget::OnPrevPageClick()
{
    MovePage(-1);
}

void CBuildMenuWidget::OnNextPageClick()
{
    MovePage(1);
}

void CBuildMenuWidget::OnSlot0Click()
{
    StartPlacementBySlot(0);
}

void CBuildMenuWidget::OnSlot1Click()
{
    StartPlacementBySlot(1);
}

void CBuildMenuWidget::OnSlot2Click()
{
    StartPlacementBySlot(2);
}

void CBuildMenuWidget::OnSlot3Click()
{
    StartPlacementBySlot(3);
}

void CBuildMenuWidget::OnSlot4Click()
{
    StartPlacementBySlot(4);
}

void CBuildMenuWidget::OnSlot5Click()
{
    StartPlacementBySlot(5);
}

void CBuildMenuWidget::OnSlot6Click()
{
    StartPlacementBySlot(6);
}

void CBuildMenuWidget::OnSlot7Click()
{
    StartPlacementBySlot(7);
}

void CBuildMenuWidget::OnSlot8Click()
{
    StartPlacementBySlot(8);
}

void CBuildMenuWidget::OnSlot9Click()
{
    StartPlacementBySlot(9);
}

void CBuildMenuWidget::OnSlot10Click()
{
    StartPlacementBySlot(10);
}

void CBuildMenuWidget::OnSlot11Click()
{
    StartPlacementBySlot(11);
}
