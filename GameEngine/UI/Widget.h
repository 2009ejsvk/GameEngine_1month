#pragma once

#include "../Object.h"
#include "UIInfo.h"

class CWidget :
    public CObject
{
    friend class CWorldUIManager;
    friend class CWidgetContainer;

protected:
    static bool SortRender(const std::shared_ptr<CWidget>& Src,
        const std::shared_ptr<CWidget>& Dest);
    static bool SortCollision(const std::shared_ptr<CWidget>& Src,
        const std::shared_ptr<CWidget>& Dest);

protected:
    CWidget();
    CWidget(const CWidget& ref);

public:
    virtual ~CWidget() = 0;

protected:
    static FMatrix mUIProjMatrix;

public:
    static void CreateUIProjection(float Width, float Height)
    {
        mUIProjMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.f,
            Width, Height, 0.f, 0.f, 1000.f);
    }

    static const FMatrix& GetProjMatrix()
    {
        return mUIProjMatrix;
    }

protected:
    bool	mEnable = true;
    bool	mAlive = true;
    std::weak_ptr<CWidget> mSelf;
    std::weak_ptr<class CWorld> mWorld;
    std::weak_ptr<class CWorldUIManager>    mUIManager;
    std::weak_ptr<CWidget>  mParent;
    std::weak_ptr<class CShader>  mShader;
    std::weak_ptr<class CMesh>  mMesh;
    std::shared_ptr<class CCBufferTransform>	mTransformCBuffer;
    std::shared_ptr<class CCBufferUIDefault>	mUIDefaultCBuffer;
    std::string     mName;
    FVector3        mPos;
    FVector3        mRenderPos;
    FVector3        mSize;
    FVector3        mPivot;
    float           mAngle = 0.f;

    int             mZOrder = 0;
    int             mPrevZOrder = 0;

    bool            mMouseOn = false;
    bool            mMouseDrag = false;

    FVector4        mWidgetColor = FVector4::White;

public:
    void SetMouseDragEnable(bool Enable)
    {
        mMouseDrag = Enable;
    }

    void SetWorld(const std::weak_ptr<class CWorld>& World)
    {
        mWorld = World;
    }

    void SetUIManager(const std::weak_ptr<class CWorldUIManager>& UIManager)
    {
        mUIManager = UIManager;
    }

public:
    void SetName(const std::string& Name)
    {
        mName = Name;
    }

    void SetParent(const std::weak_ptr<CWidget>& Parent)
    {
        mParent = Parent;
    }

    void SetSelf(const std::weak_ptr<CWidget>& Self)
    {
        mSelf = Self;
    }

    virtual void SetParentAll();

    void AddPos(const FVector2& Move)
    {
        mPos.x += Move.x;
        mPos.y += Move.y;
    }

    void SetPos(const FVector3& Pos)
    {
        mPos = Pos;
    }

    void SetPos(float x, float y)
    {
        mPos.x = x;
        mPos.y = y;
    }

    virtual void SetSize(const FVector3& Size)
    {
        mSize = Size;
    }

    virtual void SetSize(float x, float y)
    {
        mSize.x = x;
        mSize.y = y;
    }

    void SetPivot(const FVector3& Pivot)
    {
        mPivot = Pivot;
    }

    void SetPivot(float x, float y)
    {
        mPivot.x = x;
        mPivot.y = y;
    }

    void SetAngle(float Angle)
    {
        if (Angle > 360.f)
        {
            // 정수 부분만 남긴다.
            int Angle1 = (int)Angle;
            float   Angle2 = Angle - Angle1;

            Angle1 %= 360;

            mAngle = Angle1 + Angle2;
        }

        else if (Angle < 0.f)
        {
            Angle *= -1.f;

            int Angle1 = (int)Angle;
            float Angle2 = Angle - Angle1;

            Angle1 %= 360;

            mAngle = 360.f - (Angle1 + Angle2);
        }

        else
            mAngle = Angle;
    }

    virtual void SetZOrder(int ZOrder)
    {
        mPrevZOrder = mZOrder;

        mZOrder = ZOrder;
    }

    virtual void ReplaceZOrder()
    {
        mZOrder = mPrevZOrder;
    }

    void SetWidgetColor(const FVector4& Color)
    {
        mWidgetColor = Color;
    }

    void SetWidgetColor(float r, float g, float b, float a)
    {
        mWidgetColor = FVector4(r, g, b, a);
    }

    virtual void SetOpacity(float Opacity)
    {
        mWidgetColor.w = Opacity;
    }

    virtual void SetOpacityAll(float Opacity)
    {
        mWidgetColor.w = Opacity;
    }

    void SetEnable(bool Enable)
    {
        mEnable = Enable;
    }

public:
    bool GetMouseDrag() const
    {
        return mMouseDrag;
    }

    bool GetEnable()	const
    {
        return mEnable;
    }

    bool GetAlive()	const
    {
        return mAlive;
    }

    const std::string& GetName()    const
    {
        return mName;
    }

    const FVector4& GetWidgetColor()    const
    {
        return mWidgetColor;
    }

    const FVector3& GetPos()    const
    {
        return mPos;
    }

    const FVector3& GetSize()    const
    {
        return mSize;
    }

    const FVector3& GetPivot()    const
    {
        return mPivot;
    }

    float GetAngle()    const
    {
        return mAngle;
    }

    int GetZOrder() const
    {
        return mZOrder;
    }

public:
    void SetShader(const std::weak_ptr<class CShader>& Shader);
    void SetShader(const std::string& Name);

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual void Render();
    virtual void Destroy();
    virtual bool HitTest(const FVector2& MousePos);
    virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result,
        const FVector2& MousePos);
    virtual void MouseHovered();
    virtual void MouseUnHovered();
    virtual bool MouseDragStart(const FVector2& MousePos,
        std::shared_ptr<CWidget>& DragOperator);
    virtual bool MouseDrag(const FVector2& MousePos,
        const FVector2& MouseMove);
    virtual bool MouseDragEnd(const FVector2& MousePos);
    virtual CWidget* Clone()	const = 0;

protected:
    void RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos,
        const FVector3& Size);
    void RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos,
        const FVector3& Size, const FVector2& FrameStart,
        const FVector3& FrameSize);

public:
    template <typename T>
    static std::shared_ptr<T> CreateStaticWidget(const std::string& Name,
        std::weak_ptr<class CWorld> World,
        int ZOrder = 0)
    {
        std::shared_ptr<T> Widget;

        Widget.reset(new T);

        Widget->SetWorld(World);
        Widget->SetSelf(Widget);
        Widget->SetName(Name);
        Widget->SetZOrder(ZOrder);

        if (!Widget->Init())
        {
            return std::shared_ptr<T>();
        }

        // dynamic_pointer_cast 를 통해 T 타입으로 변환한
        // shared_ptr이 나오고 그걸 weak_ptr로 변환해서 반환한다.
        return std::dynamic_pointer_cast<T>(Widget);
    }
};

