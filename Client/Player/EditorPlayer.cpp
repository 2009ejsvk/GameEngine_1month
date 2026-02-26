#include "EditorPlayer.h"
#include "Component/CameraComponent.h"
#include "Component/ObjectMovementComponent.h"
#include "Device.h"
#include "Object/TileMapObject.h"
#include "World/World.h"
#include "World/Input.h"
#include "World/WorldUIManager.h"
#include "../UI/EditorWidget.h"
#include "Asset/PathManager.h"
#include "Engine.h"

CEditorPlayer::CEditorPlayer()
{
	SetClassType<CEditorPlayer>();
}

CEditorPlayer::CEditorPlayer(const CEditorPlayer& ref) :
	CGameObject(ref)
{
}

CEditorPlayer::CEditorPlayer(CEditorPlayer&& ref) noexcept :
	CGameObject(std::move(ref))
{
}

CEditorPlayer::~CEditorPlayer()
{
}

void CEditorPlayer::Begin()
{
	CGameObject::Begin();

	auto	World = mWorld.lock();

	mTileMap = World->FindObject<CTileMapObject>("TileMap");

	auto	UIManager = World->GetUIManager().lock();

	mEditorWidget = UIManager->FindWidget<CEditorWidget>("EditorWidget");

	auto EditorWidget = mEditorWidget.lock();

	if (EditorWidget)
	{
	}
}

bool CEditorPlayer::Init()
{
	CGameObject::Init();

	mCameraComponent = CreateComponent<CCameraComponent>("PlayerCamera");
	mMovement = CreateComponent<CObjectMovementComponent>("Movement");

	auto	Movement = mMovement.lock();

	if (Movement)
	{
		Movement->SetUpdateComponent(mCameraComponent);
		Movement->SetSpeed(400.f);
	}

	auto	Camera = mCameraComponent.lock();

	if (Camera)
	{
		const FResolution& RS = CDevice::GetInst()->GetResolution();
		//Camera->SetRelativePos(0.f, 0.f, -5.f);
		Camera->SetProjection(ECameraProjectionType::Ortho,
			90.f, (float)RS.Width, (float)RS.Height, 1000.f);

		Camera->SetInheritRot(false);
	}

	auto	World = mWorld.lock();

	auto	Input = World->GetInput().lock();

	Input->AddBindKey("MoveUp", 'W');
	Input->SetBindFunction<CEditorPlayer>("MoveUp",
		EInputType::Hold, this, &CEditorPlayer::MoveUp);

	Input->AddBindKey("MoveDown", 'S');
	Input->SetBindFunction<CEditorPlayer>("MoveDown",
		EInputType::Hold, this, &CEditorPlayer::MoveDown);

	Input->AddBindKey("MoveLeft", 'A');
	Input->SetBindFunction<CEditorPlayer>("MoveLeft",
		EInputType::Hold, this, &CEditorPlayer::MoveLeft);

	Input->AddBindKey("MoveRight", 'D');
	Input->SetBindFunction<CEditorPlayer>("MoveRight",
		EInputType::Hold, this, &CEditorPlayer::MoveRight);

	Input->AddBindKey("LeftClick", VK_LBUTTON);
	Input->SetBindFunction<CEditorPlayer>("LeftClick",
		EInputType::Hold, this, &CEditorPlayer::LeftClick);

	Input->AddBindKey("Save", 'S');
	Input->SetBindFunction<CEditorPlayer>("Save",
		EInputType::Press, this, &CEditorPlayer::Save);
	Input->SetKeyCtrl("Save", true);

	Input->AddBindKey("Load", 'O');
	Input->SetBindFunction<CEditorPlayer>("Load",
		EInputType::Press, this, &CEditorPlayer::Load);
	Input->SetKeyCtrl("Load", true);

	Input->AddBindKey("ChangeMode", '1');
	Input->SetBindFunction<CEditorPlayer>("ChangeMode",
		EInputType::Press, this, &CEditorPlayer::ChangeMode);

	Input->AddBindKey("ChangeTileType", '2');
	Input->SetBindFunction<CEditorPlayer>("ChangeTileType",
		EInputType::Press, this, &CEditorPlayer::ChangeTileType);

	Input->AddBindKey("ChangeFrame", '3');
	Input->SetBindFunction<CEditorPlayer>("ChangeFrame",
		EInputType::Press, this, &CEditorPlayer::ChangeFrame);

	return true;
}

void CEditorPlayer::Update(float DeltaTime)
{
	CGameObject::Update(DeltaTime);

	auto Tile = GetTile().lock();

	if (Tile && Tile->GetType() != ETileType::UnableToMove)
	{
		if (mHoveredTile != Tile)
		{
			if (mHoveredTile)
			{
				mPrevColor = Tile->GetOutLineColor();
				mHoveredTile->SetOutLineColor(mPrevColor);
			}
		}

		mHoveredTile = Tile;
		Tile->SetOutLineColor(FVector4(1.f, 1.f, 0.f, 1.f));
	}

	else
	{
		if (mHoveredTile)
		{
			mHoveredTile->SetOutLineColor(mPrevColor);
		}

		mHoveredTile.reset();
	}
}

void CEditorPlayer::MoveUp()
{
	auto	Movement = mMovement.lock();

	Movement->AddMove(GetAxis(EAxis::Y));
}

void CEditorPlayer::MoveDown()
{
	auto	Movement = mMovement.lock();

	Movement->AddMove(-GetAxis(EAxis::Y));
}

void CEditorPlayer::MoveLeft()
{
	auto	Movement = mMovement.lock();

	Movement->AddMove(-GetAxis(EAxis::X));
}

void CEditorPlayer::MoveRight()
{
	auto	Movement = mMovement.lock();

	Movement->AddMove(GetAxis(EAxis::X));
}

void CEditorPlayer::LeftClick()
{
	auto Tile = GetTile().lock();

	auto	TileMapObj = mTileMap.lock();

	auto	TileMap = TileMapObj->GetTileMap().lock();

	if (Tile)
	{
		switch (mEditorMode)
		{
		case EEditorMode::TileType:
			if (mTileType == ETileType::Normal)
			{
				Tile->SetOutLineColor(FVector4::White);
				mPrevColor = FVector4::White;
			}

			else if (mTileType == ETileType::UnableToMove)
			{
				Tile->SetOutLineColor(FVector4::Red);
				mPrevColor = FVector4::Red;
			}

			Tile->SetTileType(mTileType);
			break;
		case EEditorMode::TileFrame:
			TileMap->SetTileFrame(Tile->GetIndex(),
				mEditorFrame);
			break;
		}

	}
}

void CEditorPlayer::Save()
{
	OPENFILENAME	ofn = {};

	// 선택한 경로를 저장하기 위한 배열
	TCHAR	FullPath[MAX_PATH] = {};
	TCHAR	Filter[] = TEXT("모든파일\0*.*\0Map파일\0*.tlm\0");

	TCHAR	DefaultPath[MAX_PATH] = {};

	const TCHAR* AssetPath = CPathManager::FindPath("Asset");

	lstrcpy(DefaultPath, AssetPath);
	lstrcat(DefaultPath, TEXT("Map\\"));

	// 구조체 크기 지정
	ofn.lStructSize = sizeof(OPENFILENAME);
	// 오저 윈도우 핸들 지정
	ofn.hwndOwner = CEngine::GetInst()->GetWindowHandle();
	// 파일 필터 지정
	ofn.lpstrFilter = Filter;
	// 기본 확장자 지정. 지정해두면 .확장자를 안붙여도 자동으로
	// 붙여준다.
	ofn.lpstrDefExt = TEXT("tlm");
	// 열어줄 기본 폴더를 지정한다.
	ofn.lpstrInitialDir = DefaultPath;
	// 배열 최대 개수
	ofn.nMaxFile = MAX_PATH;
	// 파일 다이얼로그가 경로를 선택하면 여기에 지정된 주소의 배열에
	// 경로를 넣어준다.
	ofn.lpstrFile = FullPath;

	ShowCursor(TRUE);

	// 취소가 아닌 저장을 눌렀을 경우
	if (GetSaveFileName(&ofn) != 0)
	{
		auto	TileMapObj = mTileMap.lock();

		auto	TileMap = TileMapObj->GetTileMap().lock();

		TileMap->SaveFullPath(FullPath);
	}

	ShowCursor(FALSE);
}

void CEditorPlayer::Load()
{
	OPENFILENAME	ofn = {};

	// 선택한 경로를 저장하기 위한 배열
	TCHAR	FullPath[MAX_PATH] = {};
	TCHAR	Filter[] = TEXT("모든파일\0*.*\0Map파일\0*.tlm\0");

	TCHAR	DefaultPath[MAX_PATH] = {};

	const TCHAR* AssetPath = CPathManager::FindPath("Asset");

	lstrcpy(DefaultPath, AssetPath);
	lstrcat(DefaultPath, TEXT("Map\\"));

	// 구조체 크기 지정
	ofn.lStructSize = sizeof(OPENFILENAME);
	// 오저 윈도우 핸들 지정
	ofn.hwndOwner = CEngine::GetInst()->GetWindowHandle();
	// 파일 필터 지정
	ofn.lpstrFilter = Filter;
	// 기본 확장자 지정. 지정해두면 .확장자를 안붙여도 자동으로
	// 붙여준다.
	ofn.lpstrDefExt = TEXT("tlm");
	// 열어줄 기본 폴더를 지정한다.
	ofn.lpstrInitialDir = DefaultPath;
	// 배열 최대 개수
	ofn.nMaxFile = MAX_PATH;
	// 파일 다이얼로그가 경로를 선택하면 여기에 지정된 주소의 배열에
	// 경로를 넣어준다.
	ofn.lpstrFile = FullPath;

	ShowCursor(TRUE);

	// 취소가 아닌 열기를 눌렀을 경우
	if (GetOpenFileName(&ofn) != 0)
	{
		auto	TileMapObj = mTileMap.lock();

		auto	TileMap = TileMapObj->GetTileMap().lock();

		TileMap->LoadFullPath(FullPath);
	}

	ShowCursor(FALSE);
}

void CEditorPlayer::ChangeMode()
{
	mEditorMode = mEditorMode == EEditorMode::TileType ?
		EEditorMode::TileFrame : EEditorMode::TileType;

	auto	Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetEditorMode(mEditorMode);
	}
}

void CEditorPlayer::ChangeTileType()
{
	int	Type = (int)mTileType;

	++Type;

	if (Type >= (int)ETileType::End)
		Type = 0;

	mTileType = (ETileType)Type;

	auto	Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetTileType(mTileType);
	}
}

void CEditorPlayer::ChangeFrame()
{
	++mEditorFrame;

	auto	TileMapObj = mTileMap.lock();

	auto	TileMap = TileMapObj->GetTileMap().lock();

	if (mEditorFrame >= TileMap->GetTileFrameCount())
	{
		mEditorFrame = 0;
	}

	auto	Widget = mEditorWidget.lock();

	if (Widget)
	{
		Widget->SetTileFrame(mEditorFrame);
	}
}

std::weak_ptr<CTile> CEditorPlayer::GetTile()
{
	// 마우스 월드 위치를 얻어온다.
	auto	World = mWorld.lock();

	auto	Input = World->GetInput().lock();

	FVector2 MousePos = Input->GetMouseWorldPos();

	auto	TileMapObj = mTileMap.lock();

	auto	TileMap = TileMapObj->GetTileMap().lock();

	return TileMap->GetTile(MousePos);
}
