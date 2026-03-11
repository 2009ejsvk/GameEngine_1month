#pragma once

#include "../UI/UIStrings.h"
#include "Vector4.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

// 건물 카탈로그 카테고리 번호.
// BuildingCategoryInfo.h 의 카테고리 메타데이터와 순서가 일치해야 한다.
enum class EBuildingCategory
{
    Infrastructure  = 0,  // 교통 및 기반시설
    FoodResource    = 1,  // 음식 및 자원
    Industry        = 2,  // 산업
    Housing         = 3,  // 주거지
    Entertainment   = 4,  // 오락
    MediaEducation  = 5,  // 미디어 및 교육
    Tourism         = 6,  // 관광업
    PublicService   = 7,  // 공익 서비스
    Count           = 8
};

enum class EBuildingEra
{
    Colonial = 0,
    WorldWars,
    ColdWar,
    Modern
};

inline const wchar_t* GetBuildingEraDisplayName(EBuildingEra Era)
{
    switch (Era)
    {
    case EBuildingEra::Colonial:
        return UIStrings::Get(L"building.era.colonial").c_str();
    case EBuildingEra::WorldWars:
        return UIStrings::Get(L"building.era.world_wars").c_str();
    case EBuildingEra::ColdWar:
        return UIStrings::Get(L"building.era.cold_war").c_str();
    case EBuildingEra::Modern:
        return UIStrings::Get(L"building.era.modern").c_str();
    default:
        return UIStrings::Get(L"building.era.unknown").c_str();
    }
}

enum class EBuildingHousingClass
{
    None = 0,
    Collective,
    Standard,
    Elite
};

enum class EBuildingLeisureClass
{
    None = 0,
    General,
    Cultural,
    Luxury
};

enum class ETouristPreference
{
    None = 0,
    Cultural,
    Family,
    Backpacker,
    Relaxation,
    ThrillSeeker,
    Celebrity
};

inline const wchar_t* GetTouristPreferenceDisplayName(
    ETouristPreference Preference)
{
    switch (Preference)
    {
    case ETouristPreference::Cultural:
        return UIStrings::Get(L"building.tourist_preference.cultural").c_str();
    case ETouristPreference::Family:
        return UIStrings::Get(L"building.tourist_preference.family").c_str();
    case ETouristPreference::Backpacker:
        return UIStrings::Get(
            L"building.tourist_preference.backpacker").c_str();
    case ETouristPreference::Relaxation:
        return UIStrings::Get(
            L"building.tourist_preference.relaxation").c_str();
    case ETouristPreference::ThrillSeeker:
        return UIStrings::Get(
            L"building.tourist_preference.thrill_seeker").c_str();
    case ETouristPreference::Celebrity:
        return UIStrings::Get(
            L"building.tourist_preference.celebrity").c_str();
    default:
        return UIStrings::Get(L"building.tourist_preference.unknown").c_str();
    }
}

enum class EResourceType
{
    None = 0,
    Food,
    RawGoods,
    ManufacturedGoods,
    LuxuryGoods,
    Count
};

inline const wchar_t* GetResourceTypeDisplayName(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Food:
        return UIStrings::Get(L"building.resource_type.food").c_str();
    case EResourceType::RawGoods:
        return UIStrings::Get(L"building.resource_type.raw_goods").c_str();
    case EResourceType::ManufacturedGoods:
        return UIStrings::Get(
            L"building.resource_type.manufactured_goods").c_str();
    case EResourceType::LuxuryGoods:
        return UIStrings::Get(L"building.resource_type.luxury_goods").c_str();
    default:
        return UIStrings::Get(L"building.resource_type.none").c_str();
    }
}

inline bool IsExportableResourceType(EResourceType Type)
{
    switch (Type)
    {
    case EResourceType::Food:
    case EResourceType::RawGoods:
    case EResourceType::ManufacturedGoods:
    case EResourceType::LuxuryGoods:
        return true;
    default:
        return false;
    }
}

struct FResourceInventory
{
    std::array<int, static_cast<size_t>(EResourceType::Count)> Amounts = {};

    int Get(EResourceType Type) const
    {
        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return 0;

        return Amounts[Index];
    }

    int GetTotal() const
    {
        int Total = 0;

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
            Total += Amounts[Index];

        return Total;
    }

    int GetExportableTotal() const
    {
        int Total = 0;

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type))
                continue;

            Total += Amounts[Index];
        }

        return Total;
    }

    void Add(EResourceType Type, int Amount, int MaxPerType)
    {
        if (Amount <= 0 || Type == EResourceType::None)
            return;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return;

        Amounts[Index] = (std::min)(MaxPerType, Amounts[Index] + Amount);
    }

    bool Consume(EResourceType Type, int Amount)
    {
        if (Amount <= 0)
            return true;

        if (Type == EResourceType::None)
            return false;

        const size_t Index = static_cast<size_t>(Type);

        if (Index >= Amounts.size())
            return false;

        if (Amounts[Index] < Amount)
            return false;

        Amounts[Index] -= Amount;
        return true;
    }

    bool ConsumeAnyExportable(int Amount, EResourceType& OutType)
    {
        if (Amount <= 0)
        {
            OutType = EResourceType::None;
            return true;
        }

        for (size_t Index = 0; Index < Amounts.size(); ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type) ||
                Amounts[Index] < Amount)
            {
                continue;
            }

            Amounts[Index] -= Amount;
            OutType = Type;
            return true;
        }

        OutType = EResourceType::None;
        return false;
    }

    bool ConsumeExportableAmount(int Amount)
    {
        if (Amount <= 0)
            return true;

        if (GetExportableTotal() < Amount)
            return false;

        int Remaining = Amount;

        for (size_t Index = 0; Index < Amounts.size() && Remaining > 0; ++Index)
        {
            const EResourceType Type =
                static_cast<EResourceType>(Index);

            if (!IsExportableResourceType(Type) ||
                Amounts[Index] <= 0)
            {
                continue;
            }

            const int Consumed = (std::min)(Amounts[Index], Remaining);
            Amounts[Index] -= Consumed;
            Remaining -= Consumed;
        }

        return Remaining == 0;
    }
};

// PlacementController 가 생성할 비주얼 오브젝트 종류.
// 각 값은 PlacementBuildingVisual 이 어떤 메시/텍스처 세트를 쓸지 결정한다.
enum class EPlacementBuildingKind
{
    BuildingA,          // 일반 소형 건물 A
    BuildingB,          // 일반 소형 건물 B (기본값)
    Road,               // 도로 타일
    TransportOffice,    // 교통 관련 건물
    Harbor              // 항구
};

// 배치 시 점유하는 타일 영역의 모양·크기 프리셋.
// PlacementAreaObject::MakePlacementTemplate() 에서 각 값에 대응하는
// FPlacementTemplate 을 생성한다.
//
//   이름 규칙: Diamond{논리지름}x{논리지름}{마커수}Marker
//   DiamondRadius : 1 → 3×3, 2 → 5×5, 3 → 7×7
//
//   새 크기가 필요하면 이 enum 에 값을 추가하고
//   PlacementAreaObject::MakePlacementTemplate() 의 switch 에도 케이스를 추가한다.
enum class EPlacementTemplateType
{
    SingleTileMarker,       // 1x1 타일, 마커 1개 (도로 등)
    Diamond3x3SingleMarker,  // 반지름 1, 마커 1개, 입구 Gap 있음 (기본·소형)
    Diamond5x5TwoMarker,     // 반지름 2, 마커 2개 (중형)
    Diamond5x5FourMarker,    // 반지름 2, 마커 4개 (중형·4방향 입구)
    Diamond7x7ThreeMarker    // 반지름 3, 마커 3개 (대형)
};

// 배치 마커(입구·장식 오브젝트) 하나의 논리 좌표 오프셋.
// 아이소메트릭 논리 좌표계 기준 (x: 오른쪽, y: 아래).
// PlacementAreaObject 가 중심 타일의 논리 좌표에 이 오프셋을 더해 마커 위치를 결정한다.
struct FPlacementMarkerAnchor
{
    float LogicalOffsetX = 0.f;  // 중심으로부터 논리 X 오프셋
    float LogicalOffsetY = 0.f;  // 중심으로부터 논리 Y 오프셋
};

// 배치 영역 전체를 기술하는 런타임 데이터.
// BuildingCatalog 의 EPlacementTemplateType → PlacementAreaObject::MakePlacementTemplate()
// 을 통해 생성되며, PlacementAreaObject 가 보유·사용한다.
struct FPlacementTemplate
{
    EPlacementTemplateType Type =
        EPlacementTemplateType::Diamond3x3SingleMarker;  // 원본 프리셋 타입 (참조용)

    // 아이소메트릭 다이아몬드 반지름.
    // 실제 점유 타일은 |dx|+|dy| <= DiamondRadius 조건을 만족하는 격자 셀.
    // 1 → 3×3(약 9칸), 2 → 5×5(약 25칸), 3 → 7×7(약 49칸)
    int DiamondRadius = 1;

    // 건물 입구·장식 마커 앵커 목록.
    // PlacementController 가 이 위치에 BuildingMarkerOrb 를 생성한다.
    std::vector<FPlacementMarkerAnchor> MarkerAnchors;

    FVector4 AreaColor = FVector4::Blue;  // 배치 가능 영역 타일 표시 색상

    // true 면 회전 방향에 따라 영역 외곽 타일 1칸을 비워 "입구" 공간을 만든다.
    // Diamond3x3SingleMarker 에만 기본 활성화, 나머지는 false.
    bool HasDirectionalGap = true;

    // 예상 점유 타일 수를 반환한다 (Gap 1칸 제외 포함).
    // 주로 유효성 검증에 사용.
    int GetExpectedTileCount() const
    {
        const int Side = DiamondRadius * 2 + 1;
        return Side * Side - (HasDirectionalGap ? 1 : 0);
    }
};
