# 시나리오 모드 과제 정리

이 문서는 현재 구현 기준으로 시나리오 모드의 과제 흐름을 정리한 메모입니다.

기준 파일:
- `Client/World/ScenarioRunner.h`
- `Client/World/ScenarioSubsystem.cpp`
- `Client/UI/TaskWidget.cpp`
- `Client/World/EraSubsystem.cpp`
- `Client/World/MainWorldBootstrap.cpp`

## 1. 전체 흐름

시나리오 모드는 일반 시대 진행 과제와 달리 `ScenarioSubsystem`이 단계별 과제를 직접 주입합니다.

게임 시작 시:
- `MainWorldBootstrap.cpp`에서 시나리오 모드이면 `mScenario->TickPhase()`를 즉시 1회 호출
- `Intro` 단계는 바로 `SmugglersOffer` 단계로 넘어감
- 이 과정에서 과제가 생성되고 `TaskWidget`이 바로 열릴 수 있음

단계 enum:
1. `Intro`
2. `SmugglersOffer`
3. `PenultimoFarm`
4. `SmugglersRumSale`
5. `CrownExploitation`
6. `IndependencePrep`
7. `PeacePayment`
8. `EraTransitionReady`

## 2. 단계별 과제

| 단계 | 과제명 | 조건 | 보상/결과 | 비고 |
|---|---|---|---|---|
| `SmugglersOffer` | 밀수업자의 제안 | 럼주 증류소 1개 건설 | 밀수 판로 개척 단계로 진행 준비 | 완료 시 밀수업자 럼주 무역 제안 생성 |
| `PenultimoFarm` | 패놀티모의 조언 | 설탕 농장 1개 건설 | 럼주 생산 기반 확보 | 다음 단계에서 럼주 판매 과제 시작 |
| `SmugglersRumSale` | 밀수 판로 개척 | 밀수업자와 럼주 무역로 개설 후 계약 물량 수출 완료 | 럼주 밀수 완료 | `ScenarioTag=1001` 무역로 완료 여부로 판정 |
| `CrownExploitation` | 왕실의 명령 | 왕실과 럼주 무역로 개설 후 계약 물량 수출 완료 | 왕실 관계 유지 | 수락 후 이행 중 럼주 판매가격 `-50%` |
| `IndependencePrep` | 독립을 준비하라 | 군사 건물 인원 8명 확보 | 독립 준비 완료 | 월드 스냅샷의 `MilitaryWorkerCount >= 8`로 판정 |
| `PeacePayment` | 왕실의 협상 | 국고 `$10,000` 확보 후 지불 버튼 클릭 | 시나리오 승리 | 버튼형 특수 과제 |
| `EraTransitionReady` | 시나리오 승리 | 별도 과제 없음 | 시나리오 승리, 이후 별도 시나리오 없음 | 과제 UI는 승리로 표시하고 필요하면 시대 전환 선택 안내 |

## 3. 단계 전환 규칙

핵심은 "과제를 수락한 뒤 조건을 만족하면 다음 단계로 넘어간다"는 구조입니다. 다만 일부 단계는 단순 카운터가 아니라 무역 완료나 버튼 클릭으로 판정됩니다.

전환 규칙:
1. `Intro`는 시작 직후 자동으로 `SmugglersOffer` 진입
2. `SmugglersOffer`는 럼주 생산 건물 수가 1 이상이면 완료
3. 완료 시 밀수업자 럼주 무역 제안을 열고 `PenultimoFarm` 진입
4. `PenultimoFarm`은 설탕 생산 건물 수가 1 이상이면 완료
5. `SmugglersRumSale`은 시나리오 전용 밀수 럼주 무역로 완료 시 완료
6. `CrownExploitation`은 왕실 럼주 무역로 완료 시 완료
7. `IndependencePrep`은 군사 인원 8명 달성 시 완료
8. `PeacePayment`은 사용자가 직접 지불 액션을 눌러야 완료
9. 지불 성공 시 `EraTransitionReady`로 들어가며 시나리오 승리 상태가 열린다
10. 이후 별도 시나리오는 없고, 더 플레이하고 싶다면 플레이어가 직접 다음 시대 전환을 선택할 수 있다

## 4. 특수 처리

### 4-1. 럼주 무역 제안 유지

시나리오 럼주 무역 제안은 연간 갱신 등으로 일반 무역 제안이 비워져도 다시 주입됩니다.

- 밀수업자 럼주 제안: `ScenarioTag=1001`
- 왕실 럼주 제안: `ScenarioTag=1002`
- 활성 무역로도 없고 제안도 없으면 `OpenSmugglersRumRoute()` 또는 `OpenCrownRumRoute()`로 재생성

### 4-2. 왕실 착취 단계 가격 패널티

`CrownExploitation` 단계에서 왕실 요구를 수락하면:
- `ResourceTradePricing::GetScenarioRumExportBiasPercent() = -50`
- 즉, 럼주 판매가격이 절반 수준으로 적용됨

요구가 종료되거나 완료되면:
- 가격 보정값을 `0`으로 복구
- 왕실 전용 럼주 제안도 정리

### 4-3. 평화 배상금 단계

`PeacePayment`은 일반 과제처럼 자동 완료되지 않습니다.

- 목표 텍스트: `국고 $10,000 확보 후 지불 버튼 클릭`
- 예산이 부족하면 지불 실패 메시지 반환
- 예산이 충분하면 `$10,000` 차감 후 과제 완료 처리
- 이후 시나리오 승리 상태를 열고, 별도 후속 시나리오 없이 플레이어가 원하면 시대 전환 가능

## 5. TaskWidget 표시 방식

시나리오 모드에서 외세 과제는 `TaskWidget`에서 일반 외교 요청과 다르게 표시됩니다.

- 외세 과제라도 `Entry.IsScenarioTask = true`로 표시
- 발신 라벨은 국가명 대신 `시나리오 과제`
- `ObjectiveType == None`인 시나리오 과제는 일반 진행 카운터 대신 전용 목표 문구 사용
- `PeacePayment`처럼 `TargetValue > 0`인 특수 과제는 `지불` 버튼이 활성화될 수 있음

즉, 구현상 시나리오 과제는 "외세 요구 슬롯을 재사용"하지만 UI에서는 별도 시나리오 과제로 보이게 처리되어 있습니다.

## 6. 시대 전환과의 관계

일반 모드:
- 다음 시대 조건 `NextEraReady`가 충족되어야 시대 전환 가능

시나리오 모드:
- `EraTransitionUnlocked`가 켜지기 전까지는 시대 전환 차단
- `EraTransitionReady` 단계에 도달하면 `EraSubsystem`이 예외적으로 전환 허용

현재 구현에서는 평화 배상금 지불 성공 시:
- `EraTransitionUnlocked = true`
- `RefreshEraTransitionState()`
- 과제 UI에서 시나리오 승리 문구와 "이후 시나리오는 없고, 더 하고 싶으면 다음 시대로 전환" 안내를 표시

즉, 시나리오 모드는 마지막 결재 직후 시나리오 승리 상태를 열고, 이후 별도 시나리오 없이 시대 전환 여부만 플레이어 선택으로 넘깁니다.

## 7. 실무 메모

정리하면 현재 시나리오 모드 과제는 아래 성격을 가집니다.

- 초반 건설 유도: 럼주 증류소, 설탕 농장
- 중반 무역 튜토리얼: 밀수업자/왕실 럼주 계약
- 후반 군사 준비: 군사 인원 8명
- 마무리 결재: 배상금 `$10,000` 지불

추가로 수정할 때 주의할 점:
- 단순 텍스트만 바꾸면 안 되고 `TickPhase()`의 완료 조건도 같이 맞춰야 함
- 무역형 과제는 `ObjectiveType=None`이라 UI 카운터가 자동으로 안 붙음
- 왕실 단계는 가격 패널티 복구 누락 시 전체 럼주 경제가 계속 왜곡될 수 있음
- 마지막 단계는 버튼 액션 기반이라 `TaskWidget` 쪽 처리도 함께 봐야 함
