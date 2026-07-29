#include "object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);


struct Material
{
    float4 color;
    int lightingType;
    int3 padding;
    float4x4 uvTransform;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};


ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a <= 0.5)
    {
        discard;
    }
    
    PixelShaderOutput output;
    
    // 0: ライトなし (ライティング計算を行わない)
    if (gMaterial.lightingType == 0)
    {
        output.color = textureColor * gMaterial.color;
    }
    else
    {
        float NdotL = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        float lightFactor = 1.0f;

        if (gMaterial.lightingType == 1)
        {
            // 1: ランバート (Lambert)
            lightFactor = saturate(NdotL);
        }
        else if (gMaterial.lightingType == 2)
        {
            // 2: ハーフランバート (Half-Lambert)
            float halfLambert = NdotL * 0.5f + 0.5f;
            lightFactor = halfLambert * halfLambert; // pow(halfLambert, 2.0f)
        }

        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * lightFactor * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
    }

    return output;
}