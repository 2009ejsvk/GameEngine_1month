#include "BuildingMarkerOrb.h"
#include "Component/Animation2DComponent.h"

void CBuildingMarkerOrb::UpdateSpriteAnimationFromVelocity(
    const FVector3& Velocity)
{
    auto Anim = mAnimation2DComponent.lock();

    if (!Anim)
        return;

    const char* NextAnimationName = nullptr;

    if (mAnimationState.ResolveAnimationForVelocity(
        Velocity,
        NextAnimationName) &&
        NextAnimationName)
    {
        Anim->ChangeAnimation(NextAnimationName);
    }
}

int CBuildingMarkerOrb::ResolveDirectionIndexFromVelocity(
    const FVector3& Velocity) const
{
    return mAnimationState.ResolveDirectionIndexFromVelocity(Velocity);
}

const char* CBuildingMarkerOrb::GetIdleAnimationNameByDir(
    int Direction) const
{
    return mAnimationState.GetIdleAnimationNameByDir(Direction);
}

const char* CBuildingMarkerOrb::GetWalkAnimationNameByDir(
    int Direction) const
{
    return mAnimationState.GetWalkAnimationNameByDir(Direction);
}
