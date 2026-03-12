#pragma once

#include "BuildingTypes.h"
#include "../Politics/PoliticalTypes.h"
#include <array>
#include <string>
#include <vector>

struct FBuildingOperationModeEffect
{
    float ProductionMultiplier = 1.f;
    float InputConsumptionMultiplier = 1.f;
    float ServiceThroughputMultiplier = 1.f;
    float HarborProgressMultiplier = 1.f;
    float ProducedPowerMultiplier = 1.f;
    float RequiredPowerMultiplier = 1.f;
    float WarehouseSlotCapacityMultiplier = 1.f;
    float StorageLossMultiplier = 1.f;
    float PollutionMultiplier = 1.f;
    float WageMultiplier = 1.f;
    float UpkeepMultiplier = 1.f;
    int ProducedPowerDeltaMW = 0;
    int RequiredPowerDeltaMW = 0;
    int WarehouseSlotCapacityDelta = 0;
    int PollutionFlatDelta = 0;
    int WageFlatDelta = 0;
    int UpkeepFlatDelta = 0;
    int CapacityDelta = 0;
    int ServiceCapacityDelta = 0;
    int PerWorkerServiceCapacityDelta = 0;
    int HousingQualityDelta = 0;
    int JobQualityDelta = 0;
    int GenericServiceQualityDelta = 0;
    float HousingQualityMultiplier = 1.f;
    float JobQualityMultiplier = 1.f;
    float GenericServiceQualityMultiplier = 1.f;

    bool HasRuntimeEffect() const
    {
        return ProductionMultiplier != 1.f ||
            InputConsumptionMultiplier != 1.f ||
            ServiceThroughputMultiplier != 1.f ||
            HarborProgressMultiplier != 1.f ||
            ProducedPowerMultiplier != 1.f ||
            RequiredPowerMultiplier != 1.f ||
            WarehouseSlotCapacityMultiplier != 1.f ||
            StorageLossMultiplier != 1.f ||
            PollutionMultiplier != 1.f ||
            WageMultiplier != 1.f ||
            UpkeepMultiplier != 1.f ||
            ProducedPowerDeltaMW != 0 ||
            RequiredPowerDeltaMW != 0 ||
            WarehouseSlotCapacityDelta != 0 ||
            PollutionFlatDelta != 0 ||
            WageFlatDelta != 0 ||
            UpkeepFlatDelta != 0 ||
            CapacityDelta != 0 ||
            ServiceCapacityDelta != 0 ||
            PerWorkerServiceCapacityDelta != 0 ||
            HousingQualityDelta != 0 ||
            JobQualityDelta != 0 ||
            GenericServiceQualityDelta != 0 ||
            HousingQualityMultiplier != 1.f ||
            JobQualityMultiplier != 1.f ||
            GenericServiceQualityMultiplier != 1.f;
    }
};

struct FBuildingOperationModeDef
{
    std::wstring DisplayName;
    std::wstring EffectSummary;
    FBuildingOperationModeEffect Effect;
};

struct FBuildingRuntimeUpgradeDef
{
    std::wstring DisplayName;
    std::wstring EffectSummary;
    FBuildingOperationModeEffect Effect;
};

enum class EBuildingCostState
{
    None,
    Known,
    Unknown
};

// ─────────────────────────────────────────────────────────────────────────────
// FBuildingCatalogEntry
//   건물 하나의 메타데이터 정의체.
//   BuildMenuWidget이 UI에 표시할 텍스트·아이콘·카테고리 정보와
//   배치 시스템이 사용할 타일 템플릿·건물 종류를 함께 보유한다.
//
//   [ID 규칙]
//     "build_{CategoryIndex}_{CategoryLocalIndex}" 형식 (예: "build_3_1")
//     CategoryIndex       : 카테고리 번호 (0-based, BuildingCatalog.cpp 참고)
//     CategoryLocalIndex  : 해당 카테고리 내 순서 (0-based)
//
//   [타일 조정]
//     TemplateType 필드로 배치 영역 크기를 지정한다.
//     BuildingCatalog.tsv 의 TemplateType 컬럼을 수정하면 자동 적용된다.
//     컬럼이 비어 있거나 구 포맷 TSV를 쓰는 경우에만 코드 fallback을 사용한다.
//     사용 가능한 템플릿 목록은 BuildingTypes.h > EPlacementTemplateType 참고.
//
//   [비주얼 경로]
//     IconPath            : UI 카드/패널 아이콘
//     SpriteTexturePath   : 월드 배치 비주얼 텍스처
//     둘 다 BuildingCatalog.tsv 컬럼으로 관리하며 TemplateType 과는 별개다.
// ─────────────────────────────────────────────────────────────────────────────
struct FBuildingCatalogEntry
{
    std::string    Id;                  // 건물 고유 ID ("build_{cat}_{local}")
    std::wstring   DisplayName;         // UI에 표시되는 건물 이름
    std::wstring   CategoryName;        // 소속 카테고리 이름 (탭 레이블)
    std::wstring   DetailText;          // 건물 상세 설명 텍스트
    EBuildingCostState BlueprintCostState =
        EBuildingCostState::None;
    int            BlueprintCost        = 0;
    EBuildingCostState ConstructionCostState =
        EBuildingCostState::None;
    int            ConstructionCost     = 0;
    EBuildingEra   UnlockEra            = EBuildingEra::Colonial;
    ECitizenEducationLevel RequiredEducationLevel =
        ECitizenEducationLevel::Uneducated;
    EBuildingHousingClass HousingClass  = EBuildingHousingClass::None;
    EBuildingLeisureClass LeisureClass  = EBuildingLeisureClass::None;
    ETouristPreference PrimaryTouristPreference =
        ETouristPreference::None;
    EResourceType  ProducedResourceType = EResourceType::None;
    std::wstring   ProducedResourceLabel;
    EResourceType  VisitConsumptionResourceType =
        EResourceType::None;
    std::array<EResourceType, GProductionInputSlotCount>
        ProductionInputTypes =
        {
            EResourceType::None,
            EResourceType::None
        };
    std::array<int, GProductionInputSlotCount> ProductionInputAmounts = {};
    std::array<std::wstring, GProductionInputSlotCount>
        ProductionInputLabels = {};
    enum class EProductionChainStage
    {
        None,
        Primary,
        Intermediate,
        Final
    };
    EProductionChainStage ProductionChainStage =
        EProductionChainStage::None;
    std::wstring SupplyChainSummary;

    // ── 건물 기능 플래그 ──────────────────────────────────────────────────
    bool           Residential              = false;  // 주거 기능 여부
    bool           FoodProvider             = false;  // 음식 공급 기능 여부
    bool           EntertainmentProvider    = false;  // 오락 제공 기능 여부
    bool           HealthProvider           = false;  // 보건 제공 기능 여부
    bool           FaithProvider            = false;  // 신앙 제공 기능 여부
    bool           SupportsTeamsterPickup   = false;  // 팀스터 운반 대상 여부
    bool           CanExportStoredResources = false;  // 항구형 수출 허브 여부
    bool           SupportsImmigration      = false;  // 이민/이주 처리 시설 여부

    // ── UI 동작 플래그 ────────────────────────────────────────────────────
    // IsDemolish          : 배치 대신 철거 모드를 활성화하는 특수 항목
    // IsHiddenFromBuildMenu : 자연 발생 건물 등 빌드 메뉴 슬롯에 표시하지 않음
    bool           IsDemolish              = false;
    bool           IsHiddenFromBuildMenu   = false;

    // ── 시민 만족도 상한 (0~100) ──────────────────────────────────────────
    // 해당 건물이 채울 수 있는 만족도 최대치를 제한한다.
    // 기본값 100 = 제한 없음.
    int            HousingSatisfactionCap   = 100;  // 주거 만족도 상한
    int            JobSatisfactionCap       = 100;  // 직업 만족도 상한
    int            FoodSatisfactionCap      = 100;  // 음식 만족도 상한
    int            FunSatisfactionCap       = 100;  // 오락 만족도 상한
    int            HealthSatisfactionCap    = 100;  // 보건 만족도 상한
    int            FaithSatisfactionCap     = 100;  // 신앙 만족도 상한

    // ── 기타 수치 ─────────────────────────────────────────────────────────
    int            Capacity                 = 0;    // 수용 인원 (주거/직장 슬롯 수)
    int            ServiceCapacity          = 0;    // 방문 서비스 슬롯 수
    int            BaseProducedPowerMW      = 0;    // 기본 발전량
    int            BaseRequiredPowerMW      = 0;    // 기본 필요 전력
    int            BasePollutionOutput      = 0;    // 공해 배출량
    int            BasePollutionMitigation  = 0;    // 공해 정화량
    std::wstring   IconPath;                       // UI 아이콘 경로
    std::wstring   SpriteTexturePath;              // 월드 스프라이트 경로

    // ── 카탈로그 인덱스 / 정렬 정보 ──────────────────────────────────────
    EBuildingCategory Category              = EBuildingCategory::Infrastructure;
    int            CategoryLocalIndex       = 0;    // 카테고리 내 순서 (0-based)

    // ── 배치 시스템 ───────────────────────────────────────────────────────
    // TemplateType : 배치 시 점유할 타일 영역 모양·크기
    //   BuildingCatalog.tsv 의 TemplateType 컬럼으로 덮어쓸 수 있다.
    //   Diamond3x3SingleMarker  (기본) — 소형 건물
    //   Diamond5x5TwoMarker             — 중형 건물
    //   Diamond5x5FourMarker            — 중형 건물 (입구 4방향)
    //   Diamond7x7ThreeMarker           — 대형 건물
    // BuildingKind : 비주얼/특수동작 분기 타입 (텍스처 파일 경로는 SpriteTexturePath)
    EPlacementTemplateType  TemplateType    = EPlacementTemplateType::Diamond3x3SingleMarker;
    EPlacementBuildingKind  BuildingKind    = EPlacementBuildingKind::Structure;
    std::vector<FBuildingOperationModeDef> OperationModeDefs;
    std::vector<FBuildingRuntimeUpgradeDef> RuntimeUpgradeDefs;
    std::vector<std::wstring> UpgradeHints;
    std::vector<FPoliticalSignalDef> PoliticalSignals;
};

// 전체 건물 카탈로그를 반환한다.
// 내부적으로 reloadable snapshot store를 사용한다.
const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog();

void RegisterRuntimeConfig();
unsigned long long GetRuntimeConfigGeneration();

// EntryId("build_{cat}_{local}")로 카탈로그 항목 포인터를 반환한다.
// 일치하는 항목이 없으면 nullptr 반환.
const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId);

// 건물 엔트리에서 UI 아이콘 경로를 반환한다.
const wchar_t* GetCatalogEntryIconPath(const FBuildingCatalogEntry& Entry);

// Category + CategoryLocalIndex 조합으로 건물별 IconPath를 반환한다.
// 최신 TSV의 IconPath 컬럼이 우선이며, 값이 없으면 nullptr + 로그를 반환한다.
const wchar_t* GetCatalogEntryIconPath(EBuildingCategory Category, int CategoryLocalIndex);

// GetCatalogEntryIconPath(const FBuildingCatalogEntry&) 의 UTF-8 버전.
std::string GetCatalogEntryIconPathUtf8(const FBuildingCatalogEntry& Entry);

// GetCatalogEntryIconPath 의 UTF-8 버전.
// 범위를 벗어나거나 경로가 없으면 빈 문자열 반환.
std::string GetCatalogEntryIconPathUtf8(EBuildingCategory Category, int CategoryLocalIndex);

// 건물 엔트리에서 월드 스프라이트 경로를 반환한다.
const wchar_t* GetCatalogEntrySpriteTexturePath(const FBuildingCatalogEntry& Entry);

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
    FBuildingCatalogEntry::EProductionChainStage Stage);

std::wstring BuildProductionChainSummary(
    const FBuildingCatalogEntry& Entry);

std::wstring GetOperationModeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex);

std::wstring GetOperationModeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int ModeIndex);

std::wstring GetRuntimeUpgradeDisplayName(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex);

std::wstring GetRuntimeUpgradeEffectSummary(
    const FBuildingCatalogEntry& Entry,
    int UpgradeIndex);
