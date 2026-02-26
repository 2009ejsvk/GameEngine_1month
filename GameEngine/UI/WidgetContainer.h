#pragma once

#include "Widget.h"

class CWidgetContainer :
    public CWidget
{
    friend class CWorldUIManager;
	friend class CWidget;

protected:
    CWidgetContainer();
	CWidgetContainer(const CWidgetContainer& ref);

public:
    virtual ~CWidgetContainer();

protected:
    std::vector<std::shared_ptr<CWidget>>   mChildList;

public:
    void AddWidget(const std::shared_ptr<CWidget>& Widget)
    {
        Widget->mParent = mSelf;
		Widget->mUIManager = mUIManager;
		Widget->mWorld = mWorld;
        mChildList.push_back(Widget);
    }

	virtual void SetParentAll();
	virtual void SetOpacityAll(float Opacity);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result,
		const FVector2& MousePos);
	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual bool MouseDragStart(const FVector2& MousePos,
		std::shared_ptr<CWidget>& DragOperator);
	virtual bool MouseDrag(const FVector2& MousePos,
		const FVector2& MouseMove);
	virtual bool MouseDragEnd(const FVector2& MousePos);
	virtual CWidgetContainer* Clone()	const;

public:
	template <typename T>
	std::weak_ptr<T> CreateWidget(const std::string& Name,
		int ZOrder = 0)
	{
		std::shared_ptr<T> Widget;

		Widget.reset(new T);
		
		Widget->mWorld = mWorld;
		Widget->mSelf = Widget;
		Widget->mUIManager = mUIManager;
		Widget->SetName(Name);
		Widget->mParent = mSelf;
		Widget->SetZOrder(ZOrder);

		if (!Widget->Init())
		{
			return std::weak_ptr<T>();
		}

		mChildList.push_back(std::dynamic_pointer_cast<CWidget>(Widget));

		// dynamic_pointer_cast 를 통해 T 타입으로 변환한
		// shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
		return std::dynamic_pointer_cast<T>(Widget);
	}

	template <typename T>
	std::weak_ptr<T> FindWidget(const std::string& Name)
	{
		auto	iter = mChildList.begin();
		auto	iterEnd = mChildList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->GetName() == Name)
				return std::dynamic_pointer_cast<T>(*iter);
		}

		return std::weak_ptr<T>();
	}
};

