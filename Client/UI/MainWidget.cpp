#include "MainWidget.h"
#include "UI/Button.h"
#include "UI/TextBlock.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/NumberWidget.h"
#include "Inventory.h"
#include "PlayerStateWidget.h"

CMainWidget::CMainWidget()
{
}

CMainWidget::~CMainWidget()
{
}

bool CMainWidget::Init()
{
	CWidgetContainer::Init();

	/*std::shared_ptr<CButton> Button =
		CreateWidget<CButton>("TestButton").lock();

	Button->SetPos(100.f, 50.f);
	Button->SetSize(100.f, 50.f);
	Button->SetTexture(EButtonState::Normal, "StartButton",
		TEXT("Start.png"));
	Button->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	Button->SetTexture(EButtonState::Hovered, "StartButton",
		TEXT("Start.png"));
	Button->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	Button->SetTexture(EButtonState::Click, "StartButton",
		TEXT("Start.png"));
	Button->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	Button->SetTexture(EButtonState::Disable, "StartButton",
		TEXT("Start.png"));

	Button->SetSound(EButtonEventState::Hovered,
		"ButtonHovered", "TeemoSmile.mp3");
	Button->SetSound(EButtonEventState::Click,
		"ButtonClick", "TeemoStartClicck.mp3");

	std::shared_ptr<CTextBlock> ButtonText =
		CWidget::CreateStaticWidget<CTextBlock>("ButtonText", mWorld);

	ButtonText->SetText(TEXT("시작버튼"));
	ButtonText->SetTextColor(255, 255, 0, 255);

	Button->SetChild(ButtonText);*/

	/*std::shared_ptr<CTextBlock> Text =
		CreateWidget<CTextBlock>("Text").lock();

	Text->SetText(TEXT("텍스트 출력"));
	Text->SetTextColor(255, 0, 0, 255);
	Text->SetPos(100.f, 300.f);
	Text->SetSize(200.f, 100.f);
	Text->SetFontSize(30.f);
	Text->EnableShadow(true);
	Text->SetShadowOffset(3.f, 3.f);
	Text->SetShadowTextColor(128, 128, 128, 255);


	std::shared_ptr<CImage> Image =
		CreateWidget<CImage>("Image").lock();

	Image->SetPos(300.f, 50.f);
	Image->SetSize(100.f, 50.f);
	Image->SetTexture("StartButton",
		TEXT("Start.png"));
	Image->SetTint(FVector4(0.8f, 0.8f, 0.8f, 1.f));

	std::shared_ptr<CProgressBar> HPBar =
		CreateWidget<CProgressBar>("HPBar").lock();

	HPBar->SetPos(500.f, 50.f);
	HPBar->SetSize(100.f, 50.f);
	HPBar->SetTexture(EProgressBarImageType::Fill, "HPBar",
		TEXT("HPBar.png"));

	HPBar->SetTint(EProgressBarImageType::Back,
		0.3f, 0.3f, 0.3f, 1.f);

	HPBar->SetBarDir(EProgressBarDir::TopToBottom);

	HPBar->SetPercent(0.5f);*/

	mPlayerStateWidget =
		CreateWidget<CPlayerStateWidget>("PlayerState").lock();

	mPlayerStateWidget->SetPos(50.f, 50.f);
	mPlayerStateWidget->SetSize(200.f, 80.f);

	std::shared_ptr<CInventory> Inventory =
		CreateWidget<CInventory>("Inventory").lock();

	Inventory->SetPos(700.f, 50.f);
	Inventory->SetSize(400.f, 600.f);
	Inventory->SetZOrder(1);

	std::shared_ptr<CNumberWidget> Number =
		CreateWidget<CNumberWidget>("Number").lock();

	Number->SetPos(600.f, 50.f);
	Number->SetSize(15.f, 25.f);
	Number->SetNumber(325163246);
	Number->SetNumberSize(46.f, 68.f);
	Number->SetTexture("Number", TEXT("Number.png"));


	return true;
}

void CMainWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CMainWidget::Render()
{
	CWidgetContainer::Render();
}
