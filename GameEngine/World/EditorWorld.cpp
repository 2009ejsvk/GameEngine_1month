#include "EditorWorld.h"
#include "../Asset/AssetManager.h"
#include "../Asset/Animation2D/Animation2DManager.h"
#include "WorldUIManager.h"
#include "../Render/RenderManager.h"

CEditorWorld::CEditorWorld()
{
}

CEditorWorld::~CEditorWorld()
{
}

bool CEditorWorld::Init()
{
	CWorld::Init();

	LoadAnimation2D();

	LoadSound();

	CreateUI();

	return true;
}

void CEditorWorld::LoadAnimation2D()
{
}

void CEditorWorld::LoadSound()
{
}

void CEditorWorld::CreateUI()
{
}

