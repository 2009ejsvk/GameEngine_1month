#pragma once

#include "Widget.h"

class CImage :
    public CWidget
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;

protected:
	CImage();
	CImage(const CImage& ref);

public:
	virtual ~CImage();

protected:
	FUIBrush	mBrush;

public:
	bool SetTexture(
		const std::weak_ptr<class CTexture>& Texture);
	virtual bool SetTexture(const std::string& Name);
	virtual bool SetTexture(const std::string& Name,
		const TCHAR* FileName,
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
	virtual CImage* Clone()	const;
};

