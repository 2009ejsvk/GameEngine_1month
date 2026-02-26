#include "GlobalSetting.h"
#include "CollisionInfoManager.h"
#include "Render/RenderManager.h"
#include "UI/MouseWidget.h"
#include "Asset/AssetManager.h"
#include "Asset/Shader/ShaderManager.h"
#include "Shader/ShaderPostProcessHit.h"

CGlobalSetting::CGlobalSetting()
{
}

CGlobalSetting::~CGlobalSetting()
{
}

bool CGlobalSetting::Init()
{
	CCollisionInfoManager::GetInst()->CreateChannel("PlayerAttack");
	CCollisionInfoManager::GetInst()->CreateChannel("MonsterAttack");

	CCollisionInfoManager::GetInst()->CreateProfile("PlayerAttack",
		"PlayerAttack", true);
	CCollisionInfoManager::GetInst()->CreateProfile("MonsterAttack",
		"MonsterAttack", true);

	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"PlayerAttack", "PlayerAttack",
		ECollisionInteraction::Ignore);
	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"PlayerAttack", "Player",
		ECollisionInteraction::Ignore);
	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"PlayerAttack", "MonsterAttack",
		ECollisionInteraction::Ignore);

	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"MonsterAttack", "Monster",
		ECollisionInteraction::Ignore);
	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"MonsterAttack", "MonsterAttack",
		ECollisionInteraction::Ignore);

	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"Player", "Player",
		ECollisionInteraction::Ignore);

	CCollisionInfoManager::GetInst()->SetProfileInteraction(
		"Monster", "Monster",
		ECollisionInteraction::Ignore);

	// 마우스 위젯 생성
	auto MouseWidget = CRenderManager::GetInst()->SetMouseWidget<CMouseWidget>(
		EMouseState::Normal, "MouseNormal").lock();

	std::vector<const TCHAR*>	TextureFileName;

	for (int i = 0; i <= 12; ++i)
	{
		//TCHAR	FileName[MAX_PATH] = {};
		TCHAR* FileName = new TCHAR[MAX_PATH];
		memset(FileName, 0, sizeof(TCHAR) * MAX_PATH);
		wsprintf(FileName,
			TEXT("Mouse/Default/%d.png"), i);
		TextureFileName.push_back(FileName);
	}

	MouseWidget->SetSize(32.f, 31.f);
	MouseWidget->SetTexture("MouseNormal", TextureFileName);

	MouseWidget->AddBrushFrame(0.f, 0.f, 32.f, 31.f, 13);
	MouseWidget->SetBrushAnimation(true);

	for (int i = 0; i <= 12; ++i)
	{
		delete[] TextureFileName[i];
	}
	TextureFileName.clear();

	auto	ShaderMgr = CAssetManager::GetInst()->GetShaderManager().lock();

	ShaderMgr->CreateShader<CShaderPostProcessHit>("Hit");

	return true;
}
