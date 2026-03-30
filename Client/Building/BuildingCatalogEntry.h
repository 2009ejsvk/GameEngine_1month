#pragma once

#include "BuildingCatalogAspectData.h"
#include "BuildingCatalogFwd.h"
#include "BuildingTypes.h"
#include <string>
#include <vector>

struct FBuildingCatalogIdentity
{
    std::string Id;
    std::wstring DisplayName;
    std::wstring CategoryName;
    std::wstring DetailText;
};

struct FBuildingCatalogEconomy
{
    EBuildingCostState BlueprintCostState = EBuildingCostState::None;
    int BlueprintCost = 0;
    EBuildingCostState ConstructionCostState = EBuildingCostState::None;
    int ConstructionCost = 0;
};

struct FBuildingCatalogAccess
{
    EBuildingEra UnlockEra = EBuildingEra::Colonial;
    ECitizenEducationLevel RequiredEducationLevel =
        ECitizenEducationLevel::Uneducated;
    unsigned int AllowedWealthMask = GBuildingWealthMaskAll;
    EBuildingHousingClass HousingClass = EBuildingHousingClass::None;
};

struct FBuildingCatalogCapabilities
{
    bool Residential = false;
    bool FoodProvider = false;
    bool EntertainmentProvider = false;
    bool HealthProvider = false;
    bool FaithProvider = false;
    bool SupportsImmigration = false;
    bool IsCustomsOffice = false;
    bool IsWarehouse = false;
    bool IsBusGarage = false;
    bool IsBusStop = false;
    bool IsExportStorageHub = false;
};

struct FBuildingCatalogUiBehavior
{
    bool IsDemolish = false;
    bool IsHiddenFromBuildMenu = false;
};

struct FBuildingCatalogService
{
    int HousingSatisfactionCap = 100;
    int JobSatisfactionCap = 100;
    int FoodSatisfactionCap = 100;
    int FunSatisfactionCap = 100;
    int HealthSatisfactionCap = 100;
    int FaithSatisfactionCap = 100;
    int Capacity = 0;
    int ServiceCapacity = 0;
    bool ServiceCapacityUsesHouseholds = false;
    int BaseHousingQuality = 0;
    int BaseServiceQuality = 0;
    int BaseJobQuality = 0;
};

struct FBuildingCatalogInfrastructure
{
    int BuildingSizeX = 1;
    int BuildingSizeY = 1;
};

struct FBuildingCatalogPresentation
{
    std::wstring IconPath;
    std::wstring SpriteTexturePath;
};

struct FBuildingCatalogMenu
{
    int CategoryLocalIndex = 0;
    bool HasBuildMenuCategoryOverride = false;
    EBuildingCategory BuildMenuCategoryOverride =
        EBuildingCategory::Infrastructure;
};

struct FBuildingCatalogPlacement
{
    EPlacementTemplateType TemplateType =
        EPlacementTemplateType::Diamond3x3SingleMarker;
    EPlacementBuildingKind BuildingKind =
        EPlacementBuildingKind::Structure;
};

struct FBuildingCatalogPlacementConstraints
{
    bool RequiresCoastPlacement = false;
    bool RequiresRoadAdjacentPlacement = false;
};

struct FBuildingCatalogBehavior
{
    std::vector<std::wstring> UpgradeHints;
};

// FBuildingCatalogEntry는 더 이상 단일 거대 필드 뭉치가 아니라,
// 도메인별 메타데이터 조각들을 조합한 최상위 엔트리다.
struct FBuildingCatalogEntry final :
    public FBuildingCatalogIdentity,
    public FBuildingCatalogEconomy,
    public FBuildingCatalogAccess,
    public FBuildingCatalogCapabilities,
    public FBuildingCatalogUiBehavior,
    public FBuildingCatalogService,
    public FBuildingCatalogInfrastructure,
    public FBuildingCatalogPresentation,
    public FBuildingCatalogMenu,
    public FBuildingCatalogPlacement,
    public FBuildingCatalogPlacementConstraints,
    public FBuildingCatalogBehavior,
    public FBuildingCatalogRuntimeData,
    public FBuildingCatalogCitizenData,
    public FBuildingCatalogPoliticalData
{
};
