## 왜 "Mesh_" 접두사를 사용하는가? - 헷갈리는 네이밍 규칙 해설

"왜 굳이 `Mesh_LineUP2D`처럼 접두사를 붙여서 헷갈리게 만들었을까?" - 이 질문에 대한 명확한 답변입니다.

---

## TL;DR (요약)

**짧은 답:**
- **MeshManager**: `"Mesh_LineUP2D"` ← 모든 에셋을 하나의 맵에 저장하기 때문
- **WorldAssetManager**: `"LineUP2D"` ← 사용자 편의를 위해 접두사 자동 추가

**진짜 이유:**
> "Player"라는 이름의 메시도, 텍스처도, 애니메이션도 있을 수 있다. 모두 다른 것인데 이름만 봐서는 구분이 안 된다. 그래서 접두사로 타입을 명시한다.

---

## 1. 문제 상황: 이름 충돌

### 1-1. 접두사가 없다면?

게임을 만들다 보면 같은 이름을 여러 곳에서 사용하게 됩니다:

```cpp
// 플레이어 관련 에셋들
CreateMesh("Player");           // 플레이어 3D 모델
LoadTexture("Player");          // 플레이어 텍스처 이미지
CreateAnimation("Player");      // 플레이어 애니메이션

// 문제: 모두 "Player"라는 이름!
```

**이 에셋들을 하나의 맵에 저장한다면?**

```cpp
std::unordered_map<std::string, std::shared_ptr<CAsset>> mAssetMap;

// ❌ 이름 충돌 발생!
mAssetMap["Player"] = meshPlayer;        // 저장
mAssetMap["Player"] = texturePlayer;     // 덮어씀! (메시 사라짐)
mAssetMap["Player"] = animationPlayer;   // 또 덮어씀! (텍스처 사라짐)

// 결과: 마지막에 저장한 애니메이션만 남음
```

### 1-2. 실제 코드에서의 구조

**AssetManager의 각 매니저:**
```cpp
class CAssetManager
{
private:
    std::shared_ptr<CMeshManager>         mMeshManager;
    std::shared_ptr<CTextureManager>      mTextureManager;
    std::shared_ptr<CAnimation2DManager>  mAnimation2DManager;
    std::shared_ptr<CShaderManager>       mShaderManager;
};
```

각 매니저는 **독자적인 맵**을 가지고 있습니다:

```cpp
// MeshManager.h
class CMeshManager
{
private:
    std::unordered_map<std::string, std::shared_ptr<CMesh>> mMeshMap;
};

// TextureManager.h
class CTextureManager
{
private:
    std::unordered_map<std::string, std::shared_ptr<CTexture>> mTextureMap;
};
```

**여기까진 괜찮습니다.** 각자의 맵이 분리되어 있으니까요.

---

## 2. 진짜 문제: WorldAssetManager의 통합 맵

### 2-1. WorldAssetManager는 모든 에셋을 하나로 관리

**WorldAssetManager.h:**
```cpp
class CWorldAssetManager
{
private:
    // ⚠️ 모든 타입의 에셋을 하나의 맵에 저장!
    std::unordered_map<std::string, std::shared_ptr<class CAsset>> mAssetMap;
};
```

**왜 통합 맵을 사용하는가?**
- 월드별로 사용하는 에셋을 한 곳에서 추적
- 월드 종료 시 사용한 에셋을 한번에 정리
- 타입에 상관없이 일괄 관리

### 2-2. 통합 맵의 문제점

```cpp
// mAssetMap에 모든 에셋을 저장한다면?
mAssetMap["Player"] = meshPlayer;        // Mesh
mAssetMap["Player"] = texturePlayer;     // Texture (충돌!)
mAssetMap["Player"] = animationPlayer;   // Animation (충돌!)

// ❌ 이름이 같아서 덮어써짐!
```

### 2-3. 해결책: 접두사로 타입 구분

```cpp
// ✅ 접두사를 붙이면 모두 다른 키가 됨
mAssetMap["Mesh_Player"]        = meshPlayer;
mAssetMap["Texture_Player"]     = texturePlayer;
mAssetMap["Animation2D_Player"] = animationPlayer;

// 충돌 없음! 완벽하게 구분됨!
```

---

## 3. 설계 구조: 2-Layer 시스템

### 3-1. Layer 1: Manager (내부 구현)

**MeshManager.cpp:**
```cpp
// 엔진 초기화 시 메시 생성
CreateMesh("Mesh_LineUP2D", LineUp, ...);
CreateMesh("Mesh_CenterRect", CenterRect, ...);
CreateMesh("Mesh_FrameSphere2D", FrameSphere, ...);

// mMeshMap에 저장:
// "Mesh_LineUP2D" → CMesh*
// "Mesh_CenterRect" → CMesh*
```

**TextureManager.cpp:**
```cpp
// 텍스처 로드
LoadTexture("Texture_Player", "player.png");
LoadTexture("Texture_Enemy", "enemy.png");

// mTextureMap에 저장:
// "Texture_Player" → CTexture*
// "Texture_Enemy" → CTexture*
```

**이 레이어는 접두사를 명시적으로 사용합니다.**

### 3-2. Layer 2: WorldAssetManager (사용자 인터페이스)

**사용자가 사용하는 코드:**
```cpp
// 클라이언트 코드 (MainWorld.cpp 등)
mWorldAssetManager->CreateMesh("Player");         // 간단!
mWorldAssetManager->LoadTexture("Enemy");         // 간단!
mWorldAssetManager->CreateAnimation("Walk");      // 간단!
```

**WorldAssetManager 내부에서 자동으로 접두사 추가:**
```cpp
// WorldAssetManager.cpp
std::weak_ptr<CMesh> CWorldAssetManager::FindMesh(const std::string& Name)
{
    // ★ 자동으로 접두사 추가
    std::string Key = "Mesh_" + Name;  // "Player" → "Mesh_Player"

    // MeshManager에서 찾기
    auto meshMgr = CAssetManager::GetInst()->GetMeshManager().lock();
    return meshMgr->FindMesh(Key);
}

std::weak_ptr<CTexture> CWorldAssetManager::FindTexture(const std::string& Name)
{
    // ★ 자동으로 접두사 추가
    std::string Key = "Texture_" + Name;  // "Enemy" → "Texture_Enemy"

    auto textureMgr = CAssetManager::GetInst()->GetTextureManager().lock();
    return textureMgr->FindTexture(Key);
}
```

---

## 4. 구체적인 예시: ColliderLine2D

### 4-1. 메시 생성 (엔진 초기화)

**MeshManager.cpp:**
```cpp
// 엔진 시작 시 자동 생성
FVector3 LineUp[2] = {
    FVector3(0.f, 0.f, 0.f),
    FVector3(0.f, 1.f, 0.f)
};

// ★ "Mesh_" 접두사와 함께 저장
CreateMesh("Mesh_LineUP2D", LineUp, sizeof(FVector3), 2, ...);

// mMeshMap["Mesh_LineUP2D"] = shared_ptr<CMesh>
```

### 4-2. 메시 로드 (런타임)

**ColliderLine2D.cpp - 케이스 1 (WorldAssetManager 사용):**
```cpp
auto World = mWorld.lock();
if (World)
{
    auto AssetMgr = World->GetWorldAssetManager().lock();

    // ★ 접두사 없이 호출
    mMesh = AssetMgr->FindMesh("LineUP2D");

    // 내부에서 "Mesh_LineUP2D"로 변환됨
}
```

**ColliderLine2D.cpp - 케이스 2 (MeshManager 직접 사용):**
```cpp
else
{
    auto MeshMgr = CAssetManager::GetInst()->GetMeshManager().lock();

    // ★ 직접 사용 시에는 접두사 포함
    mMesh = MeshMgr->FindMesh("Mesh_LineUP2D");
}
```

### 4-3. 왜 두 가지 방법이 있는가?

**WorldAssetManager 사용 (권장):**
- ✅ 접두사 자동 처리
- ✅ 월드별 에셋 추적
- ✅ 깔끔한 코드

**MeshManager 직접 사용 (예외 상황):**
- World가 아직 생성되지 않았을 때 (드물음)
- 전역 에셋을 직접 접근해야 할 때

---

## 5. 전체 에셋 타입별 접두사

### 5-1. 접두사 규칙

| 에셋 타입 | Manager 저장 키 | WorldAssetManager 호출 | 실제 저장소 키 |
|-----------|----------------|----------------------|---------------|
| Mesh | `CreateMesh("Mesh_LineUP2D", ...)` | `FindMesh("LineUP2D")` | `"Mesh_LineUP2D"` |
| Texture | `LoadTexture("Texture_Player", ...)` | `FindTexture("Player")` | `"Texture_Player"` |
| Animation2D | `CreateAnimation("Animation2D_Walk", ...)` | `FindAnimation("Walk")` | `"Animation2D_Walk"` |
| Shader | `CreateShader("Collider", ...)` | ❌ (WorldAssetManager 없음) | `"Collider"` |

### 5-2. Shader는 왜 접두사가 없나?

**이유:**
- Shader는 **전역 자산**으로만 사용됨
- 모든 월드에서 공유
- WorldAssetManager를 거치지 않음
- 이름 충돌 가능성이 낮음

**사용 예:**
```cpp
// Shader는 직접 ShaderManager에서 가져옴
auto shaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();
mShader = shaderMgr->FindShader("Collider");  // 접두사 없음
```

---

## 6. 왜 이렇게 복잡하게 만들었는가?

### 6-1. 장점

**1) 이름 충돌 완벽 방지**
```cpp
// 모두 다른 에셋이지만 같은 이름 사용 가능
mAssetMap["Mesh_Player"]        = ...;
mAssetMap["Texture_Player"]     = ...;
mAssetMap["Animation2D_Player"] = ...;
// ✅ 충돌 없음!
```

**2) 타입 명시성**
```cpp
// 이름만 봐도 타입을 알 수 있음
"Mesh_LineUP2D"        → 메시구나!
"Texture_PlayerIdle"   → 텍스처구나!
"Animation2D_Walk"     → 애니메이션이구나!
```

**3) 디버깅 편의성**
```cpp
// 로그 출력 시
for (auto& [key, value] : mAssetMap)
{
    std::cout << key << std::endl;
    // "Mesh_Player"
    // "Texture_Enemy"
    // "Animation2D_Walk"
    // → 타입을 즉시 파악 가능
}
```

**4) 클라이언트 코드 단순화**
```cpp
// ❌ 접두사를 매번 붙여야 한다면
meshMgr->FindMesh("Mesh_Player");
textureMgr->FindTexture("Texture_Player");
animMgr->FindAnimation("Animation2D_Player");

// ✅ WorldAssetManager 사용
assetMgr->FindMesh("Player");         // 깔끔!
assetMgr->FindTexture("Player");      // 깔끔!
assetMgr->FindAnimation("Player");    // 깔끔!
```

### 6-2. 단점

**1) 처음 볼 때 헷갈림**
```cpp
// 왜 이름이 다르지?
CreateMesh("Mesh_LineUP2D", ...);     // 생성 시
FindMesh("LineUP2D");                 // 로드 시
```

**2) 문자열 연산 오버헤드**
```cpp
// 매번 문자열 결합
std::string Key = "Mesh_" + Name;  // 동적 할당
```

**3) 매직 스트링 사용**
```cpp
// 하드코딩된 문자열
"Mesh_", "Texture_", "Animation2D_"
// → 오타 가능성
```

---

## 7. 정리: 헷갈리지 않는 법

### 7-1. 기억해야 할 규칙

**규칙 1: WorldAssetManager를 사용하라**
```cpp
// ✅ 권장 (접두사 자동 처리)
auto assetMgr = World->GetWorldAssetManager().lock();
mMesh = assetMgr->FindMesh("LineUP2D");

// ❌ 비권장 (접두사 수동 관리)
auto meshMgr = CAssetManager::GetInst()->GetMeshManager().lock();
mMesh = meshMgr->FindMesh("Mesh_LineUP2D");
```

**규칙 2: Manager 내부 코드는 접두사 포함**
```cpp
// MeshManager.cpp
CreateMesh("Mesh_LineUP2D", ...);  // ← 접두사 포함

// WorldAssetManager.cpp
FindMesh("LineUP2D");  // ← 접두사 없음 (내부에서 추가됨)
```

**규칙 3: 클라이언트 코드는 접두사 없음**
```cpp
// Player.cpp, Monster.cpp, MainWorld.cpp 등
mWorldAssetManager->CreateMesh("Player");       // 접두사 없음
mWorldAssetManager->LoadTexture("Enemy");       // 접두사 없음
mWorldAssetManager->CreateAnimation("Walk");    // 접두사 없음
```

### 7-2. 시각적 정리

```
┌─────────────────────────────────────┐
│     클라이언트 코드                  │
│  "Player", "Enemy", "Walk"          │  ← 접두사 없음 (깔끔)
└──────────────┬──────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│   WorldAssetManager                   │
│   자동 변환:                          │
│   "Player" → "Mesh_Player"           │  ← 자동 처리
│   "Enemy" → "Texture_Enemy"          │
└──────────────┬───────────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│   AssetManager / Managers             │
│   실제 저장:                          │
│   mMeshMap["Mesh_Player"]            │  ← 접두사 포함 (내부)
│   mTextureMap["Texture_Enemy"]       │
└──────────────────────────────────────┘
```

---

## 8. 결론

### 왜 접두사를 사용하는가?
> **하나의 맵에 모든 타입의 에셋을 저장하기 때문**

### 왜 헷갈리는가?
> **내부 구현(접두사 포함)과 사용자 API(접두사 없음)가 다르기 때문**

### 어떻게 사용하는가?
> **WorldAssetManager를 사용하면 접두사를 신경 쓸 필요 없음**

### 설계가 좋은가?
> **장점:** 이름 충돌 방지, 타입 명시, 디버깅 용이
> **단점:** 초반 학습 곡선, 문자열 오버헤드
> **결론:** 중소 규모 프로젝트에 적합한 실용적 설계

---

## 9. 추가 참고

### 9-1. 다른 엔진은 어떻게 하는가?

**Unreal Engine:**
```cpp
// 경로 기반 네이밍
"/Game/Meshes/Player.uasset"
"/Game/Textures/Player.uasset"
// → 폴더로 타입 구분
```

**Unity:**
```cpp
// 파일 확장자로 구분
"Player.fbx"    // Mesh
"Player.png"    // Texture
"Player.anim"   // Animation
```

**GameEngine (현재 프로젝트):**
```cpp
// 접두사로 구분
"Mesh_Player"
"Texture_Player"
"Animation2D_Player"
```

### 9-2. 개선 방안 (참고)

**매직 스트링 제거:**
```cpp
namespace AssetPrefix {
    constexpr const char* Mesh = "Mesh_";
    constexpr const char* Texture = "Texture_";
    constexpr const char* Animation2D = "Animation2D_";
}

std::string Key = std::string(AssetPrefix::Mesh) + Name;
```

**타입 안전성 추가:**
```cpp
enum class EAssetType {
    Mesh, Texture, Animation2D
};

std::string MakeAssetKey(EAssetType type, const std::string& name) {
    switch (type) {
        case EAssetType::Mesh: return "Mesh_" + name;
        case EAssetType::Texture: return "Texture_" + name;
        case EAssetType::Animation2D: return "Animation2D_" + name;
    }
}
```

---

**핵심만 기억하세요:**
- 내부 코드를 볼 때: "Mesh_LineUP2D" (접두사 포함)
- 사용자 코드를 쓸 때: "LineUP2D" (접두사 없음)
- WorldAssetManager가 중간에서 자동 변환해줌!
