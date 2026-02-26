#include "EditorWidget.h"
#include "UI/TextBlock.h"

CEditorWidget::CEditorWidget()
{
}

CEditorWidget::~CEditorWidget()
{
}

void CEditorWidget::SetEditorMode(EEditorMode Mode)
{
	switch (Mode)
	{
	case EEditorMode::TileType:
		mEditorMode->SetText(TEXT("EidtorMode : Type"));
		break;
	case EEditorMode::TileFrame:
		mEditorMode->SetText(TEXT("EidtorMode : Frame"));
		break;
	}
}

void CEditorWidget::SetTileFrame(int Frame)
{
	TCHAR	Text[256] = {};

	wsprintf(Text, TEXT("TileFrame : %d"), Frame);

	mTileFrame->SetText(Text);
}

void CEditorWidget::SetTileType(ETileType TileType)
{
	switch (TileType)
	{
	case ETileType::Normal:
		mTileType->SetText(TEXT("TileType : 이동"));
		break;
	case ETileType::UnableToMove:
		mTileType->SetText(TEXT("TileType : 이동불가"));
		break;
	}
}

bool CEditorWidget::Init()
{
	CWidgetContainer::Init();

	mEditorMode =
		CreateWidget<CTextBlock>("EditorMode").lock();

	mEditorMode->SetText(TEXT("EidtorMode : Frame"));
	mEditorMode->SetTextColor(255, 0, 0, 255);
	mEditorMode->SetPos(20.f, 20.f);
	mEditorMode->SetSize(300.f, 100.f);
	mEditorMode->SetFontSize(30.f);
	mEditorMode->EnableShadow(true);
	mEditorMode->SetShadowOffset(3.f, 3.f);
	mEditorMode->SetShadowTextColor(128, 128, 128, 255);

	mTileType =
		CreateWidget<CTextBlock>("TileType").lock();

	mTileType->SetText(TEXT("TileType : 이동"));
	mTileType->SetTextColor(255, 0, 0, 255);
	mTileType->SetPos(420.f, 20.f);
	mTileType->SetSize(300.f, 100.f);
	mTileType->SetFontSize(30.f);
	mTileType->EnableShadow(true);
	mTileType->SetShadowOffset(3.f, 3.f);
	mTileType->SetShadowTextColor(128, 128, 128, 255);

	mTileFrame =
		CreateWidget<CTextBlock>("TileFrame").lock();

	mTileFrame->SetText(TEXT("TileFrame : 0"));
	mTileFrame->SetTextColor(255, 0, 0, 255);
	mTileFrame->SetPos(720.f, 20.f);
	mTileFrame->SetSize(300.f, 100.f);
	mTileFrame->SetFontSize(30.f);
	mTileFrame->EnableShadow(true);
	mTileFrame->SetShadowOffset(3.f, 3.f);
	mTileFrame->SetShadowTextColor(128, 128, 128, 255);

	return true;
}

void CEditorWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CEditorWidget::Render()
{
	CWidgetContainer::Render();
}
