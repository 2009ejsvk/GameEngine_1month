#include "ProgressBar.h"
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

CProgressBar::CProgressBar()
{
}

CProgressBar::CProgressBar(const CProgressBar& ref)	:
	CWidget(ref)
{
	for (int i = 0; i < EProgressBarImageType::End; ++i)
	{
		mBrush[i] = ref.mBrush[i];
	}

	mBarDir = ref.mBarDir;
	mPercent = ref.mPercent;
	mMinValue = ref.mMinValue;
	mMaxValue = ref.mMaxValue;
	mTextEnable = ref.mTextEnable;

	if (ref.mChild)
	{
		mChild.reset(ref.mChild->Clone());
	}

	FUIBrush	mBrush[EProgressBarImageType::End];
	EProgressBarDir	mBarDir = EProgressBarDir::RightToLeft;
	float		mPercent = 1.f;
	float		mMinValue = 0.f;
	float		mMaxValue = 1.f;
	std::shared_ptr<class CTextBlock>	mChild;
	bool		mTextEnable = false;
}

CProgressBar::~CProgressBar()
{
}

void CProgressBar::SetTextEnable(bool Enable)
{
	mTextEnable = Enable;

	if (mTextEnable)
	{
		mChild =
			CreateStaticWidget<CTextBlock>("Text", mWorld);

		mChild->SetParent(mSelf);
		mChild->SetSize(mSize);
		mChild->SetUIManager(mUIManager);

		mChild->SetTextFromFloat(mMinValue);
		mChild->AddText(TEXT(" / "));
		mChild->AddTextFromFloat(mMaxValue);
		mChild->SetTextColor(255, 255, 0, 255);
	}

	else
	{
		if (mChild)
			mChild.reset();
	}
}

void CProgressBar::SetMin(float Min)
{
	if (Min >= mMaxValue)
		return;

	mMinValue = Min;

	if (mChild)
	{
		mChild->SetTextFromFloat(mMinValue);
		mChild->AddText(TEXT(" / "));
		mChild->AddTextFromFloat(mMaxValue);
	}
}

void CProgressBar::SetMax(float Max)
{
	if (Max <= mMinValue)
		return;

	mMaxValue = Max;

	if (mChild)
	{
		mChild->SetTextFromFloat(mMinValue);
		mChild->AddText(TEXT(" / "));
		mChild->AddTextFromFloat(mMaxValue);
	}
}

void CProgressBar::SetPercent(float Percent)
{
	mPercent = Percent;

	if (mPercent < 0.f)
		mPercent = 0.f;

	else if (mPercent > 1.f)
		mPercent = 1.f;
}

void CProgressBar::SetValue(float Value)
{
	Value = Value - mMinValue;

	mPercent = Value / (mMaxValue - mMinValue);
}

void CProgressBar::SetBarDir(EProgressBarDir Dir)
{
	mBarDir = Dir;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type State,
	const std::weak_ptr<class CTexture>& Texture)
{
	mBrush[State].Texture = Texture;

	return true;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type State,
	const std::string& Name)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush[State].Texture = Texture;

	return true;
}

bool CProgressBar::SetTexture(EProgressBarImageType::Type State,
	const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr->LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush[State].Texture = Texture;

	return true;
}

void CProgressBar::SetTint(EProgressBarImageType::Type State, const FVector4& Tint)
{
	mBrush[State].Tint = Tint;
}

void CProgressBar::SetTint(EProgressBarImageType::Type State, float r, float g,
	float b, float a)
{
	mBrush[State].Tint = FVector4(r, g, b, a);
}

void CProgressBar::SetBrushAnimation(EProgressBarImageType::Type State,
	bool Animation)
{
	mBrush[State].AnimationEnable = Animation;
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type State,
	const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;
	Frame.Start = Start;
	Frame.Size = Size;

	mBrush[State].AnimationFrames.push_back(Frame);

	mBrush[State].FrameTime = mBrush[State].PlayTime /
		mBrush[State].AnimationFrames.size();
}

void CProgressBar::AddBrushFrame(EProgressBarImageType::Type State,
	float StartX, float StartY, float SizeX, float SizeY)
{
	FTextureFrame	Frame;
	Frame.Start.x = StartX;
	Frame.Start.y = StartY;
	Frame.Size.x = SizeX;
	Frame.Size.y = SizeY;

	mBrush[State].AnimationFrames.push_back(Frame);

	mBrush[State].FrameTime = mBrush[State].PlayTime /
		mBrush[State].AnimationFrames.size();
}

void CProgressBar::SetCurrentFrame(EProgressBarImageType::Type State, int Frame)
{
	mBrush[State].Frame = Frame;
}

void CProgressBar::SetAnimationPlayTime(EProgressBarImageType::Type State,
	float PlayTime)
{
	mBrush[State].PlayTime = PlayTime;

	mBrush[State].FrameTime = mBrush[State].PlayTime /
		mBrush[State].AnimationFrames.size();
}

void CProgressBar::SetAnimationPlayRate(EProgressBarImageType::Type State,
	float PlayRate)
{
	mBrush[State].PlayRate = PlayRate;
}

void CProgressBar::SetParentAll()
{
	if (mChild)
	{
		mChild->SetParent(mSelf);

		mChild->SetParentAll();
	}
}

void CProgressBar::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CProgressBar::Init()
{
	CWidget::Init();

	SetTint(EProgressBarImageType::Back,
		0, 0, 0, 0);

	return true;
}

void CProgressBar::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	mBrush[EProgressBarImageType::Back].PlayAnimation(DeltaTime);
	mBrush[EProgressBarImageType::Fill].PlayAnimation(DeltaTime);
}


void CProgressBar::Render()
{
	CWidget::Render();

	// Back 출력
	RenderBrush(mBrush[EProgressBarImageType::Back],
		mRenderPos, mSize);

	FVector3	Size = mSize;
	FVector3	Pos = mRenderPos;

	switch (mBarDir)
	{
	case EProgressBarDir::RightToLeft:
		Size.x *= mPercent;
		break;
	case EProgressBarDir::TopToBottom:
		Size.y *= mPercent;
		Pos.y += mSize.y * (1.f - mPercent);
		break;
	}

	// Fill 출력
	RenderBrush(mBrush[EProgressBarImageType::Fill], Pos, Size);

	if (mTextEnable)
	{
		mChild->Render();
	}
}

bool CProgressBar::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	if (!CWidget::CollisionMouse(Result, MousePos))
		return false;

	return true;
}

void CProgressBar::MouseHovered()
{
}

void CProgressBar::MouseUnHovered()
{
}

CProgressBar* CProgressBar::Clone()	const
{
	return new CProgressBar(*this);
}
