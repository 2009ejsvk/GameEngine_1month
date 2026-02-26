#pragma once

#include "Widget.h"

namespace EProgressBarImageType
{
	enum Type
	{
		Back,
		Fill,
		End
	};
}

enum class EProgressBarDir
{
	RightToLeft,
	TopToBottom
};

class CProgressBar :
    public CWidget
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;

protected:
	CProgressBar();
	CProgressBar(const CProgressBar& ref);

public:
	virtual ~CProgressBar();

protected:
	FUIBrush	mBrush[EProgressBarImageType::End];
	EProgressBarDir	mBarDir = EProgressBarDir::RightToLeft;
	float		mPercent = 1.f;
	float		mMinValue = 0.f;
	float		mMaxValue = 1.f;
	std::shared_ptr<class CTextBlock>	mChild;
	bool		mTextEnable = false;

public:
	void SetTextEnable(bool Enable);
	void SetMin(float Min);
	void SetMax(float Max);
	void SetPercent(float Percent);
	void SetValue(float Value);
	void SetBarDir(EProgressBarDir Dir);

public:
	bool SetTexture(EProgressBarImageType::Type State,
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(EProgressBarImageType::Type State, const std::string& Name);
	bool SetTexture(EProgressBarImageType::Type State, const std::string& Name,
		const TCHAR* FileName,
		const std::string& PathName = "Texture");
	void SetTint(EProgressBarImageType::Type State, const FVector4& Tint);
	void SetTint(EProgressBarImageType::Type State, float r, float g, float b,
		float a);
	void SetBrushAnimation(EProgressBarImageType::Type State, bool Animation);
	void AddBrushFrame(EProgressBarImageType::Type State,
		const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(EProgressBarImageType::Type State,
		float StartX, float StartY, float SizeX, float SizeY);
	void SetCurrentFrame(EProgressBarImageType::Type State, int Frame);
	void SetAnimationPlayTime(EProgressBarImageType::Type State,
		float PlayTime);
	void SetAnimationPlayRate(EProgressBarImageType::Type State,
		float PlayRate);
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
	virtual CProgressBar* Clone()	const;
};

