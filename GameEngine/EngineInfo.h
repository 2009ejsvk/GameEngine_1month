#pragma once

#include <Windows.h>
#include <vector>
#include <list>
#include <stack>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <process.h>

// 메모리 릭 체크용.
#include <crtdbg.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <dwrite_3.h>
#include <d2d1.h>

#include "Matrix.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")

#define	SAFE_DELETE(p)	if(p)	{ delete p; p = nullptr; }
#define	SAFE_DELETE_ARRAY(p)	if(p)	{ delete[] p; p = nullptr; }
#define	SAFE_RELEASE(p)	if(p)	{ p->Release(); p = nullptr; }

#define	GRAVITY2D	980.f

struct FResolution
{
	int	Width = 0;
	int	Height = 0;
};

struct FVertexBuffer
{
	// 버텍스를 저장하기 위한 버퍼
	ID3D11Buffer* Buffer = nullptr;
	// 버텍스 1개의 메모리 크기
	int		Size = 0;
	// 버택스 개수
	int		Count = 0;

	FVertexBuffer()
	{
	}

	~FVertexBuffer()
	{
		if (Buffer)
			Buffer->Release();
	}
};

struct FIndexBuffer
{
	// 인덱스를 저장하기 위한 버퍼
	ID3D11Buffer* Buffer = nullptr;
	// 인덱스 1개의 메모리 크기
	int		Size = 0;
	// 인덱스 개수
	int		Count = 0;
	// 인덱스를 이용해서 출력할 때 인덱스 타입을 지정하는 용도로 사용한다.
	DXGI_FORMAT	Fmt = DXGI_FORMAT_UNKNOWN;

	FIndexBuffer()
	{
	}

	~FIndexBuffer()
	{
		if (Buffer)
			Buffer->Release();
	}
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
	Frame,
	Array
};

enum class EAssetType
{
	None = -1,
	Mesh,
	Shader,
	ConstantBuffer,
	Material,
	Texture,
	Animation2D,
	Sound,
	Font,
	FontCollection
};

enum class EColliderType
{
	Box2D,
	Sphere2D,
	Line2D
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

namespace ECollisionChannel
{
	enum Type
	{
		Static,
		Player,
		Monster,
		Custom1,
		Custom2,
		Custom3,
		Custom4,
		Custom5,
		Custom6,
		Custom7,
		Custom8,
		Custom9,
		Custom10,
		End
	};
};

struct FCollisionChannel
{
	std::string		Name;
	ECollisionChannel::Type	Channel = ECollisionChannel::Static;
};

namespace ECollisionInteraction
{
	enum Type
	{
		Ignore,		// 무시
		Overlap,	// 겹침
		Block,		// 충돌
		End
	};
}

/*
1번프로파일
Channel : Player
Enable : true
Interaction[Static] = Collision;
Interaction[Player] = Ignore;
Interaction[Monster] = Collision;
Interaction[Custom1] = Collision;
Interaction[Custom2] = Ignore;


2번프로파일
Channel : Monster
Enable : true
Interaction[Static] = Collision;
Interaction[Player] = Ignore;
Interaction[Monster] = Ignore;
Interaction[Custom1] = Ignore;
Interaction[Custom2] = Collision;
*/
struct FCollisionProfile
{
	std::string		Name;
	// 프로파일이 어떤 채널을 사용하는지.
	FCollisionChannel*	Channel;
	bool			Enable = true;
	ECollisionInteraction::Type	Interaction[ECollisionChannel::End] = {};
};

struct FSphere2DInfo
{
	FVector3	Center;
	float		Radius = 0.f;
};

struct FLine2DInfo
{
	FVector3	Start;
	FVector3	End;
};

struct FCollisionManifold
{
	FVector3	ContactPoint;		// 접촉점
	FVector3	Normal;				// Src -> Dest 방향 벡터
	float		Penetration = 0.f;	// 침투 깊이
};

namespace EMouseState
{
	enum Type
	{
		Normal,
		State1,
		State2,
		State3,
		State4,
		State5,
		State6,
		State7,
		State8,
		State9,
		State10,
		End
	};
}

enum ETileShape
{
	Rect,
	Isometric
};

namespace ETileTextureType
{
	enum Type
	{
		Back,
		Tile,
		End
	};
}

struct FTileFrame
{
	FVector2	Start;
	FVector2	End;
};

struct FInstancingData
{
	FVector4	WVP0;
	FVector4	WVP1;
	FVector4	WVP2;
	FVector4	WVP3;
	FVector2	LTUV;
	FVector2	RBUV;
	FVector4	BaseColor;
	FVector3	PivotSize;
	int			ArrayTextureEnable = 0;
	int			AnimFrame = 0;
};

enum class ETileType
{
	Normal,
	UnableToMove,
	End
};


template <typename T>
T Clamp(T Value, T Min, T Max)
{
	if (Min >= Max)
		return Value;

	if (Value < Min)
		return Min;

	else if (Value > Max)
		return Max;

	return Value;
}
