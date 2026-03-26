#pragma once

// 시나리오 진행 단계
enum class EScenarioPhase
{
    Intro = 0,          // 게임 시작 인트로 표시
    SmugglersOffer,     // 밀수업자 제안: 럼주 증류소 1개 건설
    PenultimoFarm,      // 패놀티모 조언: 설탕 농장 1개 건설
    SmugglersRumSale,   // 밀수 판로 개척: 밀수업자와 럼주 무역로 1개 개설
    CrownExploitation,  // 왕실 압박: 왕실과 럼주 무역로 1개 (가격 -50%)
    IndependencePrep,   // 독립 준비: 군사 건물 인원 8명
    PeacePayment,       // 왕실 배상금: 국고 $10,000 지불
    EraTransitionReady  // 독립 선언 가능 (시대 전환 언락)
};
