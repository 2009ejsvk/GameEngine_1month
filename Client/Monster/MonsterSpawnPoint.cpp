#include "MonsterSpawnPoint.h"
#include "Component/SceneComponent.h"
#include "Monster.h"
#include "World/World.h"

CMonsterSpawnPoint::CMonsterSpawnPoint()
{
	SetClassType<CMonsterSpawnPoint>();
}

CMonsterSpawnPoint::CMonsterSpawnPoint(const CMonsterSpawnPoint& ref) :
	CGameObject(ref)
{
}

CMonsterSpawnPoint::CMonsterSpawnPoint(CMonsterSpawnPoint&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CMonsterSpawnPoint::~CMonsterSpawnPoint()
{
}

bool CMonsterSpawnPoint::Init()
{
	CGameObject::Init();

	CreateComponent<CSceneComponent>("Root");

	return true;
}

void CMonsterSpawnPoint::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	// 만약 생성한 몬스터가 제가 되었거나 처음 시작한다면
	// 몬스터를 생성한다.
	if (mSpawnMonster.expired())
	{
		mTime += DeltaTime;

		if (mTime >= mSpawnTime)
		{
			mTime = 0.f;

			std::weak_ptr<CObject> Origin = CObject::FindCDO(mSpawnClass);

			auto OriginMonster = std::dynamic_pointer_cast<CMonster>(Origin.lock());

			auto	World = mWorld.lock();

			std::weak_ptr<CMonster> SpawnMonster = World->CreateCloneGameObject<CMonster>("Monster", OriginMonster);

			auto	Monster = std::dynamic_pointer_cast<CMonster>(SpawnMonster.lock());

			Monster->SetWorldPos(GetWorldPos());
			Monster->SetWorldRotation(GetWorldRot());

			mSpawnMonster = Monster;
		}
	}
}