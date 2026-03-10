#include "CitizenPolitics.h"
#include "../GameConstants.h"
#include <algorithm>
#include <cstdlib>

namespace
{
    EPoliticalStance GetOppositePoliticalStance(EPoliticalStance Stance)
    {
        switch (Stance)
        {
        case EPoliticalStance::Left:
            return EPoliticalStance::Right;
        case EPoliticalStance::Right:
            return EPoliticalStance::Left;
        default:
            return EPoliticalStance::Neutral;
        }
    }

    EPoliticalSupportLevel ClampPoliticalSupportLevel(int Value)
    {
        if (Value <= static_cast<int>(EPoliticalSupportLevel::Weak))
            return EPoliticalSupportLevel::Weak;

        if (Value >= static_cast<int>(EPoliticalSupportLevel::Strong))
            return EPoliticalSupportLevel::Strong;

        return static_cast<EPoliticalSupportLevel>(Value);
    }
}

namespace CitizenPolitics
{
    void Init(FNpcPoliticalProfile& Profile, float& TickAccum)
    {
        auto MakeRandomChoice = []() -> FNpcPoliticalChoice
        {
            FNpcPoliticalChoice Choice;
            const int StanceRoll = rand() % 100;

            if (StanceRoll < 34)
                Choice.Stance = EPoliticalStance::Left;
            else if (StanceRoll < 67)
                Choice.Stance = EPoliticalStance::Neutral;
            else
                Choice.Stance = EPoliticalStance::Right;

            const int SupportRoll = rand() % 100;

            if (SupportRoll < 28)
                Choice.Support = EPoliticalSupportLevel::Weak;
            else if (SupportRoll < 76)
                Choice.Support = EPoliticalSupportLevel::Normal;
            else
                Choice.Support = EPoliticalSupportLevel::Strong;

            return Choice;
        };

        Profile.Economy = MakeRandomChoice();
        Profile.ReligionMilitarism = MakeRandomChoice();
        Profile.EnvironmentIndustry = MakeRandomChoice();
        Profile.IntellectualConservative = MakeRandomChoice();

        // 초기 갱신 타이밍을 분산시켜 한 프레임에 편향이 몰리지 않게 한다.
        TickAccum =
            static_cast<float>(rand() % 1000) / 1000.f *
                GameConstants::Politics::CitizenPoliticalShiftIntervalSeconds;
    }

    void Update(
        FNpcPoliticalProfile& Profile,
        float& TickAccum,
        float DeltaTime,
        float OverallSatisfaction)
    {
        if (DeltaTime <= 0.f)
            return;

        TickAccum += DeltaTime;

        while (TickAccum >=
            GameConstants::Politics::CitizenPoliticalShiftIntervalSeconds)
        {
            TickAccum -=
                GameConstants::Politics::CitizenPoliticalShiftIntervalSeconds;

            const int AxisIndex = rand() %
                static_cast<int>(EPoliticalAxis::Count);
            const EPoliticalAxis Axis = static_cast<EPoliticalAxis>(AxisIndex);
            UpdateChoice(Profile.Get(Axis), OverallSatisfaction);
        }
    }

    void UpdateChoice(FNpcPoliticalChoice& Choice, float OverallSatisfaction)
    {
        const int Roll = rand() % 100;

        int DriftBonus = 0;

        if (OverallSatisfaction < 40.f)
            DriftBonus = 10;
        else if (OverallSatisfaction > 75.f)
            DriftBonus = -6;

        if (Choice.Stance == EPoliticalStance::Neutral)
        {
            int MoveToSideChance = 22;

            if (OverallSatisfaction < 40.f)
                MoveToSideChance += 8;
            else if (OverallSatisfaction > 75.f)
                MoveToSideChance -= 6;

            MoveToSideChance = (std::max)(8, (std::min)(40, MoveToSideChance));

            if (Roll < MoveToSideChance)
            {
                Choice.Stance = EPoliticalStance::Left;
                Choice.Support = EPoliticalSupportLevel::Weak;
                return;
            }

            if (Roll < MoveToSideChance * 2)
            {
                Choice.Stance = EPoliticalStance::Right;
                Choice.Support = EPoliticalSupportLevel::Weak;
                return;
            }

            int SupportValue = static_cast<int>(Choice.Support);

            if (Roll >= 90)
                ++SupportValue;
            else if (Roll < 10)
                --SupportValue;

            Choice.Support = ClampPoliticalSupportLevel(SupportValue);
            return;
        }

        int ToNeutralChance = 18 + DriftBonus;
        int ToOppositeChance = 10 + DriftBonus / 2;
        ToNeutralChance = (std::max)(5, (std::min)(40, ToNeutralChance));
        ToOppositeChance = (std::max)(3, (std::min)(25, ToOppositeChance));

        if (Roll < ToNeutralChance)
        {
            Choice.Stance = EPoliticalStance::Neutral;
            Choice.Support = EPoliticalSupportLevel::Weak;
            return;
        }

        if (Roll < ToNeutralChance + ToOppositeChance)
        {
            Choice.Stance = GetOppositePoliticalStance(Choice.Stance);
            Choice.Support = EPoliticalSupportLevel::Weak;
            return;
        }

        int SupportValue = static_cast<int>(Choice.Support);
        int StrengthenThreshold = 70;

        if (OverallSatisfaction > 65.f)
            StrengthenThreshold -= 15;
        else if (OverallSatisfaction < 35.f)
            StrengthenThreshold += 8;

        StrengthenThreshold =
            (std::max)(35, (std::min)(90, StrengthenThreshold));

        if (Roll >= StrengthenThreshold)
            ++SupportValue;
        else
            --SupportValue;

        Choice.Support = ClampPoliticalSupportLevel(SupportValue);
    }
}
