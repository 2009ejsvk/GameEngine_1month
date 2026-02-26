#pragma once

#include "SceneComponent.h"
#include "../UI/Widget.h"
#include "../World/World.h"

class CWidgetComponent :
    public CSceneComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CWidgetComponent();
	CWidgetComponent(const CWidgetComponent& ref);
	CWidgetComponent(CWidgetComponent&& ref)	noexcept;

public:
	virtual ~CWidgetComponent();

protected:
	std::shared_ptr<CWidget>	mWidget;

public:
	void SetWidget(const std::shared_ptr<CWidget>& Widget)
	{
		mWidget = Widget;
	}

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

protected:
	virtual CWidgetComponent* Clone()	const;

public:
	template <typename T>
	std::weak_ptr<T> SetWidget(const std::string& Name)
	{
		mWidget.reset(new T);

		mWidget->mWorld = mWorld;
		mWidget->mSelf = mWidget;
		mWidget->mUIManager = mWorld.lock()->GetUIManager();
		mWidget->SetName(Name);

		if (!mWidget->Init())
		{
			return std::weak_ptr<T>();
		}

		// dynamic_pointer_cast 를 통해 T 타입으로 변환한
		// shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
		return std::dynamic_pointer_cast<T>(mWidget);
	}
};

