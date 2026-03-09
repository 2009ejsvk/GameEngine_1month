#include "Button.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../World/World.h"
#include "../World/Input.h"
#include "../Asset/Shader/Shader.h"
#include "../Asset/Mesh/Mesh.h"
#include "../Asset/Shader/CBufferUIDefault.h"
#include "../Asset/Shader/CBufferTransform.h"
#include "../Asset/Sound/Sound.h"
#include "../World/WorldAssetManager.h"

CButton::CButton()
{
}

CButton::CButton(const CButton& ref)	:
	CWidget(ref)
{
	if (ref.mChild)
	{
		mChild.reset(ref.mChild->Clone());
	}

	for (int i = 0; i < EButtonState::End; ++i)
	{
		mBrush[i] = ref.mBrush[i];
	}

	mState = ref.mState;

	for (int i = 0; i < EButtonEventState::End; ++i)
	{
		mSound[i] = ref.mSound[i];
		mEventCallback[i] = ref.mEventCallback[i];
	}
}

CButton::~CButton()
{
}

void CButton::SetSize(const FVector3& Size)
{
	CWidget::SetSize(Size);

	if (mChild)
	{
		mChild->SetSize(Size);
	}
}

void CButton::SetSize(float x, float y)
{
	CWidget::SetSize(x, y);

	if (mChild)
	{
		mChild->SetSize(x, y);
	}
}

bool CButton::SetTexture(EButtonState::Type State, 
	const std::weak_ptr<class CTexture>& Texture)
{
	mBrush[State].Texture = Texture;

	return true;
}

bool CButton::SetTexture(EButtonState::Type State, 
	const std::string& Name)
{
	auto	World = mWorld.lock();

	if (!World)
		return false;

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return false;

	std::weak_ptr<CTexture>	Texture =
		AssetMgr->FindTexture(Name);

	mBrush[State].Texture = Texture;

	return true;
}

bool CButton::SetTexture(EButtonState::Type State, 
	const std::string& Name, const TCHAR* FileName, 
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

	mBrush[State].Texture = Texture;

	return true;
}

void CButton::SetTint(EButtonState::Type State, const FVector4& Tint)
{
	mBrush[State].Tint = Tint;
}

void CButton::SetTint(EButtonState::Type State, float r, float g,
	float b, float a)
{
	mBrush[State].Tint = FVector4(r, g, b, a);
}

void CButton::SetBrushAnimation(EButtonState::Type State,
	bool Animation)
{
	mBrush[State].AnimationEnable = Animation;
}

void CButton::AddBrushFrame(EButtonState::Type State,
	const FVector2& Start, const FVector2& Size)
{
	FTextureFrame	Frame;
	Frame.Start = Start;
	Frame.Size = Size;

	mBrush[State].AnimationFrames.push_back(Frame);

	mBrush[State].FrameTime = mBrush[State].PlayTime /
		mBrush[State].AnimationFrames.size();
}

void CButton::AddBrushFrame(EButtonState::Type State,
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

void CButton::SetCurrentFrame(EButtonState::Type State, int Frame)
{
	mBrush[State].Frame = Frame;
}

void CButton::SetAnimationPlayTime(EButtonState::Type State, 
	float PlayTime)
{
	mBrush[State].PlayTime = PlayTime;

	mBrush[State].FrameTime = mBrush[State].PlayTime /
		mBrush[State].AnimationFrames.size();
}

void CButton::SetAnimationPlayRate(EButtonState::Type State,
	float PlayRate)
{
	mBrush[State].PlayRate = PlayRate;
}

void CButton::SetSound(EButtonEventState::Type State,
	const std::string& Name)
{
	auto	World = mWorld.lock();

	if (!World)
		return;

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return;

	mSound[State] = AssetMgr->FindSound(Name);
}

void CButton::SetSound(EButtonEventState::Type State,
	const std::string& Name, const char* FileName,
	const std::string& PathName)
{
	auto	World = mWorld.lock();

	if (!World)
		return;

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	if (!AssetMgr)
		return;

	if (!AssetMgr->LoadSound(Name, "UI", false, FileName, PathName))
		return;

	mSound[State] = AssetMgr->FindSound(Name);
}

void CButton::SetParentAll()
{
	if (mChild)
	{
		mChild->SetParent(mSelf);

		mChild->SetParentAll();
	}
}

void CButton::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CButton::Init()
{
	CWidget::Init();

	return true;
}

void CButton::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	if (mState != EButtonState::Disable)
	{
		if (mMouseOn)
		{
			// 마우스가 올라온 상태에서 마우스 왼쪽 버튼을 누르게 되면
			// Click으로 상태를 바꾼다.
			auto	World = mWorld.lock();

			if (!World)
			{
				mBrush[mState].PlayAnimation(DeltaTime);
				return;
			}

			auto	Input = World->GetInput().lock();

			if (!Input)
			{
				mBrush[mState].PlayAnimation(DeltaTime);
				return;
			}

			if (Input->GetMouseState(EMouseType::LButton,
				EInputType::Press))
			{
				mState = EButtonState::Click;

				if (!mSound[EButtonEventState::Click].expired())
				{
					auto	Sound = mSound[EButtonEventState::Click].lock();

					Sound->Play();
				}
			}

			else if (mState == EButtonState::Click &&
				Input->GetMouseState(EMouseType::LButton,
					EInputType::Release))
			{
				if (mEventCallback[EButtonEventState::Click])
					mEventCallback[EButtonEventState::Click]();

				mState = EButtonState::Hovered;
			}

			/*else if (Input->GetMouseState(EMouseType::LButton,
				EInputType::Hold))
			{
				mState = EButtonState::Click;
			}*/
		}
	}

	mBrush[mState].PlayAnimation(DeltaTime);
}

void CButton::Render()
{
	CWidget::Render();

	RenderBrush(mBrush[mState], mRenderPos, mSize);
	
	if (mChild)
		mChild->Render();
}

bool CButton::CollisionMouse(std::weak_ptr<CWidget>& Result,
	const FVector2& MousePos)
{
	if (!CWidget::CollisionMouse(Result, MousePos))
		return false;

	if (mChild)
	{
		std::weak_ptr<CWidget> ChildResult;
		mChild->CollisionMouse(ChildResult, MousePos);
	}

	Result = mSelf;
	return true;
}

void CButton::MouseHovered()
{
	if (mState == EButtonState::Normal)
	{
		if (mEventCallback[EButtonEventState::Hovered])
			mEventCallback[EButtonEventState::Hovered]();

		if (!mSound[EButtonEventState::Hovered].expired())
		{
			auto	Sound = mSound[EButtonEventState::Hovered].lock();

			Sound->Play();
		}

		mState = EButtonState::Hovered;
	}
}

void CButton::MouseUnHovered()
{
	if (mState != EButtonState::Disable)
		mState = EButtonState::Normal;

	mMouseOn = false;
}

CButton* CButton::Clone()	const
{
	return new CButton(*this);
}
