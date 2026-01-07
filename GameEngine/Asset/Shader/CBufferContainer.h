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
	FVector2	RBUV;
	int			Anim2DEnable = 0;
	int			Anim2DTextureType = -1;
	int			TextureSymmetry = 0;
	float		Empty;
};

struct FCBufferColliderData
{
	FVector4	Color;
};
