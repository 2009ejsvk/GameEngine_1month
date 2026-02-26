
cbuffer CBHit : register(b10)
{
    float4 cbColor;
};


static float4 NullPos[4] =
{
    float4(-1.f, 1.f, 0.f, 1.f),
    float4(1.f, 1.f, 0.f, 1.f),
    float4(-1.f, -1.f, 0.f, 1.f),
    float4(1.f, -1.f, 0.f, 1.f)
};

static float2 NullUV[4] =
{
    float2(0.f, 0.f),
    float2(1.f, 0.f),
    float2(0.f, 1.f),
    float2(1.f, 1.f)
};

struct VS_OUTPUT_TEX
{
    // SV_ 가 붙은 레지스터는 VS에서 값을 넣어주는 용도이며 다른곳에서 이 값을
    // 가져와서 사용하면 안된다.
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

struct PS_OUTPUT_COLOR
{
    // 0번 백버퍼에 색상을 출력한다.
    float4 Color : SV_TARGET;
};

Texture2D tbBlendTarget : register(t0);
Texture2D tbHitTexture : register(t1);

SamplerState sbPoint : register(s0);

VS_OUTPUT_TEX HitVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT_TEX output = (VS_OUTPUT_TEX) 0;
    
    output.Pos = NullPos[VertexID];
    output.UV = NullUV[VertexID];
    
    return output;
}

PS_OUTPUT_COLOR HitPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 SrcColor = tbBlendTarget.Sample(sbPoint, input.UV);
    
    float4 DestColor = tbHitTexture.Sample(sbPoint, input.UV) *
    cbColor;
    
    output.Color = SrcColor + DestColor * DestColor.a;
    
    return output;
}
