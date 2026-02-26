#include "WidgetContainer.h"

CWidgetContainer::CWidgetContainer()
{
}

CWidgetContainer::CWidgetContainer(const CWidgetContainer& ref)	:
	CWidget(ref)
{
	auto	iter = ref.mChildList.begin();
	auto	iterEnd = ref.mChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		std::shared_ptr<CWidget>	Widget;
		
		Widget.reset((*iter)->Clone());

		mChildList.push_back(Widget);
	}
}

CWidgetContainer::~CWidgetContainer()
{
}

void CWidgetContainer::SetParentAll()
{
	auto	iter = mChildList.begin();
	auto	iterEnd = mChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->SetParent(mSelf);

		(*iter)->SetParentAll();
	}
}

void CWidgetContainer::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	auto	iter = mChildList.begin();
	auto	iterEnd = mChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->SetOpacityAll(Opacity);
	}
}

bool CWidgetContainer::Init()
{
	CWidget::Init();

	return true;
}

void CWidgetContainer::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	auto	iter = mChildList.begin();
	auto	iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(DeltaTime);

		++iter;
	}
}

void CWidgetContainer::Render()
{
	CWidget::Render();

	if (mChildList.size() >= 2)
	{
		std::sort(mChildList.begin(), mChildList.end(),
			CWidget::SortRender);
	}

	auto	iter = mChildList.begin();
	auto	iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Render();

		++iter;
	}
}

bool CWidgetContainer::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	if (mChildList.size() >= 2)
	{
		std::sort(mChildList.begin(), mChildList.end(),
			CWidget::SortCollision);
	}

	auto	iter = mChildList.begin();
	auto	iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		if ((*iter)->CollisionMouse(Result, MousePos))
			return true;

		++iter;
	}

	if (CWidget::CollisionMouse(Result, MousePos))
		return true;

	return false;
}

void CWidgetContainer::MouseHovered()
{
}

void CWidgetContainer::MouseUnHovered()
{
}

bool CWidgetContainer::MouseDragStart(const FVector2& MousePos,
	std::shared_ptr<CWidget>& DragOperator)
{
	return false;
}

bool CWidgetContainer::MouseDrag(const FVector2& MousePos,
	const FVector2& MouseMove)
{
	return false;
}

bool CWidgetContainer::MouseDragEnd(const FVector2& MousePos)
{
	return false;
}

CWidgetContainer* CWidgetContainer::Clone()	const
{
	return new CWidgetContainer(*this);
}
