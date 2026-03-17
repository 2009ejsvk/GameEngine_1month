#include "StartWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "Device.h"
#include "Engine.h"
#include "World/WorldManager.h"
#include "UIStrings.h"
#include "../World/LoadingWorld.h"

namespace
{
    std::wstring GetUiStringOrDefault(
        const wchar_t* Key,
        const wchar_t* DefaultValue)
    {
        const std::wstring& Value = UIStrings::Get(Key);

        if (Value == Key)
            return std::wstring(DefaultValue);

        return Value;
    }
}

CStartWidget::CStartWidget()
{
}

CStartWidget::~CStartWidget()
{
}

bool CStartWidget::Init()
{
	CWidgetContainer::Init();

    const std::wstring StartBackgroundTexture =
        GetUiStringOrDefault(L"start.background.image", L"Back.png");
    const std::wstring StartPrimaryButtonTexture =
        GetUiStringOrDefault(L"start.button.start.image", L"Start.png");
    const std::wstring StartSecondButtonTexture =
        GetUiStringOrDefault(L"start.button.second.image", L"Start.png");
    const std::wstring StartExitButtonTexture =
        GetUiStringOrDefault(L"start.button.exit.image", L"End.png");

	std::shared_ptr<CImage> Back = CreateWidget<CImage>("Back").lock();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	Back->SetSize((float)RS.Width, (float)RS.Height);
	Back->SetTexture("StartBack", StartBackgroundTexture.c_str());

	std::shared_ptr<CButton> StartButton =
		CreateWidget<CButton>("StartButton", 1).lock();

	FVector3	ButtonPos;
	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f - 150.f;

	StartButton->SetPivot(0.5f, 0.5f);
	StartButton->SetPos(ButtonPos);
	StartButton->SetSize(200.f, 100.f);
	StartButton->SetTexture(EButtonState::Normal, "StartButton",
		StartPrimaryButtonTexture.c_str());
	StartButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	StartButton->SetTexture(EButtonState::Hovered, "StartButton",
		StartPrimaryButtonTexture.c_str());
	StartButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	StartButton->SetTexture(EButtonState::Click, "StartButton",
		StartPrimaryButtonTexture.c_str());
	StartButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	StartButton->SetTexture(EButtonState::Disable, "StartButton",
		StartPrimaryButtonTexture.c_str());

	//StartButton->SetSound(EButtonEventState::Hovered,
	//	"ButtonHovered", "TeemoSmile.mp3");
	//StartButton->SetSound(EButtonEventState::Click,
	//	"ButtonClick", "TeemoStartClicck.mp3");

	StartButton->SetEventCallback<CStartWidget>(
		EButtonEventState::Click, this, &CStartWidget::StartClick);

	std::shared_ptr<CButton> SecondButton =
		CreateWidget<CButton>("SecondButton", 1).lock();

	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f;

	SecondButton->SetPivot(0.5f, 0.5f);
	SecondButton->SetPos(ButtonPos);
	SecondButton->SetSize(200.f, 100.f);
	SecondButton->SetTexture(EButtonState::Normal, "SecondButton",
		StartSecondButtonTexture.c_str());
	SecondButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	SecondButton->SetTexture(EButtonState::Hovered, "SecondButton",
		StartSecondButtonTexture.c_str());
	SecondButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	SecondButton->SetTexture(EButtonState::Click, "SecondButton",
		StartSecondButtonTexture.c_str());
	SecondButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	SecondButton->SetTexture(EButtonState::Disable, "SecondButton",
		StartSecondButtonTexture.c_str());

	SecondButton->SetEventCallback<CStartWidget>(
		EButtonEventState::Click, this, &CStartWidget::SecondButtonClick);


	std::shared_ptr<CButton> ExitButton =
		CreateWidget<CButton>("ExitButton", 1).lock();

	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f + 150.f;

	ExitButton->SetPivot(0.5f, 0.5f);
	ExitButton->SetPos(ButtonPos);
	ExitButton->SetSize(200.f, 100.f);
	ExitButton->SetTexture(EButtonState::Normal, "ExitButton",
		StartExitButtonTexture.c_str());
	ExitButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	ExitButton->SetTexture(EButtonState::Hovered, "ExitButton",
		StartExitButtonTexture.c_str());
	ExitButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	ExitButton->SetTexture(EButtonState::Click, "ExitButton",
		StartExitButtonTexture.c_str());
	ExitButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	ExitButton->SetTexture(EButtonState::Disable, "ExitButton",
		StartExitButtonTexture.c_str());

	//ExitButton->SetSound(EButtonEventState::Hovered,
	//	"ButtonHovered", "TeemoSmile.mp3");
	//ExitButton->SetSound(EButtonEventState::Click,
	//	"ButtonClick", "TeemoStartClicck.mp3");

	ExitButton->SetEventCallback<CStartWidget>(
		EButtonEventState::Click, this, &CStartWidget::ExitClick);
	


	return true;
}

void CStartWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CStartWidget::Render()
{
	CWidgetContainer::Render();
}

void CStartWidget::StartClick()
{
	auto World = CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();

	World->Load(EWorldType::Main);
}

void CStartWidget::SecondButtonClick()
{
	auto World = CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();

	World->Load(EWorldType::Main);
}

void CStartWidget::ExitClick()
{
	CEngine::GetInst()->Destroy();
}

