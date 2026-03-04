#pragma once

#include "Widget.h"

namespace EButtonState
{
	enum Type
	{
		Normal,
		Hovered,
		Click,
		Disable,
		End
	};
}

namespace EButtonEventState
{
	enum Type
	{
		Hovered,
		Click,
		End
	};
}

class CButton :
    public CWidget
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;

protected:
	CButton();
	CButton(const CButton& ref);

public:
	virtual ~CButton();

protected:
	std::shared_ptr<CWidget>	mChild;
	FUIBrush	mBrush[EButtonState::End];
	EButtonState::Type	mState = EButtonState::Normal;

	std::weak_ptr<class CSound>	mSound[EButtonEventState::End];
	std::function<void()>	mEventCallback[EButtonEventState::End];

public:
	virtual void SetSize(const FVector3& Size) override;
	virtual void SetSize(float x, float y) override;
	void ButtonEnable(bool Enable)
	{
		mState = Enable ? EButtonState::Normal : EButtonState::Disable;
	}

	void SetChild(const std::weak_ptr<CWidget>& Child)
	{
		if (!Child.expired())
		{
			auto	_Child = Child.lock();

			_Child->SetParent(mSelf);
			_Child->SetSize(mSize);
		}

		if (!Child.expired())
		{
			mChild = Child.lock();

			mChild->SetUIManager(mUIManager);
			mChild->SetWorld(mWorld);
		}

		else
			mChild.reset();
	}
	virtual void SetOpacityAll(float Opacity);

public:
	bool SetTexture(EButtonState::Type State,
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(EButtonState::Type State, const std::string& Name);
	bool SetTexture(EButtonState::Type State, const std::string& Name,
		const TCHAR* FileName,
		const std::string& PathName = "Texture");
	void SetTint(EButtonState::Type State, const FVector4& Tint);
	void SetTint(EButtonState::Type State, float r, float g, float b,
		float a);
	void SetBrushAnimation(EButtonState::Type State, bool Animation);
	void AddBrushFrame(EButtonState::Type State,
		const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(EButtonState::Type State,
		float StartX, float StartY, float SizeX, float SizeY);
	void SetCurrentFrame(EButtonState::Type State, int Frame);
	void SetAnimationPlayTime(EButtonState::Type State,
		float PlayTime);
	void SetAnimationPlayRate(EButtonState::Type State,
		float PlayRate);
	void SetSound(EButtonEventState::Type State,
		const std::string& Name);
	void SetSound(EButtonEventState::Type State,
		const std::string& Name, const char* FileName,
		const std::string& PathName = "Sound");
	virtual void SetParentAll();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result,
		const FVector2& MousePos);
	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual CButton* Clone()	const;

public:
	template <typename T>
	void SetEventCallback(EButtonEventState::Type Type,
		T* Obj, void(T::* Func)())
	{
		mEventCallback[Type] = std::bind(Func, Obj);
	}
};

