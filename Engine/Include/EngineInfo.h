#pragma once

#include <Windows.h>
#include <vector>
#include <list>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <memory>

// 메모리 릭 체크용.
#include <crtdbg.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Matrix.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#define	SAFE_DELETE(p)	if(p)	{ delete p; p = nullptr; }
#define	SAFE_DELETE_ARRAY(p)	if(p)	{ delete[] p; p = nullptr; }
#define	SAFE_RELEASE(p)	if(p)	{ p->Release(); p = nullptr; }

struct FResolution
{
	int	Width = 0;
	int	Height = 0;
};

struct FVertexColor
{
	// 위치
	FVector3	Pos;
	
	// 색상
	FVector4	Color;

	FVertexColor()
	{
	}

	FVertexColor(float x, float y, float z, float r, float g, float b,
		float a) :
		Pos(x, y, z),
		Color(r, g, b, a)
	{
	}
};

struct FVertexTex
{
	// 위치
	FVector3	Pos;

	// UV
	FVector2	UV;

	FVertexTex()
	{
	}

	FVertexTex(float x, float y, float z, float u, float v) :
		Pos(x, y, z),
		UV(u, v)
	{
	}
};

struct FTextureFrame
{
	FVector2	Start;
	FVector2	Size;
};

enum class EAnimation2DTextureType
{
	None = -1,
	SpriteSheet,
	Frame
};

enum class EAssetType
{
	None = -1,
	Mesh,
	Shader,
	ConstantBuffer,
	Material,
	Texture,
	Animation2D
};

enum class EColliderType
{
	Box2D,
	Sphere2D
};

struct FBox2DInfo
{
	FVector3	Center;
	FVector3	Axis[2] =
	{
		FVector3::Axis[EAxis::X],
		FVector3::Axis[EAxis::Y]
	};
	FVector2	HalfSize = FVector2(1.f, 1.f);
};
