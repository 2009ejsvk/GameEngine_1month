  2. 충돌 프로파일 관리자 (CollisionInfoManager)

  2-1. 헤더 파일 (CollisionInfoManager.h)

  #pragma once
  #include "EngineInfo.h"

  // ============================================
  // 충돌 프로파일 관리자 (싱글톤)
  // - 채널(Channel) 생성 및 관리
  // - 프로파일(Profile) 생성 및 관리
  // - 충돌 규칙 설정
  // ============================================
  class CCollisionInfoManager
  {
  private:
      // 싱글톤 패턴: 생성자를 private으로
      CCollisionInfoManager();
      ~CCollisionInfoManager();

  private:
      // 채널 이름으로 검색하는 맵
      // key: 채널 이름(string), value: 채널 정보(FCollisionChannel*)
      std::unordered_map<std::string, FCollisionChannel*> mChannelMap;

      // 다음에 생성할 채널의 타입
      // Static부터 시작해서 하나씩 증가
      ECollisionChannel::Type mCreateChannel = ECollisionChannel::Static;

      // 프로파일 이름으로 검색하는 맵
      // key: 프로파일 이름(string), value: 프로파일 정보(FCollisionProfile*)
      std::unordered_map<std::string, FCollisionProfile*> mProfileMap;

  public:
      // 기본 채널과 프로파일 초기화
      bool Init();

      // 새로운 채널 생성 (Custom1~Custom10 사용)
      void CreateChannel(const std::string& Name);

      // 새로운 프로파일 생성 (채널 이름으로)
      // Name: 프로파일 이름
      // ChannelName: 속할 채널 이름
      // Enable: 충돌 활성화 여부
      // DefaultInteraction: 모든 채널에 대한 기본 상호작용
      bool CreateProfile(const std::string& Name,
          const std::string& ChannelName, bool Enable,
          ECollisionInteraction::Type DefaultInteraction =
          ECollisionInteraction::Collision);

      // 새로운 프로파일 생성 (채널 타입으로)
      bool CreateProfile(const std::string& Name,
          ECollisionChannel::Type Channel, bool Enable,
          ECollisionInteraction::Type DefaultInteraction =
          ECollisionInteraction::Collision);

      // 프로파일의 특정 채널과의 상호작용 설정 (채널 이름으로)
      // 예: SetProfileInteraction("Player", "Monster", Collision)
      // → Player 프로파일이 Monster 채널과 충돌하도록 설정
      bool SetProfileInteraction(const std::string& Name,
          const std::string& ChannelName,
          ECollisionInteraction::Type Interaction);

      // 프로파일의 특정 채널과의 상호작용 설정 (채널 타입으로)
      bool SetProfileInteraction(const std::string& Name,
          ECollisionChannel::Type Channel,
          ECollisionInteraction::Type Interaction);

      // 프로파일 활성화/비활성화
      bool SetProfileEnable(const std::string& Name, bool Enable);

      // 프로파일 검색
      FCollisionProfile* FindProfile(const std::string& Name);

  private:
      // 채널 검색 (내부용)
      FCollisionChannel* FindChannel(const std::string& Name);

  private:
      // 싱글톤 인스턴스
      static CCollisionInfoManager* mInst;

  public:
      // 싱글톤 인스턴스 얻기
      static CCollisionInfoManager* GetInst()
      {
          if (!mInst)
              mInst = new CCollisionInfoManager;
          return mInst;
      }

      // 싱글톤 인스턴스 파괴
      static void DestroyInst()
      {
          SAFE_DELETE(mInst);
      }
  };

  2-2. 구현 파일 (CollisionInfoManager.cpp)

  #include "CollisionInfoManager.h"

  // 싱글톤 인스턴스 초기화
  CCollisionInfoManager* CCollisionInfoManager::mInst = nullptr;

  CCollisionInfoManager::CCollisionInfoManager()
  {
  }

  CCollisionInfoManager::~CCollisionInfoManager()
  {
      // 모든 프로파일 메모리 해제
      auto iter = mProfileMap.begin();
      auto iterEnd = mProfileMap.end();
      for (; iter != iterEnd; ++iter)
      {
          SAFE_DELETE(iter->second);
      }

      // 모든 채널 메모리 해제
      auto iter1 = mChannelMap.begin();
      auto iter1End = mChannelMap.end();
      for (; iter1 != iter1End; ++iter1)
      {
          SAFE_DELETE(iter1->second);
      }
  }

  // ============================================
  // 초기화: 기본 채널과 프로파일 생성
  // ============================================
  bool CCollisionInfoManager::Init()
  {
      // 기본 채널 3개 생성
      CreateChannel("Static");    // 정적 오브젝트
      CreateChannel("Player");    // 플레이어
      CreateChannel("Monster");   // 몬스터

      // 기본 프로파일 3개 생성
      // 기본적으로 모든 채널과 충돌하도록 설정됨
      CreateProfile("Static", "Static", true);
      CreateProfile("Player", "Player", true);
      CreateProfile("Monster", "Monster", true);

      return true;
  }

  // ============================================
  // 새로운 채널 생성
  // ============================================
  void CCollisionInfoManager::CreateChannel(const std::string& Name)
  {
      // 더 이상 생성할 수 있는 채널이 없으면 종료
      if (mCreateChannel == ECollisionChannel::End)
          return;

      // 이미 존재하는 채널이면 종료
      FCollisionChannel* Channel = FindChannel(Name);
      if (Channel)
          return;

      // 새 채널 생성
      Channel = new FCollisionChannel;
      Channel->Name = Name;
      Channel->Channel = mCreateChannel;

      // 다음 채널 타입으로 증가 (Static → Player → Monster → Custom1 ...)
      mCreateChannel = (ECollisionChannel::Type)(mCreateChannel + 1);

      // 맵에 등록
      mChannelMap.insert(std::make_pair(Name, Channel));
  }

  // ============================================
  // 새로운 프로파일 생성 (채널 이름으로)
  // ============================================
  bool CCollisionInfoManager::CreateProfile(const std::string& Name,
      const std::string& ChannelName, bool Enable,
      ECollisionInteraction::Type DefaultInteraction)
  {
      // 이미 존재하는 프로파일이면 성공 반환
      FCollisionProfile* Profile = FindProfile(Name);
      if (Profile)
          return true;

      // 채널 찾기
      FCollisionChannel* Channel = FindChannel(ChannelName);
      if (!Channel)
          return false;  // 채널이 없으면 실패

      // 새 프로파일 생성
      Profile = new FCollisionProfile;
      Profile->Name = Name;
      Profile->Channel = Channel;
      Profile->Enable = Enable;

      // 모든 채널에 대해 기본 상호작용 설정
      // 예: DefaultInteraction이 Collision이면
      // 모든 채널과 충돌하도록 초기화
      for (int i = 0; i < ECollisionChannel::End; ++i)
      {
          Profile->Interaction[i] = DefaultInteraction;
      }

      // 맵에 등록
      mProfileMap.insert(std::make_pair(Name, Profile));

      return true;
  }

  // ============================================
  // 새로운 프로파일 생성 (채널 타입으로)
  // ============================================
  bool CCollisionInfoManager::CreateProfile(const std::string& Name,
      ECollisionChannel::Type Channel, bool Enable,
      ECollisionInteraction::Type DefaultInteraction)
  {
      // 이미 존재하는 프로파일이면 성공 반환
      FCollisionProfile* Profile = FindProfile(Name);
      if (Profile)
          return true;

      // 채널 타입으로 채널 객체 찾기
      FCollisionChannel* ChannelObject = nullptr;
      auto iter = mChannelMap.begin();
      auto iterEnd = mChannelMap.end();
      for (; iter != iterEnd; ++iter)
      {
          if (iter->second->Channel == Channel)
          {
              ChannelObject = iter->second;
              break;
          }
      }

      if (!ChannelObject)
          return false;  // 채널이 없으면 실패

      // 새 프로파일 생성 (위와 동일)
      Profile = new FCollisionProfile;
      Profile->Name = Name;
      Profile->Channel = ChannelObject;
      Profile->Enable = Enable;

      for (int i = 0; i < ECollisionChannel::End; ++i)
      {
          Profile->Interaction[i] = DefaultInteraction;
      }

      mProfileMap.insert(std::make_pair(Name, Profile));

      return true;
  }

  // ============================================
  // 프로파일의 특정 채널과의 상호작용 설정
  // ============================================
  bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
      const std::string& ChannelName, ECollisionInteraction::Type Interaction)
  {
      // 프로파일 찾기
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      // 채널 찾기
      FCollisionChannel* Channel = FindChannel(ChannelName);
      if (!Channel)
          return false;

      // 상호작용 설정
      // 예: Profile->Interaction[Monster] = Ignore
      // → 이 프로파일은 Monster 채널과 충돌하지 않음
      Profile->Interaction[Channel->Channel] = Interaction;

      return true;
  }

  bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name,
      ECollisionChannel::Type Channel, ECollisionInteraction::Type Interaction)
  {
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      Profile->Interaction[Channel] = Interaction;

      return true;
  }

  // ============================================
  // 프로파일 활성화/비활성화
  // ============================================
  bool CCollisionInfoManager::SetProfileEnable(const std::string& Name,
      bool Enable)
  {
      FCollisionProfile* Profile = FindProfile(Name);
      if (!Profile)
          return false;

      Profile->Enable = Enable;

      return true;
  }

  // ============================================
  // 프로파일 검색
  // ============================================
  FCollisionProfile* CCollisionInfoManager::FindProfile(const std::string& Name)
  {
      auto iter = mProfileMap.find(Name);

      if (iter == mProfileMap.end())
          return nullptr;

      return iter->second;
  }

  // ============================================
  // 채널 검색
  // ============================================
  FCollisionChannel* CCollisionInfoManager::FindChannel(const std::string& Name)
  {
      auto iter = mChannelMap.find(Name);

      if (iter == mChannelMap.end())
          return nullptr;

      return iter->second;
  }

  ---
