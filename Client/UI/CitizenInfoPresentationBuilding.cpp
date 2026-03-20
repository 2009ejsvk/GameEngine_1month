#include "CitizenInfoPresentationInternal.h"
#include "CitizenInfoConstants.h"
#include "../Building/BuildingCatalog.h"

using namespace CitizenInfoPresentationInternal;

namespace CitizenInfoPresentation
{
    namespace
    {
        constexpr size_t GProfileTabTitleCount = 5;
        using FProfileTabTitles =
            std::array<const wchar_t*, GProfileTabTitleCount>;
        const std::array<FProfileTabTitles, 8> GProfileTabTitles =
        {{
            {{
                nullptr,
                L"거주 상태",
                L"업그레이드",
                L"주거 효율",
                L"거주 설명"
            }},
            {{
                nullptr,
                L"무역 상태",
                L"무역 설정",
                L"수출 효율",
                L"세관 설명"
            }},
            {{
                nullptr,
                L"물류 상태",
                L"물류 업그레이드",
                L"물류 효율",
                L"물류 설명"
            }},
            {{
                nullptr,
                L"전력 상태",
                L"설비 업그레이드",
                L"전력 효율",
                L"전력 설명"
            }},
            {{
                nullptr,
                L"관광 상태",
                L"업그레이드",
                L"관광 효율",
                L"관광 설명"
            }},
            {{
                nullptr,
                L"서비스 상태",
                L"업그레이드",
                L"서비스 효율",
                L"서비스 설명"
            }},
            {{
                nullptr,
                L"생산 상태",
                L"업그레이드",
                L"생산 효율",
                L"생산 설명"
            }},
            {{
                nullptr,
                nullptr,
                L"업그레이드",
                nullptr,
                nullptr
            }}
        }};
    }

    bool HasStableProductionProfileIdentity(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (Snapshot.CanGenerateWorkOutput ||
            Snapshot.ProducedResourceType != EResourceType::None ||
            HasProductionInputRecords(Snapshot) ||
            Snapshot.ChainStage != EProductionChainStage::None ||
            !Snapshot.ProductionChainStageText.empty())
        {
            return true;
        }

        if (!Snapshot.CatalogEntry)
            return false;

        if (Snapshot.CatalogEntry->ProducedResourceType != EResourceType::None ||
            Snapshot.CatalogEntry->UsesRecipeTable ||
            Snapshot.CatalogEntry->ProductionChainStage !=
                EProductionChainStage::None)
        {
            return true;
        }

        for (int SlotIndex = 0;
            SlotIndex < GProductionInputSlotCount;
            ++SlotIndex)
        {
            if (Snapshot.CatalogEntry->ProductionInputTypes[
                    static_cast<size_t>(SlotIndex)] != EResourceType::None &&
                Snapshot.CatalogEntry->ProductionInputAmounts[
                    static_cast<size_t>(SlotIndex)] > 0)
            {
                return true;
            }
        }

        return false;
    }

    bool UseGenericBuildingWorkOverview(const FBuildingUiSnapshot& Snapshot)
    {
        return !Snapshot.Residential &&
            Snapshot.WorkProvider &&
            !Snapshot.IsRoad;
    }

    EBuildingUiProfile ResolveBuildingUiProfileInternal(
        const FBuildingUiSnapshot& Snapshot)
    {
        if (CitizenInfoBuildingRuntime::IsCustomsOfficeBuilding(Snapshot))
            return EBuildingUiProfile::Customs;

        if (Snapshot.Residential)
            return EBuildingUiProfile::Residential;

        if (Snapshot.Harbor || Snapshot.Warehouse)
            return EBuildingUiProfile::Logistics;

        const bool ProducesPower =
            Snapshot.ProducedPowerMW > 0 ||
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->BaseProducedPowerMW > 0);
        const bool UsesPower =
            Snapshot.RequiredPowerMW > 0 ||
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->BaseRequiredPowerMW > 0);
        const bool TourismIdentity =
            (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->Category == EBuildingCategory::Tourism) ||
            Snapshot.TourismArrivalCount > 0 ||
            !ResolveWorkerOverviewPreferredType(Snapshot).empty();

        if (ProducesPower)
            return EBuildingUiProfile::Power;

        if (TourismIdentity)
            return EBuildingUiProfile::Tourism;

        if (Snapshot.FoodProvider ||
            Snapshot.EntertainmentProvider ||
            Snapshot.HealthProvider ||
            Snapshot.FaithProvider)
        {
            return EBuildingUiProfile::Service;
        }

        if (HasStableProductionProfileIdentity(Snapshot))
            return EBuildingUiProfile::Production;

        if (UsesPower)
            return EBuildingUiProfile::Power;

        return EBuildingUiProfile::Generic;
    }

    std::wstring ResolveProfileTabTitle(
        EBuildingUiProfile Profile,
        int TabIndex,
        bool ShowCustomsModeSelection)
    {
        if (TabIndex == 0 && ShowCustomsModeSelection)
            return L"운영 모드";

        const size_t ProfileIndex = static_cast<size_t>(Profile);
        const size_t SafeTabIndex = static_cast<size_t>(TabIndex);

        if (ProfileIndex < GProfileTabTitles.size() &&
            SafeTabIndex < GProfileTabTitleCount)
        {
            if (const wchar_t* const Title =
                GProfileTabTitles[ProfileIndex][SafeTabIndex])
            {
                return Title;
            }
        }

        return CitizenInfoConstants::GetBuildingPageTitle(TabIndex);
    }

    std::wstring ResolveBuildingPageTitle(
        const FBuildingUiSnapshot& Snapshot,
        int TabIndex,
        bool ShowCustomsModeSelection)
    {
        return ResolveProfileTabTitle(
            ResolveBuildingUiProfileInternal(Snapshot),
            TabIndex,
            ShowCustomsModeSelection);
    }

    bool PopulateOverviewStageBadge(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        const EProductionChainStage Stage =
            ResolveProductionChainStage(Snapshot);
        const std::wstring BadgeText =
            ResolveProductionChainStageBadgeText(Stage);

        if (BadgeText.empty())
            return false;

        Result.ShowBuildingSubtitle = true;
        Result.BuildingSubtitleText = BadgeText;
        Result.BuildingSubtitleColor =
            ResolveProductionChainStageBadgeColor(Stage);
        return true;
    }

    bool PopulateBuildingUpgradeCard(
        const FBuildingUiSnapshot& Snapshot,
        CitizenInfoDataProvider::FCitizenInfoSnapshot& Result)
    {
        Result.UpgradeCardTitle.clear();
        Result.UpgradeCardDescription.clear();
        Result.UpgradeCardIconPath = nullptr;
        Result.UpgradeCardIconTextureKey.clear();

        if (const FBuildingRuntimeUpgradeDef* const ActiveUpgradeDef =
            ResolveActiveRuntimeUpgradeDef(Snapshot))
        {
            Result.UpgradeCardTitle = ActiveUpgradeDef->DisplayName;
            Result.UpgradeCardDescription =
                !Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                    Snapshot.ActiveRuntimeUpgradeEffectSummary :
                    BuildRuntimeUpgradeSummary(
                        *ActiveUpgradeDef,
                        Snapshot.ActiveRuntimeUpgradeEffectSummary.empty() ?
                            nullptr :
                            &Snapshot.ActiveRuntimeUpgradeEffectSummary);
            return !Result.UpgradeCardTitle.empty();
        }

        if (!Snapshot.ActiveOperationModeText.empty())
        {
            Result.UpgradeCardTitle = L"현재 근무 형태";
            Result.UpgradeCardDescription = Snapshot.ActiveOperationModeText;

            if (!Snapshot.ActiveOperationModeEffectSummary.empty())
            {
                Result.UpgradeCardDescription += L"\n";
                Result.UpgradeCardDescription +=
                    Snapshot.ActiveOperationModeEffectSummary;
            }

            if (Snapshot.CatalogEntry)
            {
                const std::wstring TransitionNotice =
                    GetOperationModeTransitionNotice(*Snapshot.CatalogEntry);

                if (!TransitionNotice.empty())
                {
                    Result.UpgradeCardDescription += L"\n";
                    Result.UpgradeCardDescription += TransitionNotice;
                }
            }

            return true;
        }

        const EBuildingUiProfile Profile =
            ResolveBuildingUiProfileInternal(Snapshot);

        switch (Profile)
        {
        case EBuildingUiProfile::Residential:
            if (Snapshot.CatalogEntry &&
                Snapshot.CatalogEntry->HousingClass !=
                    EBuildingHousingClass::None)
            {
                Result.UpgradeCardTitle = L"주거 등급";
                Result.UpgradeCardDescription =
                    GetHousingClassDisplayName(
                        Snapshot.CatalogEntry->HousingClass);
            }
            break;
        case EBuildingUiProfile::Logistics:
            if (!Snapshot.WarehousePolicySelectionText.empty() ||
                !Snapshot.WarehousePrioritySelectionText.empty() ||
                !Snapshot.HarborExportSelectionText.empty())
            {
                Result.UpgradeCardTitle = L"물류 정책";
                if (!Snapshot.WarehousePolicySelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"창고 정책: " +
                            Snapshot.WarehousePolicySelectionText);
                }
                if (!Snapshot.WarehousePrioritySelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"정렬 우선순위: " +
                            Snapshot.WarehousePrioritySelectionText);
                }
                if (!Snapshot.HarborExportSelectionText.empty())
                {
                    AppendLine(
                        Result.UpgradeCardDescription,
                        L"수출 차단: " + Snapshot.HarborExportSelectionText);
                }
            }
            break;
        case EBuildingUiProfile::Power:
        {
            const std::wstring ProducedPowerText =
                ResolveProducedPowerDisplayText(Snapshot);
            Result.UpgradeCardTitle = L"전력 요약";
            Result.UpgradeCardDescription =
                Ui(L"citizen_info.label.produced_power") + L": " +
                (ProducedPowerText.empty() ?
                    L"-" :
                    ProducedPowerText) +
                L"\n" +
                Ui(L"citizen_info.label.power_grid_status") + L": " +
                FormatSignedMegawattValue(
                    Snapshot.TotalProducedPowerMW -
                    Snapshot.TotalRequiredPowerMW);
            break;
        }
        case EBuildingUiProfile::Tourism:
            if (!ResolveWorkerOverviewPreferredType(Snapshot).empty())
            {
                Result.UpgradeCardTitle = L"주요 고객";
                Result.UpgradeCardDescription =
                    ResolveWorkerOverviewPreferredType(Snapshot);
            }
            break;
        case EBuildingUiProfile::Service:
            if (!Snapshot.ServiceQualityText.empty())
            {
                Result.UpgradeCardTitle = L"서비스 품질";
                Result.UpgradeCardDescription = Snapshot.ServiceQualityText;
            }
            break;
        case EBuildingUiProfile::Production:
        {
            const std::wstring ProducedResourceDisplayName =
                ResolveProducedResourceDisplayName(Snapshot);
            if (!ProducedResourceDisplayName.empty())
            {
                Result.UpgradeCardTitle = L"주요 생산품";
                Result.UpgradeCardDescription = ProducedResourceDisplayName;
            }
            break;
        }
        case EBuildingUiProfile::Customs:
        case EBuildingUiProfile::Generic:
        default:
            break;
        }

        return !Result.UpgradeCardTitle.empty();
    }
}
