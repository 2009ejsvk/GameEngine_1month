#pragma once

#include "Render/PostProcess.h"

class CPostProcessHit :
    public CPostProcess
{
    friend class CRenderManager;

protected:
    CPostProcessHit();

public:
    ~CPostProcessHit();

private:
    float       mTime = 0.f;
    float       mEnableTime = 0.1f;
    std::weak_ptr<class CTexture>   mHitTexture;
    FVector4                    mHitColor;
    std::shared_ptr<class CCBufferHit>  mHitCBuffer;

public:
    void SetEnableTime(float EnableTime)
    {
        mEnableTime = EnableTime;
    }

    void SetHitColor(float r, float g, float b, float a);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
};

