#include "MainWorld.h"
#include "../Player/Player.h"
#include "../Monster/Monster.h"
#include "../Monster/MonsterSpawnPoint.h"
#include "Asset/AssetManager.h"
#include "Asset/Animation2D/Animation2DManager.h"

CMainWorld::CMainWorld()
{
}

CMainWorld::~CMainWorld()
{
}

bool CMainWorld::Init()
{
	CWorld::Init();

	LoadAnimation2D();

	std::weak_ptr<CPlayer>	Player = CreateGameObject<CPlayer>("Player");

	std::weak_ptr<CMonster>	Monster1 = CreateGameObject<CMonster>("Monster");

	std::shared_ptr<CMonster>	Monster = Monster1.lock();

	if (Monster)
	{
		Monster->SetWorldPos(-400.f, 300.f);
		Monster->SetWorldRotationZ(180.f);
	}

	Monster1 = CreateGameObject<CMonster>("Monster");

	Monster = Monster1.lock();

	if (Monster)
	{
		Monster->SetWorldPos(400.f, 300.f);
		Monster->SetWorldRotationZ(180.f);
	}

	std::weak_ptr<CMonsterSpawnPoint>	SpawnPoint1 = CreateGameObject<CMonsterSpawnPoint>("SpawnPoint");

	std::shared_ptr<CMonsterSpawnPoint>	Point = SpawnPoint1.lock();

	if (Point)
	{
		Point->SetWorldPos(-400.f, -300.f);
		Point->SetWorldRotationZ(20.f);
		Point->SetSpawnClass<CMonster>();
		Point->SetSpawnTime(5.f);
	}

	return true;
}

void CMainWorld::LoadAnimation2D()
{
	auto	AnimMgr = 
		CAssetManager::GetInst()->GetAnimation2DManager().lock();

	if (AnimMgr)
	{
		AnimMgr->CreateAnimation("PlayerIdle");
		AnimMgr->SetAnimation2DTextureType("PlayerIdle",
			EAnimation2DTextureType::Frame);

		std::vector<const TCHAR*>	TextureFileName;

		for (int i = 0; i < 7; ++i)
		{
			//TCHAR	FileName[MAX_PATH] = {};
			TCHAR* FileName = new TCHAR[MAX_PATH];
			memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
			wsprintf(FileName, 
				TEXT("Player/PlayerFrame/adventurer-get-up-0%d.png"), i);
			TextureFileName.push_back(FileName);
		}

		AnimMgr->SetTexture("PlayerIdle", "PlayerIdle",
			TextureFileName);

		for (int i = 0; i < 7; ++i)
		{
			delete[] TextureFileName[i];
		}
		TextureFileName.clear();

		AnimMgr->AddFrame("PlayerIdle", 7, 0.f, 0.f, 50.f, 37.f);

		AnimMgr->CreateAnimation("PlayerWalk");
		AnimMgr->SetAnimation2DTextureType("PlayerWalk",
			EAnimation2DTextureType::SpriteSheet);

		AnimMgr->SetTexture("PlayerWalk", "PlayerSheet",
			TEXT("Player/Player.png"));

		AnimMgr->AddFrame("PlayerWalk", 200.f, 222.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerWalk", 250.f, 222.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerWalk", 300.f, 222.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerWalk", 0.f, 259.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerWalk", 50.f, 259.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerWalk", 100.f, 259.f, 50.f, 37.f);

		AnimMgr->CreateAnimation("PlayerAttack");
		AnimMgr->SetAnimation2DTextureType("PlayerAttack",
			EAnimation2DTextureType::SpriteSheet);

		AnimMgr->SetTexture("PlayerAttack", "PlayerSheet",
			TEXT("Player/Player.png"));

		AnimMgr->AddFrame("PlayerAttack", 0.f, 0.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerAttack", 50.f, 0.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerAttack", 100.f, 0.f, 50.f, 37.f);
		AnimMgr->AddFrame("PlayerAttack", 150.f, 0.f, 50.f, 37.f);
	}
}
