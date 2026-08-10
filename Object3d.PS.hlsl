#include "object3d.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);


struct Material
{
    float4 color;
    int lightingType;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess);
    
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
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * lightFactor * gDirectionalLight.intensity;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

        output.color.rgb = diffuse + specular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }

    return output;
}