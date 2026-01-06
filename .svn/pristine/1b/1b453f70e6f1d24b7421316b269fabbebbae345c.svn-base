#include "Animation2DComponent.h"
#include "../Asset/Animation2D/Animation2D.h"
#include "../Asset/Animation2D/Animation2DManager.h"
#include "../Asset/AssetManager.h"
#include "MeshComponent.h"
#include "../Asset/Shader/CBufferAnimation2D.h"
#include "../Asset/Texture/Texture.h"
#include "../World/World.h"
#include "../World/WorldAssetManager.h"

CAnimation2DComponent::CAnimation2DComponent()
{
	SetClassType<CAnimation2DComponent>();
}

CAnimation2DComponent::CAnimation2DComponent(
	const CAnimation2DComponent& ref)	:
	CObjectComponent(ref)
{
	mUpdateComponent = ref.mUpdateComponent;
}

CAnimation2DComponent::CAnimation2DComponent(
	CAnimation2DComponent&& ref) noexcept :
	CObjectComponent(std::move(ref))
{
}

CAnimation2DComponent::~CAnimation2DComponent()
{
}

void CAnimation2DComponent::SetUpdateComponent(
	const std::weak_ptr<class CMeshComponent>& Component)
{
	mUpdateComponent = Component;

	auto	MeshComponent = mUpdateComponent.lock();

	if (MeshComponent && mCurrentAnimation)
	{
		MeshComponent->SetAnimComponent(
			GetSelf<CAnimation2DComponent>());

		auto	Asset = mCurrentAnimation->GetAsset().lock();

		if (Asset)
		{
			mAnimCBuffer->SetAnimation2DTextureType(Asset->GetAnimationTextureType());
			if (MeshComponent->SetTexture(0, 0,
				Asset->GetTexture()))
				mUpdateEnable = true;
		}
	}
}

void CAnimation2DComponent::AddAnimation(
	const std::weak_ptr<class CAnimation2D>& Anim, float PlayTime,
	float PlayRate, bool Loop, bool Reverse)
{
	auto	_Anim = Anim.lock();

	if (!_Anim)
		return;

	auto	iter = mAnimationMap.find(_Anim->GetName());

	if (iter != mAnimationMap.end())
		return;

	std::shared_ptr<CAnimation2DSequence>	Sequence(new CAnimation2DSequence);

	Sequence->SetAnimation2D(Anim);
	Sequence->SetPlayTime(PlayTime);
	Sequence->SetPlayRate(PlayRate);
	Sequence->SetLoop(Loop);
	Sequence->SetReverse(Reverse);

	mAnimationMap.insert(std::make_pair(_Anim->GetName(), Sequence));

	if (!mCurrentAnimation)
	{
		mCurrentAnimation = Sequence;

		auto	MeshComponent = mUpdateComponent.lock();

		if (MeshComponent && mCurrentAnimation)
		{
			auto	Asset = mCurrentAnimation->GetAsset().lock();

			if (Asset)
			{
				mAnimCBuffer->SetAnimation2DTextureType(Asset->GetAnimationTextureType());
				if (MeshComponent->SetTexture(0, 0, Asset->GetTexture()))
					mUpdateEnable = true;
			}
		}
	}
}

void CAnimation2DComponent::AddAnimation(const std::string& Name, 
	float PlayTime, float PlayRate, bool Loop, bool Reverse)
{
	auto	World = mWorld.lock();

	auto	AssetMgr = World->GetWorldAssetManager().lock();

	std::weak_ptr<CAnimation2D>	Anim =
		AssetMgr->FindAnimation(Name);

	if (Anim.expired())
		return;
	
	auto	_Anim = Anim.lock();

	auto	iter = mAnimationMap.find(_Anim->GetName());

	if (iter != mAnimationMap.end())
		return;

	std::shared_ptr<CAnimation2DSequence>	Sequence(new CAnimation2DSequence);

	Sequence->SetAnimation2D(Anim);
	Sequence->SetPlayTime(PlayTime);
	Sequence->SetPlayRate(PlayRate);
	Sequence->SetLoop(Loop);
	Sequence->SetReverse(Reverse);

	mAnimationMap.insert(std::make_pair(_Anim->GetName(), Sequence));
	
	if (!mCurrentAnimation)
	{
		mCurrentAnimation = Sequence;

		auto	MeshComponent = mUpdateComponent.lock();

		if (MeshComponent && mCurrentAnimation)
		{
			auto	Asset = mCurrentAnimation->GetAsset().lock();

			if (Asset)
			{
				mAnimCBuffer->SetAnimation2DTextureType(Asset->GetAnimationTextureType());
				if (MeshComponent->SetTexture(0, 0,
					Asset->GetTexture()))
					mUpdateEnable = true;
			}
		}
	}
}

void CAnimation2DComponent::SetPlayTime(const std::string& Name,
	float PlayTime)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	iter->second->SetPlayTime(PlayTime);
}

void CAnimation2DComponent::SetPlayRate(const std::string& Name,
	float PlayRate)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	iter->second->SetPlayRate(PlayRate);
}

void CAnimation2DComponent::SetLoop(const std::string& Name,
	bool Loop)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	iter->second->SetLoop(Loop);
}

void CAnimation2DComponent::SetReverse(const std::string& Name,
	bool Reverse)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	iter->second->SetReverse(Reverse);
}

void CAnimation2DComponent::SetSymmetry(
	const std::string& Name, bool Symmetry)
{
	std::string	Key = "Animation2D_" + Name;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	iter->second->SetAnimationSymmetry(Symmetry);
}

void CAnimation2DComponent::ChangeAnimation(const std::string& Name)
{
	std::string	Key = "Animation2D_" + Name;

	if (mUpdateComponent.expired())
		return;

	else if (!mCurrentAnimation)
		return;

	else if (mCurrentAnimation->GetName() == Key)
		return;

	auto	iter = mAnimationMap.find(Key);

	if (iter == mAnimationMap.end())
		return;

	// 애니메이션 정보를 초기화한다.
	mCurrentAnimation->Clear();

	// 현재 애니메이션을 교체한다.
	mCurrentAnimation = iter->second;

	mCurrentAnimation->Clear();

	// 등록된 MeshComponent가 가지고 있는 Material의 Texture를
	// 애니메이션이 가지고 있는 Texture로 교체한다.
	auto	MeshComponent = mUpdateComponent.lock();

	if (MeshComponent)
	{
		auto	Asset = mCurrentAnimation->GetAsset().lock();

		if (Asset)
		{
			mAnimCBuffer->SetAnimation2DTextureType(Asset->GetAnimationTextureType());
			MeshComponent->SetTexture(0, 0, Asset->GetTexture());
		}
	}
}

void CAnimation2DComponent::SetShader()
{
	int Frame = mCurrentAnimation->GetFrame();

	mAnimCBuffer->SetTextureSymmetry(
		mCurrentAnimation->GetSymmetry());

	auto Asset = mCurrentAnimation->GetAsset().lock();

	if (Asset)
	{
		if (Asset->GetAnimationTextureType() ==
			EAnimation2DTextureType::SpriteSheet)
		{
			const FTextureFrame& TexFrame = Asset->GetFrame(Frame);

			auto	Texture = Asset->GetTexture().lock();

			if (Texture)
			{
				const FTextureInfo* TexInfo =
					Texture->GetTexture();

				mAnimCBuffer->SetLTUV(
					TexFrame.Start.x / TexInfo->Width,
					TexFrame.Start.y / TexInfo->Height);
				mAnimCBuffer->SetRBUV(
					(TexFrame.Start.x + TexFrame.Size.x) / TexInfo->Width,
					(TexFrame.Start.y + TexFrame.Size.y) / TexInfo->Height);
			}
		}
	}

	mAnimCBuffer->UpdateBuffer();
}

EAnimation2DTextureType CAnimation2DComponent::GetTextureType() const
{
	auto Asset = mCurrentAnimation->GetAsset().lock();

	return Asset->GetAnimationTextureType();
}

int CAnimation2DComponent::GetAnimationFrame() const
{
	return mCurrentAnimation->GetFrame();
}

bool CAnimation2DComponent::Init()
{
	mAnimCBuffer.reset(new CCBufferAnimation2D);

	mAnimCBuffer->Init();

	mAnimCBuffer->SetAnimation2DEnable(true);

	return true;
}

void CAnimation2DComponent::Update(float DeltaTime)
{
	if (mCurrentAnimation)
	{
		auto	MeshComponent = mUpdateComponent.lock();

		if (!mUpdateEnable)
		{
			if (MeshComponent)
			{
				MeshComponent->SetAnimComponent(
					GetSelf<CAnimation2DComponent>());

				auto	Asset = mCurrentAnimation->GetAsset().lock();

				if (Asset)
				{
					mAnimCBuffer->SetAnimation2DTextureType(Asset->GetAnimationTextureType());
					if (MeshComponent->SetTexture(0, 0,
						Asset->GetTexture()))
					{
						mUpdateEnable = true;
					}
				}
			}
		}

		mCurrentAnimation->Update(DeltaTime);
	}
}

void CAnimation2DComponent::PostUpdate(float DeltaTime)
{
}

void CAnimation2DComponent::Destroy()
{
	CObjectComponent::Destroy();
}

CAnimation2DComponent* CAnimation2DComponent::Clone()	const
{
	return new CAnimation2DComponent(*this);
}
