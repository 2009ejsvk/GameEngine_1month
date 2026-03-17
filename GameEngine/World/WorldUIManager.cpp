#include "WorldUIManager.h"
#include "../UI/Button.h"
#include "../UI/Image.h"
#include "../UI/TextBlock.h"
#include "../UI/Widget.h"
#include "../Device.h"
#include "Input.h"
#include "World.h"
#include <algorithm>
#include <sstream>
#include <vector>

namespace
{
    const char* GetWidgetTypeName(const std::shared_ptr<CWidget>& Widget)
    {
        if (std::dynamic_pointer_cast<CTextBlock>(Widget))
            return "TextBlock";

        if (std::dynamic_pointer_cast<CButton>(Widget))
            return "Button";

        if (std::dynamic_pointer_cast<CImage>(Widget))
            return "Image";

        if (std::dynamic_pointer_cast<CWidgetContainer>(Widget))
            return "Container";

        return "Widget";
    }

    std::wstring ToWideString(const std::string& Text)
    {
        return std::wstring(Text.begin(), Text.end());
    }

    bool IsInspectableWidget(const std::shared_ptr<CWidget>& Widget)
    {
        return Widget &&
            Widget->GetAlive() &&
            Widget->GetEnable();
    }

    std::shared_ptr<CWidget> FindDeepestWidgetAtPoint(
        const std::shared_ptr<CWidget>& Widget,
        const FVector2& MousePos)
    {
        if (!IsInspectableWidget(Widget))
            return nullptr;

        if (auto Container = std::dynamic_pointer_cast<CWidgetContainer>(Widget))
        {
            std::vector<std::shared_ptr<CWidget>> Children =
                Container->GetChildList();

            if (Children.size() >= 2)
            {
                std::stable_sort(
                    Children.begin(),
                    Children.end(),
                    [](const std::shared_ptr<CWidget>& Src,
                        const std::shared_ptr<CWidget>& Dest)
                    {
                        return Src->GetZOrder() > Dest->GetZOrder();
                    });
            }

            for (const auto& Child : Children)
            {
                if (auto HitChild = FindDeepestWidgetAtPoint(Child, MousePos))
                    return HitChild;
            }
        }

        if (auto Button = std::dynamic_pointer_cast<CButton>(Widget))
        {
            if (auto HitChild =
                FindDeepestWidgetAtPoint(Button->GetChild(), MousePos))
            {
                return HitChild;
            }
        }

        return Widget->HitTest(MousePos) ? Widget : nullptr;
    }

    std::shared_ptr<CWidget> FindTopmostWidgetAtPoint(
        const std::vector<std::shared_ptr<CWidgetContainer>>& RootWidgets,
        const FVector2& MousePos)
    {
        std::vector<std::shared_ptr<CWidgetContainer>> SortedRoots = RootWidgets;

        if (SortedRoots.size() >= 2)
        {
            std::stable_sort(
                SortedRoots.begin(),
                SortedRoots.end(),
                [](const std::shared_ptr<CWidgetContainer>& Src,
                    const std::shared_ptr<CWidgetContainer>& Dest)
                {
                    return Src->GetZOrder() > Dest->GetZOrder();
                });
        }

        for (const auto& RootWidget : SortedRoots)
        {
            if (auto HitWidget = FindDeepestWidgetAtPoint(RootWidget, MousePos))
                return HitWidget;
        }

        return nullptr;
    }

    std::string BuildClipboardWidgetPath(
        const std::shared_ptr<CWidget>& Widget)
    {
        if (!Widget)
            return std::string();

        std::vector<std::string> Segments;
        std::shared_ptr<CWidget> Current = Widget;

        while (Current)
        {
            Segments.push_back(Current->GetName());
            Current = Current->GetParent().lock();
        }

        if (Segments.empty())
            return std::string();

        std::reverse(Segments.begin(), Segments.end());

        std::string Result = "Widget.";

        for (size_t Index = 0; Index < Segments.size(); ++Index)
        {
            if (Index > 0)
                Result += "/";

            Result += Segments[Index];
        }

        return Result;
    }

    bool CopyTextToClipboard(const std::wstring& Text)
    {
        if (Text.empty())
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        struct FClipboardCloser
        {
            ~FClipboardCloser()
            {
                CloseClipboard();
            }
        } ClipboardCloser;

        if (!EmptyClipboard())
            return false;

        const size_t BufferSize = (Text.size() + 1) * sizeof(wchar_t);
        HGLOBAL GlobalMemory = GlobalAlloc(GMEM_MOVEABLE, BufferSize);

        if (!GlobalMemory)
            return false;

        void* Buffer = GlobalLock(GlobalMemory);

        if (!Buffer)
        {
            GlobalFree(GlobalMemory);
            return false;
        }

        memcpy(Buffer, Text.c_str(), BufferSize);
        GlobalUnlock(GlobalMemory);

        if (!SetClipboardData(CF_UNICODETEXT, GlobalMemory))
        {
            GlobalFree(GlobalMemory);
            return false;
        }

        return true;
    }
}

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
    UpdateInspectorToggleState();

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
    RenderInspector();
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

std::string CWorldUIManager::BuildWidgetPath(
    const std::shared_ptr<CWidget>& Widget) const
{
    if (!Widget)
        return std::string();

    std::vector<std::string> Segments;
    std::shared_ptr<CWidget> Current = Widget;

    while (Current)
    {
        Segments.push_back(Current->GetName());
        Current = Current->GetParent().lock();
    }

    if (Segments.empty())
        return std::string();

    std::reverse(Segments.begin(), Segments.end());

    std::string Result;

    for (size_t Index = 0; Index < Segments.size(); ++Index)
    {
        if (Index > 0)
            Result += "/";

        Result += Segments[Index];
    }

    return Result;
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

void CWorldUIManager::EnsureInspectorWidget()
{
    if (mInspectorText)
        return;

    mInspectorText = CWidget::CreateStaticWidget<CTextBlock>(
        "UIInspectorOverlay",
        mWorld,
        0);

    if (!mInspectorText)
        return;

    mInspectorText->SetText(TEXT(""));
    mInspectorText->SetFontSize(16.f);
    mInspectorText->SetSize(980.f, 92.f);
    mInspectorText->SetAlignH(ETextAlignH::Left);
    mInspectorText->SetAlignV(ETextAlignV::Top);
    mInspectorText->SetTextColor(255, 245, 170, 255);
    mInspectorText->EnableShadow(true);
    mInspectorText->SetShadowOffset(2.f, 2.f);
    mInspectorText->SetShadowTextColor(0, 0, 0, 255);
}

void CWorldUIManager::UpdateInspectorToggleState()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    const bool ToggleDown = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;

    if (ToggleDown && !mInspectorToggleKeyDown)
        mInspectorPinned = !mInspectorPinned;

    mInspectorToggleKeyDown = ToggleDown;

    const bool InspectorVisible =
        mInspectorPinned ||
        Input->GetAlt(EInputType::Hold);

    const bool CopyDown = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;

    if (InspectorVisible && CopyDown && !mInspectorCopyKeyDown)
    {
        const auto InspectedWidget =
            FindTopmostWidgetAtPoint(mWidgetList, Input->GetMousePos());
        const std::string ClipboardText =
            BuildClipboardWidgetPath(InspectedWidget);

        if (CopyTextToClipboard(ToWideString(ClipboardText)))
        {
            mInspectorStatusText = "Copied: " + ClipboardText;
            mInspectorStatusExpireTick = GetTickCount64() + 2000ULL;
        }
        else if (InspectedWidget)
        {
            mInspectorStatusText = "Copy failed";
            mInspectorStatusExpireTick = GetTickCount64() + 2000ULL;
        }
    }

    mInspectorCopyKeyDown = CopyDown;
}

void CWorldUIManager::RenderInspector()
{
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    const bool InspectorVisible =
        mInspectorPinned ||
        Input->GetAlt(EInputType::Hold);

    if (!InspectorVisible)
        return;

    EnsureInspectorWidget();

    if (!mInspectorText)
        return;

    const FVector2 MousePos = Input->GetMousePos();
    const auto InspectedWidget = FindTopmostWidgetAtPoint(mWidgetList, MousePos);
    const auto HoveredWidget = mHoveredWidget.lock();
    const std::string WidgetPath = BuildWidgetPath(InspectedWidget);
    const std::string WidgetName =
        InspectedWidget ? InspectedWidget->GetName() : "<none>";
    const char* WidgetType =
        InspectedWidget ? GetWidgetTypeName(InspectedWidget) : "None";
    const bool ShowHoveredTarget =
        HoveredWidget &&
        HoveredWidget != InspectedWidget;
    const bool ShowStatus =
        !mInspectorStatusText.empty() &&
        GetTickCount64() <= mInspectorStatusExpireTick;

    std::ostringstream Stream;
    Stream << "UI Inspector  [Alt: hold, F8: pin, F9: copy]\n";
    Stream << "Mouse: " << static_cast<int>(MousePos.x)
        << ", " << static_cast<int>(MousePos.y) << "\n";

    if (InspectedWidget)
    {
        Stream << "Path: Widget." << WidgetPath << "\n";
        Stream << "Name: WidgetName." << WidgetName
            << "  |  Type: " << WidgetType;

        if (ShowHoveredTarget)
        {
            Stream << "\nInput: Widget."
                << BuildWidgetPath(HoveredWidget);
        }
    }
    else
    {
        Stream << "Path: <no UI widget>\n";
        Stream << "Name: <none>  |  Type: " << WidgetType;

        if (ShowHoveredTarget)
        {
            Stream << "\nInput: Widget."
                << BuildWidgetPath(HoveredWidget);
        }
    }

    if (ShowStatus)
    {
        Stream << "\n" << mInspectorStatusText;
    }

    const std::wstring InspectorText = ToWideString(Stream.str());
    mInspectorText->SetText(InspectorText.c_str());

    const FResolution Resolution = CDevice::GetInst()->GetResolution();
    const float OverlayWidth =
        (std::max)(320.f, (std::min)(980.f, Resolution.Width - 24.f));
    float OverlayHeight = 92.f;

    if (ShowHoveredTarget)
        OverlayHeight += 24.f;

    if (ShowStatus)
        OverlayHeight += 24.f;
    float OverlayX = MousePos.x + 18.f;
    float OverlayY = MousePos.y + 18.f;

    if (OverlayX + OverlayWidth > Resolution.Width - 12.f)
        OverlayX = Resolution.Width - OverlayWidth - 12.f;

    if (OverlayY + OverlayHeight > Resolution.Height - 12.f)
        OverlayY = MousePos.y - OverlayHeight - 18.f;

    OverlayX = (std::max)(12.f, OverlayX);
    OverlayY = (std::max)(12.f, OverlayY);

    mInspectorText->SetSize(OverlayWidth, OverlayHeight);
    mInspectorText->SetPos(OverlayX, OverlayY);
    mInspectorText->Render();
}
