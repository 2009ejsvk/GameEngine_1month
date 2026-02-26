#include "WorldHUD.h"
#include "UI/TextBlock.h"
#include "UI/ProgressBar.h"

CWorldHUD::CWorldHUD()
{
}

CWorldHUD::~CWorldHUD()
{
}

void CWorldHUD::SetPlayerName(const TCHAR* Name)
{
	auto	Text = mNameText.lock();

	Text->SetText(Name);
}

void CWorldHUD::SetPlayerHP(float HP, float HPMax)
{
	auto	HPBar = mHPBar.lock();

	HPBar->SetPercent(HP / HPMax);
}

bool CWorldHUD::Init()
{
	CWidgetContainer::Init();

	mNameText = CreateWidget<CTextBlock>("Text");

	auto	Text = mNameText.lock();

	Text->SetText(TEXT("텍스트 출력"));
	Text->SetTextColor(255, 255, 255, 255);
	Text->SetSize(80.f, 20.f);
	Text->SetFontSize(12.f);
	Text->SetAlignH(ETextAlignH::Center);
	Text->EnableShadow(true);
	Text->SetShadowOffset(1.f, 1.f);
	Text->SetShadowTextColor(128, 128, 128, 255);

	mHPBar = CreateWidget<CProgressBar>("HPBar");

	auto	HPBar = mHPBar.lock();

	HPBar->SetPos(0.f, 20.f);
	HPBar->SetSize(80.f, 20.f);
	HPBar->SetTexture(EProgressBarImageType::Fill, "HPBar",
		TEXT("HPBar.png"));

	HPBar->SetTint(EProgressBarImageType::Back,
		0.3f, 0.3f, 0.3f, 1.f);

	HPBar->SetBarDir(EProgressBarDir::RightToLeft);

	HPBar->SetPercent(1.f);

	return true;
}

void CWorldHUD::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CWorldHUD::Render()
{
	CWidgetContainer::Render();
}

