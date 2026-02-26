#include "LoadingWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "Engine.h"
#include "World/WorldManager.h"
#include "../World/MainWorld.h"

CLoadingWidget::CLoadingWidget()
{
}

CLoadingWidget::~CLoadingWidget()
{
}

bool CLoadingWidget::Init()
{
	CWidgetContainer::Init();

	std::shared_ptr<CImage> Back = CreateWidget<CImage>("Back").lock();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	Back->SetSize((float)RS.Width, (float)RS.Height);
	Back->SetTexture("LoadingBack", TEXT("LoadingBack.png"));

	std::shared_ptr<CTextBlock> Text =
		CreateWidget<CTextBlock>("Text", 1).lock();

	Text->SetText(TEXT("LOADING"));
	Text->SetTextColor(255, 255, 255, 255);
	Text->SetPos(900.f, 550.f);
	Text->SetSize(500.f, 150.f);
	Text->SetFontSize(50.f);
	Text->EnableShadow(true);
	Text->SetShadowOffset(3.f, 3.f);
	Text->SetShadowTextColor(128, 128, 128, 255);

	return true;
}

void CLoadingWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CLoadingWidget::Render()
{
	CWidgetContainer::Render();
}

