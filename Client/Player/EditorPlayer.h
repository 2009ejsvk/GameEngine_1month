#pragma once

#include "Object/GameObject.h"
#include "../ClientInfo.h"

class CEditorPlayer :
    public CGameObject
{
	friend class CWorld;
	friend class CObject;

protected:
	CEditorPlayer();
	CEditorPlayer(const CEditorPlayer& ref);
	CEditorPlayer(CEditorPlayer&& ref)	noexcept;

public:
	virtual ~CEditorPlayer();

private:
	std::weak_ptr<class CCameraComponent>	mCameraComponent;
	std::weak_ptr<class CObjectMovementComponent>	mMovement;
	std::weak_ptr<class CTileMapObject>		mTileMap;
	std::weak_ptr<class CEditorWidget>		mEditorWidget;
	EEditorMode			mEditorMode = EEditorMode::TileFrame;
	int					mEditorFrame = 0;
	ETileType			mTileType = ETileType::Normal;
	std::shared_ptr<class CTile>	mHoveredTile;
	FVector4			mPrevColor = FVector4::White;

public:
	virtual void Begin();
	virtual bool Init();
	virtual void Update(float DeltaTime);

private:
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void LeftClick();
	void Save();
	void Load();
	void ChangeMode();
	void ChangeTileType();
	void ChangeFrame();


private:
	std::weak_ptr<class CTile> GetTile();
};

