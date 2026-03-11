#include "AlmanacRenderer.h"
#include "AlmanacPageData.h"
#include "AlmanacRendererCalc.h"
#include "AlmanacRendererInternal.h"

void FAlmanacRenderer::ApplyPoliticsPage(CAlmanacWidget& Widget)
    {
        const AlmanacPageData::FDataSet& Data = AlmanacPageData::Get();
        const auto& PoliticsFactions = Data.PoliticsFactions;
        const int SelectedPoliticsFactionIndex =
            (std::max)(0,
                (std::min)(
                    static_cast<int>(PoliticsFactions.size()) - 1,
                    Widget.mSelectedPoliticsFactionIndex));
        const AlmanacPageData::FPoliticsFactionEntry& SelectedFaction =
            PoliticsFactions[static_cast<size_t>(SelectedPoliticsFactionIndex)];

        for (int Index = 0; Index < static_cast<int>(PoliticsFactions.size()); ++Index)
        {
            SetPoliticsFactionTileData(
                Widget.mPoliticsFactionTiles[Index],
                GPoliticsFactionIcons[Index],
                PoliticsFactions[Index].IconTint,
                PoliticsFactions[Index].Label,
                PoliticsFactions[Index].Count,
                PoliticsFactions[Index].Favor,
                Index == SelectedPoliticsFactionIndex);
        }

        for (int Index = 0; Index < GPoliticsNeutralCount; ++Index)
        {
            if (Index >= static_cast<int>(Widget.mPoliticsNeutralTexts.size()))
                break;

            if (auto Text = Widget.mPoliticsNeutralTexts[Index].lock())
            {
                Text->SetText(
                    (std::wstring(L"무관심\n") +
                        std::to_wstring(
                            Data.PoliticsNeutralCounts[static_cast<size_t>(Index)])).c_str());
            }
        }

        if (auto Title = Widget.mPoliticsFactionTitle.lock())
            Title->SetText(SelectedFaction.Label.c_str());
        if (auto Label = Widget.mPoliticsFactionApprovalLabel.lock())
        {
            Label->SetText(
                (std::wstring(SelectedFaction.Label) + L" 우호도").c_str());
        }
        if (auto Value = Widget.mPoliticsFactionApprovalValue.lock())
            Value->SetText(std::to_wstring(SelectedFaction.Favor).c_str());

        if (auto Title = Widget.mPoliticsSupportTitle.lock())
            Title->SetText(L"지지율");

        for (int Index = 0;
            Index < static_cast<int>(Widget.mPoliticsSupportRows.size()) &&
            Index < static_cast<int>(Data.PoliticsSupportRows.size());
            ++Index)
        {
            SetDetailRowData(
                Widget.mPoliticsSupportRows[Index],
                Data.PoliticsSupportRows[static_cast<size_t>(Index)].Label,
                Data.PoliticsSupportRows[static_cast<size_t>(Index)].Value);
        }

        for (const auto& Row : Widget.mPoliticsSupportRows)
        {
            if (auto Background = Row.Background.lock())
                Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }

        if (auto Text = Widget.mPoliticsElectionText.lock())
            Text->SetText(Data.PoliticsElectionText.c_str());

        SetDetailRowData(Widget.mPoliticsDetails[0], L"▽ 세력 지도자", L"");
        SetDetailRowData(Widget.mPoliticsDetails[1], SelectedFaction.Leader, L"");
        SetDetailRowData(
            Widget.mPoliticsDetails[2],
            (std::wstring(L"▽ ") + SelectedFaction.Label + L"의 수"),
            std::to_wstring(SelectedFaction.Count));
        SetDetailRowData(
            Widget.mPoliticsDetails[3],
            std::wstring(L"보통 ") + SelectedFaction.Label,
            std::to_wstring(SelectedFaction.ModerateCount));
        SetDetailRowData(
            Widget.mPoliticsDetails[4],
            std::wstring(L"강한 ") + SelectedFaction.Label,
            std::to_wstring(SelectedFaction.StrongCount));
        SetDetailRowData(
            Widget.mPoliticsDetails[5],
            std::wstring(L"열렬한 ") + SelectedFaction.Label,
            std::to_wstring(SelectedFaction.FerventCount));
        SetDetailRowData(Widget.mPoliticsDetails[6], L"▽ 우호도 수정치", L"");
        SetDetailRowData(
            Widget.mPoliticsDetails[7],
            L"건물 수정치",
            std::to_wstring(SelectedFaction.BuildingModifierTotal),
            false,
            SelectedFaction.BuildingModifierTotal > 0 ?
                FVector4(0.18f, 0.62f, 0.34f, 1.f) :
            SelectedFaction.BuildingModifierTotal < 0 ?
                FVector4(0.80f, 0.22f, 0.18f, 1.f) :
                FVector4(0.31f, 0.27f, 0.21f, 1.f));

        for (int Index = 0; Index < static_cast<int>(SelectedFaction.Modifiers.size()); ++Index)
        {
            const int DetailIndex = 8 + Index;
            const int ModifierValue =
                SelectedFaction.Modifiers[static_cast<size_t>(Index)].Value;
            const std::wstring ModifierText =
                ModifierValue > 0 ?
                    L"+" + std::to_wstring(ModifierValue) :
                    std::to_wstring(ModifierValue);

            SetDetailRowData(
                Widget.mPoliticsDetails[DetailIndex],
                SelectedFaction.Modifiers[static_cast<size_t>(Index)].Label,
                ModifierText,
                false,
                FVector4(0.31f, 0.27f, 0.21f, 1.f));
        }

        const int PoliticsHeaderRows[3] = { 0, 2, 6 };
        for (int HeaderIndex : PoliticsHeaderRows)
        {
            if (HeaderIndex >= static_cast<int>(Widget.mPoliticsDetails.size()))
                continue;

            if (auto Background =
                    Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].Background.lock())
            {
                Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
            }
            if (auto Label =
                    Widget.mPoliticsDetails[static_cast<size_t>(HeaderIndex)].Label.lock())
            {
                Label->SetTextColor(112, 86, 38, 255);
            }
        }

        for (int Index = 0; Index < static_cast<int>(Widget.mPoliticsDetails.size()); ++Index)
        {
            if (auto Background = Widget.mPoliticsDetails[Index].Background.lock())
            {
                if (Index != 0 && Index != 2 && Index != 6)
                    Background->SetTint(1.f, 1.f, 1.f, 0.88f);
            }
        }

        if (auto Label = Widget.mPoliticsDetails[1].Label.lock())
        {
            Label->SetFontSize(18.f);
            Label->SetTextColor(86, 70, 44, 255);
        }
    }

void FAlmanacRenderer::ApplyForeignPage(CAlmanacWidget& Widget)
    {
        const AlmanacPageData::FDataSet& Data = AlmanacPageData::Get();
        const auto& ForeignPowers = Data.ForeignPowers;

        const int SelectedForeignPowerIndex =
            (std::max)(0,
                (std::min)(
                    static_cast<int>(ForeignPowers.size()) - 1,
                    Widget.mSelectedForeignPowerIndex));
        const AlmanacPageData::FForeignPowerEntry& SelectedForeignPower =
            ForeignPowers[static_cast<size_t>(SelectedForeignPowerIndex)];

        for (int Index = 0; Index < static_cast<int>(ForeignPowers.size()); ++Index)
        {
            SetForeignPowerRowData(
                Widget.mForeignRows[Index],
                GForeignPowerIcons[Index],
                ForeignPowers[Index].Name,
                ForeignPowers[Index].DisplayValue,
                static_cast<float>(Clamp01(ForeignPowers[Index].Relation / 100.0)),
                Index == SelectedForeignPowerIndex);
        }

        if (auto Title = Widget.mForeignTitle.lock())
            Title->SetText(SelectedForeignPower.Name.c_str());
        if (auto Text = Widget.mForeignStatusLabel.lock())
            Text->SetText(L"현황");
        if (auto Text = Widget.mForeignStatusValue.lock())
            Text->SetText(SelectedForeignPower.Status.c_str());

        const std::wstring RelationLabel =
            std::wstring(SelectedForeignPower.Name) + L"(와)과의 관계";
        SetDetailRowData(
            Widget.mForeignDetails[0],
            RelationLabel,
            std::to_wstring(SelectedForeignPower.Relation));
        SetDetailRowData(Widget.mForeignDetails[1], L"경제 원조",
            std::to_wstring(SelectedForeignPower.EconomicAid));
        SetDetailRowData(Widget.mForeignDetails[2], L"관계 수정치", L"");
        SetDetailRowData(Widget.mForeignDetails[3], L"건물 수정치",
            std::to_wstring(SelectedForeignPower.BuildingModifier));
        SetDetailRowData(
            Widget.mForeignDetails[4],
            SelectedForeignPower.BuildingLines[0].Label,
            SelectedForeignPower.BuildingLines[0].Value > 0 ?
                L"+" + std::to_wstring(SelectedForeignPower.BuildingLines[0].Value) :
                std::to_wstring(SelectedForeignPower.BuildingLines[0].Value));
        SetDetailRowData(Widget.mForeignDetails[5], L"칙령 수정치",
            std::to_wstring(SelectedForeignPower.EdictModifier));
        SetDetailRowData(
            Widget.mForeignDetails[6],
            SelectedForeignPower.EdictLines[0].Label,
            SelectedForeignPower.EdictLines[0].Value > 0 ?
                L"+" + std::to_wstring(SelectedForeignPower.EdictLines[0].Value) :
                std::to_wstring(SelectedForeignPower.EdictLines[0].Value));
        SetDetailRowData(Widget.mForeignDetails[7], L"헌법 수정치",
            std::to_wstring(SelectedForeignPower.ConstitutionModifier));
        SetDetailRowData(Widget.mForeignDetails[8], L"기타 수정치",
            std::to_wstring(SelectedForeignPower.MiscModifier));
        SetDetailRowData(
            Widget.mForeignDetails[9],
            SelectedForeignPower.MiscLines[0].Label,
            SelectedForeignPower.MiscLines[0].Value > 0 ?
                L"+" + std::to_wstring(SelectedForeignPower.MiscLines[0].Value) :
                std::to_wstring(SelectedForeignPower.MiscLines[0].Value));
        SetDetailRowData(Widget.mForeignDetails[10], L"무역 수정치",
            std::to_wstring(SelectedForeignPower.TradeModifier));

        if (auto Background = Widget.mForeignDetails[2].Background.lock())
            Background->SetTint(0.98f, 0.95f, 0.84f, 0.94f);
        if (auto Label = Widget.mForeignDetails[2].Label.lock())
            Label->SetTextColor(112, 86, 38, 255);

        for (int Index = 0; Index < static_cast<int>(Widget.mForeignDetails.size()); ++Index)
        {
            if (Index == 2)
                continue;

            if (auto Background = Widget.mForeignDetails[Index].Background.lock())
                Background->SetTint(1.f, 1.f, 1.f, 0.88f);
        }

        if (auto Notice = Widget.mForeignNotice.lock())
        {
            Notice->SetEnable(false);
            Notice->SetText(L"");
        }
    }

void FAlmanacRenderer::ApplyBuildingPage(CAlmanacWidget& Widget)
    {
        const AlmanacPageData::FDataSet& Data = AlmanacPageData::Get();
        const auto& BuildingCategories = Data.BuildingCategories;

        const int SelectedBuildingCategoryIndex =
            (std::max)(0,
                (std::min)(
                    static_cast<int>(BuildingCategories.size()) - 1,
                    Widget.mSelectedBuildingCategoryIndex));
        const AlmanacPageData::FBuildingCategoryEntry& SelectedBuildingCategory =
            BuildingCategories[static_cast<size_t>(SelectedBuildingCategoryIndex)];

        for (int Index = 0; Index < static_cast<int>(BuildingCategories.size()); ++Index)
        {
            SetDetailRowData(
                Widget.mBuildingRows[Index],
                BuildingCategories[Index].Label,
                std::to_wstring(BuildingCategories[Index].Count),
                Index == SelectedBuildingCategoryIndex);
        }

        if (auto Title = Widget.mBuildingCategoryTitle.lock())
            Title->SetText(SelectedBuildingCategory.Label.c_str());

        for (int Index = 0; Index < static_cast<int>(SelectedBuildingCategory.Details.size()); ++Index)
        {
            const AlmanacPageData::FModifierEntry& Detail =
                SelectedBuildingCategory.Details[static_cast<size_t>(Index)];
            const bool HasLabel =
                !Detail.Label.empty();

            SetDetailRowData(
                Widget.mBuildingDetails[Index],
                HasLabel ? Detail.Label : L"",
                HasLabel ? std::to_wstring(Detail.Value) : L"",
                false);

            if (auto Background = Widget.mBuildingDetails[Index].Background.lock())
                Background->SetTint(1.f, 1.f, 1.f, HasLabel ? 0.88f : 0.f);
            if (auto Label = Widget.mBuildingDetails[Index].Label.lock())
                Label->SetEnable(HasLabel);
            if (auto Value = Widget.mBuildingDetails[Index].Value.lock())
                Value->SetEnable(HasLabel);
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
        const bool HasRecentTaxEvent =
            Snapshot.TaxEventStatus.Active ||
            Snapshot.TaxEventStatus.NotificationDays > 0;

        if (Snapshot.RebelRiskScore >= 66.0)
            ConflictTint = FVector4(0.96f, 0.48f, 0.38f, 0.98f);
        else if (Snapshot.RebelRiskScore >= 33.0)
            ConflictTint = FVector4(0.96f, 0.78f, 0.28f, 0.98f);
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

            if (Snapshot.TaxEventStatus.Active)
            {
                Headline +=
                    L" / 파벌 경고: " +
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
            L"정치 사건",
            Snapshot.TaxEventStatus.Active ?
                Snapshot.TaxEventStatus.Title :
                (HasRecentTaxEvent ?
                    Snapshot.TaxEventStatus.Title :
                    std::wstring(L"없음")),
            Snapshot.TaxEventStatus.Active,
            Snapshot.TaxEventStatus.Active ?
                (Snapshot.TaxEventStatus.Type == ETaxPolicyEventType::BudgetCrisis ?
                    FVector4(0.82f, 0.24f, 0.18f, 1.f) :
                    FVector4(0.82f, 0.48f, 0.12f, 1.f)) :
                FVector4(0.20f, 0.56f, 0.20f, 1.f));
        SetDetailRowData(
            Widget.mConflictDetails[1],
            Snapshot.TaxEventStatus.Active ? L"파벌 경고 / 효과" :
                (ComputedData.ElectionWarningActive ? L"선거 경고" : L"사건 메모"),
            Snapshot.TaxEventStatus.Active ?
                (Snapshot.TaxEventStatus.Summary +
                    L" / " +
                    ComputedData.TaxEventWorldEffectSummary) :
                (ComputedData.ElectionWarningActive ?
                    ComputedData.ElectionWarningSummary :
                    (HasRecentTaxEvent ?
                        (Snapshot.TaxEventStatus.Summary +
                            L" / " +
                            ComputedData.TaxEventWorldEffectSummary) :
                        std::wstring(L"안정"))),
            Snapshot.TaxEventStatus.Active || ComputedData.ElectionWarningActive,
            Snapshot.TaxEventStatus.Active ?
                FVector4(0.82f, 0.48f, 0.12f, 1.f) :
                (ComputedData.ElectionWarningActive ?
                    ComputedData.ElectionWarningTint :
                    FVector4(0.31f, 0.27f, 0.21f, 1.f)));
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
