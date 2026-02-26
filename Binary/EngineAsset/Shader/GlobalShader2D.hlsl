
#include "Share.hlsli"

struct VS_INPUT_COLOR2D
{
    // POSITION 만 적으면 뒤에 0이 생략되어 POSITION0번으로 인식된다.
    float3 Pos : POSITION0;
    float4 Color : COLOR0;
};

struct VS_OUTPUT_COLOR2D
{
    // SV_ 가 붙은 레지스터는 VS에서 값을 넣어주는 용도이며 다른곳에서 이 값을
    // 가져와서 사용하면 안된다.
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

// Texture 0번 레지스터를 사용한다.
Texture2D tbBaseTexture : register(t0);
Texture2DArray tbBaseArrayTexture : register(t1);


VS_OUTPUT_COLOR2D Color2DVS(VS_INPUT_COLOR2D input)
{
    VS_OUTPUT_COLOR2D output = (VS_OUTPUT_COLOR2D)0;
    
    float3 Pos = input.Pos - cbPivotSize;
    
    // x, y, z 에는 input.Pos의 x, y, z 가 들어가고 w에는 1.f이 들어간다.
    // mul : 행렬 곱셈연산을 해준다.
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.Color = input.Color;

    return output;
}

struct PS_OUTPUT_COLOR
{
    // 0번 백버퍼에 색상을 출력한다.
    float4 Color : SV_TARGET;
};

PS_OUTPUT_COLOR Color2DPS(VS_OUTPUT_COLOR2D input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    output.Color = input.Color;
    
    return output;
}

PS_OUTPUT_COLOR MaterialColor2DPS(VS_OUTPUT_COLOR2D input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    output.Color = cbBaseColor;
    output.Color.a = cbOpacity;
    
    return output;
}

struct VS_INPUT_COLOR2D_INSTANCING
{
    // POSITION 만 적으면 뒤에 0이 생략되어 POSITION0번으로 인식된다.
    float3 Pos : POSITION0;
    float4 Color : COLOR0;
    
    float4 WVP0 : INSTANCE_WVP0;
    float4 WVP1 : INSTANCE_WVP1;
    float4 WVP2 : INSTANCE_WVP2;
    float4 WVP3 : INSTANCE_WVP3;
    float2 LTUV : INSTANCE_UV0;
    float2 RBUV : INSTANCE_UV1;
    float4 MtrlColor : INSTANCE_COLOR0;
    float3 PivotSize : INSTANCE_PIVOT0;
    int ArrayTextureEnable : INSTANCE_TEXTURETYPE0;
    int AnimFrame : INSTANCE_ANIMFRAME0;
};

VS_OUTPUT_COLOR2D Color2DInstancingVS(
    VS_INPUT_COLOR2D_INSTANCING input)
{
    VS_OUTPUT_COLOR2D output = (VS_OUTPUT_COLOR2D) 0;
    
    float3 Pos = input.Pos - input.PivotSize;
    
    matrix WVP = matrix(input.WVP0, input.WVP1, input.WVP2, input.WVP3);
    
    output.Pos = mul(float4(Pos, 1.f), WVP);
    output.Color = input.MtrlColor;

    return output;
}


struct VS_INPUT_TEX
{
    // POSITION 만 적으면 뒤에 0이 생략되어 POSITION0번으로 인식된다.
    float3 Pos : POSITION0;
    float2 UV : TEXCOORD;
};


struct VS_INPUT_TEX_INSTANCING
{
    // POSITION 만 적으면 뒤에 0이 생략되어 POSITION0번으로 인식된다.
    float3 Pos : POSITION0;
    float2 UV : TEXCOORD;
    
    float4 WVP0 : INSTANCE_WVP0;
    float4 WVP1 : INSTANCE_WVP1;
    float4 WVP2 : INSTANCE_WVP2;
    float4 WVP3 : INSTANCE_WVP3;
    float2 LTUV : INSTANCE_UV0;
    float2 RBUV : INSTANCE_UV1;
    float4 Color : INSTANCE_COLOR0;
    float3 PivotSize : INSTANCE_PIVOT0;
    int ArrayTextureEnable : INSTANCE_TEXTURETYPE0;
    int AnimFrame : INSTANCE_ANIMFRAME0;
};

struct VS_OUTPUT_TEX_INSTANCING
{
    // SV_ 가 붙은 레지스터는 VS에서 값을 넣어주는 용도이며 다른곳에서 이 값을
    // 가져와서 사용하면 안된다.
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : TEXCOORD1;
    int ArrayTextureEnable : TEXCOORD2;
    int AnimFrame : TEXCOORD3;
};


VS_OUTPUT_TEX DefaultTexVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    float3 Pos = input.Pos - cbPivotSize;
    
    // x, y, z 에는 input.Pos의 x, y, z 가 들어가고 w에는 1.f이 들어간다.
    // mul : 행렬 곱셈연산을 해준다.
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.UV = ComputeAnimation2DUV(input.UV);

    return output;
}

float2 ComputeInstancingUV(float2 UV, float2 LT, float2 RB)
{
    float2 Result;
    
    if (UV.x > 0.f)
    {
        Result.x = RB.x;
    }
    else
    {
        Result.x = LT.x;
    }
    
    if (UV.y > 0.f)
    {
        Result.y = RB.y;
    }
    else
    {
        Result.y = LT.y;
    }
    
    return Result;
}

VS_OUTPUT_TEX_INSTANCING DefaultTexInstancingVS(
    VS_INPUT_TEX_INSTANCING input, uint ID : SV_InstanceID)
{
    VS_OUTPUT_TEX_INSTANCING output = (VS_OUTPUT_TEX_INSTANCING) 0;
    
    float3 Pos = input.Pos - input.PivotSize;
    
    matrix WVP = matrix(input.WVP0, input.WVP1, input.WVP2, input.WVP3);
    
    output.Pos = mul(float4(Pos, 1.f), WVP);
    output.UV = ComputeInstancingUV(input.UV, input.LTUV,
                    input.RBUV);
    output.Color = input.Color;
    output.ArrayTextureEnable = input.ArrayTextureEnable;
    output.AnimFrame = input.AnimFrame;

    return output;
}

PS_OUTPUT_COLOR MaterialTexInstancingPS(
    VS_OUTPUT_TEX_INSTANCING input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor;
    
    if (input.ArrayTextureEnable == 0)
    {
        TextureColor = tbBaseTexture.Sample(sbPoint, input.UV);
    }
    
    else
    {
        TextureColor = tbBaseArrayTexture.Sample(sbLinear, float3(input.UV, input.AnimFrame));
    }
    
    output.Color.rgb = TextureColor.rgb * input.Color.rgb;
    output.Color.a = TextureColor.a * input.Color.a;
    
    return output;
}

VS_OUTPUT_TEX DefaultTexNoneAnimVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    float3 Pos = input.Pos - cbPivotSize;
    
    // x, y, z 에는 input.Pos의 x, y, z 가 들어가고 w에는 1.f이 들어간다.
    // mul : 행렬 곱셈연산을 해준다.
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.UV = input.UV;

    return output;
}

PS_OUTPUT_COLOR DefaultTexPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor;
    
    if (cbAnimation2DTextureType == 2)
    {
        TextureColor = tbBaseArrayTexture.Sample(sbPoint, float3(input.UV, cbAnimation2DFrame));
    }
    else
    {
        TextureColor = tbBaseTexture.Sample(sbPoint, input.UV);
    }
    
    output.Color = TextureColor;
    
    return output;
}

PS_OUTPUT_COLOR MaterialTexPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor;
    
    if (cbAnimation2DTextureType == 2)
    {
        TextureColor = tbBaseArrayTexture.Sample(sbPoint, float3(input.UV, cbAnimation2DFrame));
    }
    
    else
    {
        TextureColor = tbBaseTexture.Sample(sbPoint, input.UV);
    }
    
    output.Color.rgb = TextureColor.rgb * cbBaseColor.rgb;
    output.Color.a = TextureColor.a * cbOpacity;
    
    return output;
}

float4 FrameVS(float3 Pos : POSITION0)  : SV_POSITION
{
    float4 OutputPos = mul(float4(Pos, 1.f), cbWVP);
    
    return OutputPos;
}

PS_OUTPUT_COLOR TexPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor;
    
    if (cbAnimation2DTextureType == 2)
    {
        TextureColor = tbBaseArrayTexture.Sample(sbPoint, float3(input.UV, cbAnimation2DFrame));
    }
    else
    {
        TextureColor = tbBaseTexture.Sample(sbPoint, input.UV);
    }
    
    output.Color = TextureColor;
    
    return output;
}
