#pragma once

#include "SceneComponent.h"
#include "../Asset/Shader/CBufferContainer.h"

class CMeshComponent :
	public CSceneComponent
{
	friend class CGameObject;
	friend class CObject;

protected:
	CMeshComponent();
	CMeshComponent(const CMeshComponent& ref);
	CMeshComponent(CMeshComponent&& ref)	noexcept;

public:
	virtual ~CMeshComponent();

protected:
	std::weak_ptr<class CShader>	mShader;
	std::weak_ptr<class CMesh>		mMesh;
	std::vector<std::shared_ptr<class CMaterial>>	mMaterialSlot;
	std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
	std::weak_ptr<class CAnimation2DComponent>	mAnimComponent;
	static std::shared_ptr<class CCBufferAnimation2D>	mEmptyAnimCBuffer;

public:
	static void CreateEmptyAnimCBuffer();
	static void ClearEmptyAnimCBuffer();

public:
	void SetAnimComponent(const std::weak_ptr<class CAnimation2DComponent>& AnimComponent)
	{
		mAnimComponent = AnimComponent;
	}

public:
	void SetMesh(const std::weak_ptr<class CMesh>& Mesh);
	void SetMesh(const std::string& Name);
	void SetShader(const std::weak_ptr<class CShader>& Shader);
	void SetShader(const std::string& Name);
	void SetBlendState(int SlotIndex, const std::string& Name);
	void SetBlendState(int SlotIndex, 
		const std::weak_ptr<class CRenderState>& State);
	void AddTexture(int SlotIndex, 
		const std::weak_ptr<class CTexture>& Texture,
		int Register, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name,
		int Register, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name,
		const TCHAR* FileName,
		const std::string& PathName = "Texture",
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTextureFullPath(int SlotIndex, const std::string& Name,
		const TCHAR* FullPath,
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name,
		const std::vector<const TCHAR*>& FileName,
		const std::string& PathName = "Texture",
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	void AddTextureFullPath(int SlotIndex, const std::string& Name,
		const std::vector<const TCHAR*>& FullPath,
		int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel,
		int Index = 0);
	bool SetTexture(int SlotIndex, int TextureIndex,
		const std::weak_ptr<class CTexture>& Texture);
	bool SetTextureIndex(int SlotIndex, int TextureIndex);
	void SetMaterialBaseColor(int SlotIndex,
		float r, float g, float b, float a);
	void SetMaterialBaseColor(int SlotIndex,
		int r, int g,
		int b, int a);
	void SetMaterialBaseColor(int SlotIndex,
		const FVector4& Color);
	void SetMaterialOpacity(int SlotIndex, float Opacity);

public:
	virtual bool Init();
	virtual void Render();

protected:
	virtual CMeshComponent* Clone()	const;
};

