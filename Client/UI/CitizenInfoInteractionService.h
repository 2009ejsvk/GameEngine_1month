#pragma once

#include "CitizenInfoConstants.h"
#include <memory>
#include <string>

class CWorld;

namespace CitizenInfoInteractionService
{
    class ICitizenInfoInteractionSource
    {
    public:
        virtual ~ICitizenInfoInteractionSource() = default;

        virtual bool IsCustomsOfficeBuilding(
            const std::string& BuildingName) const = 0;
        virtual bool SetBuildingBudgetLevel(
            const std::string& BuildingName,
            int Level) = 0;
        virtual bool SetBuildingOperationMode(
            const std::string& BuildingName,
            int ModeIndex,
            std::wstring& OutMessage) = 0;
        virtual bool DemolishBuilding(
            const std::string& BuildingName) = 0;
        virtual bool BeginMoveBuilding(
            const std::string& BuildingName) = 0;
        virtual bool FocusBuilding(
            const std::string& BuildingName) = 0;
        virtual bool ExecuteBuildingCommand(
            const std::string& BuildingName,
            CitizenInfoConstants::EBuildingInfoTab Tab,
            std::wstring& OutMessage) = 0;
        virtual bool OpenTradeWidget() = 0;
    };

    std::shared_ptr<ICitizenInfoInteractionSource>
        CreateWorldInteractionSource(const std::shared_ptr<CWorld>& World);
}
