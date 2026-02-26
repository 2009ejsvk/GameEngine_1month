#include "TitleBar.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../World/World.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../World/WorldAssetManager.h"
#include "TextBlock.h"

CTitleBar::CTitleBar()
{
}

CTitleBar::CTitleBar(const CTitleBar& ref)	:
	CWidget(ref)
{
	mBrush = ref.mBrush;

	if (ref.mChild)
		mChild.reset(ref.mChild->Clone());
}

CTitleBar::~CTitleBar()
{
}

void CTitleBar::SetUpdateWidget(const std::weak_ptr<CWidget>& Widget)
{
	mUpdateWidget = Widget;
}

void CTitleBar::SetText(const TCHAR* Text)
{
	if (!mChild)
	{
		mChild =
			CreateStaticWidget<CTextBlock>("Text", mWorld);

		mChild->SetParent(mSelf);
		mChild->SetSize(mSize);
		mChild->SetUIManager(mUIManager);

		mChild->SetText(Text);
		mChild->EnableShadow(true);

		mChild->SetAlignH(ETextAlignH::Center);
	}
}

void CTitleBar::SetZOrder(int ZOrder)
{
	auto	UpdateWidget = mUpdateWidget.lock();

	if (UpdateWidget)
	{
		UpdateWidget->SetZOrder(ZOrder);
	}

	else
	{
		CWidget::SetZOrder(ZOrder);
	}
}

void CTitleBar::ReplaceZOrder()
{
	auto	UpdateWidget = mUpdateWidget.lock();

	if (UpdateWidget)
	{
		UpdateWidget->ReplaceZOrder();
	}

	else
	{
		CWidget::ReplaceZOrder();
	}
}

bool CTitleBar::SetTexture(const std::weak_ptr<class CTexture>& Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CTitleBar::SetTexture(const std::string& Name)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

bool CTitleBar::SetTexture(const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

void CTitleBar::SetTint(const FVector4& Tint)
{
	mBrush.Tint = Tint;
}

void CTitleBar::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CTitleBar::SetBrushAnimation(bool Animation)
{
	mBrush.AnimationEnable = Animation;
}

void CTitleBar::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;
	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CTitleBar::AddBrushFrame(float StartX, float StartY,
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

void CTitleBar::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CTitleBar::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CTitleBar::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

void CTitleBar::SetParentAll()
{
	if (mChild)
	{
		mChild->SetParent(mSelf);

		mChild->SetParentAll();
	}
}

void CTitleBar::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CTitleBar::Init()
{
	CWidget::Init();

	return true;
}

void CTitleBar::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	mBrush.PlayAnimation(DeltaTime);
}

void CTitleBar::Render()
{
	CWidget::Render();

	RenderBrush(mBrush, mRenderPos, mSize);

	if (mChild)
		mChild->Render();
}

bool CTitleBar::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	if (!CWidget::CollisionMouse(Result, MousePos))
		return false;

	return true;
}

void CTitleBar::MouseHovered()
{
	CWidget::MouseHovered();
}

void CTitleBar::MouseUnHovered()
{
	CWidget::MouseUnHovered();
}

bool CTitleBar::MouseDragStart(const FVector2& MousePos,
	std::shared_ptr<CWidget>& DragOperator)
{
	CWidget::MouseDragStart(MousePos, DragOperator);

	return true;
}

bool CTitleBar::MouseDrag(const FVector2& MousePos,
	const FVector2& MouseMove)
{
	CWidget::MouseDrag(MousePos, MouseMove);

	auto	UpdateWidget = mUpdateWidget.lock();

	if (UpdateWidget)
	{
		UpdateWidget->AddPos(MouseMove);
	}

	else
	{
	}

	return true;
}

bool CTitleBar::MouseDragEnd(const FVector2& MousePos)
{
	CWidget::MouseDragEnd(MousePos);

	return true;
}

CTitleBar* CTitleBar::Clone()	const
{
	return new CTitleBar(*this);
}
