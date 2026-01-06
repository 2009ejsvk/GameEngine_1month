#pragma once

#include "../../EngineInfo.h"
#include "CBufferContainer.h"

class CConstantBufferData abstract
{
public:
	CConstantBufferData();
	CConstantBufferData(const CConstantBufferData& ref);
	CConstantBufferData(CConstantBufferData&& ref)	noexcept;
	virtual ~CConstantBufferData();

protected:
	// 실제 Shader로 보내줄 CBuffer를 공유하게 한다.
	std::weak_ptr<class CConstantBuffer>	mBuffer;

protected:
	void SetConstantBuffer(const std::string& Name);

public:
	virtual bool Init() = 0;
	virtual void UpdateBuffer() = 0;
	virtual CConstantBufferData* Clone() = 0;
};

