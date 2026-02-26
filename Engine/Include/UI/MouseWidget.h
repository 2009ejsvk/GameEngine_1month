#pragma once

#include "Image.h"

class CMouseWidget :
    public CImage
{
	friend class CWorldUIManager;
	friend class CWidgetContainer;
	friend class CWidget;
	friend class CRenderManager;

protected:
	CMouseWidget();
	CMouseWidget(const CMouseWidget& ref);

public:
	virtual ~CMouseWidget();

public:
	virtual bool SetTexture(const std::string& Name);
	virtual bool SetTexture(const std::string& Name,
		const TCHAR* FileName,
		const std::string& PathName = "Texture");
	virtual bool SetTexture(const std::string& Name,
		const std::vector<const TCHAR*>& FileName,
		const std::string& PathName = "Texture");
	void AddBrushFrame(const FVector2& Start, const FVector2& Size,
		int Count);
	void AddBrushFrame(float StartX, float StartY, float SizeX,
		float SizeY, int Count);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual CMouseWidget* Clone()	const;
};

