# 04. 경제 시스템

## 4-1. 개요

경제 시스템은 매일(게임 내 하루) 수입과 지출을 정산하고 예산에 반영합니다.

```
수입원:  세금 (소비세 + 소득세 + 재산세)
         수출 수익
지출:    건물 유지비 (Upkeep)
         직원 급여 (Wage)
         수입 비용
         칙령 비용
```

---

## 4-2. 일일 정산 결과 구조체

```cpp
// EconomySystem.h
struct FDailyResult
{
    long long WageCost;              // 급여 지출
    long long UpkeepCost;            // 건물 유지비
    long long ImportExpense;         // 수입 비용
    long long ExportIncome;          // 수출 수익
    long long TaxIncome;             // 세금 합계
    long long ConsumptionTaxIncome;  // 소비세
    long long IncomeTaxIncome;       // 소득세
    long long PropertyTaxIncome;     // 재산세
    double    TaxCollectionEfficiency; // 세금 징수 효율 (0.0~1.0)
    long long NetChange;             // 순 변화량
};
```

구조체에 **모든 항목을 분리 저장**하는 이유:
- 알마낙 경제 페이지에서 항목별로 표시할 수 있습니다.
- 어떤 항목이 변화의 원인인지 추적이 쉽습니다.
- 디버깅 시 각 수치를 개별 확인 가능합니다.

---

## 4-3. 세금 정책

세금은 3가지 종류가 있으며, 각각 별도로 설정됩니다.

```cpp
struct FTaxPolicy
{
    int ConsumptionRatePercent = 10;  // 소비세 (0~20%)
    int IncomeRatePercent      = 12;  // 소득세 (0~30%)
    int PropertyRatePercent    = 35;  // 재산세 (0~60%)
};
```

| 세금 종류 | 기본값 | 최소 | 최대 | 조정 단위 |
|----------|--------|------|------|-----------|
| 소비세 | 10% | 0% | 20% | 2% |
| 소득세 | 12% | 0% | 30% | 2% |
| 재산세 | 35% | 0% | 60% | 5% |

---

## 4-4. 세금 부담 계산 (가중 평균)

시민이 세금에 얼마나 민감한지를 계산합니다.

```cpp
float GetCitizenTaxBurdenNormalized(
    const FTaxPolicy& TaxPolicy,
    bool IsWorker,      // 근로자이면 소득세 비중이 높음
    bool IsResident)    // 거주자이면 재산세 비중이 높음
{
    const float ConsumptionWeight = 0.40f;
    const float IncomeWeight   = IsWorker   ? 0.35f : 0.12f;
    const float PropertyWeight = IsResident ? 0.25f : 0.08f;

    // 기준값에서 얼마나 벗어났는지 정규화 (-1.0 ~ +1.0)
    float Deviation = (ConsumptionDeviation * ConsumptionWeight
                     + IncomeDeviation      * IncomeWeight
                     + PropertyDeviation    * PropertyWeight)
                    / TotalWeight;
    return Deviation;
}
```

**가중치 설계 관점**
- 소비세는 모든 시민에게 동일 비중(0.40)으로 영향을 줍니다.
- 근로자는 소득세(0.35 vs 0.12)에 더 민감합니다.
- 거주자는 재산세(0.25 vs 0.08)에 더 민감합니다.
- 이 수치들은 `GameBalanceTuning`으로 런타임에 조정 가능합니다.

---

## 4-5. 세금 편차 정규화

```cpp
float GetTaxPolicyDeviationNormalized(ETaxPolicyType Type, int CurrentRate)
{
    const int Default = GetTaxPolicyDefaultPercent(Type);

    if (CurrentRate == Default) return 0.f;   // 기준 = 0

    if (CurrentRate > Default)
    {
        // 기준보다 높음 → 양수 (0 ~ +1)
        return float(CurrentRate - Default) / float(Max - Default);
    }
    else
    {
        // 기준보다 낮음 → 음수 (-1 ~ 0)
        return -float(Default - CurrentRate) / float(Default - Min);
    }
}
```

- **0**: 기준 세율 (중립)
- **+1**: 최대 세율 (시민 불만 최대)
- **-1**: 0% (세금 없음, 예산 악화)

이 정규화 값이 시민 만족도 계산에 투입됩니다.

---

## 4-6. 세금 이벤트 시스템

세율을 극단적으로 올리면 이벤트가 발생합니다.

```cpp
enum class ETaxPolicyEventType
{
    None = 0,
    WorkerTaxStrike,      // 소득세 과세 → 노동자 파업
    PropertyTaxBacklash,  // 재산세 과세 → 재산권 반발
    BudgetCrisis          // 세금 부족 → 재정 위기
};
```

이벤트가 발생하면:
1. 시민 만족도에 패널티 적용
2. 제한 시간 내 해결하지 않으면 추가 페널티
3. 특정 칙령이 세금 이벤트 해결의 조건이 됩니다

---

## 4-7. 무역 시스템

```
FExportTradePolicy  — 자원 수출 가격 설정
FImportTradePolicy  — 자원 수입 가격 설정
TradeRouteRuntimeState — 활성 무역 노선 상태
```

- 각 자원별로 수출/수입 가격이 다릅니다.
- 무역 노선은 하버(Harbor) 건물을 통해 운영됩니다.
- 외교 상태에 따라 특정 무역 노선이 제한됩니다.

---

## 4-8. 칙령 비용 모디파이어

활성화된 칙령은 경제에 직접 영향을 줍니다.

```cpp
struct FGovernmentEdictModifiers
{
    float  ProductionMultiplier;         // 생산량 배율
    float  TaxRevenueMultiplier;         // 세수 배율
    float  ExportPriceMultiplier;        // 수출 가격 배율
    long long DailyBudgetDelta;          // 매일 예산 직접 증감
    long long DailyBudgetDeltaPerIndustryBuilding; // 산업 건물당 일별 예산 증감
    // ... 식량, 주거, 직업 등 만족도 직접 delta
};
```

모디파이어는 여러 칙령이 **합산**됩니다. 일일 정산 시 반영됩니다.

---

## 학습 포인트 요약

1. **구조체로 수입/지출 분리**: 항목별 추적이 가능해야 게임 피드백이 명확해집니다.
2. **정규화 (-1 ~ +1)**: 절대값 대신 편차를 정규화하면 여러 지표를 균일하게 결합할 수 있습니다.
3. **가중 평균**: 시민 유형(근로자/거주자)에 따라 가중치를 달리하면 현실적인 시뮬레이션이 됩니다.
4. **이벤트 트리거**: 극단적 정책 → 이벤트 → 플레이어 대응 패턴이 게임플레이 긴장감을 만듭니다.
5. **모디파이어 합산**: 칙령 효과를 별도 구조체로 관리하면 스택 계산이 깔끔해집니다.
