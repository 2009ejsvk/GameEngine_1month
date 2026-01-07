
// Shader에서 공용으로 사용하는 내용이 들어온다.
/*
Shader 의 레지스터
입력레지스터 : 정점정보를 넘겨받을때 사용하며 이름번호 조합으로 사용한다.
출력레지스터 : 완성된 정점정보를 넘겨줄 레지스터이다. 이 정보를 이용해서 DirectX는
화면에 출력할 위치를 잡고 색상 출력 준비를 한다.
상수레지스터 : C++코드에서 일반 데이터를 Shader로 넘겨줄 때 사용한다.
임시레지스터 : 연산 시 사용되는 레지스터
*/

// CPU -> GPU로 자주 바뀌는 값을 전달하는 기능이다.
cbuffer CBTransform : register(b0)
{
    matrix cbWorld;
    matrix cbView;
    matrix cbProj;
    matrix cbWV;
    matrix cbWVP;
    float3 cbPivotSize;
    float cbTransformEmpty;
};

cbuffer CBMaterial : register(b1)
{
    float4 cbBaseColor;
    float cbOpacity;
    float3 cbMaterialEmpty;
};

cbuffer CBAnimatino2D : register(b2)
{
    float2 cbLTUV;
    float2 cbRBUV;
    int cbAnimation2DEnable;
    int cbAnimation2DTextureType;
    int cbTextureSymmetry;
    float cbAnimation2DEmpty;
};

SamplerState sbPoint : register(s0);
SamplerState sbLinear : register(s1);

// 사각형을 그리는 정점의 UV를 인자로 받아온다.
float2 ComputeAnimation2DUV(float2 UV)
{
    // 애니메이션을 적용하지 않을 경우
    if (cbAnimation2DEnable == 0 ||
        cbAnimation2DTextureType != 0)
    {
        if (cbTextureSymmetry == 1)
        {
            if (UV.x == 1.f)
                UV.x = 0.f;
            
            else
                UV.x = 1.f;
        }
        
        return UV;
    }
    
    float2 Result;
    
    // 사각형의 왼쪽 위, 왼쪽 아래 정점일 경우
    if(UV.x == 0.f)
    {
        if (cbTextureSymmetry == 1)
            Result.x = cbRBUV.x;
        
        else
            Result.x = cbLTUV.x;

    }
    
    // 사각형의 오른쪽 위, 오른쪽 아래 정점일 경우
    else
    {
        if (cbTextureSymmetry == 1)
            Result.x = cbLTUV.x;
        
        else
            Result.x = cbRBUV.x;
    }
    
    // 왼쪽 위, 오른쪽 위 정점일 경우
    if (UV.y == 0.f)
    {
        Result.y = cbLTUV.y;
    }
    
    // 왼쪽 아래, 오른쪽 아래 정점일 경우
    else
    {
        Result.y = cbRBUV.y;
    }

    return Result;

}
