#include "TaskWidget.h"
#include "TropicoUiAssetCatalog.h"
#include "TropicoUiStyle.h"
#include "UIEnumLabels.h"
#include "UIStringShorthand.h"
#include "UIStrings.h"
#include "../StringUtils.h"
#include "../World/GovernmentCommandService.h"
#include "../World/IWorldUIAccess.h"
#include "../World/MainWorldTradeRuntime.h"
#include "../World/MainWorldUiReadAccess.h"
#include "Device.h"
#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/TextBlock.h"
#include "World/World.h"
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace
{
    using namespace TropicoUiStyle;
    using UIStringShorthand::Ui;
    using UIStringShorthand::UiText;

    constexpr int GVisibleEntryCount = 6;
    constexpr const TCHAR* GPenultimoTaskIcon = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demands.png");
    constexpr const TCHAR* GTaskPanelTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Tasks\\T_tasks_bg.png");
    constexpr const TCHAR* GTaskTabTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Tasks\\T_task_icon_bg.png");
    constexpr const TCHAR* GTaskAnswerTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Tasks\\T_tasks_answer_standard.png");
    constexpr const TCHAR* GTaskAnswerHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Tasks\\T_tasks_answer_hover.png");
    constexpr const TCHAR* GTaskAnswerSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\1_Colonial\\Tasks\\T_tasks_answer_selected.png");
    constexpr const TCHAR* GTaskPaperTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_paperBg.png");
    constexpr const TCHAR* GTaskPaperHoverTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_paperBg_hover.png");
    constexpr const TCHAR* GTaskPaperSelectedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_paperBg_selected.png");
    constexpr const TCHAR* GTaskFrameDefaultTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_iconFrame_default.png");
    constexpr const TCHAR* GTaskFrameBlueTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_iconFrame_blue.png");
    constexpr const TCHAR* GTaskFrameGoldTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_iconFrame_gold.png");
    constexpr const TCHAR* GTaskFrameRedTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_iconFrame_red.png");
    constexpr const TCHAR* GTaskFrameUltimatumTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Tasks\\T_task_iconFrame_ultimatum.png");
    constexpr const TCHAR* GPortraitNameTexture = TEXT(
        "TROPICO_ASSET\\Visuals\\UI\\Base\\0_AllEras\\Backgrounds\\T_dialog_portrait_name_bg.png");

    struct FTaskEntry
    {
        enum class EKind
        {
            PoliticalDemand = 0,
            EraMission,
            EraTransition
        };

        EKind Kind = EKind::PoliticalDemand;
        FPoliticalDemandState Demand;
        EPoliticalDemandIssuerType IssuerType =
            EPoliticalDemandIssuerType::None;
        int IssuerIndex = -1;
        int PressureDays = 0;
        std::wstring IssuerLabel;
        std::wstring SpeakerLabel;
        std::wstring RowTitle;
        std::wstring RowSubtitle;
        std::wstring CounterText;
        std::wstring DetailBody;
        std::wstring ObjectiveLine;
        std::wstring StageLine;
        std::wstring RewardText;
        std::wstring PenaltyText;
        std::wstring PrimaryButtonLabel = Ui(L"task_widget.action.accept");
        std::wstring SecondaryButtonLabel =
            Ui(L"task_widget.action.decline");
        const TCHAR* IconPath = nullptr;
        const TCHAR* PortraitPath = nullptr;
    };

    int ClampInt(int Value, int MinValue, int MaxValue)
    {
        return (std::max)(MinValue, (std::min)(MaxValue, Value));
    }

    std::wstring Ellipsize(const std::wstring& Text, size_t MaxChars)
    {
        if (Text.size() <= MaxChars || MaxChars < 4)
            return Text;

        return Text.substr(0, MaxChars - 3) + L"...";
    }

    std::string BuildTextureKey(
        const std::string& BaseKey,
        const TCHAR* TexturePath)
    {
        if (!TexturePath)
            return BaseKey + "_none";

        return BaseKey + "_" +
            std::to_string(StringUtils::HashFnv1a64(TexturePath));
    }

    const wchar_t* GetFactionName(EPoliticalFaction Faction)
    {
        switch (Faction)
        {
        case EPoliticalFaction::Communists:
            return UiText(L"task_widget.faction.communists");
        case EPoliticalFaction::Capitalists:
            return UiText(L"task_widget.faction.capitalists");
        case EPoliticalFaction::Religious:
            return UiText(L"task_widget.faction.religious");
        case EPoliticalFaction::Militarists:
            return UiText(L"task_widget.faction.militarists");
        case EPoliticalFaction::Environmentalists:
            return UiText(L"task_widget.faction.environmentalists");
        case EPoliticalFaction::Industrialists:
            return UiText(L"task_widget.faction.industrialists");
        case EPoliticalFaction::Intellectuals:
            return UiText(L"task_widget.faction.intellectuals");
        case EPoliticalFaction::Conservatives:
            return UiText(L"task_widget.faction.conservatives");
        default:
            return UiText(L"task_widget.faction.generic");
        }
    }

    std::wstring GetStageLabel(EPoliticalDemandStage Stage)
    {
        return UIEnumLabels::GetPoliticalDemandStageLabel(Stage);
    }

    std::wstring FormatDemandValue(const FPoliticalDemandState& Demand, int Value)
    {
        switch (Demand.ObjectiveType)
        {
        case EPoliticalDemandObjectiveType::ExportIncome:
            return MainWorldTradeRuntime::FormatCurrency(Value);
        case EPoliticalDemandObjectiveType::IncomeTaxCeiling:
        case EPoliticalDemandObjectiveType::PropertyTaxCeiling:
            return std::to_wstring(Value) + L"%";
        default:
            return std::to_wstring(Value);
        }
    }

    std::wstring BuildCounterText(const FPoliticalDemandState& Demand)
    {
        if (!Demand.Active)
            return L"";

        return L"(" +
            FormatDemandValue(Demand, Demand.CurrentValue) +
            L"/" +
            FormatDemandValue(Demand, Demand.TargetValue) +
            L")";
    }
}

CTaskWidget::CTaskWidget()
{
}

CTaskWidget::~CTaskWidget()
{
}

namespace
{
    const TCHAR* GetFactionIconPath(int FactionIndex)
    {
        switch (FactionIndex)
        {
        case static_cast<int>(EPoliticalFaction::Communists):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_communists.png");
        case static_cast<int>(EPoliticalFaction::Capitalists):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_capitalists.png");
        case static_cast<int>(EPoliticalFaction::Religious):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_religious.png");
        case static_cast<int>(EPoliticalFaction::Militarists):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_militarists.png");
        case static_cast<int>(EPoliticalFaction::Environmentalists):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_environmentalists.png");
        case static_cast<int>(EPoliticalFaction::Industrialists):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_industrialists.png");
        case static_cast<int>(EPoliticalFaction::Intellectuals):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_intellectuals.png");
        case static_cast<int>(EPoliticalFaction::Conservatives):
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_conservatives.png");
        default:
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demands.png");
        }
    }

    const TCHAR* GetForeignIconPath(int IssuerIndex, EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_Crown.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\FactionIcons\\T_ICO_tasks_revolutionaries.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demandsDual.png");
            }
        case EBuildingEra::WorldWars:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_Allies.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_Axis.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demandsDual.png");
            }
        case EBuildingEra::ColdWar:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_WesternPowers.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_EasterBloc.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demandsDual.png");
            }
        case EBuildingEra::Modern:
        default:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_USA.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_Russia.png");
            case 2: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_China.png");
            case 3: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_EU.png");
            case 4: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\SuperpowerIcons\\T_ICO_tasks_MiddleEast.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demandsDual.png");
            }
        }
    }

    const TCHAR* GetFactionPortraitPath(int FactionIndex, EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::WorldWars:
            switch (FactionIndex)
            {
            case static_cast<int>(EPoliticalFaction::Communists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_portrait_WW_communists.png");
            case static_cast<int>(EPoliticalFaction::Capitalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_portrait_WW_capitalists.png");
            case static_cast<int>(EPoliticalFaction::Religious): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_WW_religious.png");
            case static_cast<int>(EPoliticalFaction::Militarists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_WW_militarists.png");
            case static_cast<int>(EPoliticalFaction::Environmentalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_portrait_WW_environmentalists.png");
            case static_cast<int>(EPoliticalFaction::Industrialists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_portrait_WW_industrialists.png");
            default: break;
            }
            break;
        case EBuildingEra::ColdWar:
            switch (FactionIndex)
            {
            case static_cast<int>(EPoliticalFaction::Communists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_communists.png");
            case static_cast<int>(EPoliticalFaction::Capitalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_capitalists.png");
            case static_cast<int>(EPoliticalFaction::Religious): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_religious.png");
            case static_cast<int>(EPoliticalFaction::Militarists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_militarists.png");
            case static_cast<int>(EPoliticalFaction::Environmentalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_environmentalists.png");
            case static_cast<int>(EPoliticalFaction::Industrialists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_industrialists.png");
            default: break;
            }
            break;
        case EBuildingEra::Modern:
        default:
            break;
        }

        switch (FactionIndex)
        {
        case static_cast<int>(EPoliticalFaction::Communists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_communists.png");
        case static_cast<int>(EPoliticalFaction::Capitalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_capitalists.png");
        case static_cast<int>(EPoliticalFaction::Religious): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_religious.png");
        case static_cast<int>(EPoliticalFaction::Militarists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_militarists.png");
        case static_cast<int>(EPoliticalFaction::Environmentalists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_environmentalists.png");
        case static_cast<int>(EPoliticalFaction::Industrialists): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_industrialists.png");
        case static_cast<int>(EPoliticalFaction::Intellectuals): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_intellectuals.png");
        case static_cast<int>(EPoliticalFaction::Conservatives): return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_conservatives.png");
        default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_broker.png");
        }
    }

    const TCHAR* GetForeignPortraitPath(int IssuerIndex, EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CE_theCrown.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CE_Revolutionary.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CE_broker.png");
            }
        case EBuildingEra::WorldWars:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_WW_allies.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_portrait_WW_axis.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_WW_broker.png");
            }
        case EBuildingEra::ColdWar:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_NATO.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_easternBloc.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_broker.png");
            }
        case EBuildingEra::Modern:
        default:
            switch (IssuerIndex)
            {
            case 0: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_us.png");
            case 1: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_russia.png");
            case 2: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_China.png");
            case 3: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_eu.png");
            case 4: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_middleEast.png");
            default: return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_broker.png");
            }
        }
    }

    const TCHAR* GetFrameTexture(const FPoliticalDemandState& Demand)
    {
        if (IsPoliticalDemandAccepted(Demand))
            return GTaskFrameBlueTexture;

        switch (Demand.Stage)
        {
        case EPoliticalDemandStage::Warning: return GTaskFrameGoldTexture;
        case EPoliticalDemandStage::Ultimatum: return GTaskFrameRedTexture;
        case EPoliticalDemandStage::Revolt: return GTaskFrameUltimatumTexture;
        default: return GTaskFrameDefaultTexture;
        }
    }

    const TCHAR* GetPenultimoPortraitPath(EBuildingEra Era)
    {
        switch (Era)
        {
        case EBuildingEra::Colonial:
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CE_broker.png");
        case EBuildingEra::WorldWars:
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_WW_broker.png");
        case EBuildingEra::ColdWar:
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_CW_broker.png");
        case EBuildingEra::Modern:
        default:
            return TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_broker.png");
        }
    }

    std::wstring BuildEraMissionObjectiveText(const FEraProgressState& EraProgress)
    {
        if (!EraProgress.HasNextEra)
            return Ui(L"task_widget.placeholder.none");

        const FEraUnlockRequirement& Req = EraProgress.NextRequirement;
        std::wstring Result;

        auto AddReq = [&](const wchar_t* LabelKey, int Current, int Required)
        {
            if (Required <= 0)
                return;
            const bool Met = Current >= Required;
            Result += UIStrings::Format(
                Met ?
                    L"task_widget.objective.completed_template" :
                    L"task_widget.objective.progress_template",
                {
                    Ui(LabelKey),
                    std::to_wstring(Current),
                    std::to_wstring(Required)
                });
            Result += L"\n";
        };

        AddReq(
            L"task_widget.objective.population",
            EraProgress.Population,
            Req.MinPopulation);
        AddReq(
            L"task_widget.objective.buildings",
            EraProgress.TotalBuildings,
            Req.MinTotalBuildings);
        AddReq(
            L"task_widget.objective.food_providers",
            EraProgress.FoodProviders,
            Req.MinFoodProviders);
        AddReq(
            L"task_widget.objective.industry_buildings",
            EraProgress.IndustryBuildings,
            Req.MinIndustryBuildings);
        AddReq(
            L"task_widget.objective.public_service_buildings",
            EraProgress.PublicServiceBuildings,
            Req.MinPublicServiceBuildings);
        AddReq(
            L"task_widget.objective.entertainment_buildings",
            EraProgress.EntertainmentBuildings,
            Req.MinEntertainmentBuildings);
        AddReq(
            L"task_widget.objective.power_mw",
            EraProgress.PowerMW,
            Req.MinPowerMW);

        if (!Result.empty() && Result.back() == L'\n')
            Result.pop_back();

        return Result;
    }

    std::wstring BuildListText(const std::wstring& Text)
    {
        std::wstring Result = StringUtils::Trim(Text);

        if (Result.empty())
            return Ui(L"task_widget.placeholder.none");

        StringUtils::ReplaceAll(Result, L" / ", L"\n");
        StringUtils::ReplaceAll(Result, L"\n", L"\n* ");
        Result = L"* " + Result;
        return Result;
    }

    std::wstring BuildDemandRowSubtitle(bool Accepted, int RemainingDays)
    {
        return UIStrings::Format(
            L"task_widget.demand.row_subtitle_template",
            {
                Ui(
                    Accepted ?
                        L"task_widget.demand.status.in_progress" :
                        L"task_widget.demand.status.awaiting_response"),
                std::to_wstring(RemainingDays)
            });
    }

    std::wstring BuildObjectiveLine(
        const std::wstring& ObjectiveText,
        const std::wstring& CounterText)
    {
        if (CounterText.empty())
            return ObjectiveText;

        if (ObjectiveText.empty())
            return CounterText;

        return ObjectiveText + L" " + CounterText;
    }

    std::vector<FTaskEntry> BuildEntries(const std::shared_ptr<CWorld>& World)
    {
        std::vector<FTaskEntry> Result;
        auto HudAccess = ResolveMainWorldHudAccess(World);
        auto AlmanacAccess = ResolveMainWorldAlmanacAccess(World);

        if (!HudAccess && !AlmanacAccess)
            return Result;

        const EBuildingEra Era =
            HudAccess ? HudAccess->GetCurrentEra() :
                AlmanacAccess->GetCurrentEra();
        const auto FactionPressure =
            HudAccess ? HudAccess->GetFactionDemandPressureDays() :
                std::array<int, GPoliticalFactionCount>();
        const auto FactionStates =
            HudAccess ? HudAccess->GetFactionDemandStates() :
                std::array<FPoliticalDemandState, GPoliticalFactionCount>();
        const auto ForeignStates =
            AlmanacAccess ? AlmanacAccess->GetForeignDemandStates() :
                std::array<FPoliticalDemandState, TradeDiplomacyRuntime::GForeignPowerCount>();
        const FEraProgressState EraProgress =
            HudAccess ? HudAccess->GetEraProgress() :
                FEraProgressState();
        const FEraTransitionState EraTransitionState =
            HudAccess ? HudAccess->GetEraTransitionState() :
                FEraTransitionState();

        if (EraProgress.HasNextEra && !EraProgress.NextEraReady)
        {
            FTaskEntry Entry;
            Entry.Kind = FTaskEntry::EKind::EraMission;
            Entry.RowTitle = UIStrings::Format(
                L"task_widget.era_mission.row_title_template",
                {
                    std::wstring(
                        GetBuildingEraDisplayName(EraProgress.CurrentEra)),
                    std::wstring(
                        GetBuildingEraDisplayName(EraProgress.NextEra))
                });
            Entry.RowSubtitle = Ui(L"task_widget.era_mission.row_subtitle");
            Entry.IssuerLabel = Ui(L"task_widget.penultimo.issuer_label");
            Entry.SpeakerLabel = Ui(L"task_widget.penultimo.name");
            Entry.DetailBody = UIStrings::Format(
                L"task_widget.era_mission.detail_body_template",
                { std::wstring(GetBuildingEraDisplayName(EraProgress.NextEra)) });
            Entry.ObjectiveLine = BuildEraMissionObjectiveText(EraProgress);
            Entry.StageLine = Ui(L"task_widget.era_mission.stage_line");
            Entry.RewardText = BuildListText(UIStrings::Format(
                L"task_widget.era_mission.reward_template",
                { std::wstring(GetBuildingEraDisplayName(EraProgress.NextEra)) }));
            Entry.PenaltyText = BuildListText(Ui(L"task_widget.value.none"));
            Entry.PrimaryButtonLabel = Ui(L"task_widget.action.close");
            Entry.SecondaryButtonLabel = Ui(L"task_widget.action.close");
            Entry.Demand.Stage = EPoliticalDemandStage::Warning;
            Entry.IconPath = GPenultimoTaskIcon;
            Entry.PortraitPath = GetPenultimoPortraitPath(Era);
            Result.push_back(std::move(Entry));
        }
        else if (EraTransitionState.Stage == EEraTransitionStage::Available &&
            EraTransitionState.CanStart)
        {
            FTaskEntry Entry;
            Entry.Kind = FTaskEntry::EKind::EraTransition;
            Entry.RowTitle =
                EraTransitionState.Title.empty() ?
                    Ui(L"task_widget.era_transition.default_title") :
                    EraTransitionState.Title;
            Entry.RowSubtitle = Ui(L"task_widget.era_transition.row_subtitle");
            Entry.IssuerLabel = Ui(L"task_widget.penultimo.issuer_label");
            Entry.SpeakerLabel = Ui(L"task_widget.penultimo.name");
            Entry.DetailBody = UIStrings::Format(
                L"task_widget.era_transition.detail_body_template",
                {
                    std::wstring(
                        GetBuildingEraDisplayName(EraProgress.CurrentEra)),
                    std::wstring(
                        GetBuildingEraDisplayName(
                            EraTransitionState.TargetEra))
                });
            if (!EraTransitionState.Summary.empty())
                Entry.DetailBody += L"\n\n" + EraTransitionState.Summary;
            const std::wstring ConfirmText =
                EraTransitionState.ConfirmText.empty() ?
                    Ui(L"task_widget.era_transition.confirm_default") :
                    EraTransitionState.ConfirmText;
            Entry.ObjectiveLine = UIStrings::Format(
                L"task_widget.era_transition.objective_template",
                { ConfirmText });
            Entry.StageLine = Ui(L"task_widget.era_transition.stage_line");
            Entry.RewardText = BuildListText(UIStrings::Format(
                L"task_widget.era_mission.reward_template",
                {
                    std::wstring(
                        GetBuildingEraDisplayName(
                            EraTransitionState.TargetEra))
                }));
            Entry.PenaltyText = BuildListText(
                Ui(L"task_widget.era_transition.penalty_text"));
            Entry.PrimaryButtonLabel =
                ConfirmText;
            Entry.SecondaryButtonLabel = Ui(L"top_hud.era_transition.cancel");
            Entry.Demand.Stage = EPoliticalDemandStage::Warning;
            Entry.IconPath = GPenultimoTaskIcon;
            Entry.PortraitPath = GetPenultimoPortraitPath(Era);
            Result.push_back(std::move(Entry));
        }

        for (int Index = 0; Index < GPoliticalFactionCount; ++Index)
        {
            const auto& Demand = FactionStates[static_cast<size_t>(Index)];
            if (!Demand.Active)
                continue;

            FTaskEntry Entry;
            Entry.Demand = Demand;
            Entry.IssuerType = EPoliticalDemandIssuerType::Faction;
            Entry.IssuerIndex = Index;
            Entry.PressureDays = FactionPressure[static_cast<size_t>(Index)];
            const std::wstring FactionName =
                GetFactionName(static_cast<EPoliticalFaction>(Index));
            Entry.IssuerLabel = UIStrings::Format(
                L"task_widget.demand.issuer_label_template",
                { FactionName });
            Entry.SpeakerLabel = UIStrings::Format(
                L"task_widget.demand.speaker_label_template",
                { FactionName });
            Entry.RowTitle =
                Demand.ObjectiveText.empty() ? Demand.Title : Demand.ObjectiveText;
            Entry.RowSubtitle = BuildDemandRowSubtitle(
                IsPoliticalDemandAccepted(Demand),
                Demand.RemainingDays);
            Entry.CounterText = BuildCounterText(Demand);
            Entry.DetailBody = UIStrings::Format(
                L"task_widget.demand.detail_body_template",
                { Entry.IssuerLabel });
            if (!Demand.Summary.empty())
                Entry.DetailBody += L"\n\n" + Demand.Summary;
            Entry.ObjectiveLine = BuildObjectiveLine(
                Demand.ObjectiveText.empty() ? Demand.Title : Demand.ObjectiveText,
                Entry.CounterText);
            Entry.StageLine = UIStrings::Format(
                L"task_widget.demand.stage_line_template",
                {
                    GetStageLabel(Demand.Stage),
                    std::to_wstring(Entry.PressureDays),
                    std::to_wstring(Demand.RemainingDays)
                });
            Entry.RewardText = BuildListText(Demand.RewardText);
            Entry.PenaltyText = BuildListText(Demand.PenaltyText);
            Entry.IconPath = GetFactionIconPath(Index);
            Entry.PortraitPath = GetFactionPortraitPath(Index, Era);
            Result.push_back(std::move(Entry));
        }

        for (int Index = 0; Index < TradeDiplomacyRuntime::GForeignPowerCount; ++Index)
        {
            const auto& Demand = ForeignStates[static_cast<size_t>(Index)];
            if (!Demand.Active)
                continue;

            FTaskEntry Entry;
            Entry.Demand = Demand;
            Entry.IssuerType = EPoliticalDemandIssuerType::ForeignPower;
            Entry.IssuerIndex = Index;
            const std::wstring ForeignPowerName =
                MainWorldTradeRuntime::GetForeignPowerName(Index, Era);
            Entry.IssuerLabel = UIStrings::Format(
                L"task_widget.foreign.issuer_label_template",
                { ForeignPowerName });
            Entry.SpeakerLabel = ForeignPowerName;
            Entry.RowTitle =
                Demand.ObjectiveText.empty() ? Demand.Title : Demand.ObjectiveText;
            Entry.RowSubtitle = BuildDemandRowSubtitle(
                IsPoliticalDemandAccepted(Demand),
                Demand.RemainingDays);
            Entry.CounterText = BuildCounterText(Demand);
            Entry.DetailBody = UIStrings::Format(
                L"task_widget.foreign.detail_body_template",
                { Entry.IssuerLabel });
            if (!Demand.Summary.empty())
                Entry.DetailBody += L"\n\n" + Demand.Summary;
            Entry.ObjectiveLine = BuildObjectiveLine(
                Demand.ObjectiveText.empty() ? Demand.Title : Demand.ObjectiveText,
                Entry.CounterText);
            Entry.StageLine = UIStrings::Format(
                L"task_widget.foreign.stage_line_template",
                { std::to_wstring(Demand.RemainingDays) });
            Entry.RewardText = BuildListText(Demand.RewardText);
            Entry.PenaltyText = BuildListText(Demand.PenaltyText);
            Entry.IconPath = GetForeignIconPath(Index, Era);
            Entry.PortraitPath = GetForeignPortraitPath(Index, Era);
            Result.push_back(std::move(Entry));
        }

        return Result;
    }

    int FindEraTransitionEntryIndex(
        const std::vector<FTaskEntry>& Entries)
    {
        for (int Index = 0; Index < static_cast<int>(Entries.size()); ++Index)
        {
            if (Entries[static_cast<size_t>(Index)].Kind ==
                FTaskEntry::EKind::EraMission ||
            Entries[static_cast<size_t>(Index)].Kind ==
                FTaskEntry::EKind::EraTransition)
            {
                return Index;
            }
        }

        return -1;
    }

    int FindEntryIndex(
        const std::vector<FTaskEntry>& Entries,
        EPoliticalDemandIssuerType IssuerType,
        int IssuerIndex)
    {
        for (int Index = 0; Index < static_cast<int>(Entries.size()); ++Index)
        {
            const FTaskEntry& Entry = Entries[static_cast<size_t>(Index)];

            if (Entry.IssuerType == IssuerType &&
                Entry.IssuerIndex == IssuerIndex)
            {
                return Index;
            }
        }

        return -1;
    }
}

bool CTaskWidget::Init()
{
    CWidgetContainer::Init();

    auto Panel = CreateWidget<CImage>("TaskWidget_Background", 6).lock();
    if (Panel)
    {
        Panel->SetTexture("TaskWidget_BackgroundTexture", GTaskPanelTexture);
        mPanelBackground = Panel;
    }

    auto Title = CreateWidget<CTextBlock>("TaskWidget_Title", 8).lock();
    if (Title)
    {
        Title->SetText(UiText(L"task_widget.title"));
        Title->SetAlignH(ETextAlignH::Center);
        Title->SetAlignV(ETextAlignV::Middle);
        Title->SetTextColor(98, 75, 36, 255);
        Title->EnableShadow(true);
        Title->SetShadowTextColor(244, 235, 212, 180);
        Title->SetShadowOffset(1.f, 1.f);
        mTitleText = Title;
    }

    auto Subtitle = CreateWidget<CTextBlock>("TaskWidget_Subtitle", 8).lock();
    if (Subtitle)
    {
        Subtitle->SetAlignH(ETextAlignH::Center);
        Subtitle->SetAlignV(ETextAlignV::Middle);
        Subtitle->SetTextColor(124, 104, 72, 255);
        mSubtitleText = Subtitle;
    }

    auto CloseButton = CreateWidget<CButton>("TaskWidget_CloseButton", 8).lock();
    if (CloseButton)
    {
        ApplyButtonTextureSet(
            CloseButton,
            "TaskWidget_CloseButton",
            TropicoUiAssets::GRoundButtonTexture,
            TropicoUiAssets::GRoundButtonHoverTexture,
            TropicoUiAssets::GRoundButtonSelectedTexture,
            TropicoUiAssets::GRoundButtonTexture);
        ConfigureIconSlotButtonStyle(CloseButton);
        CloseButton->SetEventCallback<CTaskWidget>(
            EButtonEventState::Click,
            this,
            &CTaskWidget::OnCloseButtonClick);
        auto CloseText =
            CWidget::CreateStaticWidget<CTextBlock>("TaskWidget_CloseText", mWorld);
        if (CloseText)
        {
            CloseText->SetText(L"X");
            CloseText->SetAlignH(ETextAlignH::Center);
            CloseText->SetAlignV(ETextAlignV::Middle);
            CloseText->SetTextColor(98, 72, 28, 255);
            CloseButton->SetChild(CloseText);
        }
        mCloseButton = CloseButton;
    }

    mIssuerTabs.resize(GVisibleEntryCount);
    mDemandRows.resize(GVisibleEntryCount);

    for (int Index = 0; Index < GVisibleEntryCount; ++Index)
    {
        auto Button = CreateWidget<CButton>(
            "TaskWidget_Tab_" + std::to_string(Index + 1), 8).lock();
        if (Button)
        {
            ApplyButtonTextureSet(
                Button,
                "TaskWidget_TabTex_" + std::to_string(Index + 1),
                GTaskTabTexture,
                GTaskTabTexture,
                GTaskTabTexture,
                GTaskTabTexture);
            Button->SetEventCallback(
                EButtonEventState::Click,
                [this, Index]() { OnIssuerTabClick(Index); });

            auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
                "TaskWidget_TabContent_" + std::to_string(Index + 1), mWorld);
            auto Frame = CWidget::CreateStaticWidget<CImage>(
                "TaskWidget_TabFrame_" + std::to_string(Index + 1), mWorld);
            auto Icon = CWidget::CreateStaticWidget<CImage>(
                "TaskWidget_TabIcon_" + std::to_string(Index + 1), mWorld);

            if (Content && Frame && Icon)
            {
                Frame->SetTexture(Frame->GetName() + "_Texture", GTaskFrameDefaultTexture);
                Icon->SetTexture(
                    Icon->GetName() + "_Texture",
                    TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demands.png"));
                Content->AddWidget(Frame);
                Content->AddWidget(Icon);
                Button->SetChild(Content);
                mIssuerTabs[static_cast<size_t>(Index)].Frame = Frame;
                mIssuerTabs[static_cast<size_t>(Index)].Icon = Icon;
            }

            mIssuerTabs[static_cast<size_t>(Index)].Button = Button;
        }

        auto RowButton = CreateWidget<CButton>(
            "TaskWidget_Row_" + std::to_string(Index + 1), 8).lock();
        if (RowButton)
        {
            ApplyButtonTextureSet(
                RowButton,
                "TaskWidget_RowTex_" + std::to_string(Index + 1),
                GTaskPaperTexture,
                GTaskPaperHoverTexture,
                GTaskPaperSelectedTexture,
                GTaskPaperTexture);
            RowButton->SetEventCallback(
                EButtonEventState::Click,
                [this, Index]() { OnDemandRowClick(Index); });

            auto Content = CWidget::CreateStaticWidget<CWidgetContainer>(
                "TaskWidget_RowContent_" + std::to_string(Index + 1), mWorld);
            auto Frame = CWidget::CreateStaticWidget<CImage>(
                "TaskWidget_RowFrame_" + std::to_string(Index + 1), mWorld);
            auto Icon = CWidget::CreateStaticWidget<CImage>(
                "TaskWidget_RowIcon_" + std::to_string(Index + 1), mWorld);
            auto TitleText = CWidget::CreateStaticWidget<CTextBlock>(
                "TaskWidget_RowTitle_" + std::to_string(Index + 1), mWorld);
            auto SubtitleText = CWidget::CreateStaticWidget<CTextBlock>(
                "TaskWidget_RowSubtitle_" + std::to_string(Index + 1), mWorld);
            auto CounterText = CWidget::CreateStaticWidget<CTextBlock>(
                "TaskWidget_RowCounter_" + std::to_string(Index + 1), mWorld);

            if (Content && Frame && Icon && TitleText && SubtitleText && CounterText)
            {
                Frame->SetTexture(Frame->GetName() + "_Texture", GTaskFrameDefaultTexture);
                Icon->SetTexture(
                    Icon->GetName() + "_Texture",
                    TEXT("TROPICO_ASSET\\Visuals\\UI\\Icons\\TaskIcons\\T_ICO_tasks_demands.png"));
                TitleText->SetAlignH(ETextAlignH::Left);
                TitleText->SetAlignV(ETextAlignV::Middle);
                TitleText->SetTextColor(78, 62, 31, 255);
                SubtitleText->SetAlignH(ETextAlignH::Left);
                SubtitleText->SetAlignV(ETextAlignV::Top);
                SubtitleText->SetTextColor(122, 106, 78, 255);
                CounterText->SetAlignH(ETextAlignH::Right);
                CounterText->SetAlignV(ETextAlignV::Middle);
                CounterText->SetTextColor(74, 64, 40, 255);
                Content->AddWidget(Frame);
                Content->AddWidget(Icon);
                Content->AddWidget(TitleText);
                Content->AddWidget(SubtitleText);
                Content->AddWidget(CounterText);
                RowButton->SetChild(Content);
                mDemandRows[static_cast<size_t>(Index)].IconFrame = Frame;
                mDemandRows[static_cast<size_t>(Index)].Icon = Icon;
                mDemandRows[static_cast<size_t>(Index)].Title = TitleText;
                mDemandRows[static_cast<size_t>(Index)].Subtitle = SubtitleText;
                mDemandRows[static_cast<size_t>(Index)].Counter = CounterText;
            }

            mDemandRows[static_cast<size_t>(Index)].Button = RowButton;
        }
    }

    auto CreateBodyText =
        [&](const std::string& Name, unsigned char R, unsigned char G, unsigned char B)
        {
            auto Text = CreateWidget<CTextBlock>(Name, 8).lock();
            if (Text)
            {
                Text->SetAlignH(ETextAlignH::Left);
                Text->SetAlignV(ETextAlignV::Top);
                Text->SetTextColor(R, G, B, 255);
            }
            return Text;
        };

    mEmptyText = CreateBodyText("TaskWidget_Empty", 114, 104, 84);
    mDetailTitleText = CreateBodyText("TaskWidget_DetailTitle", 96, 73, 32);
    mDetailMetaText = CreateBodyText("TaskWidget_DetailMeta", 125, 107, 74);
    mDetailBodyText = CreateBodyText("TaskWidget_DetailBody", 66, 61, 53);
    mObjectiveHeaderText = CreateBodyText("TaskWidget_ObjectiveHeader", 117, 89, 43);
    mObjectiveText = CreateBodyText("TaskWidget_ObjectiveText", 68, 62, 54);
    mRewardHeaderText = CreateBodyText("TaskWidget_RewardHeader", 117, 89, 43);
    mRewardText = CreateBodyText("TaskWidget_RewardText", 68, 62, 54);
    mPenaltyHeaderText = CreateBodyText("TaskWidget_PenaltyHeader", 117, 89, 43);
    mPenaltyText = CreateBodyText("TaskWidget_PenaltyText", 68, 62, 54);
    mFeedbackText = CreateBodyText("TaskWidget_Feedback", 93, 99, 71);

    if (auto EmptyText = mEmptyText.lock())
    {
        EmptyText->SetAlignH(ETextAlignH::Center);
        EmptyText->SetAlignV(ETextAlignV::Middle);
    }

    auto PortraitCard = CreateWidget<CImage>("TaskWidget_PortraitCard", 8).lock();
    if (PortraitCard)
    {
        PortraitCard->SetTexture("TaskWidget_PortraitCardTexture", TropicoUiAssets::GSlotCardTexture);
        PortraitCard->SetAngle(4.f);
        mPortraitCard = PortraitCard;
    }

    auto PortraitImage = CreateWidget<CImage>("TaskWidget_PortraitImage", 9).lock();
    if (PortraitImage)
    {
        PortraitImage->SetTexture(
            "TaskWidget_PortraitImageTexture",
            TEXT("TROPICO_ASSET\\Visuals\\UI\\Portraits\\T_potrait_MT_broker.png"));
        PortraitImage->SetAngle(4.f);
        mPortraitImage = PortraitImage;
    }

    auto PortraitNameBackdrop =
        CreateWidget<CImage>("TaskWidget_PortraitNameBackdrop", 8).lock();
    if (PortraitNameBackdrop)
    {
        PortraitNameBackdrop->SetTexture(
            "TaskWidget_PortraitNameBackdropTexture",
            GPortraitNameTexture);
        PortraitNameBackdrop->SetAngle(4.f);
        mPortraitNameBackdrop = PortraitNameBackdrop;
    }

    auto PortraitNameText =
        CreateWidget<CTextBlock>("TaskWidget_PortraitName", 9).lock();
    if (PortraitNameText)
    {
        PortraitNameText->SetAlignH(ETextAlignH::Center);
        PortraitNameText->SetAlignV(ETextAlignV::Middle);
        PortraitNameText->SetTextColor(98, 72, 28, 255);
        PortraitNameText->SetAngle(4.f);
        mPortraitNameText = PortraitNameText;
    }

    auto PrimaryButton = CreateWidget<CButton>("TaskWidget_PrimaryButton", 8).lock();
    if (PrimaryButton)
    {
        ApplyButtonTextureSet(
            PrimaryButton,
            "TaskWidget_PrimaryButton",
            GTaskAnswerTexture,
            GTaskAnswerHoverTexture,
            GTaskAnswerSelectedTexture,
            GTaskAnswerTexture);
        PrimaryButton->SetEventCallback<CTaskWidget>(
            EButtonEventState::Click, this, &CTaskWidget::OnPrimaryButtonClick);
        mPrimaryButton = PrimaryButton;
    }

    auto PrimaryText = CreateWidget<CTextBlock>("TaskWidget_PrimaryText", 9).lock();
    if (PrimaryText)
    {
        PrimaryText->SetAlignH(ETextAlignH::Center);
        PrimaryText->SetAlignV(ETextAlignV::Middle);
        PrimaryText->SetTextColor(98, 72, 28, 255);
        mPrimaryButtonText = PrimaryText;
    }

    auto SecondaryButton = CreateWidget<CButton>("TaskWidget_SecondaryButton", 8).lock();
    if (SecondaryButton)
    {
        ApplyButtonTextureSet(
            SecondaryButton,
            "TaskWidget_SecondaryButton",
            GTaskAnswerTexture,
            GTaskAnswerHoverTexture,
            GTaskAnswerSelectedTexture,
            GTaskAnswerTexture);
        SecondaryButton->SetEventCallback<CTaskWidget>(
            EButtonEventState::Click, this, &CTaskWidget::OnSecondaryButtonClick);
        mSecondaryButton = SecondaryButton;
    }

    auto SecondaryText =
        CreateWidget<CTextBlock>("TaskWidget_SecondaryText", 9).lock();
    if (SecondaryText)
    {
        SecondaryText->SetAlignH(ETextAlignH::Center);
        SecondaryText->SetAlignV(ETextAlignV::Middle);
        SecondaryText->SetTextColor(98, 72, 28, 255);
        mSecondaryButtonText = SecondaryText;
    }

    RefreshFromState();
    return true;
}

void CTaskWidget::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    if (!mOpen)
        return;

    const FResolution& Resolution = CDevice::GetInst()->GetResolution();

    if (mLastResolutionWidth != Resolution.Width ||
        mLastResolutionHeight != Resolution.Height)
    {
        RefreshLayout();
    }

    RefreshFromState();
}

void CTaskWidget::ToggleOpen()
{
    SetOpen(!mOpen);
}

void CTaskWidget::SetOpen(bool Open)
{
    if (mOpen == Open)
        return;

    mOpen = Open;

    if (mOpen)
    {
        mSelectedDemandIndex = 0;
        mFeedbackMessage.clear();
        RefreshLayout();
    }

    RefreshFromState();
}

void CTaskWidget::OpenEraTransitionTask()
{
    mFeedbackMessage.clear();
    const std::vector<FTaskEntry> Entries = BuildEntries(mWorld.lock());
    const int EntryIndex = FindEraTransitionEntryIndex(Entries);

    SetOpen(true);

    if (EntryIndex >= 0)
        mSelectedDemandIndex = EntryIndex;

    RefreshLayout();
    RefreshFromState();
}

void CTaskWidget::OpenForDemand(
    EPoliticalDemandIssuerType IssuerType,
    int IssuerIndex)
{
    mFeedbackMessage.clear();
    SetOpen(true);

    const int EntryIndex = FindEntryIndex(
        BuildEntries(mWorld.lock()),
        IssuerType,
        IssuerIndex);

    if (EntryIndex >= 0)
        mSelectedDemandIndex = EntryIndex;

    RefreshLayout();
    RefreshFromState();
}

void CTaskWidget::RefreshLayout()
{
    const FResolution& Resolution = CDevice::GetInst()->GetResolution();
    mLastResolutionWidth = Resolution.Width;
    mLastResolutionHeight = Resolution.Height;

    const float ScreenWidth = static_cast<float>(Resolution.Width);
    const float ScreenHeight = static_cast<float>(Resolution.Height);
    const float Scale = (std::min)(
        1.f,
        (std::min)(
            (ScreenWidth - 120.f) / mPanelWidth,
            (ScreenHeight - 110.f) / mPanelHeight));
    const float PanelWidth = mPanelWidth * Scale;
    const float PanelHeight = mPanelHeight * Scale;
    const float PanelLeft = (ScreenWidth - PanelWidth) * 0.5f;
    const float PanelTop = (ScreenHeight - PanelHeight) * 0.5f - 8.f * Scale;
    const float LeftWidth = 286.f * Scale;
    const float Left = PanelLeft + 36.f * Scale;
    const float DetailLeft = Left + LeftWidth + 28.f * Scale;
    const float DetailWidth = PanelWidth - (DetailLeft - PanelLeft) - 32.f * Scale;
    const float TextWidth = DetailWidth - 240.f * Scale;
    const float RowHeight = 74.f * Scale;
    const float RowGap = 12.f * Scale;
    const float RowTop = PanelTop + 136.f * Scale;
    const float FooterTop = PanelTop + PanelHeight - 94.f * Scale;

    if (auto Panel = mPanelBackground.lock())
    {
        Panel->SetPos(PanelLeft, PanelTop);
        Panel->SetSize(PanelWidth, PanelHeight);
    }
    if (auto Title = mTitleText.lock())
    {
        Title->SetPos(PanelLeft + 250.f * Scale, PanelTop + 34.f * Scale);
        Title->SetSize(PanelWidth - 500.f * Scale, 40.f * Scale);
        Title->SetFontSize(31.f * Scale);
    }
    if (auto Subtitle = mSubtitleText.lock())
    {
        Subtitle->SetPos(PanelLeft + 180.f * Scale, PanelTop + 78.f * Scale);
        Subtitle->SetSize(PanelWidth - 360.f * Scale, 26.f * Scale);
        Subtitle->SetFontSize(15.f * Scale);
    }
    if (auto Close = mCloseButton.lock())
    {
        Close->SetPos(PanelLeft + PanelWidth - 48.f * Scale, PanelTop + 8.f * Scale);
        Close->SetSize(36.f * Scale, 36.f * Scale);
    }

    for (int Index = 0; Index < static_cast<int>(mIssuerTabs.size()); ++Index)
    {
        auto Button = mIssuerTabs[static_cast<size_t>(Index)].Button.lock();
        auto Frame = mIssuerTabs[static_cast<size_t>(Index)].Frame.lock();
        auto Icon = mIssuerTabs[static_cast<size_t>(Index)].Icon.lock();
        const float LeftPos = PanelLeft + 292.f * Scale + Index * 76.f * Scale;
        if (Button)
        {
            Button->SetPos(LeftPos, PanelTop - 14.f * Scale);
            Button->SetSize(66.f * Scale, 66.f * Scale);
        }
        if (Frame)
        {
            Frame->SetPos(3.f * Scale, 3.f * Scale);
            Frame->SetSize(60.f * Scale, 60.f * Scale);
        }
        if (Icon)
        {
            Icon->SetPos(14.f * Scale, 14.f * Scale);
            Icon->SetSize(38.f * Scale, 38.f * Scale);
        }
    }

    for (int Index = 0; Index < static_cast<int>(mDemandRows.size()); ++Index)
    {
        auto Button = mDemandRows[static_cast<size_t>(Index)].Button.lock();
        auto Frame = mDemandRows[static_cast<size_t>(Index)].IconFrame.lock();
        auto Icon = mDemandRows[static_cast<size_t>(Index)].Icon.lock();
        auto Title = mDemandRows[static_cast<size_t>(Index)].Title.lock();
        auto Subtitle = mDemandRows[static_cast<size_t>(Index)].Subtitle.lock();
        auto Counter = mDemandRows[static_cast<size_t>(Index)].Counter.lock();
        const float Top = RowTop + Index * (RowHeight + RowGap);
        if (Button)
        {
            Button->SetPos(Left, Top);
            Button->SetSize(LeftWidth, RowHeight);
        }
        if (Frame) { Frame->SetPos(10.f * Scale, 10.f * Scale); Frame->SetSize(54.f * Scale, 54.f * Scale); }
        if (Icon) { Icon->SetPos(22.f * Scale, 22.f * Scale); Icon->SetSize(30.f * Scale, 30.f * Scale); }
        if (Title) { Title->SetPos(72.f * Scale, 10.f * Scale); Title->SetSize(LeftWidth - 150.f * Scale, 28.f * Scale); Title->SetFontSize(18.f * Scale); }
        if (Subtitle) { Subtitle->SetPos(72.f * Scale, 38.f * Scale); Subtitle->SetSize(LeftWidth - 150.f * Scale, 20.f * Scale); Subtitle->SetFontSize(12.f * Scale); }
        if (Counter) { Counter->SetPos(LeftWidth - 98.f * Scale, 18.f * Scale); Counter->SetSize(86.f * Scale, 30.f * Scale); Counter->SetFontSize(18.f * Scale); }
    }

    auto SetTextBox =
        [&](const WText& Text, float X, float Y, float W, float H, float Font)
        {
            if (auto Pinned = Text.lock())
            {
                Pinned->SetPos(X, Y);
                Pinned->SetSize(W, H);
                Pinned->SetFontSize(Font * Scale);
            }
        };

    SetTextBox(mEmptyText, DetailLeft, PanelTop + 220.f * Scale, DetailWidth, 120.f * Scale, 20.f);
    SetTextBox(mDetailTitleText, DetailLeft, PanelTop + 136.f * Scale, TextWidth, 36.f * Scale, 28.f);
    SetTextBox(mDetailMetaText, DetailLeft, PanelTop + 178.f * Scale, TextWidth, 24.f * Scale, 14.f);
    SetTextBox(mDetailBodyText, DetailLeft, PanelTop + 216.f * Scale, TextWidth, 208.f * Scale, 17.f);
    SetTextBox(mObjectiveHeaderText, DetailLeft, PanelTop + 438.f * Scale, TextWidth, 24.f * Scale, 18.f);
    SetTextBox(mObjectiveText, DetailLeft, PanelTop + 468.f * Scale, TextWidth, 52.f * Scale, 16.f);
    SetTextBox(mRewardHeaderText, DetailLeft, PanelTop + 530.f * Scale, TextWidth, 24.f * Scale, 18.f);
    SetTextBox(mRewardText, DetailLeft, PanelTop + 560.f * Scale, TextWidth, 70.f * Scale, 16.f);
    SetTextBox(mPenaltyHeaderText, DetailLeft, PanelTop + 632.f * Scale, TextWidth, 24.f * Scale, 18.f);
    SetTextBox(mPenaltyText, DetailLeft, PanelTop + 662.f * Scale, TextWidth, 48.f * Scale, 16.f);
    SetTextBox(mFeedbackText, DetailLeft, FooterTop - 34.f * Scale, DetailWidth - 24.f * Scale, 24.f * Scale, 14.f);

    if (auto Card = mPortraitCard.lock())
    {
        Card->SetPos(PanelLeft + PanelWidth - 226.f * Scale, PanelTop + 150.f * Scale);
        Card->SetSize(206.f * Scale, 282.f * Scale);
    }
    if (auto Image = mPortraitImage.lock())
    {
        Image->SetPos(PanelLeft + PanelWidth - 208.f * Scale, PanelTop + 172.f * Scale);
        Image->SetSize(170.f * Scale, 220.f * Scale);
    }
    if (auto Backdrop = mPortraitNameBackdrop.lock())
    {
        Backdrop->SetPos(PanelLeft + PanelWidth - 212.f * Scale, PanelTop + 376.f * Scale);
        Backdrop->SetSize(182.f * Scale, 42.f * Scale);
    }
    if (auto Name = mPortraitNameText.lock())
    {
        Name->SetPos(PanelLeft + PanelWidth - 206.f * Scale, PanelTop + 382.f * Scale);
        Name->SetSize(170.f * Scale, 28.f * Scale);
        Name->SetFontSize(16.f * Scale);
    }

    const float FooterWidth = DetailWidth - 24.f * Scale;
    const float HalfWidth = (FooterWidth - 16.f * Scale) * 0.5f;
    if (auto Primary = mPrimaryButton.lock())
    {
        Primary->SetPos(DetailLeft, FooterTop);
        Primary->SetSize(mSelectedDemandAccepted ? 0.f : HalfWidth, mSelectedDemandAccepted ? 0.f : 42.f * Scale);
    }
    if (auto PrimaryText = mPrimaryButtonText.lock())
    {
        PrimaryText->SetPos(DetailLeft, FooterTop);
        PrimaryText->SetSize(mSelectedDemandAccepted ? 0.f : HalfWidth, mSelectedDemandAccepted ? 0.f : 42.f * Scale);
        PrimaryText->SetFontSize(20.f * Scale);
    }
    if (auto Secondary = mSecondaryButton.lock())
    {
        Secondary->SetPos(
            mSelectedDemandAccepted ? DetailLeft : DetailLeft + HalfWidth + 16.f * Scale,
            FooterTop);
        Secondary->SetSize(
            mSelectedDemandAccepted ? FooterWidth : HalfWidth,
            42.f * Scale);
    }
    if (auto SecondaryText = mSecondaryButtonText.lock())
    {
        SecondaryText->SetPos(
            mSelectedDemandAccepted ? DetailLeft : DetailLeft + HalfWidth + 16.f * Scale,
            FooterTop);
        SecondaryText->SetSize(
            mSelectedDemandAccepted ? FooterWidth : HalfWidth,
            42.f * Scale);
        SecondaryText->SetFontSize(20.f * Scale);
    }
}

void CTaskWidget::RefreshFromState()
{
    for (const auto& Child : mChildList)
    {
        if (Child)
            Child->SetEnable(mOpen);
    }

    if (!mOpen)
        return;

    const std::vector<FTaskEntry> Entries = BuildEntries(mWorld.lock());
    mHasSelectedDemand = !Entries.empty();

    if (auto Subtitle = mSubtitleText.lock())
        Subtitle->SetText(
            UiText(L"task_widget.subtitle"));

    if (!mHasSelectedDemand)
    {
        mSelectedDemandAccepted = false;
        if (auto Empty = mEmptyText.lock())
        {
            Empty->SetEnable(true);
            Empty->SetText(UiText(L"task_widget.empty"));
        }
        if (auto Detail = mDetailTitleText.lock()) Detail->SetEnable(false);
        if (auto Detail = mDetailMetaText.lock()) Detail->SetEnable(false);
        if (auto Detail = mDetailBodyText.lock()) Detail->SetEnable(false);
        if (auto Detail = mObjectiveHeaderText.lock()) Detail->SetEnable(false);
        if (auto Detail = mObjectiveText.lock()) Detail->SetEnable(false);
        if (auto Detail = mRewardHeaderText.lock()) Detail->SetEnable(false);
        if (auto Detail = mRewardText.lock()) Detail->SetEnable(false);
        if (auto Detail = mPenaltyHeaderText.lock()) Detail->SetEnable(false);
        if (auto Detail = mPenaltyText.lock()) Detail->SetEnable(false);
        if (auto Portrait = mPortraitCard.lock()) Portrait->SetEnable(false);
        if (auto Portrait = mPortraitImage.lock()) Portrait->SetEnable(false);
        if (auto Portrait = mPortraitNameBackdrop.lock()) Portrait->SetEnable(false);
        if (auto Portrait = mPortraitNameText.lock()) Portrait->SetEnable(false);
        if (auto Feedback = mFeedbackText.lock())
            Feedback->SetEnable(false);
        if (auto Primary = mPrimaryButton.lock())
            Primary->SetEnable(false);
        if (auto PrimaryText = mPrimaryButtonText.lock())
            PrimaryText->SetEnable(false);
        if (auto Secondary = mSecondaryButton.lock())
            Secondary->SetEnable(false);
        if (auto SecondaryText = mSecondaryButtonText.lock())
            SecondaryText->SetEnable(false);

        for (size_t Index = 0; Index < mIssuerTabs.size(); ++Index)
        {
            if (auto Button = mIssuerTabs[Index].Button.lock())
                Button->SetEnable(false);
        }
        for (size_t Index = 0; Index < mDemandRows.size(); ++Index)
        {
            if (auto Button = mDemandRows[Index].Button.lock())
                Button->SetEnable(false);
        }
        return;
    }

    mSelectedDemandIndex = ClampInt(
        mSelectedDemandIndex,
        0,
        static_cast<int>(Entries.size()) - 1);
    const FTaskEntry& Selected = Entries[static_cast<size_t>(mSelectedDemandIndex)];
    mSelectedDemandAccepted = IsPoliticalDemandAccepted(Selected.Demand);
    RefreshLayout();

    if (auto Empty = mEmptyText.lock())
        Empty->SetEnable(false);

    for (int Index = 0; Index < static_cast<int>(mIssuerTabs.size()); ++Index)
    {
        const bool Enabled = Index < static_cast<int>(Entries.size());
        auto Button = mIssuerTabs[static_cast<size_t>(Index)].Button.lock();
        auto Frame = mIssuerTabs[static_cast<size_t>(Index)].Frame.lock();
        auto Icon = mIssuerTabs[static_cast<size_t>(Index)].Icon.lock();
        if (Button)
            Button->SetEnable(Enabled);
        if (!Enabled)
            continue;

        if (Frame)
        {
            const TCHAR* FrameTexture =
                GetFrameTexture(Entries[static_cast<size_t>(Index)].Demand);
            Frame->SetTexture(
                BuildTextureKey(Frame->GetName() + "_Texture", FrameTexture),
                FrameTexture);
        }
        if (Icon)
        {
            const TCHAR* IconTexture = Entries[static_cast<size_t>(Index)].IconPath;
            Icon->SetTexture(
                BuildTextureKey(Icon->GetName() + "_Texture", IconTexture),
                IconTexture);
        }
    }

    for (int Index = 0; Index < static_cast<int>(mDemandRows.size()); ++Index)
    {
        const bool Enabled = Index < static_cast<int>(Entries.size());
        auto Button = mDemandRows[static_cast<size_t>(Index)].Button.lock();
        auto Frame = mDemandRows[static_cast<size_t>(Index)].IconFrame.lock();
        auto Icon = mDemandRows[static_cast<size_t>(Index)].Icon.lock();
        auto Title = mDemandRows[static_cast<size_t>(Index)].Title.lock();
        auto Subtitle = mDemandRows[static_cast<size_t>(Index)].Subtitle.lock();
        auto Counter = mDemandRows[static_cast<size_t>(Index)].Counter.lock();

        if (Button)
        {
            Button->SetEnable(Enabled);
            if (Enabled)
            {
                ApplyButtonTextureSet(
                    Button,
                    "TaskWidget_RowTex_" + std::to_string(Index + 1) +
                        (Index == mSelectedDemandIndex ? "_selected" : "_idle"),
                    Index == mSelectedDemandIndex ? GTaskPaperSelectedTexture : GTaskPaperTexture,
                    Index == mSelectedDemandIndex ? GTaskPaperSelectedTexture : GTaskPaperHoverTexture,
                    GTaskPaperSelectedTexture,
                    GTaskPaperTexture);
            }
        }
        if (!Enabled)
            continue;

        const FTaskEntry& Entry = Entries[static_cast<size_t>(Index)];
        if (Frame)
        {
            const TCHAR* FrameTexture = GetFrameTexture(Entry.Demand);
            Frame->SetTexture(
                BuildTextureKey(Frame->GetName() + "_Texture", FrameTexture),
                FrameTexture);
        }
        if (Icon)
        {
            const TCHAR* IconTexture = Entry.IconPath;
            Icon->SetTexture(
                BuildTextureKey(Icon->GetName() + "_Texture", IconTexture),
                IconTexture);
        }
        if (Title)
            Title->SetText(Ellipsize(Entry.RowTitle, 18).c_str());
        if (Subtitle)
            Subtitle->SetText(Entry.RowSubtitle.c_str());
        if (Counter)
            Counter->SetText(Entry.CounterText.c_str());
    }

    auto SetText =
        [](const WText& Text, const std::wstring& Value)
        {
            if (auto Pinned = Text.lock())
            {
                Pinned->SetEnable(true);
                Pinned->SetText(Value.c_str());
            }
        };

    SetText(mDetailTitleText, Selected.IssuerLabel);
    SetText(mDetailMetaText, Selected.StageLine);
    SetText(mDetailBodyText, Selected.DetailBody);
    SetText(
        mObjectiveHeaderText,
        Selected.Kind == FTaskEntry::EKind::EraTransition ?
            Ui(L"task_widget.detail.header.approval") :
        Selected.Kind == FTaskEntry::EKind::EraMission ?
            Ui(L"task_widget.detail.header.conditions") :
            Ui(L"task_widget.detail.header.objective"));
    SetText(mObjectiveText, Selected.ObjectiveLine);
    SetText(
        mRewardHeaderText,
        Selected.Kind == FTaskEntry::EKind::PoliticalDemand ?
            Ui(L"task_widget.detail.header.reward") :
            Ui(L"task_widget.detail.header.effect"));
    SetText(mRewardText, Selected.RewardText);
    SetText(
        mPenaltyHeaderText,
        Selected.Kind == FTaskEntry::EKind::PoliticalDemand ?
            (mSelectedDemandAccepted ?
                Ui(L"task_widget.detail.header.penalty_abandon") :
                Ui(L"task_widget.detail.header.penalty_reject_fail")) :
            Ui(L"task_widget.detail.header.defer"));
    SetText(mPenaltyText, Selected.PenaltyText);
    SetText(mPortraitNameText, Selected.SpeakerLabel);

    if (auto PortraitCard = mPortraitCard.lock())
        PortraitCard->SetEnable(true);
    if (auto PortraitImage = mPortraitImage.lock())
    {
        PortraitImage->SetEnable(true);
        PortraitImage->SetTexture(
            BuildTextureKey("TaskWidget_PortraitImage", Selected.PortraitPath),
            Selected.PortraitPath);
    }
    if (auto PortraitNameBackdrop = mPortraitNameBackdrop.lock())
        PortraitNameBackdrop->SetEnable(true);

    const bool ShowPrimary =
        Selected.Kind == FTaskEntry::EKind::EraMission ?
            false :
            !mSelectedDemandAccepted;
    if (auto Primary = mPrimaryButton.lock())
        Primary->SetEnable(ShowPrimary);
    if (auto PrimaryText = mPrimaryButtonText.lock())
    {
        PrimaryText->SetEnable(ShowPrimary);
        PrimaryText->SetText(Selected.PrimaryButtonLabel.c_str());
    }
    if (auto Secondary = mSecondaryButton.lock())
        Secondary->SetEnable(true);
    if (auto SecondaryText = mSecondaryButtonText.lock())
    {
        std::wstring ButtonLabel = Selected.SecondaryButtonLabel;

        if (Selected.Kind == FTaskEntry::EKind::PoliticalDemand &&
            mSelectedDemandAccepted)
        {
            const std::wstring PenaltyText =
                Ellipsize(StringUtils::Trim(Selected.Demand.PenaltyText), 22);
            ButtonLabel =
                PenaltyText.empty() ?
                    Ui(L"task_widget.action.abandon") :
                    UIStrings::Format(
                        L"task_widget.action.abandon_with_penalty_template",
                        { PenaltyText });
        }

        SecondaryText->SetEnable(true);
        SecondaryText->SetText(ButtonLabel.c_str());
    }

    if (auto Feedback = mFeedbackText.lock())
    {
        Feedback->SetEnable(!mFeedbackMessage.empty());
        Feedback->SetText(mFeedbackMessage.c_str());
    }
}

void CTaskWidget::OnCloseButtonClick()
{
    SetOpen(false);
}

void CTaskWidget::OnIssuerTabClick(int Index)
{
    mSelectedDemandIndex = ClampInt(Index, 0, GVisibleEntryCount - 1);
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTaskWidget::OnDemandRowClick(int Index)
{
    mSelectedDemandIndex = ClampInt(Index, 0, GVisibleEntryCount - 1);
    mFeedbackMessage.clear();
    RefreshFromState();
}

void CTaskWidget::OnPrimaryButtonClick()
{
    const std::vector<FTaskEntry> Entries = BuildEntries(mWorld.lock());

    if (Entries.empty())
        return;

    const FTaskEntry& Selected = Entries[static_cast<size_t>(ClampInt(
        mSelectedDemandIndex,
        0,
        static_cast<int>(Entries.size()) - 1))];

    if (Selected.Kind == FTaskEntry::EKind::EraMission)
    {
        SetOpen(false);
        return;
    }

    if (Selected.Kind == FTaskEntry::EKind::EraTransition)
    {
        auto* Access = ResolveWorldUIAccess(mWorld.lock().get());

        if (!Access)
        {
            mFeedbackMessage =
                Ui(L"task_widget.feedback.no_era_transition_command");
            RefreshFromState();
            return;
        }

        if (Access->Commands().TryExecuteEraTransition(
                EEraTransitionChoice::Confirm))
        {
            mFeedbackMessage.clear();
            SetOpen(false);
            return;
        }

        mFeedbackMessage = Ui(L"task_widget.feedback.era_transition_unavailable");
        RefreshFromState();
        return;
    }

    auto CommandService = ResolveGovernmentCommandService(mWorld.lock());

    if (!CommandService)
    {
        mFeedbackMessage = Ui(L"task_widget.feedback.no_admin_command");
        RefreshFromState();
        return;
    }

    std::wstring ResponseMessage;
    CommandService->RespondPoliticalDemand(
        Selected.IssuerType,
        Selected.IssuerIndex,
        true,
        ResponseMessage);
    mFeedbackMessage =
        ResponseMessage.empty() ?
            Ui(L"task_widget.feedback.accepted") :
            ResponseMessage;
    RefreshFromState();
}

void CTaskWidget::OnSecondaryButtonClick()
{
    const std::vector<FTaskEntry> Entries = BuildEntries(mWorld.lock());

    if (Entries.empty())
        return;

    const FTaskEntry& Selected = Entries[static_cast<size_t>(ClampInt(
        mSelectedDemandIndex,
        0,
        static_cast<int>(Entries.size()) - 1))];

    if (Selected.Kind == FTaskEntry::EKind::EraMission ||
        Selected.Kind == FTaskEntry::EKind::EraTransition)
    {
        mFeedbackMessage.clear();
        SetOpen(false);
        return;
    }

    auto CommandService = ResolveGovernmentCommandService(mWorld.lock());

    if (!CommandService)
    {
        mFeedbackMessage = Ui(L"task_widget.feedback.no_admin_command");
        RefreshFromState();
        return;
    }

    std::wstring ResponseMessage;
    CommandService->RespondPoliticalDemand(
        Selected.IssuerType,
        Selected.IssuerIndex,
        false,
        ResponseMessage);
    mFeedbackMessage =
        ResponseMessage.empty() ?
            Ui(L"task_widget.feedback.cleared") :
            ResponseMessage;
    RefreshFromState();
}
