#pragma once

#include "Widget.h"

class CTitleBar :
    public CWidget
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;

protected:
	CTitleBar();
	CTitleBar(const CTitleBar& ref);

public:
	virtual ~CTitleBar();

protected:
	FUIBrush	mBrush;
	std::shared_ptr<class CTextBlock>	mChild;
	std::weak_ptr<CWidget>		mUpdateWidget;

public:
	void SetUpdateWidget(const std::weak_ptr<CWidget>& Widget);
	void SetText(const TCHAR* Text);
	virtual void SetZOrder(int ZOrder);
	virtual void ReplaceZOrder();

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
	virtual CTitleBar* Clone()	const;
};

