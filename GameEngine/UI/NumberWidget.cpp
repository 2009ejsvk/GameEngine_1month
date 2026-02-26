#include "NumberWidget.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../World/World.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../World/WorldAssetManager.h"

CNumberWidget::CNumberWidget()
{
}

CNumberWidget::CNumberWidget(const CNumberWidget& ref) :
	CWidget(ref)
{
	mBrush = ref.mBrush;
	mNumber = ref.mNumber;
}

CNumberWidget::~CNumberWidget()
{
}

bool CNumberWidget::SetTexture(const std::weak_ptr<CTexture>& Texture)
{
	mBrush.Texture = Texture;

	return true;
}

bool CNumberWidget::SetTexture(const std::string& Name)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

bool CNumberWidget::SetTexture(const std::string& Name, const TCHAR* FileName,
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

void CNumberWidget::SetTint(const FVector4& Tint)
{
	mBrush.Tint = Tint;
}

void CNumberWidget::SetTint(float r, float g, float b, float a)
{
	mBrush.Tint = FVector4(r, g, b, a);
}

void CNumberWidget::SetBrushAnimation(bool Animation)
{
	mBrush.AnimationEnable = Animation;
}

void CNumberWidget::AddBrushFrame(const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;
	Frame.Start = Start;
	Frame.Size = Size;

	mBrush.AnimationFrames.push_back(Frame);

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CNumberWidget::AddBrushFrame(float StartX, float StartY,
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

void CNumberWidget::SetCurrentFrame(int Frame)
{
	mBrush.Frame = Frame;
}

void CNumberWidget::SetAnimationPlayTime(float PlayTime)
{
	mBrush.PlayTime = PlayTime;

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CNumberWidget::SetAnimationPlayRate(float PlayRate)
{
	mBrush.PlayRate = PlayRate;
}

bool CNumberWidget::Init()
{
	CWidget::Init();

	return true;
}

void CNumberWidget::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);
}


void CNumberWidget::Render()
{
	CWidget::Render();

	// 음수인지 양수인지 구한다.
	bool Minus = false;

	if (mNumber < 0.f)
		Minus = true;

	// 정수 부분을 구한다.
	int	Number = (int)mNumber;

	// 소수점 부분을 구한다. 0.12345 = 12.345
	float	Decimal = mNumber - Number;

	int	Value = 1;

	for (int i = 0; i < mDecimalCount; ++i)
	{
		Value *= 10;
	}

	Decimal *= Value;

	int	DecimalNumber = (int)Decimal;

	std::stack<int>	NumberStack;

	while (Number > 0)
	{
		NumberStack.push(Number % 10);
		Number /= 10;
	}

	mNumberArray.clear();
	mDecimalNumberArray.clear();

	while (!NumberStack.empty())
	{
		mNumberArray.push_back(NumberStack.top());
		NumberStack.pop();
	}

	if (mNumberArray.empty())
		mNumberArray.push_back(0);

	// 소수점 자리를 만든다.
	while (DecimalNumber > 0)
	{
		NumberStack.push(DecimalNumber % 10);
		DecimalNumber /= 10;
	}

	while (!NumberStack.empty())
	{
		mDecimalNumberArray.push_back(NumberStack.top());
		NumberStack.pop();
	}

	FVector3	WidgetSize;

	WidgetSize.y = mSize.y;
	WidgetSize.x = mSize.x * mNumberArray.size() +
		mSize.x * mDecimalNumberArray.size();

	if (!mDecimalNumberArray.empty())
	{
		WidgetSize.x += mSize.x;
	}

	if (Minus)
	{
		// 위젯의 가로 크기를 구한다.
		WidgetSize.x += mSize.x;
	}

	mNumberRenderPos = mRenderPos;

	mNumberRenderPos.y -= mSize.y * 0.5f;
	mNumberRenderPos.x -= WidgetSize.x * 0.5f;

	if (Minus)
	{
		// 0123456789.-
		// 여기에서 - 를 출력한다.
		FVector2	FrameStart;
		FrameStart.x = mNumberSize.x * 11;
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart,
			mNumberSize);

		mNumberRenderPos.x += mSize.x;
	}

	RenderNumber();
}

bool CNumberWidget::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	return false;
}

CNumberWidget* CNumberWidget::Clone()	const
{
	return new CNumberWidget(*this);
}

void CNumberWidget::RenderNumber()
{
	FVector2	FrameStart;

	size_t	Size = mNumberArray.size();

	// 12345
	for (size_t i = 0; i < Size; ++i)
	{
		FrameStart.x = mNumberSize.x * mNumberArray[i];
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart,
			mNumberSize);

		mNumberRenderPos.x += mSize.x;
	}

	if (!mDecimalNumberArray.empty())
	{
		// . 을 출력한다.
		FrameStart.x = mNumberSize.x * 10;
		FrameStart.y = 0.f;

		RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart,
			mNumberSize);

		mNumberRenderPos.x += mSize.x;

		Size = mDecimalNumberArray.size();

		// 12345
		for (size_t i = 0; i < Size; ++i)
		{
			FrameStart.x = mNumberSize.x * mDecimalNumberArray[i];
			FrameStart.y = 0.f;

			RenderBrush(mBrush, mNumberRenderPos, mSize, FrameStart,
				mNumberSize);

			mNumberRenderPos.x += mSize.x;
		}
	}
}

