
#include "Share.hlsli"

struct VS_INPUT_TEX
{
    float3 Pos : POSITION0;
    float2 UV : TEXCOORD;
};

struct VS_INPUT_INSTANCING_TEX
{
    float3 Pos : POSITION0;
    float2 UV : TEXCOORD;
    
    float4 WVP0 : INSTANCE_WVP0;
    float4 WVP1 : INSTANCE_WVP1;
    float4 WVP2 : INSTANCE_WVP2;
    float4 WVP3 : INSTANCE_WVP3;
    float2 LTUV : INSTANCE_UV0;
    float2 RBUV : INSTANCE_UV1;
    float4 Color : INSTANCE_COLOR0;
};

struct VS_OUTPUT_OUTLINECOLOR
{
    float4 Pos : SV_POSITION;
    float4 Color : TEXCOORD;
};

struct VS_OUTPUT_TEX_COLOR
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : TEXCOORD1;
};

cbuffer CBTileMap : register(b10)
{
    float2 cbTileLTUV;
    float2 cbTileRBUV;
};

// Texture 0번 레지스터를 사용한다.
Texture2D tbBackTexture : register(t0);
Texture2D tbTileTexture : register(t1);

float2 ComputeTileUV(float2 UV)
{
    float2 Result;
    
    if (UV.x > 0.f)
    {
        Result.x = cbTileRBUV.x;
    }
    
    else
    {
        Result.x = cbTileLTUV.x;
    }
    
    if (UV.y > 0.f)
    {
        Result.y = cbTileLTUV.y;
    }
    else
    {
        Result.y = cbTileRBUV.y;
    }
    
    return Result;
}

VS_OUTPUT_TEX TileMapVS(VS_INPUT_TEX input)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    float3 Pos = input.Pos - cbPivotSize;
    
    output.Pos = mul(float4(Pos, 1.f), cbWVP);
    output.UV = ComputeTileUV(input.UV);
	
    return output;
}

struct PS_OUTPUT_COLOR
{
    // 0번 백버퍼에 색상을 출력한다.
    float4 Color : SV_TARGET;
};

PS_OUTPUT_COLOR TileMapPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor = tbTileTexture.Sample(sbLinear, input.UV);
    
    output.Color = TextureColor;
    
    return output;
}



// 인스턴싱 출력
float2 ComputeInstancingTileUV(float2 UV, float2 LT, float2 RB)
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

VS_OUTPUT_TEX_COLOR TileMapInstancingVS(VS_INPUT_INSTANCING_TEX input,
    uint ID : SV_InstanceID)
{
    VS_OUTPUT_TEX_COLOR output = (VS_OUTPUT_TEX_COLOR) 0;
    
    matrix WVP = matrix(input.WVP0, input.WVP1, input.WVP2, input.WVP3);
    
    output.Pos = mul(float4(input.Pos, 1.f), WVP);
    //output.Pos = mul(WVP, float4(input.Pos, 1.f));
    output.UV = ComputeInstancingTileUV(input.UV, input.LTUV,
                    input.RBUV);
    output.Color = input.Color;
	
    return output;
}

PS_OUTPUT_COLOR TileMapInstancingPS(VS_OUTPUT_TEX_COLOR input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 TextureColor = tbTileTexture.Sample(sbLinear, input.UV);
    
    output.Color = TextureColor * input.Color;
    
    return output;
}

VS_OUTPUT_OUTLINECOLOR TileMapLineInstancingVS(VS_INPUT_INSTANCING_TEX input,
    uint ID : SV_InstanceID)
{
    VS_OUTPUT_OUTLINECOLOR output = (VS_OUTPUT_OUTLINECOLOR) 0;
    
    matrix WVP = matrix(input.WVP0, input.WVP1, input.WVP2, input.WVP3);
    
    output.Pos = mul(float4(input.Pos, 1.f), WVP);
    output.Color = input.Color;
	
    return output;
}

PS_OUTPUT_COLOR TileMapLineInstancingPS(VS_OUTPUT_OUTLINECOLOR input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    output.Color = input.Color;
    
    return output;
}
