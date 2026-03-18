#include "PopulationSubsystem.h"
#include "MainWorld.h"
#include "MainWorldConfig.h"
#include "../Citizen/CitizenSystem.h"
#include <string>

void CPopulationSubsystem::Reset()
{
    SpawnedNpcCount = 0;
    NpcSpawnAccum = 0.f;
    CitizenReassignAccum = 0.f;
}

void CPopulationSubsystem::Tick(float DeltaTime)
{
    CitizenReassignAccum += DeltaTime;

    while (CitizenReassignAccum >= MainWorldConfig::GCitizenReassignInterval)
    {
        CitizenReassignAccum -= MainWorldConfig::GCitizenReassignInterval;
        ReassignCitizenNeeds();
    }

    if (SpawnedNpcCount >= MainWorldConfig::GMaxNpcCount)
        return;

    NpcSpawnAccum += DeltaTime;

    while (NpcSpawnAccum >= MainWorldConfig::GNpcSpawnInterval &&
        SpawnedNpcCount < MainWorldConfig::GMaxNpcCount)
    {
        NpcSpawnAccum -= MainWorldConfig::GNpcSpawnInterval;
        SpawnCitizenOrb();
    }
}

void CPopulationSubsystem::SpawnCitizenOrb()
{
    if (!mOwner)
        return;

    CitizenSystem::SpawnCitizenOrb(mOwner, SpawnedNpcCount);
}

void CPopulationSubsystem::ReassignCitizenNeeds()
{
    if (!mOwner)
        return;

    CitizenSystem::ReassignCitizenNeeds(mOwner);
}

void CPopulationSubsystem::LoadCitizenAnimation2D()
{
    if (!mOwner)
        return;

    const auto AssetManager = mOwner->GetWorldAssetManager().lock();

    if (!AssetManager)
        return;

    auto CreateCitizenVariantAnimations = [&](const char* Prefix,
        float WalkTopY,
        float IdleTopY,
        float WalkReverseTopY)
    {
        for (int Direction = 0;
            Direction < MainWorldConfig::GCitizenDirectionCount;
            ++Direction)
        {
            const std::string IdleAnimationName =
                std::string(Prefix) + "Idle_Dir" +
                std::to_string(Direction);
            const std::string WalkAnimationName =
                std::string(Prefix) + "Walk_Dir" +
                std::to_string(Direction);
            const float RightBottomX =
                MainWorldConfig::GCitizenRightBottomStartX +
                static_cast<float>(Direction) *
                MainWorldConfig::GCitizenDirectionStepX;
            const float StartX =
                RightBottomX - MainWorldConfig::GCitizenFrameWidth;

            AssetManager->CreateAnimation(IdleAnimationName);
            AssetManager->SetAnimation2DTextureType(
                IdleAnimationName,
                EAnimation2DTextureType::SpriteSheet);
            AssetManager->SetTexture(
                IdleAnimationName,
                MainWorldConfig::GCitizenSheetTextureName,
                MainWorldConfig::GCitizenSheetTextureFile);
            AssetManager->AddFrame(
                IdleAnimationName,
                StartX,
                IdleTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);

            AssetManager->CreateAnimation(WalkAnimationName);
            AssetManager->SetAnimation2DTextureType(
                WalkAnimationName,
                EAnimation2DTextureType::SpriteSheet);
            AssetManager->SetTexture(
                WalkAnimationName,
                MainWorldConfig::GCitizenSheetTextureName,
                MainWorldConfig::GCitizenSheetTextureFile);
            AssetManager->AddFrame(
                WalkAnimationName,
                StartX,
                WalkTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);
            AssetManager->AddFrame(
                WalkAnimationName,
                StartX,
                WalkReverseTopY,
                MainWorldConfig::GCitizenFrameWidth,
                MainWorldConfig::GCitizenFrameHeight);
        }
    };

    CreateCitizenVariantAnimations(
        MainWorldConfig::GCitizenBlueAnimationPrefix,
        MainWorldConfig::GCitizenBlueWalkTopY,
        MainWorldConfig::GCitizenBlueIdleTopY,
        MainWorldConfig::GCitizenBlueWalkReverseTopY);
    CreateCitizenVariantAnimations(
        MainWorldConfig::GCitizenRedAnimationPrefix,
        MainWorldConfig::GCitizenRedWalkTopY,
        MainWorldConfig::GCitizenRedIdleTopY,
        MainWorldConfig::GCitizenRedWalkReverseTopY);
}
