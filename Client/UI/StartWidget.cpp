#include "StartWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "Device.h"
#include "Engine.h"
#include "World/WorldManager.h"
#include "../World/LoadingWorld.h"

CStartWidget::CStartWidget()
{
}

CStartWidget::~CStartWidget()
{
}

bool CStartWidget::Init()
{
	CWidgetContainer::Init();

	std::shared_ptr<CImage> Back = CreateWidget<CImage>("Back").lock();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	Back->SetSize((float)RS.Width, (float)RS.Height);
	Back->SetTexture("StartBack", TEXT("Back.png"));

	std::shared_ptr<CButton> StartButton =
		CreateWidget<CButton>("StartButton", 1).lock();

	FVector3	ButtonPos;
	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f - 150.f;

	StartButton->SetPivot(0.5f, 0.5f);
	StartButton->SetPos(ButtonPos);
	StartButton->SetSize(200.f, 100.f);
	StartButton->SetTexture(EButtonState::Normal, "StartButton",
		TEXT("Start.png"));
	StartButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	StartButton->SetTexture(EButtonState::Hovered, "StartButton",
		TEXT("Start.png"));
	StartButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	StartButton->SetTexture(EButtonState::Click, "StartButton",
		TEXT("Start.png"));
	StartButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	StartButton->SetTexture(EButtonState::Disable, "StartButton",
		TEXT("Start.png"));

	//StartButton->SetSound(EButtonEventState::Hovered,
	//	"ButtonHovered", "TeemoSmile.mp3");
	//StartButton->SetSound(EButtonEventState::Click,
	//	"ButtonClick", "TeemoStartClicck.mp3");

	StartButton->SetEventCallback<CStartWidget>(
		EButtonEventState::Click, this, &CStartWidget::StartClick);

	std::shared_ptr<CButton> EditorButton =
		CreateWidget<CButton>("EditorButton", 1).lock();

	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f;

	EditorButton->SetPivot(0.5f, 0.5f);
	EditorButton->SetPos(ButtonPos);
	EditorButton->SetSize(200.f, 100.f);
	EditorButton->SetTexture(EButtonState::Normal, "StartButton",
		TEXT("Start.png"));
	EditorButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	EditorButton->SetTexture(EButtonState::Hovered, "StartButton",
		TEXT("Start.png"));
	EditorButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	EditorButton->SetTexture(EButtonState::Click, "StartButton",
		TEXT("Start.png"));
	EditorButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	EditorButton->SetTexture(EButtonState::Disable, "StartButton",
		TEXT("Start.png"));

	EditorButton->SetEventCallback<CStartWidget>(
		EButtonEventState::Click, this, &CStartWidget::EditorClick);


	std::shared_ptr<CButton> ExitButton =
		CreateWidget<CButton>("ExitButton", 1).lock();

	ButtonPos.x = RS.Width / 2.f;
	ButtonPos.y = RS.Height / 2.f + 150.f;

	ExitButton->SetPivot(0.5f, 0.5f);
	ExitButton->SetPos(ButtonPos);
	ExitButton->SetSize(200.f, 100.f);
	ExitButton->SetTexture(EButtonState::Normal, "ExitButton",
		TEXT("End.png"));
	ExitButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	ExitButton->SetTexture(EButtonState::Hovered, "ExitButton",
		TEXT("End.png"));
	ExitButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	ExitButton->SetTexture(EButtonState::Click, "ExitButton",
		TEXT("End.png"));
	ExitButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	ExitButton->SetTexture(EButtonState::Disable, "ExitButton",
		TEXT("End.png"));

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

void CStartWidget::EditorClick()
{
	auto World = CWorldManager::GetInst()->CreateWorld<CLoadingWorld>(true).lock();

	World->Load(EWorldType::Editor);
}

void CStartWidget::ExitClick()
{
	CEngine::GetInst()->Destroy();
}

