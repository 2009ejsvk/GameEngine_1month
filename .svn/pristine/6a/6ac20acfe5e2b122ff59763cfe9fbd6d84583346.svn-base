#pragma once

#include "../Asset.h"

// 이 클래스가 하나의 모션이다.
// 걷기, 달리기, 대기, 공격 4가지의 모션을 가지고 있는 애니메이션이
// 있다면 이 클래스가 4개 생성이 되어야 한다.
class CAnimation2D :
    public CAsset
{
    friend class CAnimation2DManager;

protected:
    CAnimation2D();

public:
    virtual ~CAnimation2D();

protected:
    // 이 모션이 사용하는 Texture
    std::weak_ptr<class CTexture>   mTexture;
    EAnimation2DTextureType mTextureType =
        EAnimation2DTextureType::SpriteSheet;
    std::vector<FTextureFrame>  mFrameArray;

public:
    EAnimation2DTextureType GetAnimationTextureType()   const
    {
        return mTextureType;
    }

    const std::weak_ptr<class CTexture>& GetTexture()   const
    {
        return mTexture;
    }

    const FTextureFrame& GetFrame(int Index)    const
    {
        return mFrameArray[Index];
    }

    int GetFrameCount() const
    {
        return (int)mFrameArray.size();
    }

    void SetAnimation2DTextureType(EAnimation2DTextureType Type)
    {
        mTextureType = Type;
    }

public:
    void SetTexture(const std::weak_ptr<CTexture>& Texture);
    void SetTexture(const std::string& Name);
    void SetTexture(const std::string& Name,
        const TCHAR* FileName,
        const std::string& PathName = "Texture");
    void SetTextureFullPath(const std::string& Name,
        const TCHAR* FullPath);
    void SetTexture(const std::string& Name,
        const std::vector<const TCHAR*>& FileName,
        const std::string& PathName = "Texture");
    void SetTextureFullPath(const std::string& Name,
        const std::vector<const TCHAR*>& FullPath);
    void AddFrame(const FVector2& Start, const FVector2& Size);
    void AddFrame(float StartX, float StartY, float SizeX,
        float SizeY);
    void AddFrame(int Count, const FVector2& Start, const FVector2& Size);
    void AddFrame(int Count, float StartX, float StartY, float SizeX,
        float SizeY);
};

