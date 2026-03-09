#include "Image.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../World/World.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../World/WorldAssetManager.h"

CImage::CImage()
{
}

CImage::CImage(const CImage& ref)	:
	CWidget(ref)
{
	mBrush = ref.mBrush;
}

CImage::~CImage()
{
}

bool CImage::SetTexture(const std::weak_ptr<class CTexture>& Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CImage::SetTexture(const std::string& Name)
{
	auto	World = mWorld.lock();

	if (!World)
		return false;

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return false;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

bool CImage::SetTexture(const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	auto	World = mWorld.lock();

	if (!World)
		return false;

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return false;

	if (!AssetMgr->LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

void CImage::SetTint(const FVector4& Tint)
{
	mBrush.Tint = Tint;
}

void CImage::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CImage::SetBrushAnimation(bool Animation)
{
	mBrush.AnimationEnable = Animation;
}

void CImage::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;
	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CImage::AddBrushFrame(float StartX, float StartY,
	float SizeX, float SizeY)
{
	FTextureFrame	Frame;
	Frame.Start.x = StartX;
	Frame.Start.y = StartY;
	Frame.Size.x = SizeX;
	Frame.Size.y = SizeY;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CImage::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CImage::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CImage::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

bool CImage::Init()
{
	CWidget::Init();

	return true;
}

void CImage::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	mBrush.PlayAnimation(DeltaTime);
}


void CImage::Render()
{
	CWidget::Render();

	RenderBrush(mBrush, mRenderPos, mSize);
}

bool CImage::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	return false;
}

CImage* CImage::Clone()	const
{
	return new CImage(*this);
}
