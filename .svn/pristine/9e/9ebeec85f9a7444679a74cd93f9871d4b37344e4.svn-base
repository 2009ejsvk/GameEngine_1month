#include "BulletEffect.h"
#include "Component/MeshComponent.h"
#include "Component/Animation2DComponent.h"

CBulletEffect::CBulletEffect()
{
	SetClassType<CBulletEffect>();
}

CBulletEffect::CBulletEffect(const CBulletEffect& ref) :
	CGameObject(ref)
{
}

CBulletEffect::CBulletEffect(CBulletEffect&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CBulletEffect::~CBulletEffect()
{
}

bool CBulletEffect::Init()
{
	CGameObject::Init();

	mMeshComponent = CreateComponent<CMeshComponent>("Mesh");
	mAnimation2DComponent = CreateComponent<CAnimation2DComponent>("Animation2D");

	// 애니메이션 지정
	auto	Anim = mAnimation2DComponent.lock();

	if (Anim)
	{
		Anim->SetUpdateComponent(mMeshComponent);

		Anim->AddAnimation("Explosion");

		Anim->SetFinishNotify<CBulletEffect>("Explosion",
			this, &CBulletEffect::FinishAnimation);
	}

	auto	Mesh = mMeshComponent.lock();

	if (Mesh)
	{
		Mesh->SetShader("DefaultTexture2D");
		Mesh->SetMesh("CenterRectTex");
		Mesh->SetWorldScale(100.f, 100.f);
		Mesh->SetBlendState(0, "AlphaBlend");
	}

	return true;
}

void CBulletEffect::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);
}

CBulletEffect* CBulletEffect::Clone()
{
	return new CBulletEffect(*this);
}

void CBulletEffect::FinishAnimation()
{
	Destroy();
}
