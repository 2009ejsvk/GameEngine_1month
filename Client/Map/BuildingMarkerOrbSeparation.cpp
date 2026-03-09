#include "BuildingMarkerOrb.h"
#include "World/World.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    struct FOrbSeparationEntry
    {
        CBuildingMarkerOrb* Orb = nullptr;
        FVector3 Pos = FVector3::Zero;
        float Radius = 0.f;
        size_t NameHash = 0;
        int CellX = 0;
        int CellY = 0;
    };

    struct FOrbSpatialHashCache
    {
        CWorld* World = nullptr;
        float CellSize = 32.f;
        int ActiveCount = 0;
        std::vector<FOrbSeparationEntry> Entries;
        std::unordered_map<long long, std::vector<int>> Cells;
        std::unordered_map<const CBuildingMarkerOrb*, int> IndexByOrb;
        std::unordered_set<const CBuildingMarkerOrb*> SeenOrbs;
    };

    long long MakeCellKey(int CellX, int CellY)
    {
        return (static_cast<long long>(CellX) << 32) ^
            static_cast<unsigned int>(CellY);
    }

    int ToCellCoord(float Value, float CellSize)
    {
        return static_cast<int>(floorf(Value / CellSize));
    }

    void RebuildOrbSpatialHash(
        CWorld* World,
        const std::vector<std::weak_ptr<CBuildingMarkerOrb>>& OrbList,
        FOrbSpatialHashCache& Cache)
    {
        Cache.World = World;
        Cache.Entries.clear();
        Cache.Cells.clear();
        Cache.IndexByOrb.clear();
        Cache.SeenOrbs.clear();
        Cache.ActiveCount = 0;

        float MaxDiameter = 1.f;

        for (size_t i = 0; i < OrbList.size(); ++i)
        {
            auto Orb = OrbList[i].lock();

            if (!Orb || !Orb->GetAlive() || !Orb->GetEnable())
                continue;

            FOrbSeparationEntry Entry;
            Entry.Orb = Orb.get();
            Entry.Pos = Orb->GetWorldPos();
            Entry.Pos.z = 0.f;
            const float SafeDiameter = Orb->GetOrbDiameter() > 0.f ?
                Orb->GetOrbDiameter() : 1.f;
            Entry.Radius = SafeDiameter * 0.5f;
            Entry.NameHash = std::hash<std::string>{}(Orb->GetName());

            const int Index = static_cast<int>(Cache.Entries.size());
            Cache.Entries.push_back(Entry);
            Cache.IndexByOrb.emplace(Entry.Orb, Index);

            MaxDiameter = (std::max)(MaxDiameter, SafeDiameter);
        }

        Cache.ActiveCount = static_cast<int>(Cache.Entries.size());

        if (Cache.ActiveCount <= 0)
            return;

        Cache.CellSize = (std::max)(1.f, MaxDiameter);

        for (int i = 0; i < Cache.ActiveCount; ++i)
        {
            auto& Entry = Cache.Entries[i];
            Entry.CellX = ToCellCoord(Entry.Pos.x, Cache.CellSize);
            Entry.CellY = ToCellCoord(Entry.Pos.y, Cache.CellSize);
            Cache.Cells[MakeCellKey(Entry.CellX, Entry.CellY)].
                push_back(i);
        }
    }
}

void CBuildingMarkerOrb::ApplySoftSeparation(float DeltaTime)
{
    auto World = mWorld.lock();

    if (!World || DeltaTime <= 0.f)
        return;

    static FOrbSpatialHashCache SpatialCache;

    if (SpatialCache.World != World.get() ||
        SpatialCache.SeenOrbs.empty())
    {
        std::vector<std::weak_ptr<CBuildingMarkerOrb>> OrbList;

        if (!World->FindObjectListByType<CBuildingMarkerOrb>(OrbList))
            return;

        RebuildOrbSpatialHash(World.get(), OrbList, SpatialCache);
    }

    auto FinalizeVisit = [&]()
    {
        SpatialCache.SeenOrbs.insert(this);

        if (SpatialCache.ActiveCount <= 0 ||
            static_cast<int>(SpatialCache.SeenOrbs.size()) >=
            SpatialCache.ActiveCount)
        {
            SpatialCache.SeenOrbs.clear();
        }
    };

    if (SpatialCache.ActiveCount <= 1)
    {
        FinalizeVisit();
        return;
    }

    auto SelfIter = SpatialCache.IndexByOrb.find(this);

    if (SelfIter == SpatialCache.IndexByOrb.end())
    {
        FinalizeVisit();
        return;
    }

    const int SelfIndex = SelfIter->second;
    auto& SelfEntry = SpatialCache.Entries[SelfIndex];
    const float SelfRadius = SelfEntry.Radius;

    // 30% 겹침 허용 -> 최소 분리 거리 비율은 70%
    const float MinDistanceScale = Clamp<float>(
        1.f - mAllowedOverlapRatio, 0.1f, 1.f);
    const float Epsilon = 0.0001f;

    const FVector3 SelfPos = SelfEntry.Pos;
    FVector3 AccumulatedPush = FVector3::Zero;
    int OverlapCount = 0;

    for (int CellY = SelfEntry.CellY - 1;
        CellY <= SelfEntry.CellY + 1; ++CellY)
    {
        for (int CellX = SelfEntry.CellX - 1;
            CellX <= SelfEntry.CellX + 1; ++CellX)
        {
            auto CellIter = SpatialCache.Cells.find(
                MakeCellKey(CellX, CellY));

            if (CellIter == SpatialCache.Cells.end())
                continue;

            const auto& CellEntries = CellIter->second;

            for (size_t i = 0; i < CellEntries.size(); ++i)
            {
                const int OtherIndex = CellEntries[i];

                if (OtherIndex == SelfIndex)
                    continue;

                const auto& OtherEntry =
                    SpatialCache.Entries[OtherIndex];

                FVector3 Delta = SelfPos - OtherEntry.Pos;
                Delta.z = 0.f;

                const float Dist = Delta.Length();
                const float OtherRadius = OtherEntry.Radius;
                const float MinDist = (SelfRadius + OtherRadius) *
                    MinDistanceScale;

                if (Dist >= MinDist)
                    continue;

                FVector3 PushDir = FVector3::Zero;

                if (Dist > Epsilon)
                {
                    PushDir = Delta / Dist;
                }

                else
                {
                    const float Angle =
                        (float)((SelfEntry.NameHash ^
                            (OtherEntry.NameHash << 1)) % 6283) *
                        0.001f;
                    PushDir.x = cosf(Angle);
                    PushDir.y = sinf(Angle);
                    PushDir.z = 0.f;
                }

                const float Penetration = MinDist - Dist;
                AccumulatedPush += PushDir * Penetration;
                ++OverlapCount;
            }
        }
    }

    if (OverlapCount <= 0 || AccumulatedPush.IsZero())
    {
        FinalizeVisit();
        return;
    }

    FVector3 PushDelta = AccumulatedPush *
        (mSeparationStrength * DeltaTime / (float)OverlapCount);
    PushDelta.z = 0.f;

    const float MaxPushDist = mSeparationMaxSpeed * DeltaTime;
    const float PushLen = PushDelta.Length();

    if (PushLen > MaxPushDist && PushLen > Epsilon)
    {
        PushDelta *= (MaxPushDist / PushLen);
    }

    AddWorldPos(PushDelta);

    // 같은 프레임의 후속 오브 계산이 최신 위치를 보게 한다.
    SelfEntry.Pos += PushDelta;
    SelfEntry.Pos.z = 0.f;
    const int NewCellX = ToCellCoord(SelfEntry.Pos.x, SpatialCache.CellSize);
    const int NewCellY = ToCellCoord(SelfEntry.Pos.y, SpatialCache.CellSize);

    if (NewCellX != SelfEntry.CellX || NewCellY != SelfEntry.CellY)
    {
        auto OldCellIter = SpatialCache.Cells.find(
            MakeCellKey(SelfEntry.CellX, SelfEntry.CellY));

        if (OldCellIter != SpatialCache.Cells.end())
        {
            auto& OldCellEntries = OldCellIter->second;
            OldCellEntries.erase(
                std::remove(OldCellEntries.begin(),
                    OldCellEntries.end(), SelfIndex),
                OldCellEntries.end());
        }

        SelfEntry.CellX = NewCellX;
        SelfEntry.CellY = NewCellY;
        SpatialCache.Cells[MakeCellKey(NewCellX, NewCellY)].
            push_back(SelfIndex);
    }

    FinalizeVisit();
}
