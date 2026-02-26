#include "Slot.h"
#include "UI/TextBlock.h"
#include "UI/Image.h"
#include "SlotIcon.h"

CSlot::CSlot()
{
}

CSlot::~CSlot()
{
}

void CSlot::EnableIcon(bool Enable)
{
	mIcon->SetEnable(Enable);
}

void CSlot::SetIconImage(const std::string& Name)
{
	mIcon->SetIconImage(Name);
}

void CSlot::SetIconImage(const std::string& Name,
	const TCHAR* FileName, const std::string& PathName)
{
	mIcon->SetIconImage(Name, FileName, PathName);
}

bool CSlot::Init()
{
	CWidgetContainer::Init();

	std::shared_ptr<CImage> Back =
		CreateWidget<CImage>("Back").lock();

	Back->SetSize(50.f, 50.f);
	Back->SetTexture("SlotBack", TEXT("SlotBack.png"));

	mIcon = CreateWidget<CSlotIcon>("Icon", 1).lock();
	
	mIcon->SetPos(3.f, 3.f);
	mIcon->SetSize(44.f, 44.f);

	mIcon->SetEnable(false);

	return true;
}

void CSlot::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CSlot::Render()
{
	CWidgetContainer::Render();
}

