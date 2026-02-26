#include "SlotIcon.h"
#include "UI/Button.h"
#include "UI/TextBlock.h"
#include "UI/Image.h"
#include "UI/ProgressBar.h"
#include "Inventory.h"

CSlotIcon::CSlotIcon()
{
}

CSlotIcon::CSlotIcon(const CSlotIcon& ref)	:
	CWidgetContainer(ref)
{
	mImage = FindWidget<CImage>("Back");
}

CSlotIcon::~CSlotIcon()
{
}

void CSlotIcon::SetIconImage(const std::string& Name)
{
	auto	Image = mImage.lock();

	Image->SetTexture(Name);
}

void CSlotIcon::SetIconImage(const std::string& Name, 
	const TCHAR* FileName, const std::string& PathName)
{
	auto	Image = mImage.lock();

	Image->SetTexture(Name, FileName, PathName);
}

bool CSlotIcon::Init()
{
	CWidgetContainer::Init();

	mImage = CreateWidget<CImage>("Back");

	std::shared_ptr<CImage> Back = mImage.lock();

	Back->SetSize(44.f, 44.f);

	return true;
}

void CSlotIcon::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CSlotIcon::Render()
{
	CWidgetContainer::Render();
}


void CSlotIcon::MouseHovered()
{
	CWidgetContainer::MouseHovered();
}

void CSlotIcon::MouseUnHovered()
{
	CWidgetContainer::MouseUnHovered();
}

bool CSlotIcon::MouseDragStart(const FVector2& MousePos,
	std::shared_ptr<CWidget>& DragOperator)
{
	CWidgetContainer::MouseDragStart(MousePos, DragOperator);

	DragOperator = mSelf.lock();

	return true;
}

bool CSlotIcon::MouseDrag(const FVector2& MousePos,
	const FVector2& MouseMove)
{
	CWidgetContainer::MouseDrag(MousePos, MouseMove);

	//AddPos(MouseMove);

	return true;
}

bool CSlotIcon::MouseDragEnd(const FVector2& MousePos)
{
	CWidgetContainer::MouseDragEnd(MousePos);

	return true;
}

CSlotIcon* CSlotIcon::Clone()	const
{
	return new CSlotIcon(*this);
}
