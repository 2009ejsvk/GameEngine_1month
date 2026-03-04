
#include "Share.hlsli"

#define MAX_BLUR_COUNT  16

cbuffer CBBlur : register(b10)
{
    float2 cbTexelSize;
    int cbBlurCount;
    float cbBlueEmpty;
    float4 cbBlurWeight[(MAX_BLUR_COUNT + 1 + 3) / 4];
};


struct PS_OUTPUT_COLOR
{
    // 0번 백버퍼에 색상을 출력한다.
    float4 Color : SV_TARGET;
};

Texture2D tbBlendTarget : register(t0);

PS_OUTPUT_COLOR BlurPS(VS_OUTPUT_TEX input)
{
    PS_OUTPUT_COLOR output = (PS_OUTPUT_COLOR) 0;
    
    float4 SrcColor = tbBlendTarget.Sample(sbPoint, input.UV);
        
    if (SrcColor.a == 0.f)
    {
        // 원본픽셀로 출력하고 여기서 리턴된다.
        clip(-1);
    }
    
    SrcColor *= cbBlurWeight[0][0];
    
    uint i = 0;
    uint BlurCount = (uint)max(cbBlurCount, 0);
    float4 TargetColor;
    
    [loop]
    for (i = 1; i <= BlurCount; ++i)
    {
        float2 UV = input.UV + float2(cbTexelSize.x * (float)i, 0.f);
        
        TargetColor = tbBlendTarget.Sample(sbPoint, UV);
        
        int Index1 = (int)(i >> 2);
        int Index2 = (int)(i & 3u);
        
        //if (TargetColor.a > 0.f)
        SrcColor += TargetColor * cbBlurWeight[Index1][Index2];
        
        UV = input.UV - float2(cbTexelSize.x * (float)i, 0.f);
        
        TargetColor = tbBlendTarget.Sample(sbPoint, UV);
        
        //if (TargetColor.a > 0.f)
        SrcColor += TargetColor * cbBlurWeight[Index1][Index2];
    }
    
    [loop]
    for (i = 1; i <= BlurCount; ++i)
    {
        float2 UV = input.UV + float2(0.f, cbTexelSize.y * (float)i);
        
        TargetColor = tbBlendTarget.Sample(sbPoint, UV);
        
        int Index1 = (int)(i >> 2);
        int Index2 = (int)(i & 3u);
        
        //if (TargetColor.a > 0.f)
        SrcColor += TargetColor * cbBlurWeight[Index1][Index2];
        
        UV = input.UV - float2(0.f, cbTexelSize.y * (float)i);
        
        TargetColor = tbBlendTarget.Sample(sbPoint, UV);
        
        //if (TargetColor.a > 0.f)
        SrcColor += TargetColor * cbBlurWeight[Index1][Index2];
    }
    
    SrcColor /= 2.f;
    
    output.Color = SrcColor;
    
    return output;
}
