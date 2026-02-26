#pragma once

#include "PostProcess.h"

class CPostProcessBlur :
    public CPostProcess
{
    friend class CRenderManager;

protected:
    CPostProcessBlur();

public:
    ~CPostProcessBlur();

private:
    std::shared_ptr<class CCBufferBlur>  mBlurCBuffer;
    std::vector<float>  mBlurWeight;

private:
    void ComputeGaussianWeight(int Count);

public:
    void SetBlurCount(int Count);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

