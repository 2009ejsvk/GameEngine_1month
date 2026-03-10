#include "WorldUIManager.h"
#include "Input.h"
#include "World.h"

CWorldUIManager::CWorldUIManager()
{
}

CWorldUIManager::~CWorldUIManager()
{
}

bool CWorldUIManager::Init()
{
	return true;
}

void CWorldUIManager::Update(float DeltaTime)
{
	auto	iter = mWidgetList.begin();
	auto	iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if(!(*iter)->GetAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
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

bool CWorldUIManager::IsPointOverEnabledWidget(const FVector2& MousePos)
{
	if (mWidgetList.size() >= 2)
	{
        std::stable_sort(mWidgetList.begin(), mWidgetList.end(),
            CWorldUIManager::SortCollision);
	}

	auto	iter = mWidgetList.begin();
	auto	iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
			continue;
		}

		else if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		if ((*iter)->HitTest(MousePos))
			return true;

		++iter;
	}

	return false;
}

bool CWorldUIManager::CollisionMouse(float DeltaTime,
	const FVector2& MousePos)
{
	if (mWidgetList.size() >= 2)
	{
        std::stable_sort(mWidgetList.begin(), mWidgetList.end(),
            CWorldUIManager::SortCollision);
	}

	// 기존에 Hovered 상태의 위젯이 제거되었는지 판단한다.
	if (mHoveredWidget.expired())
	{
		mHoveredWidget = std::weak_ptr<CWidget>();
	}

	if (mDragWidget.expired())
	{
		auto	iter = mWidgetList.begin();
		auto	iterEnd = mWidgetList.end();

		for (; iter != iterEnd;)
		{
			if (!(*iter)->GetAlive())
			{
				iter = mWidgetList.erase(iter);
				iterEnd = mWidgetList.end();
				continue;
			}

			else if (!(*iter)->GetEnable())
			{
				++iter;
				continue;
			}

			std::weak_ptr<CWidget>	HoveredWidget;

			if ((*iter)->CollisionMouse(HoveredWidget, MousePos))
			{
				// 기존에 충돌된 위젯과 다를 경우
				auto	Current = HoveredWidget.lock();
				auto	Prev = mHoveredWidget.lock();

				if (Prev != Current)
				{
					if (Prev)
					{
						Prev->MouseUnHovered();

						/*if (Prev->GetMouseDrag())
						{
							Prev->SetMouseDragEnable(false);
							Prev->MouseDragEnd(MousePos);
						}*/
					}

					mHoveredWidget = HoveredWidget;
				}

				auto	World = mWorld.lock();

				auto Input = World->GetInput().lock();

				auto	Widget = mHoveredWidget.lock();

				if (Input->GetMouseState(EMouseType::LButton,
					EInputType::Press))
				{
					std::shared_ptr<CWidget>	OperatorWidget;

					if (Widget->MouseDragStart(MousePos,
						OperatorWidget))
					{
						Widget->SetMouseDragEnable(true);
						Widget->SetZOrder(999999);
						mDragWidget = mHoveredWidget;

						if (OperatorWidget)
						{
							// 복제본을 만들어준다.
							mDragOperatorWidget.reset(OperatorWidget->Clone());
							mDragOperatorWidget->SetSelf(mDragOperatorWidget);
							mDragOperatorWidget->SetParentAll();
							mDragOperatorWidget->SetPivot(0.f, 0.f);
							mDragOperatorWidget->SetOpacityAll(0.5f);
						}
					}
				}

				/*else if (Input->GetMouseState(EMouseType::LButton,
					EInputType::Hold) && Widget->GetMouseDrag())
				{
					Widget->MouseDrag(MousePos, Input->GetMouseMove());
				}

				else if (Input->GetMouseState(EMouseType::LButton,
					EInputType::Release) && Widget->GetMouseDrag())
				{
					Widget->SetMouseDragEnable(false);
					Widget->MouseDragEnd(MousePos);
					Widget->ReplaceZOrder();
				}*/

				return true;
			}

			++iter;
		}

		// 현재 프레임에 마우스와 충돌된 위젯이 없다는 의미이다.
		// 그런데 기존에 충돌된 위젯이 있을 경우 이 위젯도 UnHovered 해준다.
		if (!mHoveredWidget.expired())
		{
			auto	Widget = mHoveredWidget.lock();

			Widget->MouseUnHovered();

			mHoveredWidget = std::weak_ptr<CWidget>();
		}

		return false;
	}
		
	auto	World = mWorld.lock();

	auto Input = World->GetInput().lock();

	auto	DragWidget = mDragWidget.lock();

	if (Input->GetMouseState(EMouseType::LButton,
		EInputType::Hold) && DragWidget->GetMouseDrag())
	{
		DragWidget->MouseDrag(MousePos, Input->GetMouseMove());
	}

	else if (Input->GetMouseState(EMouseType::LButton,
		EInputType::Release) && DragWidget->GetMouseDrag())
	{
		DragWidget->SetMouseDragEnable(false);
		DragWidget->MouseDragEnd(MousePos);
		DragWidget->ReplaceZOrder();
		mDragWidget = std::weak_ptr<CWidget>();
		mDragOperatorWidget.reset();
	}

	return true;
}

void CWorldUIManager::Render()
{
	if (mWidgetList.size() >= 2)
	{
        std::stable_sort(mWidgetList.begin(), mWidgetList.end(),
            CWorldUIManager::SortRender);
	}

	auto	iter = mWidgetList.begin();
	auto	iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->GetAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
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

	// 마지막으로 Drag 되고 있는 위젯이 있을 경우 이 위젯을 반투명하게 하여
	// 화면에 마지막에 출력한다.
	if (mDragOperatorWidget)
	{
		auto	World = mWorld.lock();

		auto Input = World->GetInput().lock();

		FVector3	MousePos;
		MousePos.x = Input->GetMousePos().x;
		MousePos.y = Input->GetMousePos().y;

		FVector3 Size = mDragOperatorWidget->GetSize();

		FVector3 Center = Size * FVector3(0.5f, 0.5f, 0.f);

		mDragOperatorWidget->SetPos(MousePos - Center);

		mDragOperatorWidget->Render();
	}


	// 마우스 위젯을 출력한다.
}

std::weak_ptr<CWidgetContainer> CWorldUIManager::GetWidget(
	const std::string& Name)
{
	auto	iter = mWidgetList.begin();
	auto	iterEnd = mWidgetList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->GetName() == Name)
			return *iter;
	}

	return std::weak_ptr<CWidgetContainer>();
}

bool CWorldUIManager::SortRender(
	const std::shared_ptr<CWidgetContainer>& Src,
	const std::shared_ptr<CWidgetContainer>& Dest)
{
	return Src->GetZOrder() < Dest->GetZOrder();
}

bool CWorldUIManager::SortCollision(
	const std::shared_ptr<CWidgetContainer>& Src,
	const std::shared_ptr<CWidgetContainer>& Dest)
{
	return Src->GetZOrder() > Dest->GetZOrder();
}
