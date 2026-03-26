#pragma once

#include "Object/GameObject.h"

class CMainCamera :
    public CGameObject
{
    friend class CWorld;
    friend class CObject;

protected:
    CMainCamera();
    CMainCamera(const CMainCamera& ref);
    CMainCamera(CMainCamera&& ref) noexcept;

public:
    virtual ~CMainCamera();

private:
    std::weak_ptr<class CCameraComponent> mCameraComponent;
    std::weak_ptr<class CObjectMovementComponent> mMovement;
    float mViewDistance = 1000.f;
    float mZoomWidth = 0.f;
    float mZoomHeight = 0.f;
    float mZoomAspect = 16.f / 9.f;
    float mMinZoomWidth = 200.f;
    float mMaxZoomWidth = 10000.f;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);

private:
    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();
    void HandleEscape();
#ifdef _DEBUG
    void DebugSkipScenarioPhase();
#endif
};
