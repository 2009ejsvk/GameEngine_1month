#pragma once

#include "../EngineInfo.h"
#include "../UI/WidgetContainer.h"

class CWorldUIManager
{
	friend class CWorld;

private:
	CWorldUIManager();

public:
	~CWorldUIManager();

private:
	std::weak_ptr<class CWorld>	mWorld;
	std::weak_ptr<CWorldUIManager>	mSelf;
	std::vector<std::shared_ptr<CWidgetContainer>>	mWidgetList;
	std::weak_ptr<CWidget>		mHoveredWidget;
	std::weak_ptr<CWidget>		mDragWidget;
	std::shared_ptr<CWidget>		mDragOperatorWidget;
    std::shared_ptr<class CTextBlock> mInspectorText;
    bool mInspectorPinned = false;
    bool mInspectorToggleKeyDown = false;
    bool mInspectorCopyKeyDown = false;
    std::string mInspectorStatusText;
    ULONGLONG mInspectorStatusExpireTick = 0;

public:
	bool Init();
	void Update(float DeltaTime);
	bool IsPointOverEnabledWidget(const FVector2& MousePos);
	bool CollisionMouse(float DeltaTime, const FVector2& MousePos);
	void Render();
	std::weak_ptr<CWidgetContainer> GetWidget(const std::string& Name);

    const std::vector<std::shared_ptr<CWidgetContainer>>& GetWidgetList() const
    {
        return mWidgetList;
    }

    std::weak_ptr<CWidget> GetHoveredWidget() const
    {
        return mHoveredWidget;
    }

    std::string BuildWidgetPath(const std::shared_ptr<CWidget>& Widget) const;

private:
	static bool SortRender(const std::shared_ptr<CWidgetContainer>& Src, const std::shared_ptr<CWidgetContainer>& Dest);
	static bool SortCollision(const std::shared_ptr<CWidgetContainer>& Src, const std::shared_ptr<CWidgetContainer>& Dest);
    void EnsureInspectorWidget();
    void UpdateInspectorToggleState();
    void RenderInspector();

public:
	template <typename T>
	std::weak_ptr<T> CreateWidget(const std::string& Name,
		int ZOrder = 0)
	{
		std::shared_ptr<T> Widget;

		Widget.reset(new T);

		Widget->SetWorld(mWorld);
		Widget->SetSelf(Widget);
		Widget->SetUIManager(mSelf);
		Widget->SetName(Name);
		Widget->SetZOrder(ZOrder);

		if (!Widget->Init())
		{
			return std::weak_ptr<T>();
		}

		mWidgetList.push_back(std::dynamic_pointer_cast<CWidgetContainer>(Widget));

		// dynamic_pointer_cast 를 통해 T 타입으로 변환한
		// shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
		return std::dynamic_pointer_cast<T>(Widget);
	}

	template <typename T>
	std::weak_ptr<T> FindWidget(const std::string& Name)
	{
		auto	iter = mWidgetList.begin();
		auto	iterEnd = mWidgetList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->GetName() == Name)
				return std::dynamic_pointer_cast<T>(*iter);
		}

		return std::weak_ptr<T>();
	}
};

