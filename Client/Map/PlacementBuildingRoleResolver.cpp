#include "PlacementBuildingRoleResolver.h"
#include "../Building/BuildingCatalogEntry.h"

FPlacementBuildingRoleState ResolvePlacementBuildingRoleState(
    const FBuildingCatalogEntry& Entry)
{
    FPlacementBuildingRoleState Roles;
    Roles.Warehouse = Entry.IsWarehouse;
    Roles.BusGarage = Entry.IsBusGarage;
    Roles.BusStop = Entry.IsBusStop;
    return Roles;
}
