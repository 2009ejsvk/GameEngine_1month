#include "BuildingMarkerOrb.h"
#include "Component/Animation2DComponent.h"
#include "Component/MeshComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Render/RenderManager.h"
#include <cstdarg>
#include <cstdio>
#include <utility>

namespace
{
#ifdef _DEBUG
    void DebugOrbLog(const char* Format, ...)
    {
        char Text[512] = {};

        va_list Args;
        va_start(Args, Format);
        vsprintf_s(Text, Format, Args);
        va_end(Args);

        OutputDebugStringA(Text);
    }
#endif
}

CBuildingMarkerOrb::CBuildingMarkerOrb()
{
    SetClassType<CBuildingMarkerOrb>();
}

CBuildingMarkerOrb::CBuildingMarkerOrb(
    const CBuildingMarkerOrb& ref) :
    CGameObject(ref)
{
}

CBuildingMarkerOrb::CBuildingMarkerOrb(
    CBuildingMarkerOrb&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CBuildingMarkerOrb::~CBuildingMarkerOrb()
{
}

void CBuildingMarkerOrb::Destroy()
{
    ReleaseTeamsterReservations();
    CGameObject::Destroy();
}

bool CBuildingMarkerOrb::Init()
{
    CGameObject::Init();

    mCitizenProfileState.InitializeDefaults();

    mMeshComponent = CreateComponent<CMeshComponent>("MarkerOrbMesh");
    mAnimation2DComponent = CreateComponent<CAnimation2DComponent>(
        "NpcAnimation2D");
    mMovement = CreateComponent<CObjectMovementComponent>(
        "MarkerOrbMovement");

    auto Mesh = mMeshComponent.lock();

    if (Mesh)
    {
        auto RenderMgr = CRenderManager::GetInst();
        int MarkerOrbLayer = RenderMgr->GetLayerOrder("BuildingVisual");

        if (MarkerOrbLayer < 0)
        {
            for (int Order = 3; Order <= 100; ++Order)
            {
                RenderMgr->CreateLayer("BuildingVisual", Order, ERenderListSort::Y);
                MarkerOrbLayer = RenderMgr->GetLayerOrder("BuildingVisual");

                if (MarkerOrbLayer >= 0)
                    break;
            }
        }

        Mesh->SetShader("DefaultTexture2D");
        Mesh->SetMesh("RectTex");
        Mesh->SetBlendState(0, "AlphaBlend");
        Mesh->SetRelativeScale(mOrbDiameter, mOrbDiameter);
        Mesh->SetMaterialBaseColor(0, 1.f, 1.f, 1.f, 1.f);
        Mesh->SetEnable(true);
        Mesh->SetMaterialOpacity(0, 1.f);
        Mesh->SetRenderSortYBias(-mOrbDiameter * 0.5f);
        Mesh->SetRenderSortPriority(1);

        if (MarkerOrbLayer >= 0)
            Mesh->SetRenderLayer("BuildingVisual");
    }

    auto Anim = mAnimation2DComponent.lock();

    if (Anim)
    {
        mAnimationState.ChooseRandomVariant();
        Anim->SetUpdateComponent(mMeshComponent);

        for (int i = 0; i < CitizenOrbAnimation::GDirectionCount; ++i)
        {
            Anim->AddAnimation(
                GetIdleAnimationNameByDir(i), 1.f, 1.f, true, false);
            Anim->AddAnimation(
                GetWalkAnimationNameByDir(i), 0.6f, 1.f, true, false);
        }

        mAnimationState.CurrentDirection = 0;
        Anim->ChangeAnimation(GetIdleAnimationNameByDir(0));
    }

    auto Movement = mMovement.lock();

    if (Movement)
    {
        Movement->SetUpdateComponent(mMeshComponent);
        Movement->SetSpeed(mMoveSpeed);
        Movement->SetAcceptDistance(4.f);
    }

    SetWorldPos(0.f, 0.f, 10.f);
    mLastProgressPos = GetWorldPos();

    // 개별 orb마다 retry 위상을 조금씩 다르게 해 요청 버스트를 완화한다.
    mPathRetryInterval = 0.9f + ((float)(rand() % 61) / 100.f);

#ifdef _DEBUG
    DebugOrbLog("[Orb] Init name=%s speed=%.1f\n",
        GetName().c_str(), mMoveSpeed);
#endif

    return true;
}
