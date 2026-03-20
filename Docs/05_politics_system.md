# 05. 정치 & 선거 시스템

## 5-1. 정치 파벌

8개의 정치 파벌이 있으며, 각각 2개의 정치 축에 속합니다.

```cpp
enum class EPoliticalFaction
{
    Communists,       // 공산주의자 — 경제 축 / 좌파
    Capitalists,      // 자본주의자 — 경제 축 / 우파
    Religious,        // 종교 — 종교/군사 축 / 좌파
    Militarists,      // 군국주의자 — 종교/군사 축 / 우파
    Environmentalists,// 환경주의자 — 환경/산업 축 / 좌파
    Industrialists,   // 산업주의자 — 환경/산업 축 / 우파
    Intellectuals,    // 지식인 — 지식/보수 축 / 좌파
    Conservatives,    // 보수주의자 — 지식/보수 축 / 우파
};
```

| 축 (Axis) | 좌파 | 우파 |
|----------|------|------|
| Economy | 공산주의자 | 자본주의자 |
| ReligionMilitarism | 종교 | 군국주의자 |
| EnvironmentIndustry | 환경주의자 | 산업주의자 |
| IntellectualConservative | 지식인 | 보수주의자 |

---

## 5-2. 파벌 지지율 스냅샷

```cpp
struct FPoliticalFactionSnapshot
{
    int    MemberCount;           // 해당 파벌 시민 수
    double AverageApproval;       // 평균 지지율 (0~100)
    double AverageLifeScore;      // 생활 수준 점수
    double AverageBuildingScore;  // 건물 접근성 점수
    double AverageActionScore;    // 정부 행동(칙령/정책) 점수
    double AverageAlignmentScore; // 정치 성향 일치도 점수
    double ApprovalModifier;      // 파벌 추가 보정치
    int    PressureDays;          // 요구 사항 압박 기간
    EPoliticalDemandStage DemandStage; // 요구 단계
};
```

---

## 5-3. 시민 정치 평가 모델

각 시민은 독재자를 얼마나 지지하는지 개별적으로 평가합니다.

```cpp
struct FCitizenPoliticalEvaluation
{
    float LifeScore;             // 생활 수준 평가
    float GovernmentIdeologyScore; // 정부 이념 일치도
    float BuildingScore;         // 건물 서비스 접근성
    float ActionScore;           // 정부 행동 점수
    float FearScore;             // 공포 점수 (계엄령 등)
    float TotalSupportScore;     // 최종 지지율 (0~100, 기본 50)
    float FactionAlignmentScore; // 자신이 속한 파벌과의 일치도
    float FactionApprovalScore;  // 파벌 전체 지지율
    EPoliticalFaction PrimaryFaction; // 주 소속 파벌
    EVoteIntent VoteIntent;      // 투표 의향 (지지/반대/기권)
};
```

**지지율 계산 흐름**
```
기본 50점
  + LifeScore        (만족도 → 정치 전환)
  + IdeologyScore    (정부 이념과 내 성향 일치 시 보너스)
  + BuildingScore    (병원/학교 등 접근성)
  + ActionScore      (최근 칙령/정책 효과)
  + FearScore        (계엄령 등 강압 효과)
= TotalSupportScore → 50 이상이면 Incumbent(집권 지지)
                    → 50 미만이면 Opposition(반대)
```

---

## 5-4. 선거 시스템

```cpp
struct FElectionStatus
{
    bool  HasRecordedElection;       // 선거 기록 여부
    bool  IncumbentWonLastElection;  // 현 집권자 승리 여부
    bool  GameLost;                  // 게임 패배 여부
    int   NextElectionYear;          // 다음 선거 날짜
    int   ElectionsWon;              // 누적 선거 승리 횟수
    int   LastIncumbentVotes;        // 직전 선거 득표
    int   LastOppositionVotes;       // 직전 선거 반대 득표
    double LastVoteShare;            // 득표율
    double LastTurnoutPercent;       // 투표율
    // ... 공약 관련 필드
};
```

**선거 진행 흐름**
```
1. NextElectionDate 도달
2. 모든 시민 VoteIntent 집계
3. Incumbent > 50% → 플레이어 승리 → ElectionsWon++
4. Incumbent <= 50% → 게임 오버 (GameLost = true)
```

---

## 5-5. 선거 공약 시스템

선거 전에 공약을 발표해 득표에 영향을 줄 수 있습니다.

```cpp
struct FElectionPromiseState
{
    bool   Active;
    EElectionPromiseType Type;    // 주거/식량/건강/직업/자유/치안/신앙/수출
    int    BaselineValue;          // 공약 발표 시점의 기준값
    int    TargetValue;            // 달성 목표값
    int    CurrentValue;           // 현재 실제값
    int    SuccessVoteModifierPercent;  // 달성 시 득표율 보정
    int    FailureVoteModifierPercent;  // 실패 시 득표율 페널티
};
```

- 공약 달성 → 선거에서 보너스 득표
- 공약 미달 → 선거에서 페널티
- 최대 `GElectionPromiseCount = 2` 개까지 공약 가능

---

## 5-6. 파벌 요구 사항 시스템

파벌이 만족하지 못하면 점점 강도가 높아지는 요구를 합니다.

```cpp
enum class EPoliticalDemandStage
{
    Warning,   // 경고: 조치 없으면 다음 단계로
    Demand,    // 요구: 수락/거절 선택
    Ultimatum, // 최후통첩: 거절 시 큰 페널티
    Revolt     // 반란: 게임 위기
};
```

```cpp
struct FPoliticalDemandState
{
    EPoliticalDemandObjectiveType ObjectiveType; // 무엇을 요구하는가
    // 주거/식량/신앙/치안/자유/건강/수출/세금/무역노선
    int  TargetValue;        // 목표 수치
    int  CurrentValue;       // 현재 수치
    int  RemainingDays;      // 기한
    long long RewardBudgetDelta;        // 달성 시 보상
    int  RewardFactionApprovalDelta;    // 파벌 지지율 보상
    long long PenaltyBudgetDelta;       // 실패 시 페널티
    int  PenaltyFactionApprovalDelta;   // 파벌 지지율 페널티
};
```

---

## 5-7. 세계 위기 이벤트

극단적인 정책이나 파벌 무시 시 위기가 발생합니다.

```cpp
enum class EWorldCrisisType
{
    None,
    Raid,           // 침략/습격
    LaborStrike,    // 노동자 파업
    CrimeWave,      // 범죄 급증
    FiscalEmergency // 재정 위기
};
```

위기 상태 구조체:
```cpp
struct FWorldCrisisStatus
{
    bool   Active;
    int    RemainingDays;    // 지속 기간
    int    CooldownDays;     // 다음 위기까지 쿨다운
    int    DaysActive;       // 활성 지속 일수
};
```

---

## 5-8. 칙령 시스템

칙령은 시대별로 잠금 해제되는 정책들입니다.

```cpp
enum class EEdictEra
{
    Colonial,   // 식민지 시대
    WorldWars,  // 세계대전
    ColdWar,    // 냉전
    Modern      // 현대
};
```

칙령 슬롯은 `7열 × 2행 = 14개` 의 그리드로 표시됩니다.
시대별로 탭이 나뉘며 (4개 탭), 칙령 간 충돌이나 비용이 있습니다.

**칙령 발동 순서**
```
플레이어 칙령 선택
  → EdictSystem 유효성 검사 (시대, 비용, 충돌 여부)
  → FGovernmentEdictState 에 등록 (Active = true)
  → 매일 FGovernmentEdictModifiers 에 효과 반영
  → RemainingDays 감소 → 0이 되면 만료
```

---

## 학습 포인트 요약

1. **다축 정치 모델**: 단순 좌우 스펙트럼이 아닌 4개 축으로 다양한 정치 성향을 표현합니다.
2. **개별 시민 평가 → 집계**: 각 시민을 독립적으로 평가해 집계하면 현실적인 선거 결과가 나옵니다.
3. **단계적 요구 시스템**: 경고 → 요구 → 최후통첩 → 반란 의 단계 설계로 플레이어에게 대응 시간을 줍니다.
4. **공약 시스템**: 달성/실패에 따른 보상/패널티로 전략적 선택을 만듭니다.
5. **시대별 잠금 해제**: 시간이 지나야 열리는 콘텐츠로 게임의 깊이와 진행감을 만듭니다.
