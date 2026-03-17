#pragma once

#include "BuildingCatalogEntry.h"
#include "BuildingCatalogQuery.h"

class CPlacementAreaObject;

// 전체 건물 카탈로그를 반환한다.
// 내부적으로 reloadable snapshot store를 사용한다.
const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog();

void RegisterRuntimeConfig();
unsigned long long GetRuntimeConfigGeneration();

bool IsCustomsOfficeCatalogEntry(const FBuildingCatalogEntry& Entry);
bool IsCustomsOfficeBuildingId(const std::string& EntryId);

// 빌드 메뉴에서 사용할 최종 카테고리를 반환한다.
EBuildingCategory GetEffectiveBuildMenuCategory(
    const FBuildingCatalogEntry& Entry);

// 건물 엔트리에서 UI 아이콘 경로를 반환한다.
const wchar_t* GetCatalogEntryIconPath(const FBuildingCatalogEntry& Entry);

// Category + CategoryLocalIndex 조합으로 건물별 IconPath를 반환한다.
// 최신 TSV의 IconPath 컬럼이 우선이며, 값이 없으면 nullptr + 로그를 반환한다.
const wchar_t* GetCatalogEntryIconPath(
    EBuildingCategory Category,
    int CategoryLocalIndex);

// GetCatalogEntryIconPath(const FBuildingCatalogEntry&) 의 UTF-8 버전.
std::string GetCatalogEntryIconPathUtf8(const FBuildingCatalogEntry& Entry);

// GetCatalogEntryIconPath 의 UTF-8 버전.
// 범위를 벗어나거나 경로가 없으면 빈 문자열 반환.
std::string GetCatalogEntryIconPathUtf8(
    EBuildingCategory Category,
    int CategoryLocalIndex);

// 건물 엔트리에서 월드 스프라이트 경로를 반환한다.
const wchar_t* GetCatalogEntrySpriteTexturePath(
    const FBuildingCatalogEntry& Entry);

// Category + CategoryLocalIndex 조합으로 건물별 SpriteTexturePath 를 반환한다.
// 최신 TSV의 SpriteTexturePath 컬럼이 우선이며, 비어 있으면 IconPath 를 본다.
const wchar_t* GetCatalogEntrySpriteTexturePath(
    EBuildingCategory Category,
    int CategoryLocalIndex);

// GetCatalogEntrySpriteTexturePath(const FBuildingCatalogEntry&) 의 UTF-8 버전.
std::string GetCatalogEntrySpriteTexturePathUtf8(
    const FBuildingCatalogEntry& Entry);

// GetCatalogEntrySpriteTexturePath 의 UTF-8 버전.
std::string GetCatalogEntrySpriteTexturePathUtf8(
    EBuildingCategory Category,
    int CategoryLocalIndex);

std::wstring GetBuildingProducedResourceDisplayName(
    const FBuildingCatalogEntry& Entry);

std::wstring GetBuildingProductionInputDisplayName(
    const FBuildingCatalogEntry& Entry,
    int SlotIndex);

const wchar_t* GetProductionChainStageDisplayName(
    EBuildingProductionChainStage Stage);

std::wstring BuildProductionChainSummary(
    const FBuildingCatalogEntry& Entry,
    EResourceType EffectiveProducedType = EResourceType::None);

std::wstring BuildRuntimeProductionChainSummary(
    const CPlacementAreaObject& Building,
    const FBuildingCatalogEntry& Entry);

EBuildingProductionChainStage BuildRuntimeProductionChainStage(
    const CPlacementAreaObject& Building,
    const FBuildingCatalogEntry& Entry);

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex);

std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex);

std::wstring GetOperationModeTransitionNotice(
    const FBuildingCatalogEntry& Entry);

std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex);

std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex);
