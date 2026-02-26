#include "MouseWidget.h"
#include "../Asset/Texture/Texture.h"
#include "../Asset/Texture/TextureManager.h"
#include "../Asset/AssetManager.h"
#include "../World/WorldManager.h"

CMouseWidget::CMouseWidget()
{
}

CMouseWidget::CMouseWidget(const CMouseWidget& ref) :
	CImage(ref)
{
}

CMouseWidget::~CMouseWidget()
{
}

bool CMouseWidget::SetTexture(const std::string& Name)
{
	auto	TextureMgr = CAssetManager::GetInst()->GetTextureManager().lock();

	std::weak_ptr<CTexture>	Texture = TextureMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}

bool CMouseWidget::SetTexture(const std::string& Name,
	const TCHAR* FileName,
	const std::string& PathName)
{
	auto	TextureMgr = CAssetManager::GetInst()->GetTextureManager().lock();

	if (!TextureMgr->LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture>	Texture = TextureMgr->FindTexture(Name);

	mBrush.Texture = Texture;

	return true;
}


bool CMouseWidget::SetTexture(const std::string& Name,
	const std::vector<const TCHAR*>& FileName,
	const std::string& PathName)
{
	auto	TextureMgr = CAssetManager::GetInst()->GetTextureManager().lock();

	if (!TextureMgr->LoadTexture(Name, FileName, PathName))
		return false;

	std::weak_ptr<CTexture>	Texture = TextureMgr->FindTexture(Name);

	mBrush.Texture = Texture;
	mBrush.FrameAnimationEnable = true;

	return true;
}

void CMouseWidget::AddBrushFrame(const FVector2& Start, 
	const FVector2& Size, int Count)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame	Frame;
		Frame.Start = Start;
		Frame.Size = Size;

		mBrush.AnimationFrames.push_back(Frame);
	}

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

void CMouseWidget::AddBrushFrame(float StartX, float StartY,
	float SizeX, float SizeY, int Count)
{
	for (int i = 0; i < Count; ++i)
	{
		FTextureFrame	Frame;
		Frame.Start.x = StartX;
		Frame.Start.y = StartY;
		Frame.Size.x = SizeX;
		Frame.Size.y = SizeY;

		mBrush.AnimationFrames.push_back(Frame);
	}

	mBrush.FrameTime = mBrush.PlayTime /
		mBrush.AnimationFrames.size();
}

bool CMouseWidget::Init()
{
	CImage::Init();

	/*std::weak_ptr<CMeshManager> Weak_MeshMgr =
		CAssetManager::GetInst()->GetMeshManager();

	std::shared_ptr<CMeshManager>   MeshMgr = Weak_MeshMgr.lock();

	mMesh = MeshMgr->FindMesh("Mesh_RectTex");*/

	return true;
}

void CMouseWidget::Update(float DeltaTime)
{
	CImage::Update(DeltaTime);

	// 마우스 위치를 얻어온다.
	// RenderManager에서 관리하기 때문에 소속된 월드가 없다.
	// 그렇기 때문에 WorldManager를 통해 현재 World를 얻어와서 처리한다.
	auto	World = CWorldManager::GetInst()->GetWorld().lock();

	if (World)
	{
		auto	Input = World->GetInput().lock();

		FVector2 MousePos = Input->GetMousePos();

		SetPos(MousePos.x, MousePos.y);
	}
}


void CMouseWidget::Render()
{
	CImage::Render();
}

CMouseWidget* CMouseWidget::Clone()	const
{
	return new CMouseWidget(*this);
}

