# C++ 학생 기술 문서 (GameEngine 기준)

작성일: 2026-02-23  
대상: C++ 기초를 배웠고 게임엔진 코드를 읽기 시작한 학생

## 1) 수업 목표
1. C++ 문법을 "문제 풀이용"이 아니라 "엔진 개발용"으로 전환한다.
2. 메모리/소유권/수명주기를 안전하게 다루는 습관을 만든다.
3. 실제 프로젝트 코드(`GameEngine`)를 읽고 수정할 수 있게 한다.

## 2) 학습 전제
1. `if`, `for`, 함수, 클래스 기본 문법은 알고 있다고 가정한다.
2. Visual Studio 빌드/디버그 실행 경험이 최소 1회 있다고 가정한다.

## 3) 핵심 기술 지도 (우선순위 순)

### A. 값/참조/포인터와 const
핵심:
1. 값 복사는 안전하지만 비용이 들 수 있다.
2. 참조(`T&`)는 별칭이며 null이 될 수 없다.
3. 포인터(`T*`)는 null 가능, 소유권과 별개다.
4. `const`는 "변경 금지 계약"이다.

가르칠 포인트:
1. `const T&`를 기본 읽기 파라미터로 사용.
2. 포인터는 "소유"보다 "접근" 목적으로 사용.
3. 함수 시그니처에서 의도를 드러내기.

예제:
```cpp
void PrintName(const std::string& name); // 읽기 전용, 복사 없음
void SetTarget(GameObject* target);      // null 가능
```

### B. RAII와 소유권 모델
핵심:
1. 리소스 획득과 해제를 객체 수명에 묶는다(RAII).
2. `new/delete` 직접 사용을 최소화한다.
3. 소유권은 `unique_ptr`, 공유는 `shared_ptr`, 비소유 참조는 raw pointer/weak_ptr로 분리한다.

가르칠 포인트:
1. "누가 지우는가?"를 항상 먼저 묻기.
2. 생성자에서 얻은 리소스는 소멸자에서 정리.
3. 순환 참조 방지(`shared_ptr` + `weak_ptr`).

예제:
```cpp
class TextureHandle {
public:
    TextureHandle(ID3D11Texture2D* tex) : tex_(tex) {}
    ~TextureHandle() { if (tex_) tex_->Release(); }
private:
    ID3D11Texture2D* tex_ = nullptr;
};
```

### C. enum class와 상태 모델링
핵심:
1. 매직 넘버 대신 의미 있는 타입을 사용한다.
2. 입력 상태(Press/Hold/Release)처럼 상태 전이를 명시적으로 표현한다.

프로젝트 연결:
1. `GameEngine/World/Input.cpp`는 입력 상태를 프레임 단위로 갱신한다.

예제:
```cpp
enum class InputState { Press, Hold, Release };
```

### D. 클래스 설계: Rule of 0/5
핵심:
1. 리소스 직접 소유가 없다면 Rule of 0(특수 멤버 자동).
2. 리소스 직접 소유가 있다면 복사/이동 정책을 명확히.

가르칠 포인트:
1. 복사 금지 객체는 `= delete`로 의도 명시.
2. 이동 가능한 타입은 `noexcept` 이동 생성자 우선 고려.

### E. STL 컨테이너와 알고리즘
핵심:
1. `vector`, `unordered_map`, `array`의 용도 차이를 구분한다.
2. "데이터 구조 선택 = 성능 설계"라는 감각을 만든다.

가르칠 포인트:
1. 연속 메모리 접근(`vector`)의 캐시 친화성.
2. 탐색 중심이면 `unordered_map`.
3. 반복 중 erase 패턴의 안정성.

예제:
```cpp
std::unordered_map<std::string, Action> actions;
auto it = actions.find("Jump");
if (it != actions.end()) { it->second(); }
```

### F. 상속 vs 조합(Component 설계)
핵심:
1. 기능 확장은 상속보다 조합이 유연한 경우가 많다.
2. GameObject + Component 구조는 조합 중심 설계다.

가르칠 포인트:
1. 상속은 "is-a", 조합은 "has-a".
2. 공통 인터페이스는 얇게, 기능은 컴포넌트로 분리.

### G. 템플릿 기초와 타입 안전성
핵심:
1. 템플릿은 코드 재사용 도구이자 타입 안전성 도구다.
2. 과도한 메타프로그래밍보다 읽기 쉬운 제네릭 코드를 우선한다.

예제:
```cpp
template <typename T>
T Clamp(T v, T lo, T hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}
```

### H. 오류 처리와 디버깅 습관
핵심:
1. 정상 흐름과 오류 흐름을 분리한다.
2. 디버그에서는 assert, 릴리즈에서는 복구/로그 정책.

프로젝트 연결:
1. 입력 장치 획득 실패 시 재시도/폴백 흐름 확인.
2. 빌드 설정은 `GameEngine/GameEngine.vcxproj`의 구성별 옵션 확인.

### I. 빌드 구성 이해(Debug/Release)
핵심:
1. Debug와 Release는 동작/성능이 다르다.
2. 경계 이슈(최적화, 초기화, assert 제거)를 구분해서 확인한다.

프로젝트 연결:
1. `GameEngine`은 정적 라이브러리, `Client`는 실행 파일로 분리.
2. PostBuild에서 `Copy.bat`을 통해 런타임 에셋을 동기화.

## 4) 4주 강의 운영안

### 1주차: 메모리와 소유권
1. 값/참조/포인터 + const
2. RAII와 스마트 포인터
3. 실습: raw pointer 코드를 `unique_ptr` 기반으로 바꾸기

### 2주차: 객체 설계와 상태
1. Rule of 0/5
2. enum class 상태 머신
3. 실습: Press/Hold/Release 입력 상태 테이블 구현

### 3주차: 컨테이너와 성능 감각
1. vector/unordered_map 선택 기준
2. 반복/삭제 패턴 안전성
3. 실습: 키 바인딩 테이블 성능 비교(선형 탐색 vs 해시)

### 4주차: 프로젝트 적용
1. 실제 코드 읽기(`GameEngine/World/Input.cpp`)
2. Debug/Release 차이 관찰
3. 실습: 입력 로깅(프레임, 키, 상태) 추가

## 5) 학생 과제 템플릿
1. 목표: 무엇을 개선할지 한 문장으로 정의.
2. 변경: 파일/함수 단위로 기록.
3. 검증: 실행 결과 + 로그/스크린샷.
4. 회고: 성능/안정성/가독성 중 무엇이 좋아졌는지 기록.

## 6) 코드 리뷰 체크리스트 (학생용)
1. 소유권이 명확한가?
2. null 가능성과 수명주기 위험을 처리했는가?
3. const 정확성이 유지되는가?
4. 컨테이너 선택 근거가 있는가?
5. 디버그/릴리즈에서 모두 동작하는가?
6. 로그/어설션으로 문제 재현이 가능한가?

## 7) 자주 하는 실수
1. `shared_ptr`를 습관적으로 남용함.
2. `const`를 빼서 함수 계약이 약해짐.
3. 반복 중 컨테이너 수정으로 iterator 무효화.
4. 릴리즈 빌드 테스트를 생략함.
5. "돌아가면 됨" 기준으로 종료해 재현성이 없음.

## 8) 최소 합격 기준 (실습 평가)
1. 컴파일/실행 성공.
2. 메모리 누수/명백한 크래시 없음.
3. 변경 의도와 근거를 설명 가능.
4. 코드가 1회성 해킹이 아닌 유지보수 가능한 형태.

## 9) 다음 단계
1. 입력 리플레이 기능으로 회귀 테스트 감각 익히기.
2. 셰이더/렌더 경계(CPU-GPU 데이터 계약) 학습 확장.
3. 자동 테스트 스모크 1개를 직접 구성해보기.
