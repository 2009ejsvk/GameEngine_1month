#include "LoadingWidget.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "Device.h"
#include "Engine.h"
#include "World/WorldManager.h"
#include "UIStrings.h"
#include "../World/MainWorldAccess.h"

namespace
{
    std::wstring GetUiStringOrDefault(
        const wchar_t* Key,
        const wchar_t* DefaultValue)
    {
        const std::wstring& Value = UIStrings::Get(Key);

        if (Value == Key)
            return std::wstring(DefaultValue);

        return Value;
    }
}

CLoadingWidget::CLoadingWidget()
{
}

CLoadingWidget::~CLoadingWidget()
{
}

bool CLoadingWidget::Init()
{
	CWidgetContainer::Init();

    const std::wstring LoadingBackgroundTexture =
        GetUiStringOrDefault(L"loading.background.image", L"LoadingBack.png");
    const std::wstring LoadingText =
        GetUiStringOrDefault(L"loading.text", L"LOADING");

	std::shared_ptr<CImage> Back = CreateWidget<CImage>("Back").lock();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	Back->SetSize((float)RS.Width, (float)RS.Height);
	Back->SetTexture("LoadingBack", LoadingBackgroundTexture.c_str());

	std::shared_ptr<CTextBlock> Text =
		CreateWidget<CTextBlock>("Text", 1).lock();

	Text->SetText(LoadingText.c_str());
	Text->SetTextColor(255, 255, 255, 255);
	Text->SetPos(900.f, 550.f);
	Text->SetSize(500.f, 150.f);
	Text->SetFontSize(50.f);
	Text->EnableShadow(true);
	Text->SetShadowOffset(3.f, 3.f);
	Text->SetShadowTextColor(128, 128, 128, 255);

	return true;
}

void CLoadingWidget::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);
}

void CLoadingWidget::Render()
{
	CWidgetContainer::Render();
}


