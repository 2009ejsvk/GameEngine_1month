#include "PlacementAreaObject.h"
#include "../ObjectNames.h"
#include "Component/SceneComponent.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include "World/Input.h"
#include <algorithm>
#include <cmath>
#include <cfloat>

/*
    [PlacementAreaObject.cpp 한눈에 보기]
    이 파일은 "건물 배치 영역"을 타일맵 위에서 관리한다.

    핵심 역할:
    1) 건물이 차지하는 타일(배치 확정 영역) 계산
    2) 마우스 위치를 기준으로 배치 미리보기(가능/불가 색상) 갱신
    3) 길찾기용 정보(막힌 타일/목표 타일) 제공
    4) 오버레이 타일맵(파랑/노랑)과의 색상 동기화

    색상 의미(현재 코드 기준):
    - Blue  : 점유된(이동 불가) 영역 또는 파랑 오버레이
    - White : 기본 표시
    - Green : 미리보기 배치 가능
    - Red   : 미리보기 배치 불가 / 철거 하이라이트
    - Yellow: 중심/마커 강조

    참고:
    - lock(): weak_ptr -> shared_ptr로 "지금 살아있는 객체인지" 확인하며 접근한다.
    - Index : 타일맵 1차원 인덱스(y * 가로개수 + x)
*/
namespace
{
    // 오버레이 타일맵 1개(파랑/노랑)에 대한 전역 상태.
    // 여러 배치 오브젝트가 같은 타일에 동시에 색을 적용할 수 있어서
    // "몇 명이 이 타일을 사용 중인지"를 RefCount로 관리한다.
    struct FOverlayTileState
    {
        // 현재 RefCounts가 대응하는 타일맵 포인터.
        // 타일맵이 바뀌면 RefCounts를 다시 만든다.
        CTileMapComponent* TileMap = nullptr;
        // 타일별 참조 카운트.
        // 값이 0이면 아무도 사용 안 함, 1 이상이면 오버레이 표시 유지.
        std::vector<int> RefCounts;
    };

    // 파랑/노랑 오버레이를 분리 관리한다.
    FOverlayTileState GPrimaryOverlayState;
    FOverlayTileState GMarkerOverlayState;

    void EnsureOverlayState(
        FOverlayTileState& State,
        const std::shared_ptr<CTileMapComponent>& TileMap)
    {
        // 타일맵이 없으면 초기화 불가.
        if (!TileMap)
            return;

        const int TileCount =
            TileMap->GetTileCountX() * TileMap->GetTileCountY();

        // 빈 맵이면 관리할 타일이 없다.
        if (TileCount <= 0)
            return;

        // 타일맵이 바뀌었거나 크기가 달라졌으면
        // 참조 카운트를 "타일 개수만큼 0"으로 재구성한다.
        if (State.TileMap != TileMap.get() ||
            (int)State.RefCounts.size() != TileCount)
        {
            State.TileMap = TileMap.get();
            State.RefCounts.clear();
            State.RefCounts.resize(TileCount, 0);
        }
    }

    bool HasOverlayRef(
        const FOverlayTileState& State, int TileIndex)
    {
        // 범위 밖이면 사용 중이 아님으로 처리.
        if (TileIndex < 0 || TileIndex >= (int)State.RefCounts.size())
            return false;

        return State.RefCounts[TileIndex] > 0;
    }

    void UpdateOverlayTileRefs(
        FOverlayTileState& State,
        const std::shared_ptr<CTileMapComponent>& TileMap,
        std::vector<int>& InOutAppliedIndices,
        const std::vector<int>& NextIndices,
        const FVector4& VisibleColor)
    {
        // 이전/다음 인덱스가 같으면 갱신할 필요 없다.
        if (InOutAppliedIndices == NextIndices)
            return;

        EnsureOverlayState(State, TileMap);

        // 상태 준비 실패 시 현재 적용 목록을 비운다.
        // (다음 프레임에서 다시 정상 동기화 시도)
        if (State.TileMap != TileMap.get() ||
            State.RefCounts.empty())
        {
            InOutAppliedIndices.clear();
            return;
        }

        const int TileCount = static_cast<int>(State.RefCounts.size());

        // 1) 기존에 적용했던 타일들의 참조를 내린다.
        for (size_t i = 0; i < InOutAppliedIndices.size(); ++i)
        {
            const int Index = InOutAppliedIndices[i];

            if (Index < 0 || Index >= TileCount)
                continue;

            int& RefCount = State.RefCounts[Index];

            if (RefCount <= 0)
                continue;

            --RefCount;

            // 아직 다른 객체가 사용 중이면 색을 지우지 않는다.
            if (RefCount > 0) 
                continue;

            RefCount = 0;
            auto Tile = TileMap->GetTile(Index).lock();

            if (!Tile)
                continue;

            // 알파 0으로 만들어 사실상 보이지 않게 한다.
            Tile->SetOutLineColor(VisibleColor.x, VisibleColor.y,
                VisibleColor.z, 0.f);
        }

        // 내부 상태를 "이번 프레임 적용 목록"으로 교체.
        InOutAppliedIndices = NextIndices;

        // 2) 새로 적용할 타일들의 참조를 올린다.
        for (size_t i = 0; i < InOutAppliedIndices.size(); ++i)
        {
            const int Index = InOutAppliedIndices[i];

            if (Index < 0 || Index >= TileCount)
                continue;

            int& RefCount = State.RefCounts[Index];
            ++RefCount;

            // 0 -> 1로 바뀐 첫 사용자만 실제 색상 적용.
            // (2 이상이면 이미 다른 객체가 칠해둔 상태)
            if (RefCount != 1)
                continue;

            auto Tile = TileMap->GetTile(Index).lock();

            if (!Tile)
                continue;

            Tile->SetOutLineColor(VisibleColor);
        }
    }
}

CPlacementAreaObject::CPlacementAreaObject()
{
    // RTTI/팩토리용 타입 등록.
    SetClassType<CPlacementAreaObject>();
    // 기본 템플릿: 3x3 다이아몬드(방향성 빈 칸 1개)
    mTemplate = CreateTemplateByType(
        EPlacementTemplateType::Diamond3x3SingleMarker);
}

CPlacementAreaObject::CPlacementAreaObject(
    const CPlacementAreaObject& ref) :
    CGameObject(ref)
{
}

CPlacementAreaObject::CPlacementAreaObject(
    CPlacementAreaObject&& ref) noexcept :
    CGameObject(std::move(ref))
{
}

CPlacementAreaObject::~CPlacementAreaObject()
{
}

void CPlacementAreaObject::SetBuildingDisplayInfo(
    const std::string& DisplayName,
    const std::string& CategoryName,
    bool Residential,
    int Capacity,
    bool FoodProvider,
    bool EntertainmentProvider,
    int HousingSatisfactionCap,
    int JobSatisfactionCap,
    int FoodSatisfactionCap,
    int FunSatisfactionCap,
    int BaseMonthlyWage,
    int BaseMonthlyUpkeep)
{
    auto ClampTo100 = [](int Value)
    {
        return (std::max)(0, (std::min)(100, Value));
    };

    mBuildingDisplayName = DisplayName;
    mBuildingCategoryName = CategoryName;
    mResidential = Residential;
    mCapacity = Capacity;
    mFoodProvider = FoodProvider;
    mEntertainmentProvider = EntertainmentProvider;
    mHousingSatisfactionCap = ClampTo100(HousingSatisfactionCap);
    mJobSatisfactionCap = ClampTo100(JobSatisfactionCap);
    mFoodSatisfactionCap = ClampTo100(FoodSatisfactionCap);
    mFunSatisfactionCap = ClampTo100(FunSatisfactionCap);
    mBudgetLevel = 3;

    const int SafeCapacity = (std::max)(0, mCapacity);

    if (BaseMonthlyWage < 0)
    {
        if (mResidential)
            mBaseMonthlyWage = 0;
        else
        {
            int DerivedWage = (std::max)(1, SafeCapacity) * 120;

            if (IsTransportOffice())
                DerivedWage = (std::max)(DerivedWage, 800);

            if (IsHarbor())
                DerivedWage = (std::max)(DerivedWage, 1000);

            mBaseMonthlyWage = DerivedWage;
        }
    }
    else
    {
        mBaseMonthlyWage = (std::max)(0, BaseMonthlyWage);
    }

    if (BaseMonthlyUpkeep < 0)
    {
        int DerivedUpkeep = mResidential ?
            (80 + SafeCapacity * 4) :
            (110 + SafeCapacity * 5);

        if (IsTransportOffice())
            DerivedUpkeep += 300;

        if (IsHarbor())
            DerivedUpkeep += 450;

        if (mEntertainmentProvider && !mFoodProvider)
            DerivedUpkeep += 120;

        if (mFoodProvider)
            DerivedUpkeep += 90;

        mBaseMonthlyUpkeep = (std::max)(0, DerivedUpkeep);
    }
    else
    {
        mBaseMonthlyUpkeep = (std::max)(0, BaseMonthlyUpkeep);
    }
}

bool CPlacementAreaObject::Init()
{
    // 부모 초기화 먼저 수행.
    CGameObject::Init();

    // 씬 그래프 루트 컴포넌트 생성.
    CreateComponent<CSceneComponent>("Root");

    return true;
}

void CPlacementAreaObject::Update(float DeltaTime)
{
    // 기본 게임오브젝트 업데이트 실행.
    CGameObject::Update(DeltaTime);

    // 타일맵/초기 배치 준비가 아직이면 여기서 준비한다.
    EnsurePlacementObject();

    // 준비가 끝났으면 현재 배치 상태의 색상을 매 프레임 동기화한다.
    if (mTileMapPrepared)
    {
        std::shared_ptr<CTileMapComponent> TileMap;

        if (AcquireTileMap(TileMap))
        {
            // 점유 영역(및 마커 오버레이) 색상 갱신.
            ApplyPlacedAreaColor(TileMap);

            // 철거 모드 hover 중이면 배치 영역을 빨강으로 덮어 표시.
            if (mDemolitionHoverActive &&
                !mPrimaryPlacedIndices.empty())
            {
                SetAreaColor(
                    TileMap, mPrimaryPlacedIndices, FVector4::Red);
            }
        }
    }

    if (!mMovePreviewActive)
        return;

    // 프리뷰 모드일 때는 마우스 월드 좌표를 읽어
    // "지금 위치에 놓을 수 있는지" 실시간 계산한다.
    auto World = mWorld.lock();

    if (!World)
        return;

    auto Input = World->GetInput().lock();

    if (!Input)
        return;

    UpdatePlacementPreviewFromMouse(Input->GetMouseWorldPos());
}

void CPlacementAreaObject::Destroy()
{
    // 오브젝트 제거 전에 타일 상태를 원래대로 되돌린다.
    std::shared_ptr<CTileMapComponent> TileMap;

    if (AcquireTileMap(TileMap))
    {
        // 확정 배치 타일: 이동 불가 해제 + 기본 색 복구
        for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
        {
            auto Tile = TileMap->GetTile(mPrimaryPlacedIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::Normal);
            Tile->SetOutLineColor(FVector4::White);
        }

        // 프리뷰 타일도 복구.
        for (size_t i = 0; i < mPreviewIndices.size(); ++i)
        {
            RestoreTileColor(TileMap, mPreviewIndices[i]);
        }
    }

    mPrimaryPlacedIndices.clear();
    mMarkerTileIndices.clear();
    mPreviewIndices.clear();
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mDemolitionHoverActive = false;

    // 오버레이 참조도 반드시 해제해서 다른 객체 표시가 꼬이지 않게 한다.
    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    CGameObject::Destroy();
}

bool CPlacementAreaObject::IsNavigationObstacle() const
{
    // 이 오브젝트는 기본적으로 길찾기 충돌체로 동작한다.
    return true;
}

void CPlacementAreaObject::SetPlacementTemplateType(
    EPlacementTemplateType Type)
{
    // 미리 정의된 템플릿 타입으로 교체.
    SetPlacementTemplate(CreateTemplateByType(Type));
}

void CPlacementAreaObject::SetPlacementTemplate(
    const FPlacementTemplate& Template)
{
    // 사용자 지정 템플릿 적용 후, 유효성 보정 + 상태 리셋.
    mTemplate = Template;
    EnsureTemplateValidity();
    ResetPlacementState();
}

void CPlacementAreaObject::GetNavigationGoalTiles(
    std::vector<int>& OutIndices)
{
    // 길찾기에서 "도착 지점으로 허용할 타일" 목록을 반환한다.
    // 현재 구현에서는 노란 마커 타일이 목표 타일이다.
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    // 아직 마커 계산 전이면 현재 배치 상태 기반으로 계산한다.
    if (mMarkerTileIndices.empty())
    {
        ApplyPlacedAreaColor(TileMap);
    }

    for (size_t i = 0; i < mMarkerTileIndices.size(); ++i)
    {
        const int MarkerIndex = mMarkerTileIndices[i];

        // 안전장치: 마커가 실제 배치 영역 바깥이면 무시.
        if (!IsPlacedIndex(MarkerIndex))
            continue;

        // OutIndices 중복 삽입 방지.
        if (std::find(OutIndices.begin(), OutIndices.end(),
            MarkerIndex) == OutIndices.end())
        {
            OutIndices.push_back(MarkerIndex);
        }
    }
}

void CPlacementAreaObject::GetNavigationBlockedTiles(
    std::vector<int>& OutIndices)
{
    // 길찾기에서 "지나갈 수 없는 타일" 목록을 반환한다.
    // 단, 목표 타일은 막힌 타일에서 제외한다.
    EnsurePlacementObject();

    if (!mTileMapPrepared ||
        mPrimaryPlacedIndices.empty())
        return;

    std::vector<int> GoalTiles;
    GetNavigationGoalTiles(GoalTiles);

    // 람다: TileIndex가 GoalTiles에 있으면 true.
    auto IsGoalTile = [&](int TileIndex)
    {
        return std::find(GoalTiles.begin(), GoalTiles.end(), TileIndex) !=
            GoalTiles.end();
    };

    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        const int Index = mPrimaryPlacedIndices[i];

        // 목표 타일까지 막아버리면 에이전트가 도착할 수 없으므로 제외.
        if (IsGoalTile(Index))
        {
            continue;
        }

        OutIndices.push_back(Index);
    }
}

void CPlacementAreaObject::StartMovePreview(
    const FVector2& MouseWorldPos)
{
    // 배치 프리뷰 모드를 켜고 현재 마우스 위치로 즉시 1회 계산.
    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return;

    mMovePreviewActive = true;
    UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::RotatePreviewCW(
    const FVector2& MouseWorldPos)
{
    // 시계 방향 회전: 0~3 순환.
    mPreviewDirection = (mPreviewDirection + 1) % 4;

    // 프리뷰가 켜져 있을 때만 즉시 반영.
    if (mMovePreviewActive)
        UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::RotatePreviewCCW(
    const FVector2& MouseWorldPos)
{
    // 반시계 방향 회전: +3 mod 4 == -1 mod 4.
    mPreviewDirection = (mPreviewDirection + 3) % 4;

    if (mMovePreviewActive)
        UpdatePlacementPreviewFromMouse(MouseWorldPos);
}

void CPlacementAreaObject::ConfirmPlacement()
{
    // 프리뷰 결과를 실제 배치로 확정한다.
    EnsurePlacementObject();

    if (!mTileMapPrepared ||
        !mPreviewCanPlace ||
        mPreviewIndices.empty())
    {
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    // 이전 마커 정보 초기화(아래에서 다시 계산된다).
    mMarkerTileIndices.clear();

    // 1) 기존 점유 타일 해제
    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(mPrimaryPlacedIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::Normal);
        Tile->SetOutLineColor(FVector4::White);
    }

    std::vector<int> NextPrimaryIndices;

    // 2) 프리뷰 중심 기준 새 다이아 영역 계산
    if (!BuildDiamondAreaIndices(TileMap, mPreviewCenterIndex, NextPrimaryIndices) ||
        (int)NextPrimaryIndices.size() != mTemplate.GetExpectedTileCount())
    {
        // 계산 실패 시 기존 상태를 유지한 채 종료.
        return;
    }

    // 3) 새 영역을 이동 불가로 지정
    for (size_t i = 0; i < NextPrimaryIndices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(NextPrimaryIndices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetTileType(ETileType::UnableToMove);
    }

    // 4) 내부 상태를 새 배치로 확정
    mPrimaryPlacedIndices = NextPrimaryIndices;
    mPlacedCenterIndex = mPreviewCenterIndex;
    mPreviewIndices.clear();
    mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mMovePreviewActive = false;

    // 5) 시각 정보/오브젝트 월드 위치 동기화
    ApplyPlacedAreaColor(TileMap);
    SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
}

void CPlacementAreaObject::CancelMovePreview()
{
    // 미리보기만 취소하고 기존 확정 배치는 유지.
    ClearPreview();
    mMovePreviewActive = false;
}

bool CPlacementAreaObject::ContainsPlacedTile(
    const FVector2& MouseWorldPos)
{
    // 마우스 위치가 현재 확정 배치 영역 안인지 검사.
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    const int TileIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (TileIndex < 0)
        return false;

    return IsPlacedIndex(TileIndex);
}

float CPlacementAreaObject::GetCenterDistanceSq(
    const FVector2& MouseWorldPos) const
{
    // sqrt를 쓰지 않는 거리 비교용 값(제곱 거리).
    const FVector3 Pos = GetWorldPos();
    const float dx = Pos.x - MouseWorldPos.x;
    const float dy = Pos.y - MouseWorldPos.y;

    return dx * dx + dy * dy;
}

bool CPlacementAreaObject::GetMarkerWorldPos(FVector3& OutWorldPos)
{
    // 편의 함수: 마커가 여러 개면 첫 번째 것만 반환.
    std::vector<FVector3> MarkerWorldPosList;

    if (!GetMarkerWorldPositions(MarkerWorldPosList) ||
        MarkerWorldPosList.empty())
    {
        return false;
    }

    OutWorldPos = MarkerWorldPosList[0];

    return true;
}

bool CPlacementAreaObject::GetMarkerWorldPositions(
    std::vector<FVector3>& OutWorldPosList)
{
    // 마커 타일 인덱스를 월드 좌표로 변환해 반환.
    OutWorldPosList.clear();

    EnsurePlacementObject();

    if (!mTileMapPrepared)
        return false;

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    if (mMarkerTileIndices.empty())
    {
        ApplyPlacedAreaColor(TileMap);
    }

    if (mMarkerTileIndices.empty())
        return false;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    for (size_t i = 0; i < mMarkerTileIndices.size(); ++i)
    {
        auto MarkerTile = TileMap->GetTile(mMarkerTileIndices[i]).lock();

        if (!MarkerTile)
            continue;

        const FVector2 MarkerCenter = MarkerTile->GetCenter();
        // 타일 내부 중심 좌표 + 타일맵 오브젝트 월드 위치 = 최종 월드 좌표
        OutWorldPosList.push_back(FVector3(
            MarkerCenter.x + TileMapWorldPos.x,
            MarkerCenter.y + TileMapWorldPos.y,
            0.f));
    }

    return !OutWorldPosList.empty();
}

bool CPlacementAreaObject::GetClosestMarkerWorldPos(
    const FVector3& RefWorldPos, FVector3& OutWorldPos)
{
    // 기준점에 가장 가까운 마커를 선형 탐색으로 선택.
    std::vector<FVector3> MarkerWorldPosList;

    if (!GetMarkerWorldPositions(MarkerWorldPosList) ||
        MarkerWorldPosList.empty())
    {
        return false;
    }

    float BestDistSq = FLT_MAX;
    int BestIndex = -1;

    for (size_t i = 0; i < MarkerWorldPosList.size(); ++i)
    {
        const FVector3 Delta = MarkerWorldPosList[i] - RefWorldPos;
        // 3D 제곱 거리
        const float DistSq = Delta.x * Delta.x +
            Delta.y * Delta.y +
            Delta.z * Delta.z;

        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestIndex = (int)i;
        }
    }

    if (BestIndex < 0)
        return false;

    OutWorldPos = MarkerWorldPosList[BestIndex];

    return true;
}

bool CPlacementAreaObject::GetTileSize(FVector2& OutTileSize)
{
    // 타일 1칸의 실제 크기 반환.
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return false;

    OutTileSize = TileMap->GetTileSize();

    return true;
}

FPlacementTemplate CPlacementAreaObject::CreateTemplateByType(
    EPlacementTemplateType Type)
{
    // 템플릿은 "영역 크기/마커 기준점/방향성 빈칸 여부"를 담는다.
    FPlacementTemplate Template;
    Template.Type = Type;
    Template.AreaColor = FVector4::Blue;
    Template.HasDirectionalGap = false;

    switch (Type)
    {
    case EPlacementTemplateType::Diamond5x5TwoMarker:
        // 반지름 2 => 대략 5x5 범위의 다이아 형태.
        Template.DiamondRadius = 2;
        // 논리 좌표 기준 마커 위치(상대 오프셋).
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        break;

    case EPlacementTemplateType::Diamond5x5FourMarker:
        Template.DiamondRadius = 2;
        Template.MarkerAnchors.push_back({ 1.f, 0.5f });
        Template.MarkerAnchors.push_back({ -1.f, -0.5f });
        Template.MarkerAnchors.push_back({ 0.5f, -1.f });
        Template.MarkerAnchors.push_back({ -0.5f, 1.f });
        break;

    case EPlacementTemplateType::Diamond7x7ThreeMarker:
        Template.DiamondRadius = 3;
        Template.MarkerAnchors.push_back({ 1.5f, 1.f });
        Template.MarkerAnchors.push_back({ -1.5f, -1.f });
        Template.MarkerAnchors.push_back({ 0.f, 0.f });
        break;

    case EPlacementTemplateType::Diamond3x3SingleMarker:
    default:
        Template.DiamondRadius = 1;
        Template.MarkerAnchors.push_back({ 0.5f, 0.5f });
        // 출입구처럼 한 방향 빈칸을 만들기 위해 사용.
        Template.HasDirectionalGap = true;
        break;
    }

    return Template;
}

void CPlacementAreaObject::EnsureTemplateValidity()
{
    // 최소 반지름 보장.
    if (mTemplate.DiamondRadius < 1)
        mTemplate.DiamondRadius = 1;

    // 마커 기준점이 없으면 기본값 1개를 넣는다.
    if (mTemplate.MarkerAnchors.empty())
    {
        mTemplate.MarkerAnchors.push_back({ 0.5f, 0.5f });
    }
}

void CPlacementAreaObject::ResetPlacementState()
{
    // 이전에 칠해둔 오버레이를 먼저 지워 색상 꼬임을 방지.
    UpdatePrimaryOverlayTiles(std::vector<int>());
    UpdateMarkerOverlayTiles(std::vector<int>());
    mPrimaryPlacedIndices.clear();
    mAppliedPrimaryOverlayIndices.clear();
    mAppliedMarkerOverlayIndices.clear();
    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mTileMapPrepared = false;
    mMovePreviewActive = false;
    mPlacedCenterIndex = -1;
    mPreviewCenterIndex = -1;
    mPreviewDirection = 0;
    mMarkerTileIndices.clear();
}

void CPlacementAreaObject::EnsurePlacementObject()
{
	// 준비가 끝났으면 다시 초기화하지 않는다.
	// (Update/입력 이벤트에서 여러 번 호출돼도 1회만 실행되게 하는 가드)
	if (mTileMapPrepared)
		return;

	// 템플릿 값(반지름, 마커 기준점)이 비정상인 경우를 보정.
	// 이후 면적 계산(BuildDiamondAreaIndices)이 안전하게 동작하도록 선행 보장.
	EnsureTemplateValidity();

	// 실제 배치/색상 변경을 적용할 대상 타일맵 핸들.
	std::shared_ptr<CTileMapComponent> TileMap;

	// 월드/타일맵 오브젝트가 아직 준비되지 않았으면 지금은 아무것도 하지 않는다.
	// (다음 프레임에 다시 들어와 재시도)
	if (!AcquireTileMap(TileMap))
		return;

	const int CountX = TileMap->GetTileCountX();
	const int CountY = TileMap->GetTileCountY();

	// 맵 크기가 0이면 배치 자체가 불가능.
	if (CountX <= 0 || CountY <= 0)
		return;

	// 여러 배치 오브젝트가 있어도 동일 타일맵에 대해서만 1회 초기화한다.
	// static 지역변수:
	// - 함수가 끝나도 값이 유지됨
	// - "마지막으로 정리한 타일맵 주소"를 기억해 중복 전체 초기화를 막음
	static CTileMapComponent* sInitializedTileMap = nullptr;

	if (sInitializedTileMap != TileMap.get())
	{
		// 타일 타입에 맞춰 최초 외곽선 색을 정리한다.
        // (UnableToMove=Blue, 그 외=White)
        const int TileCount = CountX * CountY;

        for (int i = 0; i < TileCount; ++i)
		{
			auto Tile = TileMap->GetTile(i).lock();

			if (!Tile)
				continue;

			// 이미 막힌 타일은 파랑, 그 외는 흰색으로 통일.
			// (이후 배치 색/프리뷰 색 적용의 기준 상태)
			if (Tile->GetType() == ETileType::UnableToMove)
				Tile->SetOutLineColor(FVector4::Blue);

			else
				Tile->SetOutLineColor(FVector4::White);
        }

		sInitializedTileMap = TileMap.get();
	}

	// 자동 배치를 끄면 "준비 완료"만 하고 실제 점유는 하지 않는다.
	// 즉, 사용자가 직접 프리뷰/확정을 하기 전까지는 맵 타일 타입을 변경하지 않음.
	if (!mAutoPlaceOnPrepare)
	{
		// 배치 관련 캐시/상태값만 초기화하고 종료.
		mPrimaryPlacedIndices.clear();
		mMarkerTileIndices.clear();
		mPreviewIndices.clear();
		mPlacedCenterIndex = -1;
		mPreviewCenterIndex = -1;
        mPreviewCanPlace = false;
        mTileMapPrepared = true;
        return;
    }

	std::vector<int> StartPrimaryIndices;
	int StartCenterIndex = -1;

	// 우선 맵 중심(오프셋 포함)에서 배치 가능 여부 확인.
	// Clamp: 오프셋이 커도 인덱스가 맵 밖으로 나가지 않게 보정.
	const int CenterX = Clamp<int>(
		CountX / 2 + mInitialCenterOffsetX, 0, CountX - 1);
	const int CenterY = Clamp<int>(
		CountY / 2 + mInitialCenterOffsetY, 0, CountY - 1);
	const int CenterIndex = CenterY * CountX + CenterX;

	// Found 조건(모두 만족해야 true):
	// 1) 다이아몬드 영역 인덱스 계산 성공
	// 2) 계산된 타일 개수가 템플릿 기대 개수와 동일
	// 3) 해당 영역이 실제 배치 가능 상태
	bool Found = BuildDiamondAreaIndices(TileMap,
		CenterIndex, StartPrimaryIndices) &&
		(int)StartPrimaryIndices.size() == mTemplate.GetExpectedTileCount() &&
		IsAreaPlaceable(TileMap, StartPrimaryIndices);

    if (Found)
    {
        StartCenterIndex = CenterIndex;
    }

	if (!Found)
	{
		// 중심 배치 실패 시 맵 전체를 훑어서 첫 유효 위치를 찾는다.
		// 스캔 순서: 위->아래(y), 왼쪽->오른쪽(x)
		// 따라서 "가장 먼저 발견된 유효 위치"가 시작 배치 위치가 된다.
		for (int y = 0; y < CountY && !Found; ++y)
		{
			for (int x = 0; x < CountX; ++x)
			{
				const int Index = y * CountX + x;

                if (!BuildDiamondAreaIndices(TileMap, Index, StartPrimaryIndices) ||
                    (int)StartPrimaryIndices.size() != mTemplate.GetExpectedTileCount())
                {
					continue;
				}

				if (IsAreaPlaceable(TileMap, StartPrimaryIndices))
				{
					// 유효한 첫 위치 발견.
					Found = true;
					StartCenterIndex = Index;
					// 첫 성공 지점을 사용하고 종료.
					break;
				}
			}
		}
	}

	if (Found)
	{
		// 시작 배치 확정: 해당 타일을 이동 불가로 지정.
		// (이 오브젝트가 점유한 영역으로 취급되어 다른 배치/내비게이션에서 사용)
		for (size_t i = 0; i < StartPrimaryIndices.size(); ++i)
		{
			auto Tile = TileMap->GetTile(StartPrimaryIndices[i]).lock();

            if (!Tile)
                continue;

            Tile->SetTileType(ETileType::UnableToMove);
        }

		mPrimaryPlacedIndices = StartPrimaryIndices;
		mPlacedCenterIndex = StartCenterIndex;
		// 점유 영역 색/마커 표시 반영.
		ApplyPlacedAreaColor(TileMap);
		// 오브젝트 월드 위치를 "배치 중심 타일 중심"에 맞춘다.
		SyncWorldPosFromCenter(TileMap, mPlacedCenterIndex);
	}
	// Found가 false여도 함수는 실패로 끊지 않고 "준비 완료"로 마무리한다.
	// (맵 상태가 바뀌면 이후 프리뷰/재배치 로직으로 복구 가능)

	// 프리뷰 상태는 초기화한 뒤 준비 완료 플래그를 세운다.
	mPreviewIndices.clear();
	mPreviewCenterIndex = -1;
    mPreviewCanPlace = false;
    mTileMapPrepared = true;
}

void CPlacementAreaObject::UpdatePlacementPreviewFromMouse(
    const FVector2& MouseWorldPos)
{
    // 마우스 위치 기반으로 프리뷰 영역을 계산하고 색을 칠한다.
    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
        return;

    // 이전 프리뷰 색상부터 복구.
    ClearPreview();

    // 월드 좌표 -> 타일 인덱스
    const int CenterIndex = TileMap->GetTileIndex(MouseWorldPos);

    if (CenterIndex < 0)
        return;

    if (!BuildDiamondAreaIndices(TileMap, CenterIndex, mPreviewIndices) ||
        (int)mPreviewIndices.size() != mTemplate.GetExpectedTileCount())
    {
        // 맵 가장자리 등으로 영역이 완성되지 않으면 프리뷰 무효.
        mPreviewIndices.clear();
        return;
    }

    mPreviewCenterIndex = CenterIndex;
    mPreviewCanPlace = IsAreaPlaceable(TileMap, mPreviewIndices);

    const FVector4 PreviewColor = mPreviewCanPlace ?
        FVector4::Green : FVector4::Red;

    // 가능하면 초록, 불가면 빨강.
    SetAreaColor(TileMap, mPreviewIndices, PreviewColor);

    // 프리뷰에서도 중앙 타일은 노란색(O)으로 표시한다.
    if (std::find(mPreviewIndices.begin(), mPreviewIndices.end(),
        mPreviewCenterIndex) != mPreviewIndices.end())
    {
        auto CenterTile = TileMap->GetTile(mPreviewCenterIndex).lock();

        if (CenterTile)
            CenterTile->SetOutLineColor(1.f, 1.f, 0.f, 1.f);
    }
}

void CPlacementAreaObject::ClearPreview()
{
    // 프리뷰가 없으면 상태값만 리셋.
    if (mPreviewIndices.empty())
    {
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    std::shared_ptr<CTileMapComponent> TileMap;

    if (!AcquireTileMap(TileMap))
    {
        // 타일맵이 사라진 경우에도 내부 프리뷰 상태는 정리한다.
        mPreviewIndices.clear();
        mPreviewCanPlace = false;
        mPreviewCenterIndex = -1;
        return;
    }

    for (size_t i = 0; i < mPreviewIndices.size(); ++i)
    {
        RestoreTileColor(TileMap, mPreviewIndices[i]);
    }

    mPreviewIndices.clear();
    mPreviewCanPlace = false;
    mPreviewCenterIndex = -1;
}

bool CPlacementAreaObject::AcquireTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    // mWorld는 weak_ptr이므로 lock()으로 유효성 확인 후 사용.
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mTileMapObject.expired())
    {
        // 캐시가 비어 있으면 월드에서 이름으로 1회 검색해 캐시.
        mTileMapObject = World->FindObject<CTileMapObject>(GTileMapObjectName);
    }

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return false;

    OutTileMap = TileMapObj->GetTileMap().lock();

    // shared_ptr 유효 여부를 bool로 반환.
    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireBlueOverlayTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    // 파랑 오버레이 타일맵 핸들 획득(캐시 포함).
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mBlueOverlayTileMapObject.expired())
    {
        mBlueOverlayTileMapObject =
            World->FindObject<CTileMapObject>(GTileMapFloorBlueName);
    }

    auto BlueOverlayObj = mBlueOverlayTileMapObject.lock();

    if (!BlueOverlayObj)
        return false;

    OutTileMap = BlueOverlayObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

bool CPlacementAreaObject::AcquireYellowOverlayTileMap(
    std::shared_ptr<class CTileMapComponent>& OutTileMap)
{
    // 노랑 오버레이 타일맵 핸들 획득(캐시 포함).
    auto World = mWorld.lock();

    if (!World)
        return false;

    if (mYellowOverlayTileMapObject.expired())
    {
        mYellowOverlayTileMapObject =
            World->FindObject<CTileMapObject>(GTileMapFloorYellowName);
    }

    auto YellowOverlayObj = mYellowOverlayTileMapObject.lock();

    if (!YellowOverlayObj)
        return false;

    OutTileMap = YellowOverlayObj->GetTileMap().lock();

    return OutTileMap != nullptr;
}

void CPlacementAreaObject::UpdatePrimaryOverlayTiles(
    const std::vector<int>& NextIndices)
{
    // 배치 본영역(파랑 오버레이) 동기화.
    std::shared_ptr<CTileMapComponent> BlueOverlayTileMap;

    if (!AcquireBlueOverlayTileMap(BlueOverlayTileMap))
    {
        // 오버레이 맵이 없으면 적용 목록만 비워 누수 방지.
        mAppliedPrimaryOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GPrimaryOverlayState,
        BlueOverlayTileMap,
        mAppliedPrimaryOverlayIndices,
        NextIndices,
        FVector4::Blue);
}

void CPlacementAreaObject::UpdateMarkerOverlayTiles(
    const std::vector<int>& NextIndices)
{
    // 마커 영역(노랑 오버레이) 동기화.
    std::shared_ptr<CTileMapComponent> YellowOverlayTileMap;

    if (!AcquireYellowOverlayTileMap(YellowOverlayTileMap))
    {
        mAppliedMarkerOverlayIndices.clear();
        return;
    }

    UpdateOverlayTileRefs(
        GMarkerOverlayState,
        YellowOverlayTileMap,
        mAppliedMarkerOverlayIndices,
        NextIndices,
        FVector4(1.f, 1.f, 0.f, 1.f));
}

bool CPlacementAreaObject::BuildDiamondAreaIndices(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, std::vector<int>& OutIndices) const
{
    // 중심 타일을 기준으로 "다이아(마름모) 모양" 영역 인덱스를 만든다.
    OutIndices.clear();

    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return false;

    const int CountX = TileMap->GetTileCountX();
    const int CountY = TileMap->GetTileCountY();
    const int CenterX = CenterTile->GetIndexX();
    const int CenterY = CenterTile->GetIndexY();

    // Radius=1이면 중심+주변 1단계, Radius=2면 2단계...
    const int Radius = mTemplate.DiamondRadius;

    // 아이소메트릭 오프셋 격자라서 탐색 범위를 반경*2 정도로 넉넉히 잡는다.
    const int SearchRange = Radius * 2;

    // "논리 좌표(Logical)"로 바꿔서 거리 계산한다.
    // 홀수 줄(y)은 x가 0.5만큼 밀려 있으므로 이를 보정해야
    // 다이아 경계가 자연스럽게 나온다.
    const float CenterLogicalX = CenterX +
        (CenterY % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterY * 0.5f;
    const float DiamondRadius = (float)Radius;

    // 사각 범위를 훑으면서, 논리 맨해튼 거리로 다이아 내부만 채택한다.
    for (int y = CenterY - SearchRange; y <= CenterY + SearchRange; ++y)
    {
        if (y < 0 || y >= CountY)
            continue;

        for (int x = CenterX - SearchRange; x <= CenterX + SearchRange; ++x)
        {
            if (x < 0 || x >= CountX)
                continue;

            const float LogicalX = x + (y % 2 == 0 ? 0.f : 0.5f);
            const float LogicalY = y * 0.5f;
            const float DistX = fabs(LogicalX - CenterLogicalX);
            const float DistY = fabs(LogicalY - CenterLogicalY);

            // |dx| + |dy| <= 반경 이면 다이아 내부.
            if (DistX + DistY > DiamondRadius)
                continue;

            // 2D 좌표(x,y)를 1D 인덱스로 변환.
            OutIndices.push_back(y * CountX + x);
        }
    }

    // 템플릿 옵션: 방향에 따라 타일 1칸을 비워 "입구"처럼 만든다.
    if (mTemplate.HasDirectionalGap)
    {
        const int OpenTileIndex = FindPreviewOpenTileIndex(
            TileMap, CenterIndex, OutIndices);

        if (OpenTileIndex >= 0)
        {
            auto OpenIt = std::find(
                OutIndices.begin(), OutIndices.end(), OpenTileIndex);

            // 찾은 빈칸 타일을 최종 영역에서 제거.
            if (OpenIt != OutIndices.end())
                OutIndices.erase(OpenIt);
        }
    }

    return !OutIndices.empty();
}

int CPlacementAreaObject::FindPreviewOpenTileIndex(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex,
    const std::vector<int>& CandidateIndices) const
{
    // 비울 타일(입구 위치)을 현재 회전 방향(mPreviewDirection)으로 결정한다.
    if (!TileMap || CandidateIndices.empty())
        return -1;

    // 0: 아래, 1: 오른쪽, 2: 위, 3: 왼쪽
    // 논리 좌표 기준이 아닌 화면(아이소메트릭) 방향 기준으로 오프셋 설정.
    // 화면 아래 = 논리 (+0.5, +0.5), 오른쪽 = (+0.5, -0.5) 방향.
    static const FPlacementMarkerAnchor GOpenOffsets[4] =
    {
        {  0.5f,  0.5f },   // 0: 화면 아래
        {  0.5f, -0.5f },   // 1: 화면 오른쪽
        { -0.5f, -0.5f },   // 2: 화면 위
        { -0.5f,  0.5f },   // 3: 화면 왼쪽
    };

    const int Dir = (mPreviewDirection % 4 + 4) % 4;
    const FPlacementMarkerAnchor& OpenOffset = GOpenOffsets[Dir];

    // 중심에서 목표 오프셋에 가장 가까운 "영역 외곽 타일"을 반환.
    return FindMarkerTileIndexByLogicalOffset(
        TileMap,
        CenterIndex,
        CandidateIndices,
        OpenOffset.LogicalOffsetX,
        OpenOffset.LogicalOffsetY);
}

bool CPlacementAreaObject::IsAreaPlaceable(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices) const
{
    // 영역 전체가 비어 있거나(현재 오브젝트가 이미 점유한 칸 제외)
    // 유효 타일이어야 배치 가능.
    if (Indices.empty())
        return false;

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            return false;

        // 다른 오브젝트가 점유한 이동불가 타일이면 배치 불가.
        // 단, 내가 기존에 점유한 칸으로 "이동 배치"하는 경우는 허용.
        if (Tile->GetType() == ETileType::UnableToMove &&
            !IsPlacedIndex(Indices[i]))
        {
            return false;
        }
    }

    return true;
}

void CPlacementAreaObject::SetAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    const std::vector<int>& Indices, const FVector4& Color)
{
    // 전달된 타일 목록에 동일 색상을 일괄 적용.
    for (size_t i = 0; i < Indices.size(); ++i)
    {
        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        Tile->SetOutLineColor(Color);
    }
}

bool CPlacementAreaObject::IsPlacedIndex(int Index) const
{
    // 현재 확정 배치 타일 목록에 Index가 있는지 선형 탐색.
    for (size_t i = 0; i < mPrimaryPlacedIndices.size(); ++i)
    {
        if (mPrimaryPlacedIndices[i] == Index)
            return true;
    }

    return false;
}

int CPlacementAreaObject::FindMarkerTileIndexByLogicalOffset(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex, const std::vector<int>& Indices,
    float TargetOffsetX, float TargetOffsetY) const
{
    // 중심에서 (TargetOffsetX, TargetOffsetY) 방향으로
    // 가장 적합한 "외곽 타일"을 찾는다.
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return -1;

    const float CenterLogicalX = CenterTile->GetIndexX() +
        (CenterTile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
    const float CenterLogicalY = CenterTile->GetIndexY() * 0.5f;
    const float TargetLogicalX = CenterLogicalX + TargetOffsetX;
    const float TargetLogicalY = CenterLogicalY + TargetOffsetY;

    int BestIndex = -1;
    float BestScore = FLT_MAX;
    auto IsInArea = [&](int TileIndex)
    {
        // 후보 영역(Indices) 포함 여부 체크 람다.
        return std::find(Indices.begin(), Indices.end(), TileIndex) !=
            Indices.end();
    };

    for (size_t i = 0; i < Indices.size(); ++i)
    {
        const int CandidateIndex = Indices[i];
        bool IsEdge = false;

        for (int Dir = 0; Dir < 8; ++Dir)
        {
            const int Neighbor = GetIsoNeighborIndexByDir(
                TileMap, CandidateIndex, Dir);

            // 8방향 중 하나라도 영역 밖이면 외곽 타일로 본다.
            if (Neighbor < 0 || !IsInArea(Neighbor))
            {
                IsEdge = true;
                break;
            }
        }

        if (!IsEdge)
            continue;

        auto Tile = TileMap->GetTile(Indices[i]).lock();

        if (!Tile)
            continue;

        // 목표 논리 좌표와의 맨해튼 거리를 점수로 사용.
        const float LogicalX = Tile->GetIndexX() +
            (Tile->GetIndexY() % 2 == 0 ? 0.f : 0.5f);
        const float LogicalY = Tile->GetIndexY() * 0.5f;
        const float Score = fabs(LogicalX - TargetLogicalX) +
            fabs(LogicalY - TargetLogicalY);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = Indices[i];
        }
    }

    return BestIndex;
}

void CPlacementAreaObject::ApplyPlacedAreaColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap)
{
    // 확정 배치 영역은 기본적으로 흰색 라인으로 유지하고,
    // 별도 오버레이 타일맵(파랑)에 같은 인덱스를 동기화한다.
    SetAreaColor(TileMap, mPrimaryPlacedIndices, FVector4::White);
    UpdatePrimaryOverlayTiles(mPrimaryPlacedIndices);

    mMarkerTileIndices.clear();

    if (mPlacedCenterIndex < 0 ||
        mPrimaryPlacedIndices.empty())
    {
        // 배치가 없다면 마커 오버레이도 비운다.
        UpdateMarkerOverlayTiles(std::vector<int>());
        return;
    }

    auto AddMarkerUnique = [&](int MarkerIndex)
    {
        if (MarkerIndex < 0)
            return;

        if (!IsPlacedIndex(MarkerIndex))
            return;

        if (std::find(mMarkerTileIndices.begin(),
            mMarkerTileIndices.end(),
            MarkerIndex) != mMarkerTileIndices.end())
        {
            return;
        }

        mMarkerTileIndices.push_back(MarkerIndex);
    };

    // 템플릿 마커 앵커를 기준으로 네비게이션 goal 타일을 계산한다.
    // mPreviewDirection(0~3)에 맞춰 논리 오프셋을 회전 적용한다.
    const int Rotation = (mPreviewDirection % 4 + 4) % 4;

    for (size_t i = 0; i < mTemplate.MarkerAnchors.size(); ++i)
    {
        float OffsetX = mTemplate.MarkerAnchors[i].LogicalOffsetX;
        float OffsetY = mTemplate.MarkerAnchors[i].LogicalOffsetY;

        for (int r = 0; r < Rotation; ++r)
        {
            const float PrevX = OffsetX;
            OffsetX = OffsetY;
            OffsetY = -PrevX;
        }

        int MarkerIndex = -1;

        if (fabs(OffsetX) <= 0.001f &&
            fabs(OffsetY) <= 0.001f &&
            IsPlacedIndex(mPlacedCenterIndex))
        {
            MarkerIndex = mPlacedCenterIndex;
        }
        else
        {
            MarkerIndex = FindMarkerTileIndexByLogicalOffset(
                TileMap,
                mPlacedCenterIndex,
                mPrimaryPlacedIndices,
                OffsetX,
                OffsetY);
        }

        AddMarkerUnique(MarkerIndex);
    }

    if (mMarkerTileIndices.empty() &&
        !mPrimaryPlacedIndices.empty())
    {
        // 앵커 계산 실패 시 중심을 우선 사용한다.
        if (IsPlacedIndex(mPlacedCenterIndex))
            mMarkerTileIndices.push_back(mPlacedCenterIndex);
    }

    if (mMarkerTileIndices.empty() &&
        !mPrimaryPlacedIndices.empty())
    {
        // 중심 사용도 실패하면 보조 인덱스를 사용한다.
        const int FallbackEdgeIndex = FindMarkerTileIndexByLogicalOffset(
            TileMap, mPlacedCenterIndex, mPrimaryPlacedIndices, 0.f, 0.f);

        AddMarkerUnique(FallbackEdgeIndex);
    }

    UpdateMarkerOverlayTiles(mMarkerTileIndices);
}

void CPlacementAreaObject::RestoreTileColor(
    const std::shared_ptr<class CTileMapComponent>& TileMap, int Index)
{
    // 프리뷰를 지울 때 "원래 보여야 할 색"으로 복원한다.
    auto Tile = TileMap->GetTile(Index).lock();

    if (!Tile)
        return;

    if (HasOverlayRef(GPrimaryOverlayState, Index) ||
        HasOverlayRef(GMarkerOverlayState, Index))
    {
        // 오버레이 참조가 남아 있으면 흰색(기본 라인 표시)으로 유지.
        Tile->SetOutLineColor(FVector4::White);
    }

    // 오버레이 참조가 없을 때는 타일 타입 기준으로 복원.
    else if (Tile->GetType() == ETileType::UnableToMove)
        Tile->SetOutLineColor(FVector4::Blue);

    else
        Tile->SetOutLineColor(FVector4::White);
}

void CPlacementAreaObject::SyncWorldPosFromCenter(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int CenterIndex)
{
    // 게임 오브젝트의 월드 위치를 "중심 타일의 월드 중심"에 맞춘다.
    auto CenterTile = TileMap->GetTile(CenterIndex).lock();

    if (!CenterTile)
        return;

    auto TileMapObj = mTileMapObject.lock();

    if (!TileMapObj)
        return;

    const FVector2 Center = CenterTile->GetCenter();
    const FVector3 TileMapWorldPos = TileMapObj->GetWorldPos();

    // 현재 z 값은 유지하고 x/y만 타일 중심에 동기화.
    SetWorldPos(Center.x + TileMapWorldPos.x,
        Center.y + TileMapWorldPos.y, GetWorldPos().z);
}

int CPlacementAreaObject::GetIsoNeighborIndexByDir(
    const std::shared_ptr<class CTileMapComponent>& TileMap,
    int TileIndex, int DirIndex) const
{
    // 아이소메트릭 타일은 일반 직교 격자처럼 단순 x/y +/-가 아니어서
    // 보조 격자(GridX/GridY)로 변환 후 8방향 이웃을 계산한다.
    if (!TileMap || DirIndex < 0 || DirIndex >= 8)
        return -1;

    auto Tile = TileMap->GetTile(TileIndex).lock();

    if (!Tile)
        return -1;

    const int x = Tile->GetIndexX();
    const int y = Tile->GetIndexY();

    // 타일 좌표 -> 축이 기울어진 보조 격자 좌표 변환.
    const int GridX = x + ((y + (y & 1)) / 2);
    const int GridY = x - (y / 2);

    // 시계 방향 8방향 벡터.
    const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int NextGridX = GridX + DirX[DirIndex];
    const int NextGridY = GridY + DirY[DirIndex];

    // 보조 격자 -> 타일 좌표 역변환.
    const int NextY = NextGridX - NextGridY;

    if (NextY < 0 || NextY >= TileMap->GetTileCountY())
        return -1;

    const int NextX = NextGridY + (NextY / 2);

    if (NextX < 0 || NextX >= TileMap->GetTileCountX())
        return -1;

    // 최종 1차원 인덱스 반환.
    return NextY * TileMap->GetTileCountX() + NextX;
}
