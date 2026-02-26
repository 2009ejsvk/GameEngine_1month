#pragma once

#include "../EngineInfo.h"
#include "../Asset/Shader/CBufferContainer.h"

struct FTargetInfo
{
	std::weak_ptr<class CRenderTarget>	BlendTarget;
	int	Register = 0;
	int	ShaderBufferType = EShaderBufferType::Pixel;
};

class CPostProcess
{
	friend class CRenderManager;

protected:
	CPostProcess();

public:
	virtual ~CPostProcess() = 0;

protected:
	std::string	mName;
	bool		mEnable = true;
	bool		mActive = true;
	int			mOrder = 0;
	std::shared_ptr<class CRenderTarget>	mTarget;
	FTargetInfo						mBlendTarget;
	std::weak_ptr<class CShader>	mShader;
	std::shared_ptr<class CRenderState>	mBlendState;

public:
	std::weak_ptr<class CRenderTarget> GetTarget()
	{
		return mTarget;
	}

	bool GetActive()	const
	{
		return mActive;
	}

	bool GetEnable()	const
	{
		return mEnable;
	}

	const std::string& GetName()	const
	{
		return mName;
	}

	int GetOrder()	const
	{
		return mOrder;
	}

public:
	void Destroy()
	{
		mActive = false;
	}

	void SetEnable(bool Enable)
	{
		mEnable = Enable;
	}

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

	void SetOrder(int Order)
	{
		mOrder = Order;
	}

	void SetShader(const std::string& Name);
	void CreateRenderTarget(DXGI_FORMAT Fmt,
		bool DepthEnable = false);
	void SetBlendTarget(
		const std::weak_ptr<CRenderTarget>& Target)
	{
		mBlendTarget.BlendTarget = Target;
	}

	void SetBlendTargetInfo(int Register,
		int ShaderBufferType = EShaderBufferType::Pixel)
	{
		mBlendTarget.Register = Register;
		mBlendTarget.ShaderBufferType = ShaderBufferType;
	}

	void SetBlendState(const std::string& Name);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime) = 0;

protected:
	virtual void Render() = 0;
	void RenderFullScreenQuad();

private:
	void RenderPostProcess();
};

