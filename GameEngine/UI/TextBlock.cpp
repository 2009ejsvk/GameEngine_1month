#include "TextBlock.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Font/FontManager.h"
#include "../Device.h"

CTextBlock::CTextBlock()
{
}

CTextBlock::CTextBlock(const CTextBlock& ref)	:
	CWidget(ref)
{
	mTarget = ref.mTarget;
	mText = ref.mText;
	mFont = ref.mFont;
	mTextColor = ref.mTextColor;
	mTransparency = ref.mTransparency;
	mOpacity = ref.mOpacity;

	mShadow = ref.mShadow;
	mTextShadowColor = ref.mTextShadowColor;
	mShadowTransparency = ref.mShadowTransparency;
	mShadowOpacity = ref.mShadowOpacity;
	mShadowOffset = ref.mShadowOffset;

	mFontSize = ref.mFontSize;
	mAlignH = ref.mAlignH;
	mAlignV = ref.mAlignV;

	mLayout = nullptr;

	CreateTextLayout();
}

CTextBlock::~CTextBlock()
{
	SAFE_RELEASE(mLayout);
}

void CTextBlock::SetFont(const std::string& FontName)
{
	auto FontMgr = CAssetManager::GetInst()->GetFontManager().lock();

	mFont = FontMgr->FindFont(FontName);

	CreateTextLayout();
}

void CTextBlock::SetText(const TCHAR* Text)
{
	if (!Text)
		Text = TEXT("");

	if (mText == Text)
		return;

	mText = Text;

	CreateTextLayout();
}

void CTextBlock::SetTextFromInt(int Number)
{
	mText = std::to_wstring(Number);

	CreateTextLayout();
}

void CTextBlock::SetTextFromFloat(float Number)
{
	mText = std::to_wstring(Number);

	CreateTextLayout();
}

void CTextBlock::AddText(const TCHAR* Text)
{
	mText += Text;

	CreateTextLayout();
}

void CTextBlock::AddTextFromInt(int Number)
{
	mText += std::to_wstring(Number);

	CreateTextLayout();
}

void CTextBlock::AddTextFromFloat(float Number)
{
	mText += std::to_wstring(Number);

	CreateTextLayout();
}

void CTextBlock::PopBack()
{
	mText.pop_back();

	CreateTextLayout();
}

void CTextBlock::ClearText()
{
	mText.clear();

	CreateTextLayout();
}

void CTextBlock::SetFontSize(float Size)
{
	mFontSize = Size;

	if (!mLayout)
	{
		CreateTextLayout();
	}

	else
	{
		DWRITE_TEXT_RANGE	Range = {};

		Range.startPosition = 0;
		Range.length = (UINT32)mText.length();

		mLayout->SetFontSize(mFontSize, Range);
	}
}

void CTextBlock::SetAlignH(ETextAlignH Align)
{
	mAlignH = Align;

	if (!mLayout)
	{
		CreateTextLayout();
	}

	else
	{
		switch (mAlignH)
		{
		case ETextAlignH::Left:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			break;
		case ETextAlignH::Center:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			break;
		case ETextAlignH::Right:
			mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
			break;
		}
	}
}

void CTextBlock::SetAlignV(ETextAlignV Align)
{
	mAlignV = Align;

	if (!mLayout)
	{
		CreateTextLayout();
	}

	else
	{
		switch (mAlignV)
		{
		case ETextAlignV::Top:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			break;
		case ETextAlignV::Middle:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			break;
		case ETextAlignV::Bottom:
			mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
			break;
		}
	}
}

void CTextBlock::EnableTransparency(bool Transparency)
{
	mTransparency = Transparency;
}

void CTextBlock::SetOpacity(float Opacity)
{
	mOpacity = Opacity;

	if (mOpacity < 0.f)
		mOpacity = 0.f;

	else if (mOpacity > 1.f)
		mOpacity = 1.f;
}

void CTextBlock::AddOpacity(float Opacity)
{
	mOpacity += Opacity;

	if (mOpacity < 0.f)
		mOpacity = 0.f;

	else if (mOpacity > 1.f)
		mOpacity = 1.f;
}

void CTextBlock::SetTextColor(unsigned char r, unsigned char g, 
	unsigned char b, unsigned char a)
{
	auto FontMgr = CAssetManager::GetInst()->GetFontManager().lock();

	FontMgr->CreateFontColor(r, g, b, a);

	mTextColor = FontMgr->FindFontColor(r, g, b, a);
}

void CTextBlock::SetTextColor(const FVector4& Color)
{
	auto FontMgr = CAssetManager::GetInst()->GetFontManager().lock();

	FontMgr->CreateFontColor(Color);

	mTextColor = FontMgr->FindFontColor(Color);
}

void CTextBlock::EnableShadow(bool Enable)
{
	mShadow = Enable;
}

void CTextBlock::EnableShadowTransparency(bool Transparency)
{
	mShadowTransparency = Transparency;
}

void CTextBlock::SetShadowOpacity(float Opacity)
{
	mShadowOpacity = Opacity;

	if (mShadowOpacity < 0.f)
		mShadowOpacity = 0.f;

	else if (mShadowOpacity > 1.f)
		mShadowOpacity = 1.f;
}

void CTextBlock::AddShadowOpacity(float Opacity)
{
	mShadowOpacity += Opacity;

	if (mShadowOpacity < 0.f)
		mShadowOpacity = 0.f;

	else if (mShadowOpacity > 1.f)
		mShadowOpacity = 1.f;
}

void CTextBlock::SetShadowTextColor(unsigned char r, unsigned char g,
	unsigned char b, unsigned char a)
{
	auto FontMgr = CAssetManager::GetInst()->GetFontManager().lock();

	FontMgr->CreateFontColor(r, g, b, a);

	mTextShadowColor = FontMgr->FindFontColor(r, g, b, a);
}

void CTextBlock::SetShadowTextColor(const FVector4& Color)
{
	auto FontMgr = CAssetManager::GetInst()->GetFontManager().lock();

	FontMgr->CreateFontColor(Color);

	mTextShadowColor = FontMgr->FindFontColor(Color);
}

void CTextBlock::SetShadowOffset(const FVector2& Offset)
{
	mShadowOffset = Offset;
}

void CTextBlock::SetShadowOffset(float x, float y)
{
	mShadowOffset.x = x;
	mShadowOffset.y = y;
}

void CTextBlock::CreateTextLayout()
{
	SAFE_RELEASE(mLayout);

	auto	Font = mFont.lock();

	if (!Font)
		return;

	if (mSize.x <= 0.f || mSize.y <= 0.f)
		return;

	mLayout = Font->CreateLayout(mText.c_str(),
		(int)mText.length(), mSize.x, mSize.y);

	if (!mLayout)
		return;

	DWRITE_TEXT_RANGE	Range = {};

	Range.startPosition = 0;
	Range.length = (UINT32)mText.length();

	mLayout->SetFontSize(mFontSize, Range);

	switch (mAlignH)
	{
	case ETextAlignH::Left:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		break;
	case ETextAlignH::Center:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		break;
	case ETextAlignH::Right:
		mLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		break;
	}

	switch (mAlignV)
	{
	case ETextAlignV::Top:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		break;
	case ETextAlignV::Middle:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		break;
	case ETextAlignV::Bottom:
		mLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		break;
	}
}

bool CTextBlock::Init()
{
	CWidget::Init();

	mTarget = CDevice::GetInst()->Get2DTarget();

	SetFont("EngineDefault");
	SetText(TEXT("Text"));
	SetTextColor(FVector4::White);
	SetShadowTextColor(FVector4::Black);

	CreateTextLayout();

	return true;
}

void CTextBlock::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);
}

void CTextBlock::Render()
{
	CWidget::Render();

	if (!mTarget || !mLayout || !mTextColor)
		return;

	mTarget->BeginDraw();

	// 그림자를 출력할 경우
	if (mShadow)
	{
		if (!mTextShadowColor)
		{
			mTarget->EndDraw();
			return;
		}

		D2D1_POINT_2F	ShadowPoint;

		ShadowPoint.x = mRenderPos.x + mShadowOffset.x;
		ShadowPoint.y = mRenderPos.y + mShadowOffset.y;

		if (mShadowTransparency)
			mTextShadowColor->SetOpacity(mShadowOpacity);

		else
			mTextShadowColor->SetOpacity(1.f);

		/*
		D2D1_DRAW_TEXT_OPTIONS_NONE : 기본값. 레이아웃 사각형 밖으로
		나가도 클리핑을 안한다.
		D2D1_DRAW_TEXT_OPTIONS_NO_SNAP : 픽셀 스탭을 끈다. 스크롤 되는
		텍스트 출력에 권장된다.
		D2D1_DRAW_TEXT_OPTIONS_CLIP : 텍스트를 레이아웃 사각형 안에서만
		보이게 클리핑한다.
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : 컬러로 폰트 렌더링
		가능.
		D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING : 비트맵
		으로 컬러 그리핑을 처리하는 경우.
		*/
		mTarget->DrawTextLayout(ShadowPoint, mLayout,
			mTextShadowColor, D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

	if (mTransparency)
		mTextColor->SetOpacity(mOpacity);

	else
		mTextColor->SetOpacity(1.f);

	D2D1_POINT_2F	Point;
	Point.x = mRenderPos.x;
	Point.y = mRenderPos.y;

	mTarget->DrawTextLayout(Point, mLayout,
		mTextColor, D2D1_DRAW_TEXT_OPTIONS_CLIP);


	mTarget->EndDraw();
}

bool CTextBlock::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	return false;
}

void CTextBlock::MouseHovered()
{
}

void CTextBlock::MouseUnHovered()
{
}

CTextBlock* CTextBlock::Clone()	const
{
	return new CTextBlock(*this);
}
