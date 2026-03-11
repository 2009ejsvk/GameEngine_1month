#include "Navigation.h"
#include "../Component/TileMapComponent.h"
#include "../Object/GameObject.h"
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <limits>

/*
 * ============================================================================
 * [파일 안내 - C++ 비전공자용]
 * ============================================================================
 * 이 파일은 "길찾기(Path Finding)" 전용 코드입니다.
 * 캐릭터가 시작점에서 목적지까지 어떤 타일을 밟아 이동할지 계산합니다.
 *
 * 핵심 알고리즘:
 * 1) JPS(Jump Point Search) : 빠르지만 특정 상황에서 실패할 수 있음
 * 2) Fallback A*            : 느리지만 안정적인 백업 알고리즘
 *
 * 자주 등장하는 C++ 문법/타입:
 * - const T&   : "읽기 전용 참조". 복사 비용 없이 안전하게 읽기만 함.
 * - T*         : 포인터. 객체의 "주소"를 들고 다님. nullptr이면 비어 있음.
 * - std::weak_ptr<T>
 *   : 소유권이 없는 약한 참조. lock()으로 std::shared_ptr<T>를 얻어 실제 사용.
 * - std::vector<T> : 동적 배열(인덱스 접근 빠름)
 * - std::list<T>   : 연결 리스트(앞/뒤 삽입 유리)
 * - lambda ([&](...) { ... })
 *   : 함수 안에서 잠깐 쓰는 "익명 함수"
 *
 * 전체 흐름:
 * SetTileMap() -> 타일맵을 노드 배열로 변환
 * FindPath()   -> 장애물/목표 설정 + JPS 실행
 * BuildPath()  -> Parent 체인을 실제 이동 경로 점 목록으로 복원
 * (실패 시 FindPathFallbackAStar() 호출)
 */

// ── 이름 없는 네임스페이스 (이 파일 안에서만 쓸 수 있는 전역 함수/상수) ──
namespace
{
#ifdef _DEBUG
	// [디버그 로그] Visual Studio 출력 창에 길찾기 과정을 기록하는 함수.
	// 릴리즈 빌드에서는 존재하지 않음 (#ifdef _DEBUG).
	void DebugPathLog(const char* Format, ...)
	{
		char Text[512] = {};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Text, Format, Args);
		va_end(Args);

		OutputDebugStringA(Text);
	}
#endif

	// [8방향 이동 벡터]
	// Skew Grid 좌표계에서 8방향으로 이동할 때의 X축 변화량.
	// 인덱스 순서: T(위)=0, RT(오른쪽위)=1, R(오른쪽)=2, RB(오른쪽아래)=3,
	//              B(아래)=4, LB(왼쪽아래)=5, L(왼쪽)=6, LT(왼쪽위)=7
	constexpr int DIR_DX[ESearchDir::End] =
	{
		0, 1, 1, 1, 0, -1, -1, -1
	};

	// Y축 변화량. DIR_DX와 짝지어 사용.
	// 예) 방향 RT: (dx=+1, dy=+1) = 오른쪽 위 대각선
	constexpr int DIR_DY[ESearchDir::End] =
	{
		1, 1, 0, -1, -1, -1, 0, 1
	};

	constexpr float GNavRoadTileWeight = 1.f;
	constexpr float GNavOffRoadTileWeight = 3.f;
}

CNavigation::CNavigation()
{
}

CNavigation::~CNavigation()
{
}

// ════════════════════════════════════════════════════════════════════════════
// SetTileMap — 타일맵 데이터를 읽어 길찾기용 노드 배열 생성
// ════════════════════════════════════════════════════════════════════════════

/*
 * [SetTileMap] 타일맵 컴포넌트를 받아 탐색용 노드 배열(mNodeList)을 구성합니다.
 *
 * 각 타일 한 칸 = 길찾기 노드 한 개.
 * 아이소메트릭 타일의 경우, ★Skew Grid 변환★이 여기서 수행됩니다.
 */
void CNavigation::SetTileMap(
	const std::weak_ptr<CTileMapComponent>& TileMap)
{
	// weak_ptr을 멤버로 저장해 "타일맵 수명"을 강제로 늘리지 않는다.
	mTileMap = TileMap;

	// lock() 성공 시 shared_ptr 획득(실제 객체 접근 가능), 실패 시 null.
	auto OriginMap = TileMap.lock();

	if (!OriginMap)
		return;

	// 타일맵의 기본 정보를 "멤버 변수"로 복사해 둔다.
	// 이유:
	// 1) 탐색 중에 매번 TileMap->Get...()를 호출하지 않아도 되어 코드가 단순해짐.
	// 2) 좌표 변환/경계 체크에 쓰는 기준값을 "한 묶음"으로 고정.
	//    예: 탐색 중간에 맵 크기(CountX/CountY)나 모양(Shape)이 바뀌면
	//    같은 Grid 좌표라도 어떤 함수는 "맵 안", 다른 함수는 "맵 밖"으로
	//    다르게 판정할 수 있어 잘못된 인덱스/경로가 생길 수 있음.
	//    그래서 SetTileMap 시점 값을 스냅샷처럼 잡아두고 계산 기준을 맞춘다.
	// 3) 이후 함수들(StepIndex, GetIndexByGrid 등)이 TileMap에 직접 의존하지 않고
	//    mShape/mCountX/mCountY/mTileSize만으로 계산 가능.
	mShape = OriginMap->GetTileShape();
	mCountX = OriginMap->GetTileCountX();  // X축 타일 개수(가로 칸 수)
	mCountY = OriginMap->GetTileCountY();  // Y축 타일 개수(세로 칸 수)
	mTileSize = OriginMap->GetTileSize();  // 타일 1칸 크기(노드 크기 계산 기준)

	// 전체 타일 수 = 가로 칸 수 * 세로 칸 수
	int	Count = mCountX * mCountY;

	// resize: 실제 노드 개수만큼 배열 크기를 맞춘다.
	// reserve: push/emplace가 자주 일어나는 컨테이너는 미리 용량 확보.
	mNodeList.resize((size_t)Count);
	mOpenList.reserve((size_t)Count);
	mUseList.reserve((size_t)Count);

	// 타일맵의 모든 칸을 1번씩 돌며 "타일 -> 탐색 노드"로 변환한다.
	for (int i = 0; i < Count; ++i)
	{
		// 타일 객체를 가져오고, 이미 해제되었으면 이 칸은 건너뛴다.
		auto Tile = OriginMap->GetTile(i).lock();

		if (!Tile)
			continue;

		// "참조(&)" = 별명(alias) 개념.
		// Node는 mNodeList[i]의 복사본이 아니라 "원본 그 자체"를 가리킨다.
		// 따라서 Node.Pos = ... 를 하면 mNodeList[i].Pos가 바로 바뀐다.
		FNavNode& Node = mNodeList[i];
		Node.Pos = Tile->GetPos();       // 타일맵 기준(로컬) 좌표
		Node.Size = mTileSize;           // 타일 크기(충돌/거리 계산 기준)
		Node.Center = Tile->GetCenter(); // 타일 중심점(거리 계산용)

		// 여기서 좌표계가 2개 나온다:
		// 1) 로컬 좌표: "타일맵 내부" 기준 좌표 (타일맵 원점이 0,0)
		// 2) 월드 좌표: "게임 세계 전체" 기준 좌표
		//
		// 예) 타일 중심이 로컬 (10, 20), 타일맵 오브젝트가 월드 (300, 100)에 놓였으면
		//     실제 월드 중심은 (310, 120).
		// 그래서 Owner(타일맵을 가진 게임오브젝트)의 월드 위치를 더해
		// Center를 "월드 기준"으로 맞춘다.
		// 그래야 거리 계산(경로 비용)이 화면/월드 위치와 일치한다.
		auto Owner = OriginMap->GetOwner().lock();

		if (Owner)
		{
			const FVector3 OwnerWorldPos = Owner->GetWorldPos();
			Node.Center.x += OwnerWorldPos.x;
			Node.Center.y += OwnerWorldPos.y;
		}
		// Owner가 없으면(드문 예외 상황) 로컬 중심값을 그대로 사용한다.
		Node.Index = i;
		Node.IndexX = Tile->GetIndexX();  // 타일맵 내 가로 번호
		Node.IndexY = Tile->GetIndexY();  // 타일맵 내 세로 번호

		// [사각형 타일] 그리드 좌표 = 타일 좌표 그대로
		if (mShape == ETileShape::Rect)
		{
			Node.GridX = Node.IndexX;
			Node.GridY = Node.IndexY;
		}

		else
		{
			// ★★★ [핵심] Skew Grid 변환 ★★★
			// 아이소메트릭(지그재그) 좌표 → 직교 그리드 좌표로 변환.
			//
			// 왜 필요한가?
			// JPS는 "한 방향으로 쭉 직진"해야 하는데,
			// 지그재그 배치에서는 홀수/짝수 행마다 이웃 규칙이 달라서
			// 직진할 수 없음. 변환하면 8방향 이동이 항상 균일한 (dx,dy).
			//
			// 계산식 해석:
			// 1) GridX = IndexX + ceil(IndexY / 2)
			//    코드의 ((IndexY + (IndexY & 1)) / 2)는 정수 연산으로 ceil(y/2)를 만든 것.
			//    - y=0 -> (0+0)/2=0
			//    - y=1 -> (1+1)/2=1
			//    - y=2 -> (2+0)/2=1
			//    - y=3 -> (3+1)/2=2
			//    즉, 행(y)이 내려갈수록 GridX를 반 칸씩 누적 보정(홀수 행에서 +1 추가)한다.
			//
			// 2) GridY = IndexX - floor(IndexY / 2)
			//    C++ 정수 나눗셈(y/2)은 floor와 동일(양수 기준).
			//    y=0,1일 때 0 / y=2,3일 때 1 / y=4,5일 때 2 ...
			//
			// 3) 왜 +ceil, -floor 조합인가?
			//    이 파일의 역변환(GetIndexByGrid)과 정확히 짝이 맞도록 만든 식이다.
			//    역변환은:
			//      IndexY = GridX - GridY
			//      IndexX = GridY + floor(IndexY / 2)
			//    위 두 식을 IndexX/IndexY 기준으로 풀면
			//      GridX = IndexX + ceil(IndexY / 2)
			//      GridY = IndexX - floor(IndexY / 2)
			//    가 되어 현재 코드와 동일해진다.
			//
			// (IndexY & 1) : 홀수행이면 1, 짝수행이면 0.
			// → 홀수행일 때 ceil 보정을 만들기 위한 비트 연산.
			Node.GridX = Node.IndexX +
				((Node.IndexY + (Node.IndexY & 1)) / 2);
			Node.GridY = Node.IndexX -
				(Node.IndexY / 2);
		}
	}
}

// ════════════════════════════════════════════════════════════════════════════
// FindPath — JPS 메인 함수 (경로 탐색의 시작점)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [FindPath] 시작 좌표(Start)에서 끝 좌표(End)까지의 경로를 계산합니다.
 *
 * 매개변수:
 *   Start, End          → 월드 좌표 (예: NPC위치, 건물위치)
 *   PathList (출력)     → 결과 경로 점들의 목록
 *   BlockMask           → "이 타일은 못 간다" 비트마스크 (외부 제공)
 *   GoalIndices/GoalCount → "이 타일에 도착하면 성공" 목록 (외부 제공)
 *
 * 반환값: true = 경로 찾음, false = 경로 없음
 *
 * 동작 순서:
 *   1) 장애물 마스크 준비
 *   2) 시작/목표 노드 유효성 검사
 *   3) JPS 탐색 루프 (Open List 기반)
 *   4) JPS 실패 시 → Fallback A* 시도
 */
bool CNavigation::FindPath(const FVector3& Start, const FVector3& End,
	std::list<FVector3>& PathList,
	const unsigned char* BlockMask,
	int BlockMaskByteCount,
	const int* GoalIndices,
	int GoalCount)
{
	// lock()으로 현재 프레임에서 유효한 타일맵 객체를 잠깐 사용한다.
	auto TileMap = mTileMap.lock();

	if (!TileMap)
	{
		PathList.clear();
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: tile map missing\n");
#endif
		return false;
	}

	// 유효한 탐색 범위(전체 타일 개수).
	const int TileCount = mCountX * mCountY;

	if (TileCount <= 0)
	{
		PathList.clear();
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: invalid tile count=%d\n", TileCount);
#endif
		return false;
	}

	// "타일 개수 비트"를 담기 위해 필요한 "바이트 개수" 계산.
	// 예) 타일 10개면 비트 10개 필요 -> 바이트 2개.
	const int MaskByteCount = (TileCount + 7) / 8;

	// 비트마스크(Bitmask) = "여러 개의 true/false 상태"를 비트(0/1)로 압축 저장하는 방식.
	// 여기서는 "타일 1개당 1비트"를 쓴다.
	// - 1비트 = 막힘(통과 불가)
	// - 0비트 = 통과 가능
	// 즉 mBlockedMask는 변수명이고, "비트마스크"는 데이터 표현 방식 이름이다.
	//
	// assign(MaskByteCount, 0):
	// - 필요한 바이트 수만큼 배열 크기 맞춤
	// - 모든 비트를 0으로 초기화(처음에는 전부 통과 가능 상태)
	mBlockedMask.assign((size_t)MaskByteCount, 0);

	// 비트마스크 생성 알고리즘(2가지 입력 경로):
	// A) 외부(BlockMask)에서 이미 만들어준 마스크가 있으면 "그대로 복사"
	// B) 없으면 TileMap의 타일 타입을 순회해서 내부에서 직접 생성
	//
	// 왜 이렇게 나누나?
	// - 서버/상위 시스템이 이미 계산한 장애물 정보를 재사용하면 빠르고 일관됨(A)
	// - 외부 정보가 없을 때도 엔진 단독으로 동작 가능(B)
	if (BlockMask && BlockMaskByteCount > 0)
	{
		// 외부에서 미리 만든 장애물 마스크가 있으면 복사해서 사용.
		// CopySize = 둘 중 더 작은 값(안전한 최소 길이).
		// - MaskByteCount      : "우리 쪽 버퍼 크기"
		// - BlockMaskByteCount : "외부에서 준 데이터 크기"
		// 큰 쪽 기준으로 복사하면 버퍼 오버런 위험이 있으므로 min 사용.
		const int CopySize = MaskByteCount < BlockMaskByteCount ?
			MaskByteCount : BlockMaskByteCount;

		if (CopySize > 0)
		{
			// memcpy: 바이트 단위로 빠르게 그대로 복사.
			// 복사 후 의미:
			// - 복사된 1비트 = 막힘
			// - 복사된 0비트 = 통과 가능
			memcpy(&mBlockedMask[0], BlockMask, (size_t)CopySize);
		}

		// 참고: 외부 데이터가 더 짧으면 남은 바이트는 0(통과 가능) 상태 유지.
		//      (위에서 assign(0) 초기화를 이미 했기 때문)
	}

	else
	{
		// 외부 마스크가 없으면, 타일맵 자체의 "이동 불가" 정보로 생성
		for (int i = 0; i < TileCount; ++i)
		{
			if (TileMap->GetTileType(i) != ETileType::UnableToMove)
				continue;

			// i번째 타일을 막힘으로 표시:
			// - i/8     : 몇 번째 바이트인지
			// - i%8     : 그 바이트 안에서 몇 번째 비트인지(0~7)
			// - 1 << ...: 해당 비트 위치만 1인 마스크 생성
			// - |=      : 기존 비트는 유지하고 해당 비트만 1로 켜기
			//
			// 예) i=13
			//   byteIndex = 13/8 = 1   (두 번째 바이트)
			//   bitIndex  = 13%8 = 5   (6번째 비트)
			//   mBlockedMask[1]의 bit5를 1로 설정
			mBlockedMask[i / 8] |= (unsigned char)(1 << (i % 8));
		}
	}

	// 시작 지점의 타일 인덱스를 구한다.
	int StartIndex = TileMap->GetTileIndex(Start);

	if (StartIndex < 0 || StartIndex >= TileCount)
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Fail: invalid start index=%d start=(%.1f, %.1f)\n",
			StartIndex, Start.x, Start.y);
#endif
		return false;
	}

	// 도착 좌표의 타일 인덱스(막힌 타일일 수 있어 이후 보정 가능).
	int EndIndex = TileMap->GetTileIndex(End);

	// 시작 인덱스는 멤버에 저장(충돌 검사 시 시작칸 예외 처리에 사용).
	mStartIndex = StartIndex;
	// 목표 마스크/목표 목록은 이번 탐색 기준으로 새로 만든다.
	mGoalMask.assign((size_t)MaskByteCount, 0);
	mGoalIndices.clear();

	// 람다 함수:
	// - FindPath 내부에서만 쓰는 "목표 등록용 미니 함수"
	// - [&](...)는 바깥 변수(mGoalMask, mGoalIndices 등)를 참조로 캡처
	auto SetGoal = [&](int Index)
	{
		// 맵 범위를 벗어난 인덱스는 무시.
		if (Index < 0 || Index >= TileCount)
			return;

		// Goal 비트마스크에서 Index가 속한 비트 위치 계산.
		const unsigned char Bit = (unsigned char)(1 << (Index % 8));
		unsigned char& Byte = mGoalMask[Index / 8];

		// 이미 같은 목표가 등록돼 있으면 중복 추가하지 않는다.
		if (Byte & Bit)
			return;

		// 목표 비트 ON + 목표 목록에 기록.
		Byte |= Bit;
		mGoalIndices.emplace_back(Index);

		// 목표 타일은 탐색 불가 마스크에서 제외한다.
		// (= 목표칸이 원래 막힘으로 표기돼 있어도 "도착 자체"는 허용)
		// ~Bit : 해당 위치만 0, 나머지는 1
		// &=   : 해당 비트만 강제로 0으로 내림
		mBlockedMask[Index / 8] &= (unsigned char)~Bit;
	};

	if (GoalIndices && GoalCount > 0)
	{
		// 외부에서 여러 목표를 넘겨주면 모두 목표로 등록.
		for (int i = 0; i < GoalCount; ++i)
		{
			SetGoal(GoalIndices[i]);
		}
	}

	else
	{
		// 목표 목록을 받지 못한 경우:
		// End 타일이 막혀 있으면 주변 칸 중 대체 목표를 찾는다.
		EndIndex = FindFallbackGoalIndex(StartIndex, EndIndex);

		if (EndIndex < 0)
		{
#ifdef _DEBUG
			DebugPathLog(
				"[Path] Fail: fallback goal missing start=%d end=%d\n",
				StartIndex, TileMap->GetTileIndex(End));
#endif
			return false;
		}

		SetGoal(EndIndex);
	}

	// 최종적으로 유효한 목표가 하나도 없으면 탐색 불가.
	if (mGoalIndices.empty())
	{
#ifdef _DEBUG
		DebugPathLog(
			"[Path] Fail: goal list empty start=%d end=%d inputGoalCount=%d blockMaskBytes=%d\n",
			StartIndex,
			TileMap->GetTileIndex(End),
			GoalCount,
			BlockMaskByteCount);
#endif
		return false;
	}

	// 결과 경로는 새 탐색 결과로 덮어쓰므로 먼저 비운다.
	PathList.clear();

#ifdef _DEBUG
	DebugPathLog(
		"[Path] Start start=%d end=%d goals=%zu blockMaskBytes=%d\n",
		StartIndex,
		TileMap->GetTileIndex(End),
		mGoalIndices.size(),
		(int)mBlockedMask.size());
#endif

	// ── 이전 탐색에서 사용했던 노드를 모두 초기 상태로 되돌린다 ──
	// (매 탐색마다 전체 배열을 초기화하면 느리니까,
	//  "실제로 건드린 노드만" 기억해뒀다가 그것만 초기화)
	// size_t: 컨테이너 크기/인덱스에 주로 쓰는 부호 없는 정수 타입.
	size_t	UseSize = mUseList.size();

	for (size_t i = 0; i < UseSize; ++i)
	{
		FNavNode* Used = mUseList[i];
		Used->NodeType = ENavNodeType::None;   // 미탐색 상태로
		Used->Dist = FLT_MAX;                  // 거리 무한대로
		Used->Huristic = FLT_MAX;              // 휴리스틱 무한대로
		Used->Total = FLT_MAX;                 // 총 비용 무한대로
		Used->Parent = nullptr;                // 부모 연결 끊기
		Used->SearchDirList.clear();           // 탐색 방향 목록 비우기
	}

	// 초기화 완료 후, "사용 목록/오픈 목록" 컨테이너를 비운다.
	mUseList.clear();
	mOpenList.clear();

	// ── 시작 노드 설정 ──
	FNavNode* StartNode = &mNodeList[StartIndex];

	// 시작점이 이미 목표라면 → 이동 필요 없음. 바로 성공.
	if (IsGoalIndex(StartIndex))
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Start already goal index=%d\n", StartIndex);
#endif
		return true;
	}

	if (!mUseJumpPointSearch)
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Weighted pathing active. using A* only\n");
#endif
		return FindPathFallbackAStar(StartIndex, PathList);
	}

	// 시작 노드를 Open 상태로 설정
	// Open = "아직 주변을 탐색하지 않은 후보 노드"
	//
	// JPS/A* 공통 점수 의미:
	// - Dist     : 시작점 -> 현재 노드까지 실제 누적 거리 (g)
	// - Huristic : 현재 노드 -> 목표까지의 추정 거리 (h)
	// - Total    : g + h (우선순위 점수, 작을수록 유망)
	StartNode->NodeType = ENavNodeType::Open;
	StartNode->Dist = 0.f;                              // 시작점이므로 거리 = 0
	StartNode->Huristic = ComputeHeuristic(StartIndex);  // 목표까지의 예상 거리
	StartNode->Total = StartNode->Huristic;              // 총 비용 = 거리 + 예상

	// 시작 노드는 8방향 전부를 탐색 방향으로 등록 (아직 어디서 왔는지 모르니까)
	for (int i = 0; i < ESearchDir::End; ++i)
	{
		StartNode->SearchDirList.emplace_back((ESearchDir::Type)i);
	}

	// OpenList  : 아직 확정되지 않은 후보 노드 집합
	// UseList   : 이번 탐색에서 상태가 바뀐 노드 기록(다음 탐색 시작 시 빠른 초기화용)
	mOpenList.emplace_back(StartNode);   // 탐색 후보 목록에 추가
	mUseList.emplace_back(StartNode);    // "건드린 노드" 기록 (나중에 초기화용)

	// ══════════════════════════════════════════════════════════
	// [JPS 메인 루프] Open List가 빌 때까지 반복
	// ══════════════════════════════════════════════════════════
	// 루프 1회 = "가장 유망한 후보 1개 확정 + 그 후보에서 점프 탐색으로 새 후보 생성"
	while (!mOpenList.empty())
	{
		// Open List에서 총 비용(Total)이 가장 작은 노드를 꺼낸다.
		// (배열 뒤쪽이 최소값이 되도록 내림차순 정렬해둠)
		FNavNode* Node = mOpenList.back();
		mOpenList.pop_back();

		// 이미 Close(탐색 완료) 상태면 건너뜀
		// (같은 노드가 OpenList에 중복 들어올 수 있으므로 방어적으로 체크)
		if (Node->NodeType == ENavNodeType::Close)
			continue;

		// 이 노드를 Close로 변경 (= "이 노드는 더 이상 안 봄")
		// 의미: 이 노드까지 오는 최선 경로가 확정되었다고 본다.
		Node->NodeType = ENavNodeType::Close;

		// ★ 목표 도달! → 경로를 역추적하여 결과 생성
		if (IsGoalIndex(Node->Index))
		{
			// 더 이상 탐색할 필요가 없으므로 후보 목록 정리 후 바로 반환.
			mOpenList.clear();
			const bool Built = BuildPath(Node, PathList);
#ifdef _DEBUG
			DebugPathLog(
				"[Path] Goal reached index=%d built=%d pathPoints=%zu\n",
				Node->Index, Built ? 1 : 0, PathList.size());
#endif
			return Built;
		}

		// 이 노드에서 각 방향으로 Jump하여 코너 노드를 찾는다.
		// 핵심: 일반 A*처럼 "바로 옆 8칸"만 보는 게 아니라,
		//      한 방향으로 쭉 전진해서 의미 있는 지점(Jump Point)만 후보로 등록.
		FindNode(Node, PathList);

		// Open List를 총 비용 기준 내림차순 정렬
		// → .back()이 항상 최소 비용 노드가 되도록
		if (mOpenList.size() >= 2)
		{
			std::sort(mOpenList.begin(), mOpenList.end(),
				CNavigation::SortOpenList);
		}
	}

	// ── JPS가 경로를 못 찾은 경우 ──
	mOpenList.clear();
#ifdef _DEBUG
	DebugPathLog("[Path] Fail: open list exhausted start=%d goals=%zu\n",
		StartIndex, mGoalIndices.size());
#endif

	// ★ Fallback: 일반 A*로 재시도
	// JPS가 특수한 맵 구조에서 실패할 수 있으므로, 더 느리지만 확실한 A*를 시도.
	if (FindPathFallbackAStar(StartIndex, PathList))
	{
#ifdef _DEBUG
		DebugPathLog("[Path] Fallback A* success pathPoints=%zu\n",
			PathList.size());
#endif
		return true;
	}

#ifdef _DEBUG
	DebugPathLog("[Path] Fallback A* failed\n");
#endif

	return false;
}

// ════════════════════════════════════════════════════════════════════════════
// FindNode — 한 노드에서 탐색 방향을 결정하고, 각 방향으로 Jump 실행
// ════════════════════════════════════════════════════════════════════════════

/*
 * [FindNode] 현재 노드(Node)에서 탐색해야 할 방향들을 결정한 뒤,
 * 각 방향으로 Jump()을 실행하여 "코너 노드"를 찾습니다.
 * 코너 노드가 발견되면 Open List에 추가하여 이후 탐색 대상이 됩니다.
 */
bool CNavigation::FindNode(FNavNode* Node, std::list<FVector3>& PathList)
{
	// 부모 노드가 있는지, 부모→현재 방향이 무엇인지 확인
	const bool HasParent = Node->Parent != nullptr;
	const ESearchDir::Type ParentDir = GetParentDir(Node);

	// 탐색할 방향 목록을 세팅 (AddDir가 JPS의 가지치기 규칙 적용)
	// - 시작 노드: 8방향 모두
	// - 그 외 노드: 부모 방향/강제이웃 기반으로 필요한 방향만 남김
	AddDir(ParentDir, Node, HasParent);

	// 각 탐색 방향에 대해 Jump 실행
	// 반복자(iterator):
	// begin() ~ end() 구간을 순회하는 C++ 표준 방식.
	auto iter = Node->SearchDirList.begin();
	auto iterEnd = Node->SearchDirList.end();

	for (; iter != iterEnd; ++iter)
	{
		// Jump: 해당 방향으로 쭉 직진하며 "코너"를 찾음
		const int CornerIndex = Jump(Node, *iter);

		if (CornerIndex < 0)    // 코너 없음 (벽이나 맵 끝에 막힘)
			continue;

		FNavNode* Corner = &mNodeList[CornerIndex];

		if (Corner->NodeType == ENavNodeType::Close)  // 이미 탐색 완료된 노드
			continue;

		// 시작점에서 이 코너까지의 실제 이동 거리 계산
		const float Dist = Node->Dist +
			Node->Center.Distance(Corner->Center);

		// [경우 1] 이미 Open에 있는 노드 → 더 짧은 경로 발견 시 업데이트
		if (Corner->NodeType == ENavNodeType::Open)
		{
			if (Corner->Dist > Dist)  // 기존보다 더 짧은 경로?
			{
				// 더 좋은 부모를 찾았으므로 g/f와 Parent를 교체.
				Corner->Dist = Dist;
				Corner->Total = Dist + Corner->Huristic;
				Corner->Parent = Node;  // 부모를 현재 노드로 변경
			}
		}

		// [경우 2] 처음 발견된 노드 → Open List에 새로 추가
		else
		{
			// 처음 발견된 Jump Point라면 점수 계산 후 Open에 넣는다.
			Corner->NodeType = ENavNodeType::Open;
			Corner->Dist = Dist;
			Corner->Huristic = ComputeHeuristic(CornerIndex);
			Corner->Total = Dist + Corner->Huristic;
			Corner->Parent = Node;

			mOpenList.emplace_back(Corner);  // 탐색 후보에 추가
			mUseList.emplace_back(Corner);   // 초기화 목록에 기록
		}
	}

	// 현재 구현에서는 PathList를 이 함수에서 직접 쓰지 않고,
	// Goal 도달 시 BuildPath에서 실제 경로를 만든다.
	return false;
}

// ════════════════════════════════════════════════════════════════════════════
// Jump — JPS의 핵심: 한 방향으로 "코너"가 나올 때까지 직진
// ════════════════════════════════════════════════════════════════════════════

/*
 * [Jump] 주어진 방향(Dir)으로 한 칸씩 전진하며:
 *   1) 목표 도달 → 해당 인덱스 반환
 *   2) 강제이웃(코너) 발견 → 해당 인덱스 반환  
 *   3) 벽/맵 끝 → -1 반환 (이 방향은 막힘)
 *   4) 아무것도 없음 → 재귀로 다음 칸 계속 전진
 *
 * 대각선 이동의 경우, 수평/수직 방향으로도 Jump을 시도하여
 * 그쪽에서 코너가 발견되면 현재 위치를 코너로 보고합니다.
 */
int CNavigation::Jump(FNavNode* Node, ESearchDir::Type Dir)
{
	int NextIndex = -1;

	// 한 칸 전진. 벽이면 -1 반환.
	if (!StepIndex(Node->Index, Dir, NextIndex, true))
		return -1;

	// ★ 목표 타일 도달!
	if (IsGoalIndex(NextIndex))
		return NextIndex;

	// ★ 강제이웃(코너) 발견! → 이 노드가 중요한 분기점
	// (여기서 방향이 갈라질 수 있으므로 Jump Point로 채택)
	if (HasForcedNeighbor(NextIndex, Dir))
		return NextIndex;

	// [대각선 이동] 수평/수직 성분으로 분해하여 각각 Jump 시도
	// 예) 오른쪽위(↗) = 오른쪽(→) + 위(↑) 로 분해
	if (IsDiagonalDir(Dir))
	{
		int dx = 0;
		int dy = 0;

		// Dir이 비정상 값이면 더 진행하지 않고 실패 처리.
		if (!GetMoveDelta(Dir, dx, dy))
			return -1;

		// 수평 방향(→ 또는 ←)으로 코너가 있는지 확인
		const ESearchDir::Type Horizontal = GetDirFromDelta(dx, 0);
		const ESearchDir::Type Vertical = GetDirFromDelta(0, dy);

		if (Horizontal != ESearchDir::End &&
			Jump(&mNodeList[NextIndex], Horizontal) != -1)
		{
			return NextIndex;  // 수평 탐색에서 코너 발견 → 현재 위치가 분기점
		}

		// 수직 방향(↑ 또는 ↓)으로 코너가 있는지 확인
		if (Vertical != ESearchDir::End &&
			Jump(&mNodeList[NextIndex], Vertical) != -1)
		{
			return NextIndex;  // 수직 탐색에서 코너 발견 → 현재 위치가 분기점
		}
	}

	// 아무 이벤트 없음 → 같은 방향으로 다음 칸 계속 점프 (재귀 호출)
	// 재귀(recursion): 함수가 자기 자신을 다시 호출하는 방식.
	// 여기서는 "한 방향 직진"을 간결하게 표현하기 위해 사용.
	// 종료는 아래 3가지 중 하나에서 반드시 걸린다:
	// 1) 벽/맵끝(StepIndex 실패) 2) 목표 도달 3) 강제이웃 발견
	return Jump(&mNodeList[NextIndex], Dir);
}

// ════════════════════════════════════════════════════════════════════════════
// AddDir — JPS 가지치기(Pruning): 탐색할 방향만 남기기
// ════════════════════════════════════════════════════════════════════════════

/*
 * [AddDir] 부모→현재 이동 방향을 기준으로,
 * 이 노드에서 실제로 탐색해야 할 방향만 선별합니다.
 *
 * JPS가 A*보다 빠른 이유의 핵심!
 * 부모를 통해 온 방향과 "강제이웃" 방향만 남기고 나머지는 버립니다.
 *
 * [강제이웃이란?]
 * 벽 옆을 지날 때, 벽 너머로 우회하면 갈 수 있는 대각선 방향.
 * 예) → 로 직진 중, 위에 벽이 있고 오른쪽위가 빈 칸이면
 *     오른쪽위(↗)가 "강제이웃" → 반드시 탐색해야 함.
 */
void CNavigation::AddDir(ESearchDir::Type ParentDir, FNavNode* Node,
	bool HasParent)
{
	Node->SearchDirList.clear();

	// 중복 방향 추가 방지용 배열
	// bool 배열을 false로 초기화: "{}"는 전부 0/false 초기화.
	bool DirAdded[ESearchDir::End] = {};

	// AddUnique도 람다: "중복 체크 + 추가" 패턴을 재사용.
	auto AddUnique = [&](ESearchDir::Type Dir)
	{
		if (Dir == ESearchDir::End)
			return;

		if (DirAdded[Dir])
			return;

		DirAdded[Dir] = true;
		Node->SearchDirList.emplace_back(Dir);
	};

	// 부모가 없으면 (= 시작 노드) → 8방향 전부 탐색
	if (!HasParent)
	{
		for (int i = 0; i < ESearchDir::End; ++i)
		{
			AddUnique((ESearchDir::Type)i);
		}

		return;
	}

	int dx = 0;
	int dy = 0;

	// 부모 방향이 유효하지 않으면 탐색 방향을 만들 수 없다.
	if (!GetMoveDelta(ParentDir, dx, dy))
		return;

	const int gx = Node->GridX;
	const int gy = Node->GridY;

	// ── [대각선으로 온 경우] (dx≠0 이고 dy≠0) ──
	if (dx != 0 && dy != 0)
	{
		// 자연스러운 이웃(Natural Neighbors): 같은 방향 + 수평/수직 성분
		AddUnique(ParentDir);                   // 계속 같은 대각선
		AddUnique(GetDirFromDelta(dx, 0));      // 수평 성분
		AddUnique(GetDirFromDelta(0, dy));      // 수직 성분

		// 강제이웃(Forced Neighbors):
		// 뒤쪽이 벽이고, 벽 너머 대각선이 뚫려있으면 그 방향도 탐색
		if (!IsWalkableGrid(gx - dx, gy) &&       // ← 뒤쪽이 벽?
			IsWalkableGrid(gx - dx, gy + dy))     // ← 벽 너머 대각선은 빈칸?
		{
			AddUnique(GetDirFromDelta(-dx, dy));   // → 강제이웃 방향 추가!
		}

		if (!IsWalkableGrid(gx, gy - dy) &&
			IsWalkableGrid(gx + dx, gy - dy))
		{
			AddUnique(GetDirFromDelta(dx, -dy));
		}
	}

	// ── [수평으로 온 경우] (dx≠0, dy=0) ──
	else if (dx != 0)
	{
		AddUnique(ParentDir);  // 계속 같은 수평 방향

		// 위에 벽 + 오른쪽위 빈칸 → 강제이웃
		if (!IsWalkableGrid(gx, gy + 1) &&
			IsWalkableGrid(gx + dx, gy + 1))
		{
			AddUnique(GetDirFromDelta(dx, 1));
		}

		// 아래에 벽 + 오른쪽아래 빈칸 → 강제이웃
		if (!IsWalkableGrid(gx, gy - 1) &&
			IsWalkableGrid(gx + dx, gy - 1))
		{
			AddUnique(GetDirFromDelta(dx, -1));
		}
	}

	// ── [수직으로 온 경우] (dx=0, dy≠0) ──
	else if (dy != 0)
	{
		AddUnique(ParentDir);  // 계속 같은 수직 방향

		// 오른쪽에 벽 + 오른쪽위 빈칸 → 강제이웃
		if (!IsWalkableGrid(gx + 1, gy) &&
			IsWalkableGrid(gx + 1, gy + dy))
		{
			AddUnique(GetDirFromDelta(1, dy));
		}

		// 왼쪽에 벽 + 왼쪽위 빈칸 → 강제이웃
		if (!IsWalkableGrid(gx - 1, gy) &&
			IsWalkableGrid(gx - 1, gy + dy))
		{
			AddUnique(GetDirFromDelta(-1, dy));
		}
	}
}

// ════════════════════════════════════════════════════════════════════════════
// HasForcedNeighbor — 강제이웃이 존재하는지 검사
// ════════════════════════════════════════════════════════════════════════════

/*
 * [HasForcedNeighbor] 현재 위치(Index)에서 진행 방향(Dir) 기준으로
 * "강제이웃"(벽 너머의 우회 가능한 대각선 칸)이 존재하는지 확인합니다.
 * 강제이웃이 있으면 Jump()이 이 위치를 "코너"로 보고합니다.
 */
bool CNavigation::HasForcedNeighbor(int Index, ESearchDir::Type Dir) const
{
	// Dir을 (dx, dy)로 바꿔 "현재 진행 방향"을 숫자로 얻는다.
	// 예) 오른쪽이면 (1, 0), 오른쪽위면 (1, 1)
	int dx = 0;
	int dy = 0;

	// 잘못된 방향 입력이면 강제이웃도 없음으로 처리.
	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	// 검사 기준이 되는 현재 노드의 Grid 좌표.
	const FNavNode& Node = mNodeList[Index];
	const int gx = Node.GridX;
	const int gy = Node.GridY;

	// [대각선 진행] dx,dy가 둘 다 0이 아님.
	// 대각선에서는 "뒤쪽 직교칸이 막혔는데, 그 뒤 대각선이 열려있는지" 2곳을 본다.
	// 이 상황은 직진 경로만으로는 자연스럽게 도달되지 않아서 강제이웃이 된다.
	if (dx != 0 && dy != 0)
	{
		// 검사 A:
		// (gx-dx, gy)      : 진행 반대쪽 x 방향 옆칸(직교칸)
		// (gx-dx, gy+dy)   : 그 옆칸의 대각선 앞칸
		// 직교칸은 벽, 대각선은 통과 가능이면 "우회 가능한 분기"가 생긴다.
		if (!IsWalkableGrid(gx - dx, gy) &&
			IsWalkableGrid(gx - dx, gy + dy))
		{
			return true;
		}

		// 검사 B:
		// (gx, gy-dy)      : 진행 반대쪽 y 방향 옆칸(직교칸)
		// (gx+dx, gy-dy)   : 그 옆칸의 대각선 앞칸
		// 위와 같은 논리로 분기점이면 강제이웃.
		if (!IsWalkableGrid(gx, gy - dy) &&
			IsWalkableGrid(gx + dx, gy - dy))
		{
			return true;
		}
	}

	// [수평 진행] dx!=0, dy==0
	// 위/아래가 벽인데 "진행 방향 쪽 대각선"이 열려 있으면 강제이웃.
	else if (dx != 0)
	{
		// 위쪽 벽 + (앞쪽 위 대각선) 오픈
		if (!IsWalkableGrid(gx, gy + 1) &&
			IsWalkableGrid(gx + dx, gy + 1))
		{
			return true;
		}

		// 아래쪽 벽 + (앞쪽 아래 대각선) 오픈
		if (!IsWalkableGrid(gx, gy - 1) &&
			IsWalkableGrid(gx + dx, gy - 1))
		{
			return true;
		}
	}

	// [수직 진행] dx==0, dy!=0
	// 좌/우가 벽인데 "진행 방향 쪽 대각선"이 열려 있으면 강제이웃.
	else if (dy != 0)
	{
		// 오른쪽 벽 + (오른쪽 앞 대각선) 오픈
		if (!IsWalkableGrid(gx + 1, gy) &&
			IsWalkableGrid(gx + 1, gy + dy))
		{
			return true;
		}

		// 왼쪽 벽 + (왼쪽 앞 대각선) 오픈
		if (!IsWalkableGrid(gx - 1, gy) &&
			IsWalkableGrid(gx - 1, gy + dy))
		{
			return true;
		}
	}

	// 어떤 패턴도 해당하지 않으면 강제이웃 없음.
	return false;
}

// ════════════════════════════════════════════════════════════════════════════
// 유틸리티 함수들
// ════════════════════════════════════════════════════════════════════════════

// [IsDiagonalDir] 이 방향이 대각선인지 확인 (dx와 dy가 모두 0이 아니면 대각선)
bool CNavigation::IsDiagonalDir(ESearchDir::Type Dir) const
{
	int dx = 0;
	int dy = 0;

	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	return dx != 0 && dy != 0;
}

// [GetMoveDelta] 방향 → (dx, dy) 변환. 예) RT → (+1, +1)
bool CNavigation::GetMoveDelta(ESearchDir::Type Dir, int& OutDx, int& OutDy) const
{
	// enum 범위를 벗어난 값 방어.
	if (Dir < 0 || Dir >= ESearchDir::End)
		return false;

	OutDx = DIR_DX[Dir];
	OutDy = DIR_DY[Dir];
	return true;
}

// [GetDirFromDelta] (dx, dy) → 방향 열거형으로 역변환.
// dx, dy를 -1/0/+1로 정규화한 뒤, DIR_DX/DIR_DY 테이블에서 일치하는 방향을 찾음.
ESearchDir::Type CNavigation::GetDirFromDelta(int dx, int dy) const
{
	dx = (dx > 0) - (dx < 0);  // 양수→1, 0→0, 음수→-1 로 정규화
	dy = (dy > 0) - (dy < 0);

	for (int i = 0; i < ESearchDir::End; ++i)
	{
		if (DIR_DX[i] == dx && DIR_DY[i] == dy)
			return (ESearchDir::Type)i;
	}

	return ESearchDir::End;
}

// [GetParentDir] 부모→현재 이동 방향을 계산. Skew Grid 좌표 차이로 결정.
ESearchDir::Type CNavigation::GetParentDir(FNavNode* Node) const
{
	// 부모가 없으면 "어느 방향에서 왔는지"를 정의할 수 없다.
	if (!Node || !Node->Parent)
		return ESearchDir::End;

	const int dx = Node->GridX - Node->Parent->GridX;
	const int dy = Node->GridY - Node->Parent->GridY;

	return GetDirFromDelta(dx, dy);
}

/*
 * [GetIndexByGrid] ★역방향 변환★ Skew Grid 좌표 → 타일 배열 인덱스.
 *
 * SetTileMap에서 (IndexX, IndexY) → (GridX, GridY)로 변환했었음.
 * 이 함수는 그 반대: (GridX, GridY) → 1차원 타일 배열 인덱스.
 *
 * 사각형: GridX/GridY가 곧 IndexX/IndexY이므로 단순 곱셈.
 * 아이소메트릭: 역변환 공식 적용.
 *   IndexY = GridX - GridY
 *   IndexX = GridY + (IndexY / 2)
 */
int CNavigation::GetIndexByGrid(int GridX, int GridY) const
{
	// [사각형 타일맵] 단순 변환
	if (mShape == ETileShape::Rect)
	{
		// 직사각 타일맵은 단순히 범위 체크 후 1차원 인덱스로 변환.
		if (GridX < 0 || GridX >= mCountX ||
			GridY < 0 || GridY >= mCountY)
		{
			return -1;
		}

		return GridY * mCountX + GridX;
	}

	// [아이소메트릭] Skew Grid → Staggered 역변환
	const int y = GridX - GridY;        // IndexY 복원

	// 복원된 y가 맵 범위 밖이면 유효하지 않은 좌표.
	if (y < 0 || y >= mCountY)
		return -1;

	const int x = GridY + (y / 2);     // IndexX 복원

	// 복원된 x가 맵 범위 밖이면 유효하지 않은 좌표.
	if (x < 0 || x >= mCountX)
		return -1;

	return y * mCountX + x;
}

/*
 * [StepIndex] 현재 타일에서 한 칸 이동한 결과 인덱스를 반환.
 * Skew Grid 좌표에 (dx, dy)를 더한 뒤 역변환하여 실제 타일 인덱스를 얻음.
 * CheckCollision=true이면 벽/대각선 코너 끼임도 검사.
 */
bool CNavigation::StepIndex(int FromIndex, ESearchDir::Type Dir,
	int& OutIndex, bool CheckCollision) const
{
	int dx = 0;
	int dy = 0;

	// 잘못된 방향이면 이동 불가.
	if (!GetMoveDelta(Dir, dx, dy))
		return false;

	const FNavNode& FromNode = mNodeList[FromIndex];

	// ★ Skew Grid에서 단순 덧셈으로 이동 (변환의 진가!)
	const int NextGridX = FromNode.GridX + dx;
	const int NextGridY = FromNode.GridY + dy;

	// 역변환으로 실제 타일 인덱스 확인
	const int NextIndex = GetIndexByGrid(NextGridX, NextGridY);

	// 다음 칸이 맵 밖이면 이동 실패.
	if (NextIndex < 0)
		return false;

	if (CheckCollision)
	{
		if (IsBlockedIndex(NextIndex))
			return false;

		// [대각선 이동 시] 양쪽 직교 방향이 둘 다 막혀있으면 통과 불가.
		// 이유: 대각선으로 벽 모서리를 비집고 지나가는 것을 방지.
		if (dx != 0 && dy != 0)
		{
			const int Side1 = GetIndexByGrid(FromNode.GridX + dx,
				FromNode.GridY);
			const int Side2 = GetIndexByGrid(FromNode.GridX,
				FromNode.GridY + dy);

			// 옆 칸 자체가 맵 밖이어도 대각선 이동은 허용하지 않는다.
			if (Side1 < 0 || Side2 < 0)
				return false;

			if (IsBlockedIndex(Side1) || IsBlockedIndex(Side2))
				return false;
		}
	}

	OutIndex = NextIndex;
	return true;
}

// [StepIndexRaw] 충돌 검사 없이 한 칸 이동 (이웃 타일 확인용)
int CNavigation::StepIndexRaw(int FromIndex, ESearchDir::Type Dir) const
{
	int Index = -1;

	// StepIndex의 핵심 계산을 재사용하되 충돌 검사만 끈다.
	if (!StepIndex(FromIndex, Dir, Index, false))
		return -1;

	return Index;
}

// [IsBlockedIndex] 해당 타일 인덱스가 통과 불가인지 확인.
// 시작점은 항상 통과 가능 (NPC가 벽 위에서 출발할 수도 있으므로).
bool CNavigation::IsBlockedIndex(int Index) const
{
	if (Index < 0 || Index >= (int)mNodeList.size())
		return true;   // 범위 밖 = 항상 막힘

	if (Index == mStartIndex)
		return false;  // 시작점은 무조건 통과 가능

	// 비트마스크가 준비되어 있으면 그것으로 판별
	if (!mBlockedMask.empty())
	{
		// 특정 타일(Index)의 막힘 여부를 비트마스크에서 꺼내는 식:
		// 1) mBlockedMask[Index/8]  : 해당 바이트 선택
		// 2) (1 << (Index%8))       : 해당 비트 위치 마스크 생성
		// 3) &(AND)                 : 그 비트가 켜져 있으면 0이 아닌 값
		return (mBlockedMask[Index / 8] &
			(unsigned char)(1 << (Index % 8))) != 0;
	}

	// 비트마스크가 없으면 타일맵에 직접 질문
	auto TileMap = mTileMap.lock();

	// 타일맵 접근 자체가 불가능하면 안전하게 "막힘"으로 본다.
	if (!TileMap)
		return true;

	return TileMap->GetTileType(Index) == ETileType::UnableToMove;
}

// [IsWalkableGrid] Skew Grid 좌표가 걸을 수 있는 칸인지 확인.
bool CNavigation::IsWalkableGrid(int GridX, int GridY) const
{
	const int Index = GetIndexByGrid(GridX, GridY);

	// Grid -> Index 변환 실패면 이동 불가.
	if (Index < 0)
		return false;

	return !IsBlockedIndex(Index);
}

// [IsGoalIndex] 해당 인덱스가 목표(Goal) 타일인지 비트마스크로 확인.
bool CNavigation::IsGoalIndex(int Index) const
{
	// 범위를 벗어난 값은 절대 목표가 될 수 없다.
	if (Index < 0 || Index >= (int)mNodeList.size())
		return false;

	// 목표 마스크 자체가 없으면 목표 없음.
	if (mGoalMask.empty())
		return false;

	// Goal도 동일한 비트마스크 방식:
	// 해당 비트가 1이면 "목표 타일", 0이면 "목표 아님".
	return (mGoalMask[Index / 8] &
		(unsigned char)(1 << (Index % 8))) != 0;
}

/*
 * [ComputeHeuristic] 현재 타일에서 "가장 가까운 목표"까지의 직선 거리를 계산.
 * A* JPS에서 "이 노드가 대충 얼마나 멀리 있는지" 추정하는 값 (= 휴리스틱).
 * 여러 Goal 중 가장 가까운 것의 거리를 반환.
 */
float CNavigation::ComputeHeuristic(int Index) const
{
	// 목표가 없으면 추정거리 의미가 없으므로 0 반환.
	if (mGoalIndices.empty())
		return 0.f;

	// FLT_MAX: float가 가질 수 있는 매우 큰 값(사실상 무한대처럼 사용)
	float Best = FLT_MAX;

	const FVector2& Center = mNodeList[Index].Center;

	for (size_t i = 0; i < mGoalIndices.size(); ++i)
	{
		const int GoalIndex = mGoalIndices[i];
		const float Dist = Center.Distance(mNodeList[GoalIndex].Center);

		// 현재까지 찾은 최솟값 갱신.
		if (Dist < Best)
			Best = Dist;
	}

	return Best;
}

float CNavigation::ComputeTraversalCost(int FromIndex, int ToIndex) const
{
	if (FromIndex < 0 || FromIndex >= (int)mNodeList.size() ||
		ToIndex < 0 || ToIndex >= (int)mNodeList.size())
	{
		return FLT_MAX;
	}

	return mNodeList[FromIndex].Center.Distance(
		mNodeList[ToIndex].Center) * GetTileTraversalWeight(ToIndex);
}

float CNavigation::GetTileTraversalWeight(int Index) const
{
	return IsRoadTile(Index) ? GNavRoadTileWeight : GNavOffRoadTileWeight;
}

bool CNavigation::IsRoadTile(int Index) const
{
	auto TileMap = mTileMap.lock();

	if (!TileMap)
		return false;

	return TileMap->IsRoadTile(Index);
}

/*
 * [FindFallbackGoalIndex] 목표 타일이 벽 위일 때,
 * 그 주변 8방향 중 "통과 가능하고 출발점에서 가장 가까운" 이웃을 대체 목표로 반환.
 */
int CNavigation::FindFallbackGoalIndex(int StartIndex, int EndIndex) const
{
	// 도착 인덱스 자체가 잘못됐으면 대체 목표도 만들 수 없음.
	if (EndIndex < 0 || EndIndex >= (int)mNodeList.size())
		return -1;

	// 원래 도착 타일이 막혀 있지 않으면 그대로 사용.
	if (!IsBlockedIndex(EndIndex))
		return EndIndex;

	const FVector2& StartCenter = mNodeList[StartIndex].Center;

	float BestDist = FLT_MAX;
	int BestIndex = -1;

	// 도착 타일 주변 8칸을 검사해서 "걸을 수 있는 후보"를 고른다.
	for (int i = 0; i < ESearchDir::End; ++i)
	{
		const int Neighbor = StepIndexRaw(EndIndex, (ESearchDir::Type)i);

		// 맵 밖이면 후보 제외.
		if (Neighbor < 0)
			continue;

		// 막힌 타일이면 후보 제외.
		if (IsBlockedIndex(Neighbor))
			continue;

		const float Dist =
			StartCenter.Distance(mNodeList[Neighbor].Center);

		// 시작점과 가장 가까운 후보를 대체 목표로 채택.
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestIndex = Neighbor;
		}
	}

	return BestIndex;
}

// ════════════════════════════════════════════════════════════════════════════
// BuildPath — 목표→시작으로 부모를 역추적하여 경로 점 목록 생성
// ════════════════════════════════════════════════════════════════════════════

/*
 * [BuildPath] 도착 노드(GoalNode)에서 Parent를 따라 시작 노드까지 거슬러 올라간 뒤,
 * 경유 노드 사이를 한 칸씩 채워 넣어 실제 이동 경로 점 목록을 만듭니다.
 *
 * JPS는 "코너→코너"만 기록하므로, 코너 사이의 중간 칸들을 이 함수에서 보간.
 */
bool CNavigation::BuildPath(FNavNode* GoalNode,
	std::list<FVector3>& PathList)
{
	// 이전 결과 제거.
	PathList.clear();

	if (!GoalNode)
		return false;

	// ChainIndices:
	// Goal -> Parent -> Parent ... 순서로 인덱스를 모으는 임시 배열.
	// 나중에 reverse해서 Start -> Goal 순서로 뒤집는다.
	std::vector<int> ChainIndices;

	FNavNode* CurrentNode = GoalNode;

	// Goal -> ... -> Start 순으로 부모 체인을 수집.
	while (CurrentNode)
	{
		ChainIndices.emplace_back(CurrentNode->Index);
		CurrentNode = CurrentNode->Parent;
	}

	if (ChainIndices.empty())
		return false;

	// 수집 순서를 Start -> ... -> Goal로 뒤집는다.
	std::reverse(ChainIndices.begin(), ChainIndices.end());

	// 시작=목표인 경우 실제 이동 점은 0개여도 성공.
	if (ChainIndices.size() <= 1)
		return true;

	// 연속한 코너 쌍마다 중간 칸을 채워 실제 이동 경로로 복원.
	for (size_t i = 0; i + 1 < ChainIndices.size(); ++i)
	{
		const int FromIndex = ChainIndices[i];
		const int ToIndex = ChainIndices[i + 1];

		const int dx = mNodeList[ToIndex].GridX -
			mNodeList[FromIndex].GridX;
		const int dy = mNodeList[ToIndex].GridY -
			mNodeList[FromIndex].GridY;

		const ESearchDir::Type Dir = GetDirFromDelta(dx, dy);

		// 코너 사이 방향을 해석할 수 없으면 경로가 깨진 상태.
		if (Dir == ESearchDir::End)
			return false;

		int CurrentIndex = FromIndex;

		// From -> To까지 한 칸씩 전진하며 점을 PathList에 추가.
		while (CurrentIndex != ToIndex)
		{
			CurrentIndex = StepIndexRaw(CurrentIndex, Dir);

			// 보간 중 맵 밖으로 나가면 잘못된 체인으로 판단.
			if (CurrentIndex < 0)
				return false;

			const FVector2& Center = mNodeList[CurrentIndex].Center;
			PathList.emplace_back(Center.x, Center.y, 0.f);
		}
	}

	return true;
}

// ════════════════════════════════════════════════════════════════════════════
// FindPathFallbackAStar — JPS 실패 시 사용하는 일반 A* 알고리즘
// ════════════════════════════════════════════════════════════════════════════

/*
 * [FindPathFallbackAStar] 순수한 A* 탐색.
 * JPS보다 느리지만 (모든 이웃을 하나씩 검사), 더 다양한 맵 구조에서 동작합니다.
 *
 * GScore[i] = 시작→i까지의 최단 실제 거리
 * FScore[i] = GScore + 휴리스틱 (= 총 예상 비용)
 */
bool CNavigation::FindPathFallbackAStar(int StartIndex,
	std::list<FVector3>& PathList)
{
	// A*는 mNodeList를 그래프의 정점 배열로 사용한다.
	const int NodeCount = (int)mNodeList.size();

	// 시작 인덱스 유효성 검사.
	if (StartIndex < 0 || StartIndex >= NodeCount)
		return false;

	// 목표가 없으면 A*도 수행 의미가 없다.
	if (mGoalIndices.empty()) 
		return false;

	const float Inf = (std::numeric_limits<float>::max)();

	std::vector<float> GScore((size_t)NodeCount, Inf);   // 모든 노드 거리 = 무한대
	std::vector<float> FScore((size_t)NodeCount, Inf);
	std::vector<int> Parent((size_t)NodeCount, -1);
	std::vector<unsigned char> OpenMask((size_t)NodeCount, 0);
	std::vector<unsigned char> ClosedMask((size_t)NodeCount, 0);

	// 시작 노드 초기값 세팅.
	GScore[(size_t)StartIndex] = 0.f;
	FScore[(size_t)StartIndex] = ComputeHeuristic(StartIndex);
	OpenMask[(size_t)StartIndex] = 1;

	int GoalIndex = -1;

	while (true)
	{
		int Current = -1;
		float BestF = Inf;

		// 단순 구현: Open 집합 전체를 매번 선형 탐색해 최소 F 노드를 선택.
		// (priority_queue를 쓰는 구현보다 느리지만, 구조가 직관적이라 이해 쉬움)
		for (int i = 0; i < NodeCount; ++i)
		{
			if (!OpenMask[(size_t)i])
				continue;

			if (FScore[(size_t)i] < BestF)
			{
				BestF = FScore[(size_t)i];
				Current = i;
			}
		}

		if (Current < 0)
			break;

		// 목표를 꺼냈으면 탐색 종료.
		if (IsGoalIndex(Current))
		{
			GoalIndex = Current;
			break;
		}

		// Current는 더 볼 필요 없으므로 Open -> Closed 이동.
		OpenMask[(size_t)Current] = 0;
		ClosedMask[(size_t)Current] = 1;

		// 8방향 이웃 검사.
		for (int Dir = 0; Dir < ESearchDir::End; ++Dir)
		{
			int Next = -1;

			// 이동 불가(벽/맵밖/대각선 끼임)면 이웃 제외.
			if (!StepIndex(Current, (ESearchDir::Type)Dir, Next, true))
				continue;

			if (Next < 0 || Next >= NodeCount)
				continue;

			// 이미 확정(Closed)된 노드는 다시 계산하지 않는다.
			if (ClosedMask[(size_t)Next])
				continue;

			const float StepCost = ComputeTraversalCost(Current, Next);
			const float NewG = GScore[(size_t)Current] + StepCost;

			// 처음 발견했거나, 더 짧은 경로를 찾았으면 갱신.
			if (!OpenMask[(size_t)Next] || NewG < GScore[(size_t)Next])
			{
				Parent[(size_t)Next] = Current;
				GScore[(size_t)Next] = NewG;
				FScore[(size_t)Next] = NewG + ComputeHeuristic(Next);
				OpenMask[(size_t)Next] = 1;
			}
		}
	}

	if (GoalIndex < 0)
		return false;

	// Goal에서 Start까지 Parent 체인을 따라 올라간다.
	std::vector<int> Chain;
	Chain.reserve((size_t)NodeCount);

	int Current = GoalIndex;

	while (Current >= 0 && Current < NodeCount)
	{
		Chain.emplace_back(Current);

		if (Current == StartIndex)
			break;

		Current = Parent[(size_t)Current];
	}

	// Start까지 연결되지 못한 체인은 실패 처리.
	if (Chain.empty() || Chain.back() != StartIndex)
		return false;

	// Start -> Goal 순서로 뒤집고, mNodeList의 Parent 포인터에 반영.
	std::reverse(Chain.begin(), Chain.end());

	for (int i = 0; i < NodeCount; ++i)
	{
		// 이전 탐색 흔적 제거.
		mNodeList[(size_t)i].Parent = nullptr;
	}

	for (size_t i = 1; i < Chain.size(); ++i)
	{
		const int Child = Chain[i];
		const int ParentIndex = Chain[i - 1];
		// BuildPath가 그대로 재사용되도록 Parent 링크를 다시 구성.
		mNodeList[(size_t)Child].Parent = &mNodeList[(size_t)ParentIndex];
	}

	return BuildPath(&mNodeList[(size_t)GoalIndex], PathList);
}

// [SortOpenList] Open List 정렬 비교 함수.
// Total이 큰 것이 앞에, 작은 것이 뒤에 → .back()이 항상 최소 비용 노드.
bool CNavigation::SortOpenList(FNavNode* Src, FNavNode* Dest)
{
	return Src->Total > Dest->Total;
}
