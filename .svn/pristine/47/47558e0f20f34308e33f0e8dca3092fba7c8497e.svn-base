#pragma once

#include "../../EngineInfo.h"
#include "Shader.h"
#include "CBufferContainer.h"

class CShaderManager
{
	friend class CAssetManager;

private:
	CShaderManager();

public:
	~CShaderManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CShader>>	mShaderMap;

	std::unordered_map<std::string, std::shared_ptr<class CConstantBuffer>> mCBufferMap;

public:
	bool Init();

	bool CreateCBuffer(const std::string& Name,
		int Size, int Register,
		int ShaderBuffer = EShaderBufferType::VP);

	std::weak_ptr<CConstantBuffer> FindCBuffer(const std::string& Name);
	std::weak_ptr<CShader> FindShader(const std::string& Name);
	void ReleaseShader(const std::string& Name);
	void ReleaseCBuffer(const std::string& Name);

public:
	template <typename T>
	bool CreateShader(const std::string& Name, 
		const std::string& PathName = "Shader")
	{
		std::weak_ptr<CShader>	Check = FindShader(Name);

		// 있을 경우
		if (!Check.expired())
			return true;

		T* Origin = new T;

		std::shared_ptr<CShader> Shader;

		Shader.reset(Origin);

		if (!Shader->Init(PathName))
		{
			return false;
		}

		Shader->SetName(Name);

		mShaderMap.insert(std::make_pair(Name, Shader));

		return true;
	}
};

