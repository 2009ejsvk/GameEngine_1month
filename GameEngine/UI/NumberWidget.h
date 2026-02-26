#pragma once

#include "Widget.h"

class CNumberWidget :
    public CWidget
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;

protected:
	CNumberWidget();
	CNumberWidget(const CNumberWidget& ref);

public:
	virtual ~CNumberWidget();

protected:
	FUIBrush	mBrush;
	float		mNumber = 0.f;
	int			mDecimalCount = 2;
	FVector3	mNumberSize;
	std::vector<int>	mNumberArray;
	std::vector<int>	mDecimalNumberArray;
	FVector3	mNumberRenderPos;

public:
	void SetDecimalCount(int Count)
	{
		mDecimalCount = Count;
	}

	void SetNumber(float Number)
	{
		mNumber = Number;
	}

	void AddNumber(float Number)
	{
		mNumber += Number;
	}

	void SetNumberSize(const FVector3& Size)
	{
		mNumberSize = Size;
	}

	void SetNumberSize(float x, float y)
	{
		mNumberSize.x = x;
		mNumberSize.y = y;
	}

public:
	bool SetTexture(const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(const std::string& Name);
	bool SetTexture(const std::string& Name, const TCHAR* FileName,
		const std::string& PathName = "Texture");
	void SetTint(const FVector4& Tint);
	void SetTint(float r, float g, float b, float a);
	void SetBrushAnimation(bool Animation);
	void AddBrushFrame(const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(float StartX, float StartY, float SizeX,
		float SizeY);
	void SetCurrentFrame(int Frame);
	void SetAnimationPlayTime(float PlayTime);
	void SetAnimationPlayRate(float PlayRate);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result,
		const FVector2& MousePos);
	virtual CNumberWidget* Clone()	const;

private:
	void RenderNumber();
};

