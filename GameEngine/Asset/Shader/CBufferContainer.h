#pragma once

#include "../../Matrix.h"

namespace EShaderBufferType
{
	enum Type : unsigned int
	{
		Vertex = 0x1,
		Pixel = 0x2,
		Domain = 0x4,
		Hull = 0x8,
		Geometry = 0x10,
		Compute = 0x20,
		VP = Vertex | Pixel,
		Graphic = Vertex | Pixel | Domain | Hull | Geometry,
		All = Graphic | Compute
	};
}

// Shader에 선언한 상수버퍼와 매칭되게 만들어야 한다.
struct FCBufferTransformData
{
	FMatrix	World;
	FMatrix	View;
	FMatrix	Proj;
	FMatrix	WV;
	FMatrix	WVP;
	FVector3	PivotSize;
	float		Empty;
};

struct FCBufferMaterialData
{
	FVector4	BaseColor;
	float		Opacity;
	FVector3	Empty;
};

struct FCBufferAnimation2DData
{
	FVector2	LTUV;
	FVector2	RBUV = FVector2(1.f, 1.f);
	int			Anim2DEnable = 0;
	int			Anim2DTextureType = -1;
	int			TextureSymmetry = 0;
	int			AnimFrame;
};

struct FCBufferColliderData
{
	FVector4	Color;
};

struct FCBufferUIDefaultData
{
	// 이미지 색상에 곱해줄 색상.
	FVector4 BrushTint;
	FVector4 WidgetColor;
	FVector2 BrushLTUV;
	FVector2 BrushRBUV;
	int BrushAnimEnable;
	int BrushTextureEnable;
	FVector2 BrushEmpty;
};

#define MAX_BLUR_COUNT  16

struct FCBufferBlur
{
	FVector2	TexelSize;
	int			Count;
	float BlueEmpty;
	FVector4 BlurWeight[(MAX_BLUR_COUNT + 1 + 3) / 4];
};

struct FCBufferTileMapData
{
	FVector2	LTUV;
	FVector2	RBUV;
};
