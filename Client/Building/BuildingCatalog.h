#pragma once

#include "BuildingTypes.h"
#include <string>
#include <vector>

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
//     BuildingCatalog.cpp 의 ResolveTemplateTypeByBuildingId() GRules 벡터에
//     건물 ID → TemplateType 규칙을 추가하면 자동 적용된다.
//     사용 가능한 템플릿 목록은 BuildingTypes.h > EPlacementTemplateType 참고.
// ─────────────────────────────────────────────────────────────────────────────
struct FBuildingCatalogEntry
{
    std::string    Id;                  // 건물 고유 ID ("build_{cat}_{local}")
    std::wstring   DisplayName;         // UI에 표시되는 건물 이름
    std::wstring   CategoryName;        // 소속 카테고리 이름 (탭 레이블)
    std::wstring   DetailText;          // 건물 상세 설명 텍스트

    // ── 건물 기능 플래그 ──────────────────────────────────────────────────
    bool           Residential              = false;  // 주거 기능 여부
    bool           FoodProvider             = false;  // 음식 공급 기능 여부
    bool           EntertainmentProvider    = false;  // 오락 제공 기능 여부

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

    // ── 기타 수치 ─────────────────────────────────────────────────────────
    int            Capacity                 = 0;    // 수용 인원 (주거/직장 슬롯 수)

    // ── 카탈로그 인덱스 (아이콘 경로 조회에 사용) ─────────────────────────
    EBuildingCategory Category              = EBuildingCategory::Infrastructure;
    int            CategoryLocalIndex       = 0;    // 카테고리 내 순서 (0-based)

    // ── 배치 시스템 ───────────────────────────────────────────────────────
    // TemplateType : 배치 시 점유할 타일 영역 모양·크기
    //   Diamond3x3SingleMarker  (기본) — 소형 건물
    //   Diamond5x5TwoMarker             — 중형 건물
    //   Diamond5x5FourMarker            — 중형 건물 (입구 4방향)
    //   Diamond7x7ThreeMarker           — 대형 건물
    // BuildingKind : PlacementController 가 생성할 비주얼 오브젝트 종류
    EPlacementTemplateType  TemplateType    = EPlacementTemplateType::Diamond3x3SingleMarker;
    EPlacementBuildingKind  BuildingKind    = EPlacementBuildingKind::BuildingB;
};

// 전체 건물 카탈로그를 반환한다.
// 내부적으로 static 벡터로 초기화되어 최초 호출 시 한 번만 생성된다.
const std::vector<FBuildingCatalogEntry>& GetBuildingCatalog();

// EntryId("build_{cat}_{local}")로 카탈로그 항목 포인터를 반환한다.
// 일치하는 항목이 없으면 nullptr 반환.
const FBuildingCatalogEntry* FindBuildingCatalogEntry(const std::string& EntryId);

// Category + CategoryLocalIndex 조합으로 아이콘 파일 경로를 반환한다.
// 반환 타입은 wchar_t*(UTF-16). 범위를 벗어나면 nullptr 반환.
const wchar_t* GetCatalogEntryIconPath(EBuildingCategory Category, int CategoryLocalIndex);

// GetCatalogEntryIconPath 의 UTF-8 버전.
// 범위를 벗어나거나 경로가 없으면 빈 문자열 반환.
std::string GetCatalogEntryIconPathUtf8(EBuildingCategory Category, int CategoryLocalIndex);
