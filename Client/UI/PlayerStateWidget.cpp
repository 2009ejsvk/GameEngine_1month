#include "PlayerStateWidget.h"
#include "UI/TextBlock.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"

CPlayerStateWidget::CPlayerStateWidget()
{
}

CPlayerStateWidget::~CPlayerStateWidget()
{
}

void CPlayerStateWidget::SetPlayerName(const TCHAR* Name)
{
	auto	Text = mNameText.lock();

	Text->SetText(Name);
}

void CPlayerStateWidget::SetPlayerHP(float HP, float HPMax)
{
	auto	HPBar = mHPBar.lock();

	HPBar->SetPercent(HP / HPMax);
}

void CPlayerStateWidget::SetPlayerMP(float MP, float MPMax)
{
	auto	MPBar = mMPBar.lock();

	MPBar->SetPercent(MP / MPMax);
}

bool CPlayerStateWidget::Init()
{
	CWidgetContainer::Init();

	auto Back = CreateWidget<CImage>("Back", 1).lock();

	Back->SetTexture("PlayerStateBack", TEXT("CharacterHUD.png"));
	Back->SetSize(200.f, 80.f);

	mNameText = CreateWidget<CTextBlock>("Text", 2);

	auto	Text = mNameText.lock();

	Text->SetText(TEXT("텍스트 출력"));
	Text->SetTextColor(255, 255, 255, 255);
	Text->SetSize(200.f, 26.f);
	Text->SetFontSize(15.f);
	Text->SetAlignH(ETextAlignH::Center);
	Text->EnableShadow(true);
	Text->SetShadowOffset(2.f, 2.f);
	Text->SetShadowTextColor(128, 128, 128, 255);

	mHPBar = CreateWidget<CProgressBar>("HPBar");

	auto	HPBar = mHPBar.lock();

	HPBar->SetPos(24.f, 32.f);
	HPBar->SetSize(168.f, 14.f);
	HPBar->SetTexture(EProgressBarImageType::Fill, "HPBar",
		TEXT("HPBar.png"));

	HPBar->SetTint(EProgressBarImageType::Back,
		0.3f, 0.3f, 0.3f, 1.f);

	HPBar->SetBarDir(EProgressBarDir::RightToLeft);

	HPBar->SetPercent(1.f);

	mMPBar = CreateWidget<CProgressBar>("MPBar");

	auto	MPBar = mMPBar.lock();

	MPBar->SetPos(24.f, 50.f);
	MPBar->SetSize(168.f, 14.f);
	MPBar->SetTexture(EProgressBarImageType::Fill, "MPBar",
		TEXT("MPBar.png"));

	MPBar->SetTint(EProgressBarImageType::Back,
		0.3f, 0.3f, 0.3f, 1.f);

	MPBar->SetBarDir(EProgressBarDir::RightToLeft);

	MPBar->SetPercent(1.f);

	return true;
}

void CPlayerStateWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CPlayerStateWidget::Render()
{
	CWidgetContainer::Render();
}
