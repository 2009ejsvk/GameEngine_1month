#pragma once

#include "../EngineInfo.h"

class CTile
{
	friend class CTileMapComponent;

private:
	CTile();
	CTile(const CTile& ref);
	CTile(CTile&& ref)	noexcept;

public:
	~CTile();

private:
	ETileType	mType = ETileType::Normal;
	FVector2	mPos;
	FVector2	mSize;
	FVector2	mCenter;
	int			mTextureFrame = -1;
	FVector2	mFrameStart;
	FVector2	mFrameEnd;
	bool		mRender = false;
	bool		mOutLineRender = false;
	FVector4	mOutLineColor = FVector4::White;
	int			mIndexX = 0;
	int			mIndexY = 0;
	int			mIndex = 0;

public:
	int GetIndexX()	const
	{
		return mIndexX;
	}

	int GetIndexY()	const
	{
		return mIndexY;
	}

	int GetIndex()	const
	{
		return mIndex;
	}

	const FVector4& GetOutLineColor()	const
	{
		return mOutLineColor;
	}

	bool GetOutLineRender()	const
	{
		return mOutLineRender;
	}

	const FVector2& GetFrameStart()	const
	{
		return mFrameStart;
	}

	const FVector2& GetFrameEnd()	const
	{
		return mFrameEnd;
	}

	bool GetRender()	const
	{
		return mRender;
	}

	ETileType GetType()	const
	{
		return mType;
	}

	int GetTextureFrame()	const
	{
		return mTextureFrame;
	}

	const FVector2& GetPos()	const
	{
		return mPos;
	}

	const FVector2& GetSize()	const
	{
		return mSize;
	}

	const FVector2& GetCenter()	const
	{
		return mCenter;
	}

public:
	void SetIndex(int x, int y, int Index)
	{
		mIndexX = x;
		mIndexY = y;
		mIndex = Index;
	}

	void SetOutLineRender(bool Render)
	{
		mOutLineRender = Render;
	}

	void SetOutLineColor(const FVector4& Color)
	{
		mOutLineColor = Color;
	}

	void SetOutLineColor(float r, float g, float b, float a)
	{
		mOutLineColor = FVector4(r, g, b, a);
	}

	void SetTileType(ETileType Type)
	{
		mType = Type;
	}

	void SetTextureFrame(int Frame)
	{
		mTextureFrame = Frame;
	}

	void SetPos(const FVector2& Pos)
	{
		mPos = Pos;
	}

	void SetPos(float x, float y)
	{
		mPos.x = x;
		mPos.y = y;
	}

	void SetSize(const FVector2& Size)
	{
		mSize = Size;
	}

	void SetSize(float x, float y)
	{
		mSize.x = x;
		mSize.y = y;
	}

	void SetCenter(const FVector2& Center)
	{
		mCenter = Center;
	}

	void SetCenter(float x, float y)
	{
		mCenter.x = x;
		mCenter.y = y;
	}

	void SetFrame(const FVector2& Start, const FVector2& End)
	{
		mFrameStart = Start;
		mFrameEnd = End;

		mRender = true;
	}

	void SetFrame(float StartX, float StartY,
		float EndX, float EndY)
	{
		mFrameStart.x = StartX;
		mFrameStart.y = StartY;

		mFrameEnd.x = EndX;
		mFrameEnd.y = EndY;

		mRender = true;
	}

public:
	void Save(FILE* File);
	void Load(FILE* File);
};

