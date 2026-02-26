#pragma once

#include "SceneComponent.h"

class CTileMapRender :
    public CSceneComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CTileMapRender();
	CTileMapRender(const CTileMapRender& ref);
	CTileMapRender(CTileMapRender&& ref)	noexcept;

public:
	virtual ~CTileMapRender();

protected:
	std::weak_ptr<class CShader>	mBackShader;
	std::weak_ptr<class CMesh>		mBackMesh;
	std::weak_ptr<class CTexture>	mTexture[ETileTextureType::End];
	std::weak_ptr<class CRenderState>	mAlphaBlend;
	std::weak_ptr<class CRenderState>	mTileAlphaBlend;
	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
	std::weak_ptr<class CTileMapComponent>	mTileMap;
	std::vector<FTileFrame>		mTileFrame;

public:
	std::weak_ptr<class CTexture> GetTexture(ETileTextureType::Type Type)	const
	{
		return mTexture[Type];
	}

public:
	void SetTileMapComponent(
		const std::weak_ptr<class CTileMapComponent>& TileMap);
	void EnableAlphaBlend();
	void EnableTileAlphaBlend();
	bool SetTexture(ETileTextureType::Type Type,
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(ETileTextureType::Type Type,
		const std::string& Name);
	bool SetTexture(ETileTextureType::Type Type,
		const std::string& Name, const TCHAR* FileName,
		const std::string& PathName = "Texture");
	bool SetTextureFullPath(ETileTextureType::Type Type,
		const std::string& Name, const TCHAR* FullPath);
	bool SetTextureFullPath(ETileTextureType::Type Type,
		const std::string& Name, 
		const std::vector<const TCHAR*>& FullPath);
	void SetBackMesh(const std::weak_ptr<class CMesh>& Mesh);
	void SetBackMesh(const std::string& Name);
	void SetBackShader(const std::weak_ptr<class CShader>& Shader);
	void SetBackShader(const std::string& Name);
	void SetTileFrameSize(float x, float y);
	void AddTileFrame(const FVector2& Start,
		const FVector2& End);
	void AddTileFrame(float StartX, float StartY,
		float EndX, float EndY);

public:
	virtual bool Init();
	virtual void Render();

protected:
	virtual CTileMapRender* Clone()	const;

public:
	void Save(FILE* File);
	void Load(FILE* File);
};

