#pragma once

#include "../EngineInfo.h"
#include <unordered_set>

class CRenderInstancing
{
	friend class CRenderManager;

private:
	CRenderInstancing();

public:
	~CRenderInstancing();

private:
	std::list<std::weak_ptr<class CSceneComponent>>	mRenderList;
	std::weak_ptr<class CMesh>		mMesh;
	std::weak_ptr<class CTexture>	mTexture;
	std::weak_ptr<class CShader>	mShader;
	bool							mRender = false;
	std::vector<FInstancingData>	mInstancingData;
	std::weak_ptr<class CRenderState>	mBlendState;
	std::weak_ptr<class CWorld>		mWorld;
	FVertexBuffer	mInstancingBuffer;
	int				mInstancingCount = 0;
	std::unordered_set<const class CSceneComponent*>	mRenderSet;
	std::unordered_set<const class CSceneComponent*>	mSeenThisFrame;
	unsigned int	mBuildFrameToken = 0;

public:
	size_t GetRenderCount()	const
	{
		return mRenderList.size();
	}

	bool IsEmpty() const
	{
		return mRenderList.empty();
	}

	bool CheckMesh(const std::weak_ptr<class CMesh>& Mesh)	const;
	bool CheckTexture(const std::weak_ptr<class CTexture>& Texture)	const;
	void BeginFrameBuild(unsigned int FrameToken);
	void FinalizeFrame(unsigned int FrameToken);

public:
	bool ComparisonAsset(const std::weak_ptr<class CMesh>& Mesh, const std::weak_ptr<class CTexture>& Texture);
	void SetMesh(const std::weak_ptr<class CMesh>& Mesh);
	void SetTexture(
		const std::weak_ptr<class CTexture>& Texture);
	void AddRenderList(const std::weak_ptr<class CSceneComponent>& Obj);
	void Update(float DeltaTime);
	void Render();
	void RenderClear();

private:
	bool CreateInstancingBuffer(int Size, int Count);
	bool SetInstancingData(void* Data, int Count);
};

