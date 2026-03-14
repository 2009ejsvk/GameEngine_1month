/*
 * ============================================================================
 * World.cpp - 게임 세계(월드) 관리 파일
 * ============================================================================
 *
 * [이 파일이 하는 일]
 * 게임에서 "지금 플레이 중인 하나의 세계(씬)"를 대표하는 클래스입니다.
 * 예를 들어, 시작 화면도 하나의 월드이고, 실제 게임 맵도 하나의 월드입니다.
 *
 * 이 월드 안에는:
 *   - 게임 오브젝트들 (건물, NPC 등)
 *   - 카메라 (화면을 어디서 바라보는지)
 *   - 입력 처리 (키보드/마우스)
 *   - 충돌 판정 (오브젝트끼리 부딪히는지)
 *   - 내비게이션 (NPC 길찾기)
 *   - UI (버튼, 패널 등 화면 위의 인터페이스)
 * 가 전부 들어있습니다.
 *
 * [C++ 기초 용어 안내]
 *   #include "파일명"  → 다른 파일의 코드를 가져다 쓰겠다는 선언
 *   void               → "반환값 없음"이라는 뜻. 함수가 결과를 돌려주지 않음
 *   bool               → true(참) 또는 false(거짓) 둘 중 하나만 담는 타입
 *   float              → 소수점 있는 숫자 (예: 3.14f, 0.016f)
 *   auto               → "타입을 컴파일러가 알아서 추론해줘"라는 뜻
 *   ->                 → 포인터(주소)를 통해 해당 객체의 함수/변수에 접근
 *   .lock()            → weak_ptr(약한 참조)을 shared_ptr(강한 참조)로 변환
 *   .reset(new 타입)   → 기존 객체를 버리고 새 객체를 생성하여 대입
 *   ::                 → "이 클래스 소속의"라는 뜻 (예: CWorld::Init = CWorld 클래스의 Init 함수)
 *   lambda [&](...){}  → 주변 변수를 빌려 쓸 수 있는 "이름 없는 함수"
 * ============================================================================
 */

// ─── 다른 파일 가져오기 (Include) ───
#include "World.h"                          // CWorld 클래스의 설계도(헤더). 멤버 변수/함수 선언이 여기 있음
#include "../Component/TileMapComponent.h"  // 타일맵(격자 지도) 컴포넌트. 길찾기 스냅샷에 필요

// ════════════════════════════════════════════════════════════════════════════
// 생성자 / 소멸자
// ════════════════════════════════════════════════════════════════════════════

/*
 * [생성자] CWorld가 메모리에 만들어질 때 자동 호출.
 * 여기서는 특별히 할 일이 없어 비어 있음.
 * 실질적인 초기화는 아래 Init() 함수에서 수행.
 */
CWorld::CWorld()
{
}

/*
 * [소멸자] CWorld가 메모리에서 제거될 때 자동 호출.
 * 내부의 shared_ptr 멤버들은 자동으로 해제되므로 별도 정리 불필요.
 */
CWorld::~CWorld()
{
}

// ════════════════════════════════════════════════════════════════════════════
// 입력 장치 활성화 / 비활성화
// ════════════════════════════════════════════════════════════════════════════

/*
 * [InputActive] 게임 창이 활성화(포커스를 받음)되었을 때 호출.
 * 키보드/마우스 입력 장치를 다시 연결(Acquire)합니다.
 *
 * 왜 필요한가?
 * → 사용자가 다른 프로그램을 쓰다 돌아오면, 입력 장치가 해제된 상태일 수 있음.
 *   이 함수로 다시 잡아줘야 키보드/마우스 입력을 받을 수 있음.
 */
void CWorld::InputActive()
{
	mInput->DeviceAcquire();
}

/*
 * [InputDeactive] 게임 창이 비활성화(포커스를 잃음)되었을 때 호출.
 * 키보드/마우스 입력 장치를 해제(UnAcquire)합니다.
 *
 * 왜 필요한가?
 * → 게임 창 밖을 클릭했는데 게임 내 키입력이 먹히면 안 되니까.
 */
void CWorld::InputDeactive()
{
	mInput->DeviceUnAcquire();
}

// ════════════════════════════════════════════════════════════════════════════
// 초기화 (Init)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [Init] 월드의 모든 하위 시스템(서브시스템)을 순서대로 생성하고 초기화합니다.
 *
 * 비유: 게임 세계라는 "방"을 세팅하는 과정.
 *   1. 카메라를 설치한다           (어디를 비출지)
 *   2. 리소스 창고를 연다           (그림, 소리 등 자원 관리)
 *   3. 리모컨(입력장치)을 연결한다  (키보드, 마우스)
 *   4. 충돌 감지기를 배치한다       (물체끼리 부딪히는지 확인)
 *   5. UI 매니저를 준비한다         (버튼, 패널 등 화면 위 요소)
 *   6. 길찾기 시스템을 준비한다     (NPC가 길을 찾을 수 있게)
 *   7. 오브젝트 목록 공간을 확보한다
 *
 * 반환값:
 *   true  = 모든 시스템이 정상 초기화됨
 *   false = 하나라도 실패하면 월드 생성 불가
 */
bool CWorld::Init()
{
	// ── 1. 카메라 매니저 ──
	// 게임 화면에 어디를 비출지 관리.
	// .reset(new ...) : 새 객체를 만들어서 스마트 포인터에 넣음.
	mCameraManager.reset(new CCameraManager);

	// 초기화 실패 시 false 반환 → 월드 생성 중단
	if (!mCameraManager->Init())
		return false;

	// ── 2. 월드 전용 에셋(리소스) 매니저 ──
	// 이 월드에서만 쓰는 텍스처, 메쉬 등을 관리.
	mWorldAssetManager.reset(new CWorldAssetManager);

	if (!mWorldAssetManager->Init())
		return false;

	// ── 3. 입력 시스템 ──
	// 키보드/마우스 입력을 감지하고, 등록된 콜백 함수를 호출해줌.
	mInput.reset(new CInput);

	// mInput이 "나는 어느 월드에 소속인지" 알 수 있도록 연결.
	// mSelf는 "자기 자신을 가리키는 weak_ptr" (순환 참조 방지용).
	mInput->mWorld = mSelf;

	if (!mInput->Init())
		return false;

	// ── 4. 충돌 시스템 ──
	// QuadTree를 이용해 오브젝트들의 충돌(겹침)을 빠르게 감지.
	mCollision.reset(new CWorldCollision);

	mCollision->SetWorld(mSelf);

	if (!mCollision->Init())
		return false;

	// ── 5. UI 매니저 ──
	// 게임 화면 위에 표시되는 버튼, 텍스트, 패널 등 관리.
	mUIManager.reset(new CWorldUIManager);

	mUIManager->mWorld = mSelf;
	mUIManager->mSelf = mUIManager;

	if (!mUIManager->Init())
		return false;

	// ── 6. 내비게이션(길찾기) 시스템 ──
	// NPC가 건물 사이를 돌아다닐 때 경로를 계산해주는 시스템.
	// 멀티스레드로 동작하여 메인 게임이 느려지지 않게 함.
	mNavigation.reset(new CWorldNavigation);

	mNavigation->mWorld = mSelf;
	mNavigation->mSelf = mNavigation;

	if (!mNavigation->Init())
		return false;

	// ── 7. 오브젝트 목록 메모리 예약 ──
	// .reserve(N) : 미리 N개 분량의 메모리를 확보해둠.
	// 나중에 오브젝트가 추가될 때 메모리 재할당을 줄여 성능 향상.
	mObjList.reserve(10000);     // 일반 오브젝트: 최대 10,000개 예상
	mStartObjList.reserve(200);  // 시작 대기 오브젝트: 최대 200개 예상

	return true;
}

// ════════════════════════════════════════════════════════════════════════════
// Update (매 프레임 로직 처리)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [Update] 매 프레임마다 호출되는 "게임 로직의 심장" 함수.
 *
 * DeltaTime = 이전 프레임과 현재 프레임 사이에 흐른 시간(초).
 *   예) 60fps라면 DeltaTime ≈ 0.016초 (1/60).
 *   움직이는 속도에 DeltaTime을 곱하면 프레임 수와 무관하게 일정한 속도 유지.
 *
 * 처리 순서:
 *   1. Begin()         → 대기 중이던 새 오브젝트를 활성화
 *   2. Input::Update() → 키보드/마우스 입력 처리 (★ 반드시 오브젝트보다 먼저!)
 *   3. 모든 오브젝트 Update() → 각자의 로직 실행 (이동, AI 판단 등)
 *   4. 카메라 Update()        → 카메라 위치/줌 갱신
 *   5. 에셋매니저 Update()     → 리소스 로딩 상태 갱신
 *   6. 내비게이션 Update()     → 길찾기 스레드 결과 수신
 *   7. UI매니저 Update()       → UI 위젯 상태 갱신
 */
void CWorld::Update(float DeltaTime)
{
	// 대기 중인 새 오브젝트들을 게임에 참여시킨다.
	Begin();

	// ★ 입력은 반드시 오브젝트 업데이트보다 먼저!
	// 이유: 오브젝트가 "이번 프레임에 눌린 키"를 읽으려면,
	//       입력 시스템이 먼저 키 상태를 갱신해야 합니다.
	mInput->Update(DeltaTime);

	// ── 모든 게임 오브젝트를 순회하며 Update 호출 ──
	// mObjList는 "이름→오브젝트" 쌍으로 저장된 목록(map)입니다.
	// iter->first = 오브젝트 이름 (문자열)
	// iter->second = 오브젝트 자체 (shared_ptr)
	auto	iter = mObjList.begin();    // 목록의 첫 번째 위치
	auto	iterEnd = mObjList.end();   // 목록의 "끝 다음" 위치 (경계)

	for (; iter != iterEnd;)
	{
		// [검사 1] 참조 카운트가 0이면 → 아무도 안 쓰는 오브젝트 → 삭제
		// use_count() : 이 shared_ptr을 몇 군데서 참조하고 있는지 반환.
		// 0이면 이미 다른 곳에서 전부 놓은 상태 → 의미 없는 껍데기.
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);   // 목록에서 제거하고 다음 항목으로
			iterEnd = mObjList.end();      // 목록 크기가 변했으므로 끝 위치 재계산
			continue;                       // 아래 코드 건너뛰고 다음 반복으로
		}

		// [검사 2] "살아있지 않은" 오브젝트 → 삭제
		// GetAlive() : 게임 내에서 "파괴됨" 처리된 오브젝트는 false 반환.
		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		// [검사 3] "비활성화된" 오브젝트 → 삭제하진 않지만 이번 프레임은 건너뜀
		// GetEnable() false = 일시 정지 상태. 다시 Enable되면 동작 재개.
		else if (!iter->second->GetEnable())
		{
			++iter;    // 다음 항목으로 이동 (삭제하지 않음)
			continue;
		}

		// 위 3가지 검사를 통과한 "정상 오브젝트"만 Update 실행
		iter->second->Update(DeltaTime);
		++iter;
	}

	// ── 나머지 매니저들도 업데이트 ──
	mCameraManager->Update(DeltaTime);     // 카메라 위치/줌 갱신
	mWorldAssetManager->Update(DeltaTime); // 리소스 로딩 상태 확인
	mNavigation->Update(DeltaTime);        // 길찾기 스레드 결과 수신/처리
	mUIManager->Update(DeltaTime);         // UI 위젯 상태 갱신
	OnUiManagerUpdated();                  // 클라이언트별 UI 오버라이드 훅
}

// ════════════════════════════════════════════════════════════════════════════
// PostUpdate (후처리 업데이트)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [PostUpdate] Update() 이후에 호출되는 "마무리 단계".
 *
 * 왜 Update와 PostUpdate로 나누는가?
 * → Update에서 모든 오브젝트가 자기 로직을 먼저 실행한 후,
 *   PostUpdate에서 충돌 판정과 좌표 최종 확정을 해야
 *   "이번 프레임의 최종 상태"가 일관성 있게 결정됩니다.
 *
 * 처리 순서:
 *   1. Begin()                 → 혹시 이 단계에서 추가된 새 오브젝트 활성화
 *   2. 모든 오브젝트 PostUpdate() → 후처리 로직 (위치 보정 등)
 *   3. Collision::Update()     → 충돌 판정 (★ 모든 이동이 끝난 뒤에 해야 함)
 *   4. 모든 오브젝트 UpdateTransform() → 최종 위치/회전/크기 행렬 계산
 */
void CWorld::PostUpdate(float DeltaTime)
{
	Begin();

	// ── 1단계: 모든 오브젝트의 PostUpdate 호출 ──
	// (Update와 동일한 3단계 검사: 참조=0 → 삭제, 죽음 → 삭제, 비활성 → 건너뜀)
	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}

	// ── 2단계: 충돌 판정 ──
	// ★ 모든 오브젝트의 위치가 갱신된 "이후에" 충돌을 검사해야 정확합니다.
	// 만약 이동 도중에 충돌을 검사하면 아직 안 움직인 오브젝트와의 판정이 어긋남.
	mCollision->Update(DeltaTime);

	// ── 3단계: 최종 Transform(변환 행렬) 갱신 ──
	// 충돌로 인해 위치가 밀려났을 수 있으므로,
	// 최종적으로 "화면에 어디에 그릴지" 행렬을 다시 계산합니다.
	// (행렬 = 크기 × 회전 × 이동을 하나로 합친 수학 데이터)
	iter = mObjList.begin();
	iterEnd = mObjList.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		// UpdateTransform() : 부모-자식 관계를 따라
		// 크기/회전/이동 → 월드 행렬(World Matrix)을 최종 계산.
		iter->second->UpdateTransform();
		++iter;
	}
}

// ════════════════════════════════════════════════════════════════════════════
// 렌더링 관련
// ════════════════════════════════════════════════════════════════════════════

/*
 * [Render] 월드 레벨의 렌더링 처리.
 * 실제 오브젝트 그리기는 RenderManager가 담당하고,
 * 여기서는 디버그용 충돌 영역 표시와 충돌 트리 노드 반환만 수행.
 */
void CWorld::Render()
{
	Begin();

	// #ifdef _DEBUG : "디버그 빌드일 때만" 아래 코드를 실행.
	// 릴리즈(배포용) 빌드에서는 이 부분이 아예 사라짐 → 성능 부담 없음.
#ifdef _DEBUG
	// 충돌 영역을 시각적으로 그려줌 (빨간 박스, 초록 원 등)
	// → 개발 중 "이 오브젝트의 충돌 범위가 맞는지" 눈으로 확인하기 위한 것.
	mCollision->Render();
#endif // _DEBUG

	// 이번 프레임에 사용한 QuadTree 노드를 풀(Pool)에 반환.
	// (매 프레임 새로 할당하면 느리니까, 재사용 풀에서 빌려쓰고 돌려주는 방식)
	mCollision->ReturnNodePool();
}

/*
 * [PostRender] 렌더링이 끝난 뒤 추가 처리.
 * 오브젝트들의 PostRender()를 호출 → 후처리 렌더 효과 등.
 */
void CWorld::PostRender()
{
	auto	iter = mObjList.begin();
	auto	iterEnd = mObjList.end();

	// Update/PostUpdate와 동일한 3단계 검사 후 PostRender 호출
	for (; iter != iterEnd;)
	{
		if (iter->second.use_count() == 0)
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetAlive())
		{
			iter = mObjList.erase(iter);
			iterEnd = mObjList.end();
			continue;
		}

		else if (!iter->second->GetEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostRender();
		++iter;
	}
}

/*
 * [RenderUI] UI(유저 인터페이스)를 화면에 그립니다.
 * 게임 월드 위에 "덮어씌우는" 형태로 렌더링됨 (깊이판정 없이 항상 최상위).
 */
void CWorld::RenderUI()
{
	mUIManager->Render();
}

void CWorld::OnUiManagerUpdated()
{
}

/*
 * [ClearWorld] 월드를 정리할 때 호출. 리소스를 해제합니다.
 * 월드 전환(예: 메인화면 → 게임)할 때 이전 월드의 자원을 정리해야 메모리 절약.
 */
void CWorld::ClearWorld()
{
	mWorldAssetManager->ClearAsset();
}

// ════════════════════════════════════════════════════════════════════════════
// BuildNavigationSnapshot (길찾기용 맵 스냅샷 생성)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [BuildNavigationSnapshot]
 * NPC가 길을 찾기 전에, "지금 이 순간 어떤 타일이 막혀있고 어떤 타일이 목적지인지"
 * 를 사진 찍듯 캡처하는 함수입니다.
 *
 * 왜 스냅샷이 필요한가?
 * → 건물은 수시로 배치/철거되고, 매 프레임 바뀔 수 있음.
 *   길찾기 알고리즘이 동작하는 동안 맵 상태가 바뀌면 결과가 꼬이므로,
 *   "이 시점의 상태"를 복사해서 넘겨줍니다.
 *
 * 매개변수 설명:
 *   TileMap                         → 타일맵 컴포넌트 (격자 지도 데이터)
 *   EndTileIndex                    → NPC가 가려는 목적지의 타일 번호
 *   PreferredTargetObjectName       → "이 이름의 건물로 가고 싶다"는 힌트 (비어있을 수 있음)
 *   OutBlockedMask (출력)           → "이 타일은 못 지나간다" 비트 배열
 *   OutGoalIndices (출력)           → "이 타일에 도착하면 된다" 목록
 *   OutResolvedTargetObjectName (출력) → 실제로 매칭된 목적지 건물 이름
 *
 * 반환값: true = 스냅샷 성공, false = 타일맵 없음 등의 이유로 실패
 */
bool CWorld::BuildNavigationSnapshot(
	const std::shared_ptr<class CTileMapComponent>& TileMap,
	int EndTileIndex,
	const std::string& PreferredTargetObjectName,
	std::vector<unsigned char>& OutBlockedMask,
	std::vector<int>& OutGoalIndices,
	std::string& OutResolvedTargetObjectName)
{
	// 출력 매개변수 초기화 (이전 호출의 찌꺼기를 제거)
	OutBlockedMask.clear();
	OutGoalIndices.clear();
	OutResolvedTargetObjectName.clear();

	// 타일맵이 없으면 길찾기 불가 → 실패
	if (!TileMap)
		return false;

	// 전체 타일 수 = 가로 칸 수 × 세로 칸 수
	const int TileCount = TileMap->GetTileCountX() * TileMap->GetTileCountY();

	if (TileCount <= 0)
		return false;

	// ── 비트마스크 준비 ──
	// 타일 하나당 1비트만 사용하여 "막힘/안막힘"을 표시.
	// 비트마스크란? : 0과 1의 나열로 상태를 아주 적은 메모리로 저장하는 방법.
	// 예) 8개 타일 → 1바이트(8비트)로 표현. 10000개 타일 → 1250바이트만 필요.
	// (TileCount + 7) / 8 : 올림 나눗셈. 타일이 9개면 → (9+7)/8 = 2바이트 필요.
	const int MaskByteCount = (TileCount + 7) / 8;
	OutBlockedMask.resize(MaskByteCount);

	// ── 람다(lambda) 함수 : SetBlocked ──
	// 특정 타일 번호를 "통과 불가"로 표시하는 간편 함수.
	// [&] : 바깥의 변수(OutBlockedMask, TileCount 등)를 빌려 사용.
	//
	// 비트 연산 설명:
	//   Index / 8   → 몇 번째 바이트인지
	//   Index % 8   → 그 바이트 안에서 몇 번째 비트인지
	//   1 << (Index % 8) → 해당 비트만 1인 값 생성
	//   |= → OR 연산으로 해당 비트를 1로 켬 (= "막힘" 표시)
	auto SetBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		OutBlockedMask[Index / 8] |=
			(unsigned char)(1 << (Index % 8));
	};

	// ── 람다 함수 : ClearBlocked ──
	// 특정 타일 번호의 "통과 불가" 표시를 제거하는 함수.
	//   ~(1 << (Index % 8)) → 해당 비트만 0이고 나머지는 1인 값 생성
	//   &= → AND 연산으로 해당 비트만 0으로 끔 (= "통과 가능" 복원)
	auto ClearBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		OutBlockedMask[Index / 8] &=
			(unsigned char)~(1 << (Index % 8));
	};

	// ── 람다 함수 : IsBlocked ──
	// 특정 타일이 막혀있는지 확인하는 함수.
	//   & → AND 연산으로 해당 비트가 1인지 검사
	auto IsBlocked = [&](int Index)
	{
		if (Index < 0 || Index >= TileCount)
			return true;   // 범위 밖은 항상 "막힘" 취급

		return (OutBlockedMask[Index / 8] &
			(unsigned char)(1 << (Index % 8))) != 0;
	};

	// ── 람다 함수 : AddGoalUnique ──
	// 목표(Goal) 타일을 중복 없이 추가하는 함수.
	// SeenGoalMask로 "이미 추가한 적 있는지"를 비트마스크로 체크하여
	// 같은 Goal이 두 번 들어가는 것을 방지.
	std::vector<unsigned char>	SeenGoalMask;
	SeenGoalMask.resize((size_t)MaskByteCount);

	auto AddGoalUnique = [&](int Index, std::vector<int>& GoalList)
	{
		if (Index < 0 || Index >= TileCount)
			return;

		unsigned char& GoalByte = SeenGoalMask[Index / 8];
		const unsigned char GoalBit = (unsigned char)
			(1 << (Index % 8));

		// 이미 추가된 적 있으면 건너뜀
		if (GoalByte & GoalBit)
			return;

		GoalByte |= GoalBit;                // "추가됨" 표시
		GoalList.emplace_back(Index);        // 목록에 추가
	};

	// ── 람다 함수 : CollectNeighborIndices ──
	// 특정 타일의 8방향 이웃 타일 인덱스를 수집하는 함수.
	// 사각형(Rect) 타일맵과 아이소메트릭(Isometric) 타일맵에서 이웃 계산 방식이 다름.
	auto CollectNeighborIndices = [&](int Index, std::vector<int>& OutNeighbors)
	{
		OutNeighbors.clear();

		if (Index < 0 || Index >= TileCount)
			return;

		auto Tile = TileMap->GetTile(Index).lock();

		if (!Tile)
			return;

		const int CountX = TileMap->GetTileCountX();
		const int CountY = TileMap->GetTileCountY();

		// ── 사각형 타일맵의 경우 ──
		// 단순히 상하좌우 + 대각선 = 8방향을 dx, dy로 계산.
		if (TileMap->GetTileShape() == ETileShape::Rect)
		{
			const int BaseX = Tile->GetIndexX();
			const int BaseY = Tile->GetIndexY();

			// dy : -1(위), 0(같은 행), +1(아래)
			// dx : -1(왼), 0(같은 열), +1(오른)
			for (int dy = -1; dy <= 1; ++dy)
			{
				for (int dx = -1; dx <= 1; ++dx)
				{
					if (dx == 0 && dy == 0)   // 자기 자신은 제외
						continue;

					const int nx = BaseX + dx;
					const int ny = BaseY + dy;

					// 맵 범위를 벗어나면 무시
					if (nx < 0 || nx >= CountX ||
						ny < 0 || ny >= CountY)
					{
						continue;
					}

					OutNeighbors.emplace_back(ny * CountX + nx);
				}
			}
		}
		// ── 아이소메트릭 타일맵의 경우 ──
		// Staggered(지그재그) 배치에서는 이웃이 행마다 다르므로
		// Skew Grid 좌표로 변환 → 8방향 → 역변환으로 이웃을 구함.
		else
		{
			const int x = Tile->GetIndexX();
			const int y = Tile->GetIndexY();

			// [정방향 변환] Staggered → Skew Grid
			// 이 변환을 하면 8방향 이동이 일정한 (dx, dy) 덧셈으로 가능해짐.
			const int GridX = x + ((y + (y & 1)) / 2);
			const int GridY = x - (y / 2);

			// 8방향 이동 벡터 (Skew Grid 기준)
			const int DirX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
			const int DirY[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

			for (int d = 0; d < 8; ++d)
			{
				// Skew Grid에서 한 칸 이동
				const int NextGridX = GridX + DirX[d];
				const int NextGridY = GridY + DirY[d];

				// [역방향 변환] Skew Grid → Staggered
				const int NextY = NextGridX - NextGridY;   // IndexY 복원

				if (NextY < 0 || NextY >= CountY)
					continue;

				const int NextX = NextGridY + (NextY / 2); // IndexX 복원

				if (NextX < 0 || NextX >= CountX)
					continue;

				// 1차원 배열 인덱스로 변환하여 결과에 추가
				OutNeighbors.emplace_back(NextY * CountX + NextX);
			}
		}
	};

	// ══════════════════════════════════════
	// 여기서부터 실제 스냅샷 구성 시작
	// ══════════════════════════════════════

	// ── Step 1: 기본 이동 불가 타일 반영 ──
	// 원래부터 벽/물 등으로 지정된 타일들을 "막힘"으로 표시.
	for (int i = 0; i < TileCount; ++i)
	{
		if (TileMap->GetTileType(i) == ETileType::UnableToMove)
		{
			SetBlocked(i);
		}
	}

	// ── Step 2: 선호 목적지(건물 이름) 확인 ──
	// NPC가 "이 건물로 가고 싶다"고 지정한 경우, 해당 오브젝트를 찾음.
	std::shared_ptr<CGameObject> PreferredTarget;

	if (!PreferredTargetObjectName.empty())
	{
		PreferredTarget =
			FindObject<CGameObject>(PreferredTargetObjectName).lock();
	}

	std::shared_ptr<CGameObject> ResolvedTarget;  // 실제로 매칭된 목적지 건물
	std::vector<int> TargetGoalIndices;            // 매칭된 건물의 입구 타일들
	std::vector<int> GlobalGoalIndices;            // 전체 건물들의 입구 타일 모음

	// ── Step 3: 모든 오브젝트를 순회하며 장애물/입구 정보 수집 ──
	auto iter = mObjList.begin();
	auto iterEnd = mObjList.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Obj = iter->second;

		// 비활성이거나 죽은 오브젝트는 무시
		if (!Obj || !Obj->GetAlive() || !Obj->GetEnable())
			continue;

		// 길찾기용 장애물이 아닌 오브젝트는 건너뜀
		// (건물만 IsNavigationObstacle()이 true를 반환)
		if (!Obj->IsNavigationObstacle())
			continue;

		// 이 건물이 차지하는 타일들 → "통과 불가"로 마킹
		std::vector<int> BlockedTiles;
		Obj->GetNavigationBlockedTiles(BlockedTiles);

		for (size_t i = 0; i < BlockedTiles.size(); ++i)
		{
			SetBlocked(BlockedTiles[i]);
		}

		// 이 건물의 입구(Goal) 타일들 → 전역 입구 목록에 추가
		std::vector<int> GoalTiles;
		Obj->GetNavigationGoalTiles(GoalTiles);

		for (size_t i = 0; i < GoalTiles.size(); ++i)
		{
			AddGoalUnique(GoalTiles[i], GlobalGoalIndices);
		}

		// ── 이 건물이 NPC의 목적지인지 판별 ──
		bool IsTarget = false;

		// 방법 1: 이름으로 지정된 선호 목적지가 바로 이 오브젝트인 경우
		if (PreferredTarget && Obj.get() == PreferredTarget.get())
		{
			IsTarget = true;
		}

		// 방법 2: 이름이 없고 EndTileIndex가 주어진 경우,
		//         이 건물의 영역이 목적지 타일을 포함하는지 검사
		else if (!PreferredTarget && EndTileIndex >= 0)
		{
			bool ContainsEnd = false;

			// 건물 본체(blocked) 타일 중에 끝점이 있는지 확인
			for (size_t i = 0; i < BlockedTiles.size(); ++i)
			{
				if (BlockedTiles[i] == EndTileIndex)
				{
					ContainsEnd = true;
					break;
				}
			}

			// 입구(goal) 타일 중에 끝점이 있는지도 확인
			if (!ContainsEnd)
			{
				for (size_t i = 0; i < GoalTiles.size(); ++i)
				{
					if (GoalTiles[i] == EndTileIndex)
					{
						ContainsEnd = true;
						break;
					}
				}
			}

			IsTarget = ContainsEnd;
		}

		// 이 건물이 NPC의 목적지라면, 매칭 정보를 기록
		if (IsTarget)
		{
			ResolvedTarget = Obj;
			TargetGoalIndices = GoalTiles;
		}
	}

	// ── Step 4: 입구(Goal) 타일은 통과 가능하게 열어줌 ──
	// 건물 본체가 Blocked로 마킹되더라도, 입구 타일만은 NPC가 도착할 수 있어야 함.
	for (size_t i = 0; i < GlobalGoalIndices.size(); ++i)
	{
		ClearBlocked(GlobalGoalIndices[i]);
	}

	// ── Step 5: 고립된 입구 방지 ──
	// 입구(Goal) 타일이 사방이 전부 막혀있으면,
	// NPC가 도착은 해도 다음 목적지로 출발할 수 없음.
	// → 인접 타일 중 하나를 강제로 열어 "탈출구"를 보장.
	std::vector<int> NeighborIndices;

	for (size_t i = 0; i < GlobalGoalIndices.size(); ++i)
	{
		const int GoalIndex = GlobalGoalIndices[i];

		// 이 입구의 8방향 이웃 수집
		CollectNeighborIndices(GoalIndex, NeighborIndices);

		// 열린 이웃이 하나라도 있는지 확인
		bool HasOpenNeighbor = false;

		for (size_t n = 0; n < NeighborIndices.size(); ++n)
		{
			if (!IsBlocked(NeighborIndices[n]))
			{
				HasOpenNeighbor = true;
				break;
			}
		}

		// 이미 탈출구가 있으면 OK → 다음 입구로
		if (HasOpenNeighbor)
			continue;

		// 사방이 전부 막힌 경우 → 이웃 중 첫 번째 하나를 강제로 열어줌
		if (!NeighborIndices.empty())
			ClearBlocked(NeighborIndices[0]);
	}

	// ── Step 6: 최종 결과 구성 ──
	// 매칭된 목적지 건물의 입구 타일들 중, 막히지 않은 것만 최종 출력에 추가.
	SeenGoalMask.assign((size_t)MaskByteCount, 0);

	for (size_t i = 0; i < TargetGoalIndices.size(); ++i)
	{
		const int Index = TargetGoalIndices[i];

		if (Index < 0 || Index >= TileCount)
			continue;

		if (IsBlocked(Index))
			continue;

		AddGoalUnique(Index, OutGoalIndices);
	}

	// ── Step 7: 매칭된 목적지 건물의 이름을 출력 ──
	if (ResolvedTarget)
	{
		OutResolvedTargetObjectName = ResolvedTarget->GetName();
	}

	return true;
}

// ════════════════════════════════════════════════════════════════════════════
// Begin (대기 오브젝트 활성화)
// ════════════════════════════════════════════════════════════════════════════

/*
 * [Begin] "대기열에 있는 새 오브젝트"들을 게임에 정식으로 참여시킵니다.
 *
 * 왜 바로 추가하지 않고 대기시키는가?
 * → Update 루프 도중에 목록(mObjList)에 새 항목을 추가하면
 *   반복자(iterator)가 무효화되어 프로그램이 뻗을(crash) 수 있습니다.
 *   그래서 "다음 프레임 시작 시" 일괄 추가하는 안전한 방식을 사용.
 *
 * .lock() : weak_ptr → shared_ptr로 변환.
 *   weak_ptr은 "관찰만 할게" 약속이라, 실제 사용하려면 lock()으로 빌려와야 함.
 */
void CWorld::Begin()
{
	if (!mStartObjList.empty())
	{
		size_t	Size = mStartObjList.size();

		for (size_t i = 0; i < Size; ++i)
		{
			auto	Obj = mStartObjList[i].lock();

			// 각 오브젝트의 Begin() 호출 → 컴포넌트 초기화, 렌더 등록 등
			Obj->Begin();
		}

		// 대기열 비우기 (이미 전부 활성화했으므로)
		mStartObjList.clear();
	}
}

/*
 * [BeginManager] 매니저 레벨의 Begin 처리.
 * 내비게이션 시스템이 시작 시 필요한 추가 초기화를 수행.
 */
void CWorld::BeginManager()
{
	mNavigation->Begin();
}
