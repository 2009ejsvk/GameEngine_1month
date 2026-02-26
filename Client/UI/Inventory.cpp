#include "Inventory.h"
#include "UI/Button.h"
#include "UI/TextBlock.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "UI/TitleBar.h"
#include "Slot.h"

CInventory::CInventory()
{
}

CInventory::~CInventory()
{
}

bool CInventory::Init()
{
	CWidgetContainer::Init();

	// 400, 600
	std::shared_ptr<CTitleBar> TitleBar =
		CreateWidget<CTitleBar>("TitleBar").lock();

	TitleBar->SetSize(360.f, 40.f);
	TitleBar->SetTexture("TitleBar", TEXT("SlotBack.png"));

	TitleBar->SetText(TEXT("Inventory"));

	TitleBar->SetUpdateWidget(mSelf);

	std::shared_ptr<CButton> CloseButton =
		CreateWidget<CButton>("CloseButton").lock();

	CloseButton->SetPos(360.f, 0.f);
	CloseButton->SetSize(40.f, 40.f);
	CloseButton->SetTexture(EButtonState::Normal, "InvenCloseButton",
		TEXT("CloseButton.png"));
	CloseButton->SetTint(EButtonState::Normal, FVector4(0.8f, 0.8f, 0.8f, 1.f));

	CloseButton->SetTexture(EButtonState::Hovered, "InvenCloseButton",
		TEXT("CloseButton.png"));
	CloseButton->SetTint(EButtonState::Hovered, FVector4(1.f, 1.f, 1.f, 1.f));

	CloseButton->SetTexture(EButtonState::Click, "InvenCloseButton",
		TEXT("CloseButton.png"));
	CloseButton->SetTint(EButtonState::Click, FVector4(0.6f, 0.6f, 0.6f, 1.f));

	CloseButton->SetTexture(EButtonState::Disable, "InvenCloseButton",
		TEXT("CloseButton.png"));

	CloseButton->SetEventCallback<CInventory>(EButtonEventState::Click, this, &CInventory::CloseButtonClick);

	std::shared_ptr<CImage> Back =
		CreateWidget<CImage>("Back").lock();

	Back->SetPos(0.f, 40.f);
	Back->SetSize(400.f, 560.f);
	Back->SetTexture("InventoryBack", TEXT("SlotBack.png"));

	for (int i = 0; i < 9; ++i)
	{
		for (int j = 0; j < 7; ++j)
		{
			std::string	Name = "Slot" + i;

			std::shared_ptr<CSlot> Slot =
				CreateWidget<CSlot>(Name, 1).lock();

			Slot->SetPos(10.f + j * 55.f, 60.f + i * 55.f);
			Slot->SetSize(50.f, 50.f);

			mSlotArray.push_back(Slot);
		}
	}

	auto Slot = mSlotArray[0].lock();

	Slot->SetIconImage("Shield", TEXT("Icon_Shield.PNG"));
	Slot->EnableIcon(true);

	Slot = mSlotArray[1].lock();

	Slot->SetIconImage("Sword", TEXT("Icon_Sword.PNG"));
	Slot->EnableIcon(true);

	Slot = mSlotArray[7].lock();

	Slot->SetIconImage("Sword", TEXT("Icon_Sword.PNG"));
	Slot->EnableIcon(true);

	return true;
}

void CInventory::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CInventory::Render()
{
	CWidgetContainer::Render();
}

void CInventory::CloseButtonClick()
{
	SetEnable(false);
}

