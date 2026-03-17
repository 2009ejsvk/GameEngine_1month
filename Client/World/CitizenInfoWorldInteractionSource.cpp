#include "CitizenInfoWorldInteractionSource.h"
#include "../UI/TradeWidget.h"
#include "../Building/BuildingCatalog.h"
#include "../Map/PlacementAreaObject.h"
#include "../Map/PlacementController.h"
#include "../ObjectNames.h"
#include "../Player/MainCamera.h"
#include "../World/GovernmentCommandService.h"
#include "MainWorldBuildingControlAccess.h"
#include "World/World.h"

namespace
{
    using EBuildingInfoTab = CitizenInfoConstants::EBuildingInfoTab;

    class CWorldCitizenInfoInteractionSource final :
        public CitizenInfoInteractionService::ICitizenInfoInteractionSource
    {
    public:
        explicit CWorldCitizenInfoInteractionSource(
            const std::shared_ptr<CWorld>& World)
            : mWorld(World)
        {
        }

        virtual bool IsCustomsOfficeBuilding(
            const std::string& BuildingName) const override
        {
            const auto Building = ResolveBuilding(BuildingName);
            return Building &&
                IsCustomsOfficeBuildingId(Building->GetBuildingId());
        }

        virtual bool SetBuildingBudgetLevel(
            const std::string& BuildingName,
            int Level) override
        {
            const auto Building = ResolveBuilding(BuildingName);

            if (!Building)
                return false;

            Building->SetBudgetLevel(Level);
            return true;
        }

        virtual bool SetBuildingOperationMode(
            const std::string& BuildingName,
            int ModeIndex,
            std::wstring& OutMessage) override
        {
            const auto Building = ResolveBuilding(BuildingName);

            if (!Building)
                return false;

            return Building->SetActiveOperationMode(ModeIndex, OutMessage);
        }

        virtual bool DemolishBuilding(
            const std::string& BuildingName) override
        {
            const auto Controller = ResolvePlacementController();
            return Controller &&
                Controller->DemolishBuildingByName(BuildingName);
        }

        virtual bool BeginMoveBuilding(
            const std::string& BuildingName) override
        {
            const auto Controller = ResolvePlacementController();
            return Controller &&
                Controller->BeginMoveExistingBuilding(BuildingName);
        }

        virtual bool FocusBuilding(
            const std::string& BuildingName) override
        {
            const auto Building = ResolveBuilding(BuildingName);
            const auto MainCamera = ResolveMainCamera();

            if (!Building || !MainCamera)
                return false;

            const FVector3 BuildingWorldPos = Building->GetWorldPos();
            const FVector3 CameraWorldPos = MainCamera->GetWorldPos();

            MainCamera->SetWorldPos(
                BuildingWorldPos.x,
                BuildingWorldPos.y,
                CameraWorldPos.z);
            return true;
        }

        virtual bool ExecuteBuildingCommand(
            const std::string& BuildingName,
            EBuildingInfoTab Tab,
            std::wstring& OutMessage) override
        {
            const auto Building = ResolveBuilding(BuildingName);

            if (!Building)
                return false;

            if (Tab == EBuildingInfoTab::Statistics &&
                Building->GetDamageLevel() != EBuildingDamageLevel::None)
            {
                const auto BuildingConditionAccess =
                    ResolveBuildingConditionAccess();
                return BuildingConditionAccess &&
                    BuildingConditionAccess->TryRepairBuilding(
                        BuildingName,
                        OutMessage);
            }

            if (!Building->IsHarbor())
            {
                switch (Tab)
                {
                case EBuildingInfoTab::Overview:
                    if (Building->HasOperationModes())
                        return Building->CycleOperationMode(OutMessage);

                    if (Building->HasWarehouseStorageControls())
                    {
                        return Building->CycleWarehouseStoragePolicy(
                            OutMessage);
                    }

                    return false;
                case EBuildingInfoTab::Upgrades:
                    return Building->HasRuntimeUpgrades() &&
                        Building->CycleRuntimeUpgrade(OutMessage);
                case EBuildingInfoTab::Information:
                    return Building->HasWarehouseStorageControls() &&
                        Building->CycleWarehousePriority(OutMessage);
                default:
                    return false;
                }
            }

            const auto CommandService = ResolveCommandService();

            if (!CommandService)
                return false;

            switch (Tab)
            {
            case EBuildingInfoTab::Overview:
                return CommandService->CycleAutoImportResource(OutMessage);
            case EBuildingInfoTab::Statistics:
                return CommandService->CycleDomesticReservePolicy(OutMessage);
            case EBuildingInfoTab::Upgrades:
                return CommandService->CycleImportPerResourceCap(OutMessage);
            case EBuildingInfoTab::Efficiency:
                return CommandService->CycleImportBudgetPolicy(OutMessage);
            case EBuildingInfoTab::Information:
                return CommandService->CycleExportBlockedResource(OutMessage);
            default:
                return false;
            }
        }

        virtual bool OpenTradeWidget() override
        {
            auto World = mWorld.lock();

            if (!World)
                return false;

            auto UIManager = World->GetUIManager().lock();

            if (!UIManager)
                return false;

            auto TradeWidget =
                UIManager->FindWidget<CTradeWidget>(GTradeWidgetName).lock();

            if (!TradeWidget)
                return false;

            TradeWidget->SetOpen(true);
            return true;
        }

    private:
        std::shared_ptr<CPlacementAreaObject> ResolveBuilding(
            const std::string& BuildingName) const
        {
            auto World = mWorld.lock();

            if (!World || BuildingName.empty())
                return nullptr;

            auto Building = World->FindObject<CPlacementAreaObject>(
                BuildingName).lock();

            if (!Building || !Building->GetAlive() || !Building->GetEnable())
                return nullptr;

            return Building;
        }

        std::shared_ptr<CPlacementController> ResolvePlacementController() const
        {
            auto World = mWorld.lock();

            if (!World)
                return nullptr;

            return World->FindObject<CPlacementController>(
                GPlacementControllerName).lock();
        }

        std::shared_ptr<IGovernmentCommandService> ResolveCommandService() const
        {
            auto World = mWorld.lock();
            return ResolveGovernmentCommandService(World);
        }

        std::shared_ptr<IMainWorldBuildingConditionAccess>
            ResolveBuildingConditionAccess() const
        {
            auto World = mWorld.lock();
            return ResolveMainWorldBuildingConditionAccess(World);
        }

        std::shared_ptr<CMainCamera> ResolveMainCamera() const
        {
            auto World = mWorld.lock();

            if (!World)
                return nullptr;

            return World->FindObject<CMainCamera>(
                GMainCameraName).lock();
        }

    private:
        std::weak_ptr<CWorld> mWorld;
    };
}

namespace CitizenInfoWorldInteractionSource
{
    std::shared_ptr<CitizenInfoInteractionService::ICitizenInfoInteractionSource>
        CreateWorldInteractionSource(const std::shared_ptr<CWorld>& World)
    {
        if (!World)
            return nullptr;

        return std::make_shared<CWorldCitizenInfoInteractionSource>(World);
    }
}
