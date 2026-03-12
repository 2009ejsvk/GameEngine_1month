#include "AlmanacRenderer.h"
#include "AlmanacRendererCalc.h"
#include "AlmanacRendererInternal.h"
#include "../Building/BuildingCategoryInfo.h"

namespace
{
    std::vector<std::pair<std::wstring, int>> BuildLogisticsWarningEntries(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        bool Bottleneck)
    {
        std::vector<std::pair<std::wstring, int>> Entries;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const auto& Resource =
                Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];
            const auto& SourceEntries =
                Bottleneck ?
                    Resource.TopShortageBuildings :
                    Resource.TopOverflowBuildings;

            for (size_t Index = 0; Index < SourceEntries.size(); ++Index)
            {
                if (SourceEntries[Index].second <= 0)
                    continue;

                Entries.push_back(
                    {
                        SourceEntries[Index].first +
                            L" [" +
                            GetResourceTypeDisplayName(Resource.Type) +
                            L"]",
                        SourceEntries[Index].second
                    });
            }
        }

        std::sort(
            Entries.begin(),
            Entries.end(),
            [](const std::pair<std::wstring, int>& A,
                const std::pair<std::wstring, int>& B)
            {
                if (A.second != B.second)
                    return A.second > B.second;

                return A.first < B.first;
            });

        return Entries;
    }

    std::wstring BuildLogisticsWarningSummary(
        const std::vector<std::pair<std::wstring, int>>& Entries,
        size_t MaxCount,
        const wchar_t* UnitSuffix = nullptr)
    {
        std::wstring Result;

        for (size_t Index = 0;
            Index < Entries.size() && Index < MaxCount;
            ++Index)
        {
            if (Entries[Index].second <= 0)
                continue;

            if (!Result.empty())
                Result += L" / ";

            Result += Entries[Index].first;
            Result += L" ";
            Result += std::to_wstring(Entries[Index].second);

            if (UnitSuffix && *UnitSuffix)
                Result += UnitSuffix;
        }

        return Result;
    }

    struct FPoliticsFactionRuntimeData
    {
        EPoliticalFaction Faction = EPoliticalFaction::Communists;
        EPoliticalAxis Axis = EPoliticalAxis::Economy;
        EPoliticalStance Stance = EPoliticalStance::Left;
        std::wstring Label;
        int Count = 0;
        int NeutralCount = 0;
        int Favor = 50;
        double AverageApproval = 50.0;
        double AverageLifeScore = 0.0;
        double AverageBuildingScore = 0.0;
        double AverageActionScore = 0.0;
        std::wstring GovernmentLine;
        int AlignmentScore = 0;
    };

    struct FForeignContribution
    {
        std::wstring Label;
        int Value = 0;
    };

    struct FForeignPowerRuntimeData
    {
        std::wstring Name;
        int Relation = 0;
        std::wstring Status;
        int EconomicAid = 0;
        int BuildingModifier = 0;
        FForeignContribution BuildingLine;
        int EdictModifier = 0;
        FForeignContribution EdictLine;
        int GovernmentModifier = 0;
        int MiscModifier = 0;
        FForeignContribution MiscLine;
        int TradeModifier = 0;
    };

    struct FForeignContext
    {
        double HarborStrength = 0.0;
        double TradeStrength = 0.0;
        double IndustryStrength = 0.0;
        double TourismStrength = 0.0;
        double FaithStrength = 0.0;
        double FoodStrength = 0.0;
        double EducationStrength = 0.0;
        double WelfareStrength = 0.0;
        double LibertyStrength = 0.0;
        double SecurityStrength = 0.0;
        double PowerStability = 0.0;
        double MarketPreference = 0.0;
        double StatePreference = 0.0;
        double ReligionPreference = 0.0;
        double MilitaryPreference = 0.0;
        double EnvironmentPreference = 0.0;
        double IndustryPreference = 0.0;
        double IntellectualPreference = 0.0;
        double ConservativePreference = 0.0;
        int DiplomaticPartyBonus = 0;
        int DetenteBonus = 0;
        int TradeTreatyBonus = 0;
        int TourismStateBonus = 0;
        int TaxCutBonus = 0;
        int AusterityPenalty = 0;
    };

    int ClampInt(int Value, int MinValue, int MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    std::wstring FormatSignedInt(int Value)
    {
        if (Value > 0)
            return L"+" + std::to_wstring(Value);

        return std::to_wstring(Value);
    }

    std::wstring FormatInteger(long long Value)
    {
        const bool Negative = Value < 0;
        unsigned long long AbsValue = Negative ?
            static_cast<unsigned long long>(-Value) :
            static_cast<unsigned long long>(Value);
        std::wstring Digits = std::to_wstring(AbsValue);
        std::wstring Result;
        int GroupCount = 0;

        for (int Index = static_cast<int>(Digits.size()) - 1;
            Index >= 0;
            --Index)
        {
            if (GroupCount == 3)
            {
                Result.insert(Result.begin(), L',');
                GroupCount = 0;
            }

            Result.insert(Result.begin(), Digits[static_cast<size_t>(Index)]);
            ++GroupCount;
        }

        if (Negative)
            Result.insert(Result.begin(), L'-');

        return Result;
    }

    double NormalizeBias(float Value)
    {
        return Clamp01(0.5 + static_cast<double>(Value) * 0.5);
    }

    double GetGovernmentAffinity(
        const FNpcPoliticalChoice& Choice,
        EPoliticalStance Target)
    {
        const double SupportWeight =
            Choice.Support == EPoliticalSupportLevel::Weak ? 0.18 :
            Choice.Support == EPoliticalSupportLevel::Strong ? 0.40 :
            0.28;

        if (Choice.Stance == Target)
            return Clamp01(0.55 + SupportWeight);
        if (Choice.Stance == EPoliticalStance::Neutral)
            return 0.50;

        return Clamp01(0.28 - SupportWeight * 0.35);
    }

    bool HasActiveEdictFragment(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const wchar_t* Fragment)
    {
        if (!Fragment)
            return false;

        for (size_t Index = 0; Index < Snapshot.ActiveEdictLines.size(); ++Index)
        {
            if (Snapshot.ActiveEdictLines[Index].find(Fragment) !=
                std::wstring::npos)
            {
                return true;
            }
        }

        return false;
    }

    int CountActiveResourceTypes(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        int Count = 0;

        for (int ResourceIndex = 1;
            ResourceIndex < static_cast<int>(EResourceType::Count);
            ++ResourceIndex)
        {
            const auto& Resource =
                Snapshot.ResourceTypes[static_cast<size_t>(ResourceIndex)];

            if (Resource.TotalStock > 0 ||
                Resource.AvailableStock > 0 ||
                Resource.ProducerBuildingCount > 0 ||
                Resource.ConsumerBuildingCount > 0 ||
                Resource.ReservedIncoming > 0)
            {
                ++Count;
            }
        }

        return Count;
    }

    std::wstring FormatTopBuilding(
        const std::vector<std::pair<std::wstring, int>>& TopBuildings,
        size_t Index)
    {
        if (Index >= TopBuildings.size())
            return L"없음";

        return TopBuildings[Index].first +
            L" (" + FormatInteger(TopBuildings[Index].second) + L")";
    }

    std::wstring FormatElectionPromiseValue(
        const FElectionPromiseState& Promise,
        int Value)
    {
        if (Promise.Type == EElectionPromiseType::ExportIncome)
            return FormatInteger(Value);

        return std::to_wstring(Value);
    }

    int CountActiveElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (ElectionStatus.ActivePromises[static_cast<size_t>(Index)].Active)
                ++Count;
        }

        return Count;
    }

    int CountMetElectionPromises(const FElectionStatus& ElectionStatus)
    {
        int Count = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            if (IsElectionPromiseMet(
                    ElectionStatus.ActivePromises[static_cast<size_t>(Index)]))
            {
                ++Count;
            }
        }

        return Count;
    }

    int ComputeElectionPromiseModifierPercent(
        const FElectionStatus& ElectionStatus)
    {
        int VoteModifierPercent = 0;

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            const FElectionPromiseState& Promise =
                ElectionStatus.ActivePromises[static_cast<size_t>(Index)];

            if (!Promise.Active)
                continue;

            if (IsElectionPromiseMet(Promise))
                VoteModifierPercent += Promise.SuccessVoteModifierPercent;
            else
                VoteModifierPercent -= Promise.FailureVoteModifierPercent;
        }

        return VoteModifierPercent;
    }

    std::wstring BuildElectionPromiseProgressText(
        const FElectionStatus& ElectionStatus)
    {
        const int ActiveCount = CountActiveElectionPromises(ElectionStatus);

        if (ActiveCount <= 0)
            return L"현재 발표된 선거 공약 없음";

        std::wstring Result =
            L"이행 " +
            std::to_wstring(CountMetElectionPromises(ElectionStatus)) +
            L"/" +
            std::to_wstring(ActiveCount) +
            L"개";

        for (int Index = 0; Index < GElectionPromiseCount; ++Index)
        {
            const FElectionPromiseState& Promise =
                ElectionStatus.ActivePromises[static_cast<size_t>(Index)];

            if (!Promise.Active)
                continue;

            Result += L" / ";
            Result += Promise.Title;
            Result += L" ";
            Result += FormatElectionPromiseValue(
                Promise,
                Promise.CurrentValue);
            Result += L"/";
            Result += FormatElectionPromiseValue(
                Promise,
                Promise.TargetValue);
        }

        return Result;
    }

    std::wstring BuildElectionPromiseEvaluationText(
        const FElectionStatus& ElectionStatus)
    {
        const int ActiveCount = CountActiveElectionPromises(ElectionStatus);

        if (ActiveCount > 0)
        {
            const int MetCount = CountMetElectionPromises(ElectionStatus);
            const int FailedCount = ActiveCount - MetCount;
            return
                L"예상 표 보정 " +
                FormatSignedInt(
                    ComputeElectionPromiseModifierPercent(ElectionStatus)) +
                L"% / 미이행 " +
                std::to_wstring((std::max)(0, FailedCount)) +
                L"개";
        }

        if (!ElectionStatus.HasPromiseEvaluation)
            return L"-";

        return
            L"직전 선거 " +
            std::to_wstring(ElectionStatus.LastPromiseMetCount) +
            L"개 이행, " +
            std::to_wstring(ElectionStatus.LastPromiseFailedCount) +
            L"개 미이행 / 표 보정 " +
            FormatSignedInt(ElectionStatus.LastPromiseVoteModifierPercent) +
            L"%";
    }

    std::wstring BuildPoliticsElectionText(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        if (Snapshot.ElectionStatus.GameLost)
        {
            if (Snapshot.ElectionStatus.HasRecordedElection &&
                Snapshot.ElectionStatus.LastElectionYear > 0)
            {
                return L"정권 상실\n최근 선거 " +
                    FormatDate(
                        Snapshot.ElectionStatus.LastElectionYear,
                        Snapshot.ElectionStatus.LastElectionMonth,
                        Snapshot.ElectionStatus.LastElectionDay);
            }

            return L"정권 상실";
        }

        if (Snapshot.ElectionStatus.NextElectionYear > 0 &&
            Snapshot.ElectionStatus.NextElectionMonth > 0 &&
            Snapshot.ElectionStatus.NextElectionDay > 0)
        {
            std::wstring Result =
                L"다음 선거\n" +
                FormatDate(
                    Snapshot.ElectionStatus.NextElectionYear,
                    Snapshot.ElectionStatus.NextElectionMonth,
                    Snapshot.ElectionStatus.NextElectionDay);

            if (Snapshot.DaysUntilNextElection >= 0)
            {
                Result += L"\n";
                Result += std::to_wstring(Snapshot.DaysUntilNextElection);
                Result += L"일 남음";
            }

            const int ActivePromiseCount =
                CountActiveElectionPromises(Snapshot.ElectionStatus);

            if (ActivePromiseCount > 0)
            {
                Result += L"\n공약 ";
                Result += std::to_wstring(
                    CountMetElectionPromises(Snapshot.ElectionStatus));
                Result += L"/";
                Result += std::to_wstring(ActivePromiseCount);
                Result += L"개 이행 중";
            }
            else if (Snapshot.ElectionStatus.HasPromiseEvaluation)
            {
                Result += L"\n직전 공약 표 보정 ";
                Result += FormatSignedInt(
                    Snapshot.ElectionStatus.LastPromiseVoteModifierPercent);
                Result += L"%";
            }

            return Result;
        }

        if (Snapshot.ElectionStatus.HasRecordedElection &&
            Snapshot.ElectionStatus.LastElectionYear > 0)
        {
            return L"최근 선거\n" +
                FormatDate(
                    Snapshot.ElectionStatus.LastElectionYear,
                    Snapshot.ElectionStatus.LastElectionMonth,
                    Snapshot.ElectionStatus.LastElectionDay);
        }

        return L"선거 일정 없음";
    }

    FVector4 GetPoliticsFactionTint(int Index)
    {
        static const FVector4 GTints[GPoliticsFactionTileCount] =
        {
            FVector4(0.34f, 0.78f, 0.40f, 0.96f),
            FVector4(0.84f, 0.24f, 0.18f, 0.96f),
            FVector4(0.90f, 0.86f, 0.78f, 0.96f),
            FVector4(0.56f, 0.44f, 0.16f, 0.96f),
            FVector4(0.22f, 0.70f, 0.48f, 0.96f),
            FVector4(0.56f, 0.46f, 0.18f, 0.96f),
            FVector4(0.92f, 0.72f, 0.22f, 0.96f),
            FVector4(0.46f, 0.52f, 0.60f, 0.96f)
        };

        if (Index < 0 || Index >= GPoliticsFactionTileCount)
            return FVector4(0.90f, 0.72f, 0.18f, 0.96f);

        return GTints[Index];
    }

    FVector4 GetSignedTint(int Value)
    {
        if (Value > 0)
            return FVector4(0.18f, 0.62f, 0.34f, 1.f);
        if (Value < 0)
            return FVector4(0.80f, 0.22f, 0.18f, 1.f);

        return FVector4(0.31f, 0.27f, 0.21f, 1.f);
    }

    FVector4 GetSignedTint(double Value)
    {
        if (Value > 0.05)
            return FVector4(0.18f, 0.62f, 0.34f, 1.f);
        if (Value < -0.05)
            return FVector4(0.80f, 0.22f, 0.18f, 1.f);

        return FVector4(0.31f, 0.27f, 0.21f, 1.f);
    }

    FPoliticsFactionRuntimeData BuildPoliticsFactionRuntimeData(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        int Index)
    {
        FPoliticsFactionRuntimeData Result;
        if (Index < 0 || Index >= GPoliticsFactionTileCount)
            return Result;

        Result.Faction = static_cast<EPoliticalFaction>(Index);
        Result.Axis = GetPoliticalFactionAxis(Result.Faction);
        Result.Stance = GetPoliticalFactionStance(Result.Faction);
        Result.Label =
            GetPoliticalFactionVerboseName(Result.Axis, Result.Stance);

        const int AxisIndex = static_cast<int>(Result.Axis);
        Result.Count =
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                MemberCount;
        Result.NeutralCount =
            Snapshot.PoliticalCount[AxisIndex][
                static_cast<int>(EPoliticalStance::Neutral)];
        Result.AverageApproval =
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                AverageApproval;
        Result.AverageLifeScore =
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                AverageLifeScore;
        Result.AverageBuildingScore =
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                AverageBuildingScore;
        Result.AverageActionScore =
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                AverageActionScore;
        Result.Favor = ClampInt(
            static_cast<int>(std::lround(Result.AverageApproval)),
            0,
            100);

        const FNpcPoliticalChoice& GovernmentChoice =
            Snapshot.GovernmentProfile.Ideology.Get(Result.Axis);
        Result.AlignmentScore = static_cast<int>(std::lround(
            Snapshot.PoliticalSnapshot.Factions[static_cast<size_t>(Index)].
                AverageAlignmentScore));

        Result.GovernmentLine =
            std::wstring(GetPoliticalFactionDisplayName(
                Result.Axis,
                GovernmentChoice.Stance)) +
            L" / " +
            GetPoliticalSupportDisplayName(GovernmentChoice.Support);
        return Result;
    }

    FForeignContribution GetStrongestContribution(
        const std::array<FForeignContribution, 3>& Contributions)
    {
        FForeignContribution Best;
        int BestMagnitude = -1;

        for (size_t Index = 0; Index < Contributions.size(); ++Index)
        {
            const int Magnitude = std::abs(Contributions[Index].Value);
            if (Magnitude <= BestMagnitude)
                continue;

            Best = Contributions[Index];
            BestMagnitude = Magnitude;
        }

        if (Best.Label.empty())
            Best.Label = L"실시간 영향 없음";

        return Best;
    }

    FForeignContext BuildForeignContext(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
    {
        FForeignContext Context;
        const int IndustryIndex =
            BuildingCategoryInfo::ToIndex(EBuildingCategory::Industry);
        const int HighEducationCount =
            Snapshot.EducationCount[1] + Snapshot.EducationCount[2];
        const double CitizenCount =
            static_cast<double>((std::max)(1, Snapshot.ActiveCitizenCount));
        const double ExportStrength = Clamp01(
            static_cast<double>((std::max)(0LL, Snapshot.DailyExportIncome)) /
            8000.0);
        const double ResourceStrength = Clamp01(
            static_cast<double>((std::max)(0LL, Snapshot.TotalResourceStock)) /
            14000.0);

        Context.HarborStrength =
            Clamp01(static_cast<double>(Snapshot.HarborCount) / 6.0);
        Context.TradeStrength =
            Clamp01(Context.HarborStrength * 0.45 + ExportStrength * 0.55);
        Context.IndustryStrength =
            Clamp01(
                static_cast<double>(
                    Snapshot.BuildingCategoryCount[IndustryIndex]) / 8.0 +
                ResourceStrength * 0.35 +
                Clamp01(
                    static_cast<double>(Snapshot.JobCapacity) / 2500.0) * 0.15);
        Context.TourismStrength =
            Clamp01(
                static_cast<double>(Snapshot.TourismBuildingCount) / 8.0 +
                static_cast<double>(Snapshot.EntertainmentBuildingCount) /
                    14.0 +
                Clamp01(Snapshot.AverageFun / 100.0) * 0.25);
        Context.FaithStrength =
            Clamp01(static_cast<double>(Snapshot.FaithBuildingCount) / 6.0);
        Context.FoodStrength =
            Clamp01(
                static_cast<double>(Snapshot.FoodProviderCount) / 12.0 +
                Clamp01(Snapshot.AverageFood / 100.0) * 0.40);
        Context.EducationStrength =
            Clamp01(
                static_cast<double>(HighEducationCount) / CitizenCount * 1.2 +
                static_cast<double>(Snapshot.FreedomInfluenceBuildingCount) /
                    8.0 * 0.35);
        Context.WelfareStrength =
            Clamp01(
                NormalizeBias(Snapshot.GovernmentProfile.WelfareBias) * 0.50 +
                Clamp01(Snapshot.AverageHealth / 100.0) * 0.25 +
                Clamp01(Snapshot.AverageHousing / 100.0) * 0.25);
        Context.LibertyStrength =
            Clamp01(
                NormalizeBias(Snapshot.GovernmentProfile.LibertyBias) * 0.60 +
                Clamp01(Snapshot.AverageFreedom / 100.0) * 0.40);
        Context.SecurityStrength =
            Clamp01(
                NormalizeBias(Snapshot.GovernmentProfile.Militarization) *
                    0.35 +
                Clamp01(Snapshot.AverageSecurity / 100.0) * 0.35 +
                Clamp01(
                    static_cast<double>(
                        Snapshot.SecurityInfluenceBuildingCount) / 8.0) *
                    0.20 +
                (Snapshot.MartialLawActive ? 0.10 : 0.0));
        Context.PowerStability =
            Snapshot.TotalRequiredPowerMW > 0 ?
                Clamp01(
                    static_cast<double>((std::max)(
                        0,
                        Snapshot.TotalProducedPowerMW)) /
                    static_cast<double>(Snapshot.TotalRequiredPowerMW)) :
                (Snapshot.DisconnectedConsumerCount > 0 ? 0.25 : 1.0);

        Context.MarketPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.Economy,
                EPoliticalStance::Left);
        Context.StatePreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.Economy,
                EPoliticalStance::Right);
        Context.ReligionPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.ReligionMilitarism,
                EPoliticalStance::Left);
        Context.MilitaryPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.ReligionMilitarism,
                EPoliticalStance::Right);
        Context.EnvironmentPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.EnvironmentIndustry,
                EPoliticalStance::Left);
        Context.IndustryPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.EnvironmentIndustry,
                EPoliticalStance::Right);
        Context.IntellectualPreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.IntellectualConservative,
                EPoliticalStance::Left);
        Context.ConservativePreference =
            GetGovernmentAffinity(
                Snapshot.GovernmentProfile.Ideology.IntellectualConservative,
                EPoliticalStance::Right);

        Context.DiplomaticPartyBonus =
            HasActiveEdictFragment(Snapshot, L"외교적 슈퍼 파티") ? 10 : 0;
        Context.DetenteBonus =
            HasActiveEdictFragment(Snapshot, L"데탕트") ? 9 : 0;
        Context.TradeTreatyBonus =
            HasActiveEdictFragment(Snapshot, L"카리브해 무역 조약") ? 10 : 0;
        Context.TourismStateBonus =
            HasActiveEdictFragment(Snapshot, L"관광 국가") ? 8 : 0;
        Context.TaxCutBonus =
            HasActiveEdictFragment(Snapshot, L"감세") ? 6 : 0;
        Context.AusterityPenalty =
            HasActiveEdictFragment(Snapshot, L"긴축 예산") ? -8 : 0;

        return Context;
    }

    FForeignPowerRuntimeData BuildForeignPowerRuntimeData(
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const FForeignContext& Context,
        int Index)
    {
        FForeignPowerRuntimeData Result;
        std::array<FForeignContribution, 3> BuildingContributions = {};
        std::array<FForeignContribution, 3> EdictContributions = {};
        std::array<FForeignContribution, 3> MiscContributions = {};

        switch (Index)
        {
        case 0:
            Result.Name = L"중국";
            BuildingContributions =
            {{
                { L"산업 기반", static_cast<int>(std::lround(
                    Context.IndustryStrength * 8.0 +
                    Context.IndustryPreference * 4.0)) },
                { L"항만·무역", static_cast<int>(std::lround(
                    Context.TradeStrength * 10.0)) },
                { L"치안 안정", static_cast<int>(std::lround(
                    Context.SecurityStrength * 6.0 -
                    Context.LibertyStrength * 3.0)) }
            }};
            EdictContributions =
            {{
                { L"카리브해 무역 조약", Context.TradeTreatyBonus },
                { L"외교적 슈퍼 파티", Context.DiplomaticPartyBonus },
                { L"긴축 예산", Context.AusterityPenalty / 2 }
            }};
            Result.GovernmentModifier = ClampInt(
                static_cast<int>(std::lround(
                    (Context.StatePreference - 0.5) * 16.0 +
                    (Context.IndustryPreference - 0.5) * 14.0)),
                -25,
                25);
            MiscContributions =
            {{
                { L"수출 실적", static_cast<int>(std::lround(
                    Context.TradeStrength * 8.0)) },
                { L"재정 안정", Snapshot.DailyNetChange >= 0 ? 4 : -4 },
                { L"반란 위험", Snapshot.RebelRiskScore < 33.0 ? 2 :
                    Snapshot.RebelRiskScore >= 66.0 ? -6 : -2 }
            }};
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 15.0 +
                    Context.HarborStrength * 5.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    38.0 +
                    Context.TradeStrength * 26.0 +
                    Context.IndustryPreference * 18.0 +
                    Context.SecurityStrength * 10.0 +
                    Context.StatePreference * 8.0 -
                    Context.EnvironmentPreference * 6.0)),
                0,
                100);
            Result.EconomicAid =
                Snapshot.DailyNetChange < 0 && Result.Relation >= 70 ?
                    ClampInt(
                        static_cast<int>(std::lround(
                            static_cast<double>(Result.Relation - 70) *
                            65.0 *
                            Clamp01(
                                static_cast<double>(-Snapshot.DailyNetChange) /
                                6000.0))),
                        0,
                        15000) :
                    0;
            break;
        case 1:
            Result.Name = L"러시아";
            BuildingContributions =
            {{
                { L"치안·군사 기반", static_cast<int>(std::lround(
                    Context.SecurityStrength * 10.0 +
                    Context.MilitaryPreference * 5.0)) },
                { L"전력 안정", static_cast<int>(std::lround(
                    Context.PowerStability * 8.0)) },
                { L"항만 인프라", static_cast<int>(std::lround(
                    Context.HarborStrength * 4.0)) }
            }};
            EdictContributions =
            {{
                { L"데탕트 정책", Context.DetenteBonus },
                { L"계엄령", Snapshot.MartialLawActive ? 8 : 0 },
                { L"긴축 예산", Context.AusterityPenalty / 2 }
            }};
            Result.GovernmentModifier = ClampInt(
                static_cast<int>(std::lround(
                    (Context.MilitaryPreference - 0.5) * 18.0 +
                    (Context.ConservativePreference - 0.5) * 8.0)),
                -25,
                25);
            MiscContributions =
            {{
                { L"정치 안정", Snapshot.OppositionPercent < 35.0 ? 5 : -3 },
                { L"치안 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageSecurity / 100.0) - 0.5) *
                    12.0)) },
                { L"자유 압박", Snapshot.MartialLawActive ? -6 : 0 }
            }};
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 8.0 +
                    Context.PowerStability * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    35.0 +
                    Context.SecurityStrength * 20.0 +
                    Context.MilitaryPreference * 18.0 +
                    Context.PowerStability * 10.0 +
                    Context.StatePreference * 8.0 +
                    Context.DetenteBonus * 0.6 -
                    Context.LibertyStrength * 6.0)),
                0,
                100);
            Result.EconomicAid = 0;
            break;
        case 2:
            Result.Name = L"미국";
            BuildingContributions =
            {{
                { L"관광·오락 산업", static_cast<int>(std::lround(
                    Context.TourismStrength * 10.0)) },
                { L"자유 영향 건물", static_cast<int>(std::lround(
                    Context.LibertyStrength * 9.0)) },
                { L"교육 기반", static_cast<int>(std::lround(
                    Context.EducationStrength * 6.0)) }
            }};
            EdictContributions =
            {{
                { L"감세", Context.TaxCutBonus },
                { L"관광 국가", Context.TourismStateBonus },
                { L"외교적 슈퍼 파티", Context.DiplomaticPartyBonus }
            }};
            Result.GovernmentModifier = ClampInt(
                static_cast<int>(std::lround(
                    (Context.MarketPreference - 0.5) * 16.0 +
                    (Context.IntellectualPreference - 0.5) * 10.0 +
                    (Context.LibertyStrength - 0.5) * 10.0)),
                -25,
                25);
            MiscContributions =
            {{
                { L"정권 지지율", Snapshot.SupportPercent >= 50.0 ? 5 : -4 },
                { L"자유 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageFreedom / 100.0) - 0.5) *
                    14.0)) },
                { L"계엄령", Snapshot.MartialLawActive ? -8 : 0 }
            }};
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 9.0 +
                    Context.TourismStrength * 5.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    36.0 +
                    Context.MarketPreference * 18.0 +
                    Context.LibertyStrength * 22.0 +
                    Context.TourismStrength * 16.0 +
                    Context.EducationStrength * 12.0 +
                    Context.DiplomaticPartyBonus * 0.5 +
                    Context.TaxCutBonus * 0.4 -
                    (Snapshot.MartialLawActive ? 12.0 : 0.0) -
                    (Snapshot.TaxEventStatus.Active ? 6.0 : 0.0))),
                0,
                100);
            Result.EconomicAid =
                Snapshot.DailyNetChange < 0 && Result.Relation >= 72 ?
                    ClampInt(
                        static_cast<int>(std::lround(
                            static_cast<double>(Result.Relation - 68) *
                            110.0 *
                            Clamp01(
                                static_cast<double>(-Snapshot.DailyNetChange) /
                                7000.0))),
                        0,
                        20000) :
                    0;
            break;
        case 3:
            Result.Name = L"중동";
            BuildingContributions =
            {{
                { L"신앙 건물", static_cast<int>(std::lround(
                    Context.FaithStrength * 12.0)) },
                { L"식량 공급망", static_cast<int>(std::lround(
                    Context.FoodStrength * 8.0)) },
                { L"치안 안정", static_cast<int>(std::lround(
                    Context.SecurityStrength * 5.0)) }
            }};
            EdictContributions =
            {{
                { L"계엄령", Snapshot.MartialLawActive ? 6 : 0 },
                { L"외교적 슈퍼 파티", Context.DiplomaticPartyBonus / 2 },
                { L"긴축 예산", Context.AusterityPenalty / 3 }
            }};
            Result.GovernmentModifier = ClampInt(
                static_cast<int>(std::lround(
                    (Context.ReligionPreference - 0.5) * 18.0 +
                    (Context.ConservativePreference - 0.5) * 8.0)),
                -25,
                25);
            MiscContributions =
            {{
                { L"식량 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageFood / 100.0) - 0.5) *
                    12.0)) },
                { L"치안 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageSecurity / 100.0) - 0.5) *
                    10.0)) },
                { L"정권 지지율", Snapshot.SupportPercent >= 45.0 ? 3 : -3 }
            }};
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 7.0 +
                    Context.FoodStrength * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    34.0 +
                    Context.ReligionPreference * 22.0 +
                    Context.FaithStrength * 18.0 +
                    Context.SecurityStrength * 10.0 +
                    Context.FoodStrength * 8.0 +
                    Context.ConservativePreference * 8.0 -
                    Context.LibertyStrength * 4.0)),
                0,
                100);
            Result.EconomicAid = 0;
            break;
        default:
            Result.Name = L"유럽연합";
            BuildingContributions =
            {{
                { L"교육·미디어 기반", static_cast<int>(std::lround(
                    Context.EducationStrength * 10.0)) },
                { L"환경 친화 노선", static_cast<int>(std::lround(
                    Context.EnvironmentPreference * 8.0 +
                    Context.WelfareStrength * 3.0)) },
                { L"관광 산업", static_cast<int>(std::lround(
                    Context.TourismStrength * 5.0)) }
            }};
            EdictContributions =
            {{
                { L"데탕트 정책", Context.DetenteBonus },
                { L"관광 국가", Context.TourismStateBonus / 2 },
                { L"외교적 슈퍼 파티", Context.DiplomaticPartyBonus / 2 }
            }};
            Result.GovernmentModifier = ClampInt(
                static_cast<int>(std::lround(
                    (Context.EnvironmentPreference - 0.5) * 16.0 +
                    (Context.IntellectualPreference - 0.5) * 14.0 +
                    (Context.WelfareStrength - 0.5) * 10.0)),
                -25,
                25);
            MiscContributions =
            {{
                { L"자유 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageFreedom / 100.0) - 0.5) *
                    14.0)) },
                { L"보건 만족도", static_cast<int>(std::lround(
                    (Clamp01(Snapshot.AverageHealth / 100.0) - 0.5) *
                    12.0)) },
                { L"계엄령", Snapshot.MartialLawActive ? -8 : 0 }
            }};
            Result.TradeModifier = ClampInt(
                static_cast<int>(std::lround(
                    Context.TradeStrength * 8.0 +
                    Context.EducationStrength * 4.0)),
                -20,
                20);
            Result.Relation = ClampInt(
                static_cast<int>(std::lround(
                    40.0 +
                    Context.EnvironmentPreference * 18.0 +
                    Context.IntellectualPreference * 18.0 +
                    Context.LibertyStrength * 16.0 +
                    Context.EducationStrength * 12.0 +
                    Context.WelfareStrength * 10.0 +
                    Context.DetenteBonus * 0.5 -
                    (Snapshot.MartialLawActive ? 14.0 : 0.0))),
                0,
                100);
            Result.EconomicAid =
                Snapshot.DailyNetChange < 0 && Result.Relation >= 70 ?
                    ClampInt(
                        static_cast<int>(std::lround(
                            static_cast<double>(Result.Relation - 66) *
                            90.0 *
                            Clamp01(
                                static_cast<double>(-Snapshot.DailyNetChange) /
                                6500.0))),
                        0,
                        18000) :
                    0;
            break;
        }

        auto SumContributionValues =
            [](const std::array<FForeignContribution, 3>& Source) -> int
        {
            int Total = 0;
            for (size_t ContributionIndex = 0;
                ContributionIndex < Source.size();
                ++ContributionIndex)
            {
                Total += Source[ContributionIndex].Value;
            }
            return Total;
        };

        Result.BuildingModifier = SumContributionValues(BuildingContributions);
        Result.BuildingLine = GetStrongestContribution(BuildingContributions);
        Result.EdictModifier = SumContributionValues(EdictContributions);
        Result.EdictLine = GetStrongestContribution(EdictContributions);
        Result.MiscModifier = SumContributionValues(MiscContributions);
        Result.MiscLine = GetStrongestContribution(MiscContributions);
        const auto& ForeignState =
            Snapshot.ForeignPowerStates[static_cast<size_t>(ClampInt(
                Index,
                0,
                GForeignPowerCount - 1))];
        Result.Relation = ForeignState.Relation;
        Result.TradeModifier = ForeignState.TradeModifier;

        const int AidScale =
            Index == 0 ? 65 :
            Index == 2 ? 110 :
            Index == 4 ? 90 :
            0;
        Result.EconomicAid =
            Snapshot.DailyNetChange < 0 &&
                Result.Relation >= 68 &&
                AidScale > 0 ?
                ClampInt(
                    static_cast<int>(std::lround(
                        static_cast<double>(Result.Relation - 66) *
                        static_cast<double>(AidScale) *
                        Clamp01(
                            static_cast<double>(-Snapshot.DailyNetChange) /
                            7000.0) *
                        (1.0 +
                            static_cast<double>((std::max)(
                                0,
                                ForeignState.Standing)) / 120.0))),
                    0,
                    22000) :
                0;
        Result.Status =
            TradeDiplomacyRuntime::GetForeignPowerStatusText(Result.Relation);

        return Result;
    }

    void SetSelectableDetailRowEnabled(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        bool Enabled)
    {
        auto Button = Row.Button.lock();
        auto Background = Row.Background.lock();
        auto Label = Row.Label.lock();
        auto Value = Row.Value.lock();

        if (Button)
        {
            Button->SetEnable(Enabled);
            Button->ButtonEnable(Enabled);
        }
        if (Background && !Enabled)
            Background->SetTint(1.f, 1.f, 1.f, 0.f);
        if (Label)
        {
            if (!Enabled)
                Label->SetText(L"");
            Label->SetEnable(Enabled);
        }
        if (Value)
        {
            if (!Enabled)
                Value->SetText(L"");
            Value->SetEnable(Enabled);
        }
    }

    std::wstring FormatDemandValue(
        const FPoliticalDemandState& Demand,
        int Value)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::ExportIncome:
            return FormatCurrency(Value);
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return std::to_wstring(Value) + L"%";
        case EPoliticalDemandObjectiveType::ActiveTradeRoutes:
            return FormatInteger(Value) + L"개";
        case EPoliticalDemandObjectiveType::Housing:
        case EPoliticalDemandObjectiveType::Food:
        case EPoliticalDemandObjectiveType::Faith:
        case EPoliticalDemandObjectiveType::Security:
        case EPoliticalDemandObjectiveType::Freedom:
        case EPoliticalDemandObjectiveType::Health:
        case EPoliticalDemandObjectiveType::None:
        default:
            return std::to_wstring(Value);
        }
    }

    std::wstring BuildDemandProgressText(const FPoliticalDemandState& Demand)
    {
        if (!Demand.Active)
            return L"대기 중";

        const bool CeilingObjective =
            Demand.ObjectiveType == EPoliticalDemandObjectiveType::IncomeTaxCeiling ||
            Demand.ObjectiveType == EPoliticalDemandObjectiveType::PropertyTaxCeiling;

        return
            (Demand.Status == EPoliticalDemandStatus::Accepted ?
                L"수행 중" :
                L"응답 대기") +
            std::wstring(L" / 현재 ") +
            FormatDemandValue(Demand, Demand.CurrentValue) +
            L" / 목표 " +
            FormatDemandValue(Demand, Demand.TargetValue) +
            (CeilingObjective ? L" 이하" : L" 이상");
    }

    std::wstring BuildDemandEffectText(const FPoliticalDemandState& Demand)
    {
        if (!Demand.Active)
            return L"-";

        return L"보상: " +
            Demand.RewardText +
            L" / 실패: " +
            Demand.PenaltyText;
    }

    void ConfigureDemandActionRow(
        const CAlmanacWidget::FDetailRowWidgets& Row,
        bool Enabled,
        const std::wstring& Label)
    {
        SetSelectableDetailRowEnabled(Row, Enabled);

        if (!Enabled)
            return;

        SetDetailRowData(Row, Label, L"");
    }
}

void FAlmanacRenderer::ApplyPoliticsPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    std::array<FPoliticsFactionRuntimeData, GPoliticsFactionTileCount>
        Factions = {};

    for (int Index = 0; Index < GPoliticsFactionTileCount; ++Index)
    {
        Factions[static_cast<size_t>(Index)] =
            BuildPoliticsFactionRuntimeData(Snapshot, Index);
    }

    const int SelectedPoliticsFactionIndex =
        ClampInt(
            Widget.mSelectedPoliticsFactionIndex,
            0,
            GPoliticsFactionTileCount - 1);
    Widget.mSelectedPoliticsFactionIndex = SelectedPoliticsFactionIndex;
    const FPoliticsFactionRuntimeData& SelectedFaction =
        Factions[static_cast<size_t>(SelectedPoliticsFactionIndex)];

    for (int Index = 0;
        Index < static_cast<int>(Widget.mPoliticsFactionTiles.size()) &&
            Index < GPoliticsFactionTileCount;
        ++Index)
    {
        SetPoliticsFactionTileData(
            Widget.mPoliticsFactionTiles[static_cast<size_t>(Index)],
            GPoliticsFactionIcons[Index],
            GetPoliticsFactionTint(Index),
            Factions[static_cast<size_t>(Index)].Label,
            Factions[static_cast<size_t>(Index)].Count,
            Factions[static_cast<size_t>(Index)].Favor,
            Index == SelectedPoliticsFactionIndex);
    }

    for (int Index = 0; Index < GPoliticsNeutralCount; ++Index)
    {
        if (Index >= static_cast<int>(Widget.mPoliticsNeutralTexts.size()))
            break;

        if (auto Text = Widget.mPoliticsNeutralTexts[static_cast<size_t>(Index)].lock())
        {
            Text->SetText(
                (std::wstring(GetPoliticalAxisDisplayName(
                    static_cast<EPoliticalAxis>(Index))) +
                    L"\n" +
                    std::to_wstring(
                        Snapshot.PoliticalCount[Index][
                            static_cast<int>(EPoliticalStance::Neutral)])).c_str());
        }
    }

    if (auto Title = Widget.mPoliticsFactionTitle.lock())
        Title->SetText(SelectedFaction.Label.c_str());
    if (auto Label = Widget.mPoliticsFactionApprovalLabel.lock())
    {
        Label->SetText(
            (SelectedFaction.Label + L" 승인도").c_str());
    }
    if (auto Value = Widget.mPoliticsFactionApprovalValue.lock())
        Value->SetText(std::to_wstring(SelectedFaction.Favor).c_str());

    if (auto Title = Widget.mPoliticsSupportTitle.lock())
        Title->SetText(L"지지율");

    const std::wstring PoliticsSupportValues[GPoliticsSupportRowCount] =
    {
        FormatInteger(Snapshot.ActiveCitizenCount),
        FormatPercent(Snapshot.SupportPercent),
        FormatPercent(Snapshot.AbstainPercent),
        FormatPercent(Snapshot.OppositionPercent)
    };
    const wchar_t* PoliticsSupportLabels[GPoliticsSupportRowCount] =
    {
        L"유권자",
        L"현 정권",
        L"부동층",
        L"야권"
    };

    for (int Index = 0;
        Index < static_cast<int>(Widget.mPoliticsSupportRows.size()) &&
            Index < GPoliticsSupportRowCount;
        ++Index)
    {
        SetDetailRowData(
            Widget.mPoliticsSupportRows[static_cast<size_t>(Index)],
            PoliticsSupportLabels[Index],
            PoliticsSupportValues[Index]);
    }

    for (size_t Index = 0; Index < Widget.mPoliticsSupportRows.size(); ++Index)
    {
        if (auto Background =
            Widget.mPoliticsSupportRows[Index].Background.lock())
        {
            Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }
    }

    if (auto Text = Widget.mPoliticsElectionText.lock())
        Text->SetText(BuildPoliticsElectionText(Snapshot).c_str());

    const double SelectedCountRatio =
        Snapshot.ActiveCitizenCount > 0 ?
            static_cast<double>(SelectedFaction.Count) /
                static_cast<double>(Snapshot.ActiveCitizenCount) :
            0.0;
    const double NeutralCountRatio =
        Snapshot.ActiveCitizenCount > 0 ?
            static_cast<double>(SelectedFaction.NeutralCount) /
                static_cast<double>(Snapshot.ActiveCitizenCount) :
            0.0;
    const std::wstring ElectionWarningText =
        BuildElectionWarningSummary(
            Snapshot.ElectionStatus.GameLost,
            Snapshot.DaysUntilNextElection,
            Snapshot.ElectionWarningScore,
            Snapshot.TaxEventStatus);
    const std::wstring PromiseProgressText =
        BuildElectionPromiseProgressText(Snapshot.ElectionStatus);
    const std::wstring PromiseEvaluationText =
        BuildElectionPromiseEvaluationText(Snapshot.ElectionStatus);
    const std::wstring ElectionOutcomeText =
        PromiseEvaluationText == L"-" ?
            ElectionWarningText :
            ElectionWarningText + L" / " + PromiseEvaluationText;
    const int PromiseModifierDisplay =
        CountActiveElectionPromises(Snapshot.ElectionStatus) > 0 ?
            ComputeElectionPromiseModifierPercent(Snapshot.ElectionStatus) :
            Snapshot.ElectionStatus.LastPromiseVoteModifierPercent;
    const FPoliticalDemandState& SelectedDemand =
        Snapshot.FactionDemandStates[
            static_cast<size_t>(SelectedPoliticsFactionIndex)];

    SetDetailRowData(Widget.mPoliticsDetails[0], L"▽ 세력 개요", L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[1],
        SelectedFaction.Label +
            L" / " +
            GetPoliticalAxisDisplayName(SelectedFaction.Axis),
        L"");
    SetDetailRowData(Widget.mPoliticsDetails[2], L"▽ 시민 분포", L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[3],
        L"해당 성향 시민",
        FormatCountWithPercent(SelectedFaction.Count, SelectedCountRatio));
    SetDetailRowData(
        Widget.mPoliticsDetails[4],
        L"동축 무관심 시민",
        FormatCountWithPercent(SelectedFaction.NeutralCount, NeutralCountRatio));
    SetDetailRowData(
        Widget.mPoliticsDetails[5],
        L"정부 노선",
        SelectedFaction.GovernmentLine);
    SetDetailRowData(Widget.mPoliticsDetails[6], L"▽ 현재 정치 지표", L"");
    SetDetailRowData(
        Widget.mPoliticsDetails[7],
        L"정권 정렬도",
        FormatSignedInt(SelectedFaction.AlignmentScore),
        false,
        GetSignedTint(SelectedFaction.AlignmentScore));
    SetDetailRowData(
        Widget.mPoliticsDetails[8],
        L"세력 승인도 평균",
        FormatFixed1(SelectedFaction.AverageApproval));
    SetDetailRowData(
        Widget.mPoliticsDetails[9],
        L"생활 평가",
        FormatFixed1(SelectedFaction.AverageLifeScore));
    SetDetailRowData(
        Widget.mPoliticsDetails[10],
        L"건물 효과",
        FormatSignedFixed1(SelectedFaction.AverageBuildingScore),
        false,
        GetSignedTint(SelectedFaction.AverageBuildingScore));
    SetDetailRowData(
        Widget.mPoliticsDetails[11],
        L"정책 효과",
        FormatSignedFixed1(SelectedFaction.AverageActionScore),
        false,
        GetSignedTint(SelectedFaction.AverageActionScore));
    SetDetailRowData(
        Widget.mPoliticsDetails[12],
        L"공약 진행",
        PromiseProgressText);
    SetDetailRowData(
        Widget.mPoliticsDetails[13],
        L"선거 경보 / 판정",
        ElectionOutcomeText,
        false,
        GetSignedTint(PromiseModifierDisplay));
    SetDetailRowData(Widget.mPoliticsDetails[14], L"▽ 세력 요구", L"");

    if (SelectedDemand.Active)
    {
        SetDetailRowData(
            Widget.mPoliticsDetails[15],
            SelectedDemand.Title,
            BuildDemandProgressText(SelectedDemand));
        SetDetailRowData(
            Widget.mPoliticsDetails[16],
            L"보상 / 불이익",
            BuildDemandEffectText(SelectedDemand));
        ConfigureDemandActionRow(
            Widget.mPoliticsDetails[17],
            true,
            SelectedDemand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 수락됨" :
                L"요구 수락");
        ConfigureDemandActionRow(
            Widget.mPoliticsDetails[18],
            true,
            SelectedDemand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 포기" :
                L"요구 거절");
    }
    else
    {
        SetDetailRowData(
            Widget.mPoliticsDetails[15],
            L"현재 요구 없음",
            L"이 세력은 당분간 추가 요구를 제시하지 않습니다.");
        SetDetailRowData(
            Widget.mPoliticsDetails[16],
            L"대기 상태",
            L"-");
        ConfigureDemandActionRow(
            Widget.mPoliticsDetails[17],
            false,
            L"");
        ConfigureDemandActionRow(
            Widget.mPoliticsDetails[18],
            false,
            L"");
    }

    const int PoliticsHeaderRows[4] = { 0, 2, 6, 14 };
    for (int HeaderIndex : PoliticsHeaderRows)
    {
        if (HeaderIndex >= static_cast<int>(Widget.mPoliticsDetails.size()))
            continue;

        if (auto Background =
            Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].
                Background.lock())
        {
            Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
        }
        if (auto Label =
            Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].
                Label.lock())
        {
            Label->SetTextColor(112, 86, 38, 255);
        }
    }

    for (int Index = 0;
        Index < static_cast<int>(Widget.mPoliticsDetails.size());
        ++Index)
    {
        if (Index == 0 || Index == 2 || Index == 6 || Index == 14)
            continue;

        if (auto Background =
            Widget.mPoliticsDetails[static_cast<size_t>(Index)].
                Background.lock())
        {
            Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }
    }

    if (auto Label = Widget.mPoliticsDetails[1].Label.lock())
    {
        Label->SetFontSize(18.f);
        Label->SetTextColor(86, 70, 44, 255);
    }
}

void FAlmanacRenderer::ApplyForeignPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    const FForeignContext Context = BuildForeignContext(Snapshot);
    std::array<FForeignPowerRuntimeData, GForeignPowerCount> ForeignPowers = {};

    for (int Index = 0; Index < GForeignPowerCount; ++Index)
    {
        ForeignPowers[static_cast<size_t>(Index)] =
            BuildForeignPowerRuntimeData(Snapshot, Context, Index);
    }

    const int SelectedForeignPowerIndex =
        ClampInt(
            Widget.mSelectedForeignPowerIndex,
            0,
            GForeignPowerCount - 1);
    Widget.mSelectedForeignPowerIndex = SelectedForeignPowerIndex;
    const FForeignPowerRuntimeData& SelectedForeignPower =
        ForeignPowers[static_cast<size_t>(SelectedForeignPowerIndex)];
    const auto& SelectedForeignState =
        Snapshot.ForeignPowerStates[
            static_cast<size_t>(SelectedForeignPowerIndex)];
    const FPoliticalDemandState& SelectedForeignDemand =
        Snapshot.ForeignDemandStates[
            static_cast<size_t>(SelectedForeignPowerIndex)];

    for (int Index = 0;
        Index < static_cast<int>(Widget.mForeignRows.size()) &&
            Index < GForeignPowerCount;
        ++Index)
    {
        SetForeignPowerRowData(
            Widget.mForeignRows[static_cast<size_t>(Index)],
            GForeignPowerIcons[Index],
            ForeignPowers[static_cast<size_t>(Index)].Name,
            std::to_wstring(ForeignPowers[static_cast<size_t>(Index)].Relation),
            static_cast<float>(
                Clamp01(
                    static_cast<double>(
                        ForeignPowers[static_cast<size_t>(Index)].Relation) /
                    100.0)),
            Index == SelectedForeignPowerIndex);
    }

    if (auto Title = Widget.mForeignTitle.lock())
        Title->SetText(SelectedForeignPower.Name.c_str());
    if (auto Text = Widget.mForeignStatusLabel.lock())
        Text->SetText(L"외교 상태");
    if (auto Text = Widget.mForeignStatusValue.lock())
        Text->SetText(SelectedForeignPower.Status.c_str());

    SetDetailRowData(
        Widget.mForeignDetails[0],
        SelectedForeignPower.Name + L" 외교 지수",
        std::to_wstring(SelectedForeignPower.Relation));
    SetDetailRowData(
        Widget.mForeignDetails[1],
        L"경제 원조",
        FormatCompactCurrency(SelectedForeignPower.EconomicAid));
    SetDetailRowData(Widget.mForeignDetails[2], L"관계 수정치", L"");
    SetDetailRowData(
        Widget.mForeignDetails[3],
        L"건물 수정치",
        FormatSignedInt(SelectedForeignPower.BuildingModifier),
        false,
        GetSignedTint(SelectedForeignPower.BuildingModifier));
    SetDetailRowData(
        Widget.mForeignDetails[4],
        SelectedForeignPower.BuildingLine.Label,
        FormatSignedInt(SelectedForeignPower.BuildingLine.Value),
        false,
        GetSignedTint(SelectedForeignPower.BuildingLine.Value));
    SetDetailRowData(
        Widget.mForeignDetails[5],
        L"칙령 수정치",
        FormatSignedInt(SelectedForeignPower.EdictModifier),
        false,
        GetSignedTint(SelectedForeignPower.EdictModifier));
    SetDetailRowData(
        Widget.mForeignDetails[6],
        SelectedForeignPower.EdictLine.Label,
        FormatSignedInt(SelectedForeignPower.EdictLine.Value),
        false,
        GetSignedTint(SelectedForeignPower.EdictLine.Value));
    SetDetailRowData(
        Widget.mForeignDetails[7],
        L"정권 노선",
        FormatSignedInt(SelectedForeignPower.GovernmentModifier),
        false,
        GetSignedTint(SelectedForeignPower.GovernmentModifier));
    SetDetailRowData(
        Widget.mForeignDetails[8],
        L"무역 standing",
        FormatSignedInt(SelectedForeignState.Standing),
        false,
        GetSignedTint(SelectedForeignState.Standing));
    SetDetailRowData(
        Widget.mForeignDetails[9],
        L"계약 현황",
        std::wstring(L"활성 ") +
            FormatInteger(SelectedForeignState.ActiveContractCount) +
            L" / 완료 " +
            FormatInteger(SelectedForeignState.CompletedContractCount) +
            L" / 실패 " +
            FormatInteger(SelectedForeignState.FailedContractCount));
    SetDetailRowData(
        Widget.mForeignDetails[10],
        L"무역 수정치",
        FormatSignedInt(SelectedForeignPower.TradeModifier),
        false,
        GetSignedTint(SelectedForeignPower.TradeModifier));
    SetDetailRowData(Widget.mForeignDetails[11], L"▽ 외교 요구", L"");

    if (SelectedForeignDemand.Active)
    {
        SetDetailRowData(
            Widget.mForeignDetails[12],
            SelectedForeignDemand.Title,
            BuildDemandProgressText(SelectedForeignDemand));
        SetDetailRowData(
            Widget.mForeignDetails[13],
            L"보상 / 불이익",
            BuildDemandEffectText(SelectedForeignDemand));
        ConfigureDemandActionRow(
            Widget.mForeignDetails[14],
            true,
            SelectedForeignDemand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 수락됨" :
                L"요구 수락");
        ConfigureDemandActionRow(
            Widget.mForeignDetails[15],
            true,
            SelectedForeignDemand.Status == EPoliticalDemandStatus::Accepted ?
                L"요구 포기" :
                L"요구 거절");
    }
    else
    {
        SetDetailRowData(
            Widget.mForeignDetails[12],
            L"현재 요구 없음",
            L"해당 초강대국은 현재 별도 조건을 제시하지 않습니다.");
        SetDetailRowData(
            Widget.mForeignDetails[13],
            L"대기 상태",
            L"-");
        ConfigureDemandActionRow(
            Widget.mForeignDetails[14],
            false,
            L"");
        ConfigureDemandActionRow(
            Widget.mForeignDetails[15],
            false,
            L"");
    }

    if (auto Background = Widget.mForeignDetails[2].Background.lock())
        Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
    if (auto Label = Widget.mForeignDetails[2].Label.lock())
        Label->SetTextColor(112, 86, 38, 255);
    if (auto Background = Widget.mForeignDetails[11].Background.lock())
        Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
    if (auto Label = Widget.mForeignDetails[11].Label.lock())
        Label->SetTextColor(112, 86, 38, 255);

    for (int Index = 0;
        Index < static_cast<int>(Widget.mForeignDetails.size());
        ++Index)
    {
        if (Index == 2 || Index == 11)
            continue;

        if (auto Background =
            Widget.mForeignDetails[static_cast<size_t>(Index)].
                Background.lock())
        {
            Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }
    }

    if (auto Notice = Widget.mForeignNotice.lock())
    {
        const bool HasRecentDelta =
            SelectedForeignState.LastRelationChange != 0 ||
            SelectedForeignState.LastStandingChange != 0;
        Notice->SetEnable(HasRecentDelta);
        Notice->SetText(
            HasRecentDelta ?
                (std::wstring(L"최근 계약 변화: 관계 ") +
                    FormatSignedInt(SelectedForeignState.LastRelationChange) +
                    L", standing " +
                    FormatSignedInt(SelectedForeignState.LastStandingChange))
                    .c_str() :
                L"");
    }
}

void FAlmanacRenderer::ApplyBuildingPage(
    CAlmanacWidget& Widget,
    const AlmanacDataProvider::FAlmanacSnapshot& Snapshot)
{
    constexpr int GActualBuildingCategoryCount =
        BuildingCategoryInfo::GBuildingCategoryCount;
    const int SelectedBuildingCategoryIndex =
        ClampInt(
            Widget.mSelectedBuildingCategoryIndex,
            0,
            GActualBuildingCategoryCount - 1);
    Widget.mSelectedBuildingCategoryIndex = SelectedBuildingCategoryIndex;

    for (int Index = 0; Index < static_cast<int>(Widget.mBuildingRows.size()); ++Index)
    {
        auto& Row = Widget.mBuildingRows[static_cast<size_t>(Index)];
        const bool Enabled = Index < GActualBuildingCategoryCount;
        SetSelectableDetailRowEnabled(Row, Enabled);

        if (!Enabled)
            continue;

        const auto& Category =
            Snapshot.BuildingCategories[static_cast<size_t>(Index)];
        SetDetailRowData(
            Row,
            BuildingCategoryInfo::GetDisplayName(Category.Category),
            FormatInteger(Category.Count),
            Index == SelectedBuildingCategoryIndex);
    }

    const auto& SelectedCategory =
        Snapshot.BuildingCategories[
            static_cast<size_t>(SelectedBuildingCategoryIndex)];
    const double CategoryRatio =
        Snapshot.TotalBuildingCount > 0 ?
            static_cast<double>(SelectedCategory.Count) /
                static_cast<double>(Snapshot.TotalBuildingCount) :
            0.0;
    const int EducatedCitizenCount =
        Snapshot.EducationCount[1] + Snapshot.EducationCount[2];
    const int ActiveResourceTypeCount = CountActiveResourceTypes(Snapshot);
    const std::vector<std::pair<std::wstring, int>> BottleneckWarnings =
        BuildLogisticsWarningEntries(Snapshot, true);
    const std::vector<std::pair<std::wstring, int>> OverflowWarnings =
        BuildLogisticsWarningEntries(Snapshot, false);
    const std::wstring BottleneckWarningSummary =
        BuildLogisticsWarningSummary(BottleneckWarnings, 2);
    const std::wstring OverflowWarningSummary =
        BuildLogisticsWarningSummary(OverflowWarnings, 2, L"%");

    if (auto Title = Widget.mBuildingCategoryTitle.lock())
    {
        Title->SetText(
            BuildingCategoryInfo::GetDisplayName(
                SelectedCategory.Category));
    }

    std::array<std::pair<std::wstring, std::wstring>, GBuildingDetailCount>
        DetailRows =
    {{
        { L"총 건물 수", FormatInteger(SelectedCategory.Count) },
        { L"전체 비중", FormatPercent(CategoryRatio * 100.0) },
        { L"대표 건물", FormatTopBuilding(SelectedCategory.TopBuildings, 0) },
        { L"차상위 건물", FormatTopBuilding(SelectedCategory.TopBuildings, 1) },
        { L"", L"" },
        { L"", L"" },
        { L"", L"" },
        { L"", L"" }
    }};
    std::array<FVector4, GBuildingDetailCount> DetailRowValueColors =
    {
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f),
        FVector4(0.31f, 0.27f, 0.21f, 1.f)
    };

    switch (SelectedCategory.Category)
    {
    case EBuildingCategory::Infrastructure:
        DetailRows[4] = { L"항만 시설", FormatInteger(Snapshot.HarborCount) };
        DetailRows[5] = {
            L"전력 생산",
            FormatInteger(Snapshot.TotalProducedPowerMW) + L" MW" };
        DetailRows[6] = {
            L"전력 수요",
            FormatInteger(Snapshot.TotalRequiredPowerMW) + L" MW" };
        DetailRows[7] = {
            L"단전 소비자",
            FormatInteger(Snapshot.DisconnectedConsumerCount) };
        break;
    case EBuildingCategory::FoodResource:
        DetailRows[4] = {
            L"식량 공급 건물",
            FormatInteger(Snapshot.FoodProviderCount) };
        DetailRows[5] = {
            L"활성 자원 종류",
            FormatInteger(ActiveResourceTypeCount) };
        DetailRows[6] = {
            L"총 자원 재고",
            FormatInteger(Snapshot.TotalResourceStock) };
        DetailRows[7] = {
            L"일일 수출",
            FormatCompactCurrency(Snapshot.DailyExportIncome) };
        break;
    case EBuildingCategory::Industry:
        DetailRows[4] = { L"일자리 수용량", FormatInteger(Snapshot.JobCapacity) };
        DetailRows[5] = {
            L"빈 일자리 건물",
            FormatInteger(Snapshot.WorkVacancyBuildingCount) };
        DetailRows[6] = {
            L"총 자원 재고",
            FormatInteger(Snapshot.TotalResourceStock) };
        DetailRows[7] = {
            L"일일 수출",
            FormatCompactCurrency(Snapshot.DailyExportIncome) };
        break;
    case EBuildingCategory::Housing:
        DetailRows[4] = {
            L"주거 수용 가구",
            FormatInteger(Snapshot.ResidentialCapacity) };
        DetailRows[5] = {
            L"입주 가구",
            FormatInteger(Snapshot.AssignedHomeCount) };
        DetailRows[6] = { L"무주택자", FormatInteger(Snapshot.HomelessCount) };
        DetailRows[7] = {
            L"빈집 건물",
            FormatInteger(Snapshot.ResidentialVacancyBuildingCount) };
        break;
    case EBuildingCategory::Entertainment:
        DetailRows[4] = {
            L"유흥 건물",
            FormatInteger(Snapshot.EntertainmentBuildingCount) };
        DetailRows[5] = {
            L"관광 건물",
            FormatInteger(Snapshot.TourismBuildingCount) };
        DetailRows[6] = { L"평균 유흥", FormatFixed1(Snapshot.AverageFun) };
        DetailRows[7] = { L"평균 음식", FormatFixed1(Snapshot.AverageFood) };
        break;
    case EBuildingCategory::MediaEducation:
        DetailRows[4] = {
            L"자유 영향 건물",
            FormatInteger(Snapshot.FreedomInfluenceBuildingCount) };
        DetailRows[5] = {
            L"고학력 시민",
            FormatInteger(EducatedCitizenCount) };
        DetailRows[6] = { L"평균 자유", FormatFixed1(Snapshot.AverageFreedom) };
        DetailRows[7] = { L"평균 보건", FormatFixed1(Snapshot.AverageHealth) };
        break;
    case EBuildingCategory::Tourism:
        DetailRows[4] = {
            L"관광 건물",
            FormatInteger(Snapshot.TourismBuildingCount) };
        DetailRows[5] = { L"항만 시설", FormatInteger(Snapshot.HarborCount) };
        DetailRows[6] = { L"평균 유흥", FormatFixed1(Snapshot.AverageFun) };
        DetailRows[7] = {
            L"일일 수출",
            FormatCompactCurrency(Snapshot.DailyExportIncome) };
        break;
    case EBuildingCategory::PublicService:
    default:
        DetailRows[4] = {
            L"치안 영향 건물",
            FormatInteger(Snapshot.SecurityInfluenceBuildingCount) };
        DetailRows[5] = {
            L"신앙 건물",
            FormatInteger(Snapshot.FaithBuildingCount) };
        DetailRows[6] = { L"평균 보건", FormatFixed1(Snapshot.AverageHealth) };
        DetailRows[7] = { L"평균 치안", FormatFixed1(Snapshot.AverageSecurity) };
        break;
    }

    if (!BottleneckWarningSummary.empty())
    {
        DetailRows[6] = {
            L"물류 병목 경고",
            BottleneckWarningSummary
        };
        DetailRowValueColors[6] = FVector4(0.80f, 0.24f, 0.18f, 1.f);
    }

    if (!OverflowWarningSummary.empty())
    {
        DetailRows[7] = {
            L"과잉 재고 경고",
            OverflowWarningSummary
        };
        DetailRowValueColors[7] = FVector4(0.82f, 0.52f, 0.16f, 1.f);
    }

    for (int Index = 0;
        Index < static_cast<int>(Widget.mBuildingDetails.size()) &&
            Index < GBuildingDetailCount;
        ++Index)
    {
        const bool HasLabel =
            !DetailRows[static_cast<size_t>(Index)].first.empty();

        SetDetailRowData(
            Widget.mBuildingDetails[static_cast<size_t>(Index)],
            HasLabel ?
                DetailRows[static_cast<size_t>(Index)].first :
                std::wstring(),
            HasLabel ?
                DetailRows[static_cast<size_t>(Index)].second :
                std::wstring(),
            false,
            DetailRowValueColors[static_cast<size_t>(Index)]);

        if (auto Background =
            Widget.mBuildingDetails[static_cast<size_t>(Index)].
                Background.lock())
        {
            Background->SetTint(1.f, 1.f, 1.f, HasLabel ? 0.88f : 0.f);
        }
        if (auto Label =
            Widget.mBuildingDetails[static_cast<size_t>(Index)].
                Label.lock())
        {
            Label->SetEnable(HasLabel);
        }
        if (auto Value =
            Widget.mBuildingDetails[static_cast<size_t>(Index)].
                Value.lock())
        {
            Value->SetEnable(HasLabel);
        }
    }
}

void FAlmanacRenderer::ApplyConflictPage(
        CAlmanacWidget& Widget,
        const AlmanacDataProvider::FAlmanacSnapshot& Snapshot,
        const AlmanacRendererCalc::FConflictPageComputedData& ComputedData)
    {
        auto ConflictHeadlineBackground = Widget.mConflictHeadlineBackground.lock();
        auto ConflictHeadlineText = Widget.mConflictHeadlineText.lock();
        FVector4 ConflictTint(0.82f, 0.92f, 0.76f, 0.98f);
        const bool HasActiveWorldCrisis = Snapshot.WorldCrisisStatus.Active;
        const bool HasRecentWorldCrisis =
            Snapshot.WorldCrisisStatus.Active ||
            Snapshot.WorldCrisisStatus.NotificationDays > 0;
        const bool HasRecentTaxEvent =
            Snapshot.TaxEventStatus.Active ||
            Snapshot.TaxEventStatus.NotificationDays > 0;

        if (Snapshot.RebelRiskScore >= 66.0)
            ConflictTint = FVector4(0.96f, 0.48f, 0.38f, 0.98f);
        else if (Snapshot.RebelRiskScore >= 33.0)
            ConflictTint = FVector4(0.96f, 0.78f, 0.28f, 0.98f);
        else if (HasActiveWorldCrisis)
            ConflictTint =
                Snapshot.WorldCrisisStatus.Type == EWorldCrisisType::FiscalEmergency ?
                    FVector4(0.94f, 0.54f, 0.40f, 0.98f) :
                    FVector4(0.94f, 0.70f, 0.30f, 0.98f);
        else if (Snapshot.TaxEventStatus.Active)
            ConflictTint =
                Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                    FVector4(0.94f, 0.54f, 0.40f, 0.98f) :
                    FVector4(0.94f, 0.76f, 0.32f, 0.98f);
        else if (ComputedData.ElectionWarningActive)
            ConflictTint = ComputedData.ElectionWarningTint;

        if (ConflictHeadlineBackground)
            ConflictHeadlineBackground->SetTint(ConflictTint);

        if (ConflictHeadlineText)
        {
            std::wstring Headline =
                L"반란 위험: " + Snapshot.RebelRiskLabel;

            if (HasActiveWorldCrisis)
            {
                Headline +=
                    L" / 월드 위기: " +
                    Snapshot.WorldCrisisStatus.Summary;
            }
            else if (HasRecentWorldCrisis &&
                !Snapshot.WorldCrisisStatus.Summary.empty())
            {
                Headline +=
                    L" / 최근 위기: " +
                    Snapshot.WorldCrisisStatus.Summary;
            }

            if (Snapshot.TaxEventStatus.Active)
            {
                Headline +=
                    L"\n파벌 경고: " +
                    Snapshot.TaxEventStatus.Summary;
                Headline +=
                    L"\n월드 효과: " +
                    ComputedData.TaxEventWorldEffectSummary;
            }
            else if (Snapshot.TaxEventStatus.NotificationDays > 0 &&
                !Snapshot.TaxEventStatus.Summary.empty())
            {
                Headline +=
                    L" / 최근 경고: " +
                    Snapshot.TaxEventStatus.Summary;
                Headline +=
                    L"\n정상화 상태: " +
                    ComputedData.TaxEventWorldEffectSummary;
            }

            if (ComputedData.ElectionWarningActive)
            {
                Headline +=
                    L"\n선거 경고: " +
                    ComputedData.ElectionWarningSummary;
            }

            Headline +=
                L"\n평균 자유 만족도 " + FormatFixed1(Snapshot.AverageFreedom) +
                L" / 평균 치안 만족도 " + FormatFixed1(Snapshot.AverageSecurity) +
                L" / 평균 음식 만족도 " + FormatFixed1(Snapshot.AverageFood) +
                L"\n계엄령 " +
                std::wstring(
                    Snapshot.MartialLawActive ? L"활성" : L"비활성") +
                L" / 평균 보건 만족도 " + FormatFixed1(Snapshot.AverageHealth);
            ConflictHeadlineText->SetText(Headline.c_str());
        }

        SetDetailRowData(
            Widget.mConflictDetails[0],
            HasRecentWorldCrisis ? L"월드 위기" : L"정치 사건",
            HasActiveWorldCrisis ?
                Snapshot.WorldCrisisStatus.Title :
                (HasRecentWorldCrisis ?
                    Snapshot.WorldCrisisStatus.Title :
                    (Snapshot.TaxEventStatus.Active ?
                        Snapshot.TaxEventStatus.Title :
                        (HasRecentTaxEvent ?
                            Snapshot.TaxEventStatus.Title :
                            std::wstring(L"없음")))),
            HasActiveWorldCrisis || Snapshot.TaxEventStatus.Active,
            HasActiveWorldCrisis ?
                (Snapshot.WorldCrisisStatus.Type == EWorldCrisisType::FiscalEmergency ?
                    FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                    FVector4(0.82f, 0.48f, 0.12f, 1.f)) :
                (Snapshot.TaxEventStatus.Active ?
                    (Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                        FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                        FVector4(0.82f, 0.48f, 0.12f, 1.f)) :
                    FVector4(0.20f, 0.56f, 0.20f, 1.f)));
        SetDetailRowData(
            Widget.mConflictDetails[1],
            HasActiveWorldCrisis ? L"위기 현황 / 연쇄" :
                (HasRecentWorldCrisis ? L"최근 위기 메모" :
                    (Snapshot.TaxEventStatus.Active ? L"파벌 경고 / 효과" :
                        (ComputedData.ElectionWarningActive ? L"선거 경고" : L"사건 메모"))),
            HasActiveWorldCrisis ?
                Snapshot.WorldCrisisStatus.Summary :
                (HasRecentWorldCrisis ?
                    Snapshot.WorldCrisisStatus.Summary :
                    (Snapshot.TaxEventStatus.Active ?
                        (Snapshot.TaxEventStatus.Summary +
                            L" / " +
                            ComputedData.TaxEventWorldEffectSummary) :
                        (ComputedData.ElectionWarningActive ?
                            ComputedData.ElectionWarningSummary :
                            (HasRecentTaxEvent ?
                                (Snapshot.TaxEventStatus.Summary +
                                    L" / " +
                                    ComputedData.TaxEventWorldEffectSummary) :
                                std::wstring(L"안정"))))),
            HasActiveWorldCrisis ||
                Snapshot.TaxEventStatus.Active ||
                ComputedData.ElectionWarningActive,
            HasActiveWorldCrisis ?
                FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                (Snapshot.TaxEventStatus.Active ?
                    FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                    (ComputedData.ElectionWarningActive ?
                        ComputedData.ElectionWarningTint :
                        FVector4(0.31f, 0.27f, 0.21f, 1.f))));
        SetDetailRowData(
            Widget.mConflictDetails[2],
            L"반란 위험 지수",
            FormatFixed1(Snapshot.RebelRiskScore),
            true,
            Snapshot.RebelRiskScore >= 66.0 ?
                FVector4(0.78f, 0.18f, 0.18f, 1.f) :
                (Snapshot.RebelRiskScore >= 33.0 ?
                    FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                    FVector4(0.20f, 0.56f, 0.20f, 1.f)));
        SetDetailRowData(
            Widget.mConflictDetails[3],
            L"평균 음식 만족도",
            FormatFixed1(Snapshot.AverageFood));
        SetDetailRowData(
            Widget.mConflictDetails[4],
            L"평균 보건 만족도",
            FormatFixed1(Snapshot.AverageHealth));
        SetDetailRowData(
            Widget.mConflictDetails[5],
            L"실업률",
            FormatPercent(ComputedData.UnemploymentRate * 100.0));
        SetDetailRowData(
            Widget.mConflictDetails[6],
            L"야권 지지도",
            FormatPercent(Snapshot.OppositionPercent));
        SetDetailRowData(
            Widget.mConflictDetails[7],
            L"재정 압박",
            FormatPercent(ComputedData.FiscalStress * 100.0));

        SetMetricRowData(
            Widget.mConflictMetrics[0],
            L"반란 위험",
            FormatPercent(Snapshot.RebelRiskScore),
            static_cast<float>(Clamp01(Snapshot.RebelRiskScore / 100.0)),
            FVector4(0.82f, 0.24f, 0.18f, 0.95f),
            true);
        SetMetricRowData(
            Widget.mConflictMetrics[1],
            L"체제 안정도",
            FormatPercent(ComputedData.Stability * 100.0),
            static_cast<float>(ComputedData.Stability),
            FVector4(0.18f, 0.66f, 0.32f, 0.95f));
        SetMetricRowData(
            Widget.mConflictMetrics[2],
            L"통제 강도",
            FormatPercent(ComputedData.ControlStrength * 100.0),
            static_cast<float>(ComputedData.ControlStrength),
            FVector4(0.24f, 0.52f, 0.88f, 0.95f));
    }
